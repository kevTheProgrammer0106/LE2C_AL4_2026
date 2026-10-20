#define NOMINMAX
#include "EffectParticleBurst.h"
#include "GlobalVariables.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <string>

using namespace KamataEngine;

namespace {
constexpr float kDeltaTime = 1.0f / 60.0f; // same fixed 60FPS as BaseEffect

constexpr const char* kGroupNames[] = {"ParticleBurstHit",   "ParticleBurstJump", "ParticleBurstLand",        "ParticleBurstExplosion", "ParticleBurstSmoke", "ParticleBurstAura", "ParticleBurstBulletBlock", "ParticleBurstBulletEntity", "ParticleBurstSpawn"};
static_assert(std::size(kGroupNames) == static_cast<size_t>(BurstType::kCount), "kGroupNames must have one entry per BurstType");

const char* GroupName(size_t i) { return kGroupNames[i]; }
} // namespace

Camera* EffectParticleBurst::camera_ = nullptr;
Model* EffectParticleBurst::model_ = nullptr;

void EffectParticleBurst::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	for (size_t i = 0; i < kPresets.size(); ++i) {
		const char* g = GroupName(i);
		Preset& p = kPresets[i];
		gv->CreateGroup(g);
		gv->AddItem(g, "Count", p.count);
		gv->AddItem(g, "ColorStart", p.colorStart);
		gv->AddItem(g, "ColorEnd", p.colorEnd);
		gv->AddItem(g, "AlphaStart", p.alphaStart);
		gv->AddItem(g, "LifeMin", p.lifeMin);
		gv->AddItem(g, "LifeMax", p.lifeMax);
		gv->AddItem(g, "SpeedMin", p.speedMin);
		gv->AddItem(g, "SpeedMax", p.speedMax);
		gv->AddItem(g, "Spread", p.spread);
		gv->AddItem(g, "Gravity", p.gravity);
		gv->AddItem(g, "Drag", p.drag);
		gv->AddItem(g, "SizeStart", p.sizeStart);
		gv->AddItem(g, "SizeEnd", p.sizeEnd);
		gv->AddItem(g, "SpawnRadius", p.spawnRadius);
		gv->AddItem(g, "Additive", p.additive);
		gv->AddItem(g, "LoopDuration", p.loopDuration); // 粒子を再生成し続ける秒数(0=ループなし)
	}
}

void EffectParticleBurst::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	for (size_t i = 0; i < kPresets.size(); ++i) {
		const char* g = GroupName(i);
		Preset& p = kPresets[i];
		p.count = gv->GetIntValue(g, "Count");
		p.colorStart = gv->GetVector3Value(g, "ColorStart");
		p.colorEnd = gv->GetVector3Value(g, "ColorEnd");
		p.alphaStart = gv->GetFloatValue(g, "AlphaStart");
		p.lifeMin = gv->GetFloatValue(g, "LifeMin");
		p.lifeMax = std::max(gv->GetFloatValue(g, "LifeMax"), p.lifeMin);
		p.speedMin = gv->GetFloatValue(g, "SpeedMin");
		p.speedMax = std::max(gv->GetFloatValue(g, "SpeedMax"), p.speedMin);
		p.spread = gv->GetFloatValue(g, "Spread");
		p.gravity = gv->GetFloatValue(g, "Gravity");
		p.drag = gv->GetFloatValue(g, "Drag");
		p.sizeStart = gv->GetFloatValue(g, "SizeStart");
		p.sizeEnd = gv->GetFloatValue(g, "SizeEnd");
		p.spawnRadius = gv->GetFloatValue(g, "SpawnRadius");
		p.additive = gv->GetIntValue(g, "Additive");
		p.loopDuration = std::max(gv->GetFloatValue(g, "LoopDuration"), 0.0f);
	}
}

EffectParticleBurst* EffectParticleBurst::Create(uint32_t textureHandle, BurstType type, const Vector3& position, const Vector2& dir, float loopDurationOverride) {
	EffectParticleBurst* instance = new EffectParticleBurst();
	assert(instance);
	instance->Initialize(textureHandle, type, position, dir, loopDurationOverride);
	return instance;
}

void EffectParticleBurst::Initialize(uint32_t textureHandle, BurstType type, const Vector3& position, const Vector2& dir, float loopDurationOverride) {
	random_.Initialize();
	preset_ = kPresets[static_cast<size_t>(type)];
	if (loopDurationOverride >= 0.0f) {
		preset_.loopDuration = loopDurationOverride; // 呼び出し側からの上書き
	}
	const Preset& p = preset_;

	loopDuration_ = p.loopDuration;
	emitTimer_ = 0.0f;
	origin_ = position;

	// 全体の寿命 = ループ継続 + 再生成待ち(最大lifeMin) + 粒子の寿命(最大lifeMax)
	// 最後に再生成された粒子が消えるまで、エフェクト自体を終わらせない
	const float total = loopDuration_ + (loopDuration_ > 0.0f ? p.lifeMin : 0.0f) + p.lifeMax;
	BaseInitialize(textureHandle, total * 0.5f, std::max(total * 0.5f, 0.01f));

	// 中心方向(0なら上向き)
	Vector2 d = dir;
	const float len = std::sqrt(d.x * d.x + d.y * d.y);
	if (len > 0.0001f) {
		d.x /= len;
		d.y /= len;
	} else {
		d = {0.0f, 1.0f};
	}
	baseAngle_ = std::atan2(d.y, d.x);

	count_ = static_cast<size_t>(std::clamp(p.count, 0, static_cast<int>(kMaxParticles)));
	for (size_t i = 0; i < count_; ++i) {
		Particle& particle = particles_[i];
		// WorldTransform/ObjectColorの初期化は最初の1回だけ(再生成では呼ばない)
		particle.worldTransform.Initialize();
		particle.objectColor.Initialize();
		SpawnParticle(particle, 1.0f, 0.0f);
	}
}

void EffectParticleBurst::SpawnParticle(Particle& particle, float alphaScale, float initialAge) {
	const Preset& p = preset_;

	const float angle = baseAngle_ + random_.GetRandValue(-p.spread, p.spread);
	const float speed = random_.GetRandValue(p.speedMin, p.speedMax);
	particle.velocity = {std::cos(angle) * speed, std::sin(angle) * speed, random_.GetRandValue(-0.3f, 0.3f)};

	particle.worldTransform.translation_ = {origin_.x + random_.GetRandValue(-p.spawnRadius, p.spawnRadius), origin_.y + random_.GetRandValue(-p.spawnRadius, p.spawnRadius), origin_.z};
	particle.worldTransform.scale_ = {p.sizeStart, p.sizeStart, p.sizeStart};
	particle.lifetime = random_.GetRandValue(p.lifeMin, p.lifeMax);
	particle.age = initialAge;
	particle.alphaScale = alphaScale;
	particle.active = true;
	particle.objectColor.SetColor({p.colorStart.x, p.colorStart.y, p.colorStart.z, p.alphaStart * alphaScale});
	WorldTransformUpdate(particle.worldTransform);
}

void EffectParticleBurst::OnCommonUpdate() {
	const Preset& p = preset_;
	emitTimer_ += kDeltaTime;

	for (size_t i = 0; i < count_; ++i) {
		Particle& particle = particles_[i];
		if (!particle.active) {
			continue;
		}
		particle.age += kDeltaTime;

		// 再生成待ち中は動かさない
		if (particle.age < 0.0f) {
			continue;
		}

		if (particle.age >= particle.lifetime) {
			if (emitTimer_ < loopDuration_) {
				// ループ中: 発生位置で再生成する。時間が経つほど薄くし、待ち時間をばらつかせて煙が途切れず続くようにする
				const float remain = std::clamp(1.0f - emitTimer_ / loopDuration_, 0.0f, 1.0f);
				SpawnParticle(particle, remain, -random_.GetRandValue(0.0f, p.lifeMin));
			} else {
				particle.active = false;
			}
			continue;
		}
		const float t = particle.age / particle.lifetime; // 0=出現直後 / 1=消滅直前

		particle.velocity.y -= p.gravity * kDeltaTime;
		particle.velocity.x *= p.drag;
		particle.velocity.y *= p.drag;
		particle.velocity.z *= p.drag;
		particle.worldTransform.translation_ += {particle.velocity.x * kDeltaTime, particle.velocity.y * kDeltaTime, particle.velocity.z * kDeltaTime};

		const float size = Lerp(p.sizeStart, p.sizeEnd, t);
		particle.worldTransform.scale_ = {size, size, size};

		particle.objectColor.SetColor(
		    {Lerp(p.colorStart.x, p.colorEnd.x, t), Lerp(p.colorStart.y, p.colorEnd.y, t), Lerp(p.colorStart.z, p.colorEnd.z, t), Lerp(p.alphaStart, 0.0f, t) * particle.alphaScale});

		WorldTransformUpdate(particle.worldTransform);
	}
}

void EffectParticleBurst::Draw() {
	if (!model_ || !camera_) {
		return;
	}
	// 両面描画・深度は読み取りのみ。火花は加算、土煙は通常合成
	Model::PreDraw(Model::CullingMode::kNone, preset_.additive ? Model::BlendMode::kAdd : Model::BlendMode::kNormal, Model::DepthTestMode::kReadOnly);
	for (size_t i = 0; i < count_; ++i) {
		Particle& particle = particles_[i];
		if (particle.active && particle.age >= 0.0f) { // 再生成待ち中は描画しない
			model_->Draw(particle.worldTransform, *camera_, textureHandle_, &particle.objectColor);
		}
	}
	Model::PostDraw();
}