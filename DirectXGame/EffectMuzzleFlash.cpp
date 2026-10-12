#define NOMINMAX
#include "GlobalVariables.h"
#include "EffectMuzzleFlash.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "EffectMuzzleFlash";
constexpr float kDeltaTime = 1.0f / 60.0f; // BaseEffectと同じ固定60FPS
} // namespace

Camera* EffectMuzzleFlash::camera_ = nullptr;
Model* EffectMuzzleFlash::model_ = nullptr;

void EffectMuzzleFlash::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "FireCount", kFireCount);
	gv->AddItem(kGroupName, "FireColorStart", kFireColorStart);
	gv->AddItem(kGroupName, "FireColorEnd", kFireColorEnd);
	gv->AddItem(kGroupName, "FireAlphaStart", kFireAlphaStart);
	gv->AddItem(kGroupName, "FireLifeMin", kFireLifeMin);
	gv->AddItem(kGroupName, "FireLifeMax", kFireLifeMax);
	gv->AddItem(kGroupName, "FireSpeedMin", kFireSpeedMin);
	gv->AddItem(kGroupName, "FireSpeedMax", kFireSpeedMax);
	gv->AddItem(kGroupName, "FireSpread", kFireSpread);
	gv->AddItem(kGroupName, "FireSizeStart", kFireSizeStart);
	gv->AddItem(kGroupName, "FireSizeEnd", kFireSizeEnd);
	gv->AddItem(kGroupName, "SmokeCount", kSmokeCount);
	gv->AddItem(kGroupName, "SmokeColorStart", kSmokeColorStart);
	gv->AddItem(kGroupName, "SmokeColorEnd", kSmokeColorEnd);
	gv->AddItem(kGroupName, "SmokeAlphaStart", kSmokeAlphaStart);
	gv->AddItem(kGroupName, "SmokeLifeMin", kSmokeLifeMin);
	gv->AddItem(kGroupName, "SmokeLifeMax", kSmokeLifeMax);
	gv->AddItem(kGroupName, "SmokeDelayMax", kSmokeDelayMax);
	gv->AddItem(kGroupName, "SmokeSpeedMin", kSmokeSpeedMin);
	gv->AddItem(kGroupName, "SmokeSpeedMax", kSmokeSpeedMax);
	gv->AddItem(kGroupName, "SmokeRiseMin", kSmokeRiseMin);
	gv->AddItem(kGroupName, "SmokeRiseMax", kSmokeRiseMax);
	gv->AddItem(kGroupName, "SmokeSizeStart", kSmokeSizeStart);
	gv->AddItem(kGroupName, "SmokeSizeEnd", kSmokeSizeEnd);
	gv->AddItem(kGroupName, "FollowTime", kFollowTime);
}

void EffectMuzzleFlash::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kFireCount = gv->GetIntValue(kGroupName, "FireCount");
	kFireColorStart = gv->GetVector3Value(kGroupName, "FireColorStart");
	kFireColorEnd = gv->GetVector3Value(kGroupName, "FireColorEnd");
	kFireAlphaStart = gv->GetFloatValue(kGroupName, "FireAlphaStart");
	kFireLifeMin = gv->GetFloatValue(kGroupName, "FireLifeMin");
	kFireLifeMax = std::max(gv->GetFloatValue(kGroupName, "FireLifeMax"), kFireLifeMin);
	kFireSpeedMin = gv->GetFloatValue(kGroupName, "FireSpeedMin");
	kFireSpeedMax = std::max(gv->GetFloatValue(kGroupName, "FireSpeedMax"), kFireSpeedMin);
	kFireSpread = gv->GetFloatValue(kGroupName, "FireSpread");
	kFireSizeStart = gv->GetFloatValue(kGroupName, "FireSizeStart");
	kFireSizeEnd = gv->GetFloatValue(kGroupName, "FireSizeEnd");
	kSmokeCount = gv->GetIntValue(kGroupName, "SmokeCount");
	kSmokeColorStart = gv->GetVector3Value(kGroupName, "SmokeColorStart");
	kSmokeColorEnd = gv->GetVector3Value(kGroupName, "SmokeColorEnd");
	kSmokeAlphaStart = gv->GetFloatValue(kGroupName, "SmokeAlphaStart");
	kSmokeLifeMin = gv->GetFloatValue(kGroupName, "SmokeLifeMin");
	kSmokeLifeMax = std::max(gv->GetFloatValue(kGroupName, "SmokeLifeMax"), kSmokeLifeMin);
	kSmokeDelayMax = gv->GetFloatValue(kGroupName, "SmokeDelayMax");
	kSmokeSpeedMin = gv->GetFloatValue(kGroupName, "SmokeSpeedMin");
	kSmokeSpeedMax = std::max(gv->GetFloatValue(kGroupName, "SmokeSpeedMax"), kSmokeSpeedMin);
	kSmokeRiseMin = gv->GetFloatValue(kGroupName, "SmokeRiseMin");
	kSmokeRiseMax = std::max(gv->GetFloatValue(kGroupName, "SmokeRiseMax"), kSmokeRiseMin);
	kSmokeSizeStart = gv->GetFloatValue(kGroupName, "SmokeSizeStart");
	kSmokeSizeEnd = gv->GetFloatValue(kGroupName, "SmokeSizeEnd");
	kFollowTime = gv->GetFloatValue(kGroupName, "FollowTime");
}

void EffectMuzzleFlash::SetPlayerAnchor(const Vector3& pos) {
	sAnchorDelta_ = {};
	if (sHasAnchor_) {
		const Vector3 d = pos - sAnchor_;
		// 武器の持ち替えなどで大きく飛んだときは追従させない
		if (d.x * d.x + d.y * d.y < 4.0f) {
			sAnchorDelta_ = d;
		}
	}
	sAnchor_ = pos;
	sHasAnchor_ = true;
}

EffectMuzzleFlash* EffectMuzzleFlash::Create(uint32_t textureHandle, const Vector3& muzzlePos, const Vector2& aimDir, bool followPlayer) {
	EffectMuzzleFlash* instance = new EffectMuzzleFlash();
	assert(instance);
	instance->Initialize(textureHandle, muzzlePos, aimDir, followPlayer);
	return instance;
}

void EffectMuzzleFlash::Initialize(uint32_t textureHandle, const Vector3& muzzlePos, const Vector2& aimDir, bool followPlayer) {
	random_.Initialize();
	follow_ = followPlayer;
	firstFrame_ = true;

	// kExpand = 火花の時間 / kFadeOut = 残りの煙の時間(全パーティクルが消えるまで)
	const float total = std::max(kFireLifeMax, kSmokeDelayMax + kSmokeLifeMax);
	const float expand = kFireLifeMax;
	const float fade = std::max(total - expand, 0.01f);
	BaseInitialize(textureHandle, expand, fade);

	// 発射方向を正規化(0なら右向き)
	Vector2 dir = aimDir;
	const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	if (len > 0.0001f) {
		dir.x /= len;
		dir.y /= len;
	} else {
		dir = {1.0f, 0.0f};
	}
	const float baseAngle = std::atan2(dir.y, dir.x);

	// ---- 火花: 発射方向へ勢いよく弾ける ----
	fireCount_ = static_cast<size_t>(std::clamp(kFireCount, 0, static_cast<int>(kMaxFire)));
	for (size_t i = 0; i < fireCount_; ++i) {
		Particle& p = fire_[i];
		p.worldTransform.Initialize();
		p.objectColor.Initialize();
		const float angle = baseAngle + random_.GetRandValue(-kFireSpread, kFireSpread);
		const float speed = random_.GetRandValue(kFireSpeedMin, kFireSpeedMax);
		p.velocity = {std::cos(angle) * speed, std::sin(angle) * speed, random_.GetRandValue(-0.5f, 0.5f)};
		p.worldTransform.translation_ = {muzzlePos.x + random_.GetRandValue(-0.05f, 0.05f), muzzlePos.y + random_.GetRandValue(-0.05f, 0.05f), muzzlePos.z};
		p.worldTransform.scale_ = {kFireSizeStart, kFireSizeStart, kFireSizeStart};
		p.lifetime = random_.GetRandValue(kFireLifeMin, kFireLifeMax);
		p.age = 0.0f;
		p.active = true;
		p.objectColor.SetColor({kFireColorStart.x, kFireColorStart.y, kFireColorStart.z, kFireAlphaStart});
		WorldTransformUpdate(p.worldTransform);
	}

	// ---- 煙: 少し遅れて出て、ゆっくり上昇しながら広がる ----
	smokeCount_ = static_cast<size_t>(std::clamp(kSmokeCount, 0, static_cast<int>(kMaxSmoke)));
	for (size_t i = 0; i < smokeCount_; ++i) {
		Particle& p = smoke_[i];
		p.worldTransform.Initialize();
		p.objectColor.Initialize();
		const float angle = baseAngle + random_.GetRandValue(-kFireSpread, kFireSpread) * 0.8f;
		const float speed = random_.GetRandValue(kSmokeSpeedMin, kSmokeSpeedMax);
		const float rise = random_.GetRandValue(kSmokeRiseMin, kSmokeRiseMax);
		p.velocity = {std::cos(angle) * speed, std::sin(angle) * speed + rise, random_.GetRandValue(-0.3f, 0.3f)};
		p.worldTransform.translation_ = {muzzlePos.x + random_.GetRandValue(-0.1f, 0.1f), muzzlePos.y + random_.GetRandValue(-0.1f, 0.1f), muzzlePos.z};
		p.worldTransform.scale_ = {kSmokeSizeStart, kSmokeSizeStart, kSmokeSizeStart};
		p.lifetime = random_.GetRandValue(kSmokeLifeMin, kSmokeLifeMax);
		p.age = -random_.GetRandValue(0.0f, kSmokeDelayMax); // 火花より遅れて出現
		p.active = true;
		p.objectColor.SetColor({kSmokeColorStart.x, kSmokeColorStart.y, kSmokeColorStart.z, kSmokeAlphaStart});
		WorldTransformUpdate(p.worldTransform);
	}
}

void EffectMuzzleFlash::UpdateParticle(Particle& particle, bool isSmoke) {
	particle.age += kDeltaTime;
	if (particle.age >= particle.lifetime) {
		particle.active = false;
		return;
	}

	Vector3& position = particle.worldTransform.translation_;

	// 銃口への追従: 待機中は100%、その後 followTime かけて0%へ弱める
	if (follow_ && !firstFrame_ && kFollowTime > 0.0f) {
		const float weight = std::clamp(1.0f - std::max(particle.age, 0.0f) / kFollowTime, 0.0f, 1.0f);
		position += {sAnchorDelta_.x * weight, sAnchorDelta_.y * weight, sAnchorDelta_.z * weight};
	}

	// 待機中は動かさず、描画もしない
	if (particle.age < 0.0f) {
		WorldTransformUpdate(particle.worldTransform);
		return;
	}

	const float t = particle.age / particle.lifetime; // 0=出現直後 / 1=消滅直前
	position += {
	    particle.velocity.x * kDeltaTime,
	    particle.velocity.y * kDeltaTime,
	    particle.velocity.z * kDeltaTime
	};

	// 火花は急減速、煙はゆるやかに減速
	const float drag = isSmoke ? 0.96f : 0.88f;
	particle.velocity.x *= drag;
	particle.velocity.y *= drag;
	particle.velocity.z *= drag;

	const float sizeStart = isSmoke ? kSmokeSizeStart : kFireSizeStart;
	const float sizeEnd = isSmoke ? kSmokeSizeEnd : kFireSizeEnd;
	const float size = Lerp(sizeStart, sizeEnd, t);
	particle.worldTransform.scale_ = {size, size, size};

	const Vector3& c0 = isSmoke ? kSmokeColorStart : kFireColorStart;
	const Vector3& c1 = isSmoke ? kSmokeColorEnd : kFireColorEnd;
	const float alpha = Lerp(isSmoke ? kSmokeAlphaStart : kFireAlphaStart, 0.0f, t);
	particle.objectColor.SetColor({Lerp(c0.x, c1.x, t), Lerp(c0.y, c1.y, t), Lerp(c0.z, c1.z, t), alpha});

	WorldTransformUpdate(particle.worldTransform);
}

// BaseEffect::Update から毎フレーム呼ばれる(拡大/フェードの状態に関わらず全粒を更新)
void EffectMuzzleFlash::OnCommonUpdate() {
	for (size_t i = 0; i < fireCount_; ++i) {
		if (fire_[i].active)
			UpdateParticle(fire_[i], false);
	}
	for (size_t i = 0; i < smokeCount_; ++i) {
		if (smoke_[i].active)
			UpdateParticle(smoke_[i], true);
	}
	firstFrame_ = false;
}

void EffectMuzzleFlash::Draw() {
	if (!model_ || !camera_) {
		return;
	}
	// 煙(通常合成)を先に、火花(加算合成)を上に重ねる。両面描画・深度は読み取りのみ
	Model::PreDraw(Model::CullingMode::kNone, Model::BlendMode::kNormal, Model::DepthTestMode::kReadOnly);
	for (size_t i = 0; i < smokeCount_; ++i) {
		const Particle& p = smoke_[i];
		if (p.active && p.age >= 0.0f)
			model_->Draw(const_cast<WorldTransform&>(p.worldTransform), *camera_, textureHandle_, const_cast<ObjectColor*>(&p.objectColor));
	}
	Model::PostDraw();

	Model::PreDraw(Model::CullingMode::kNone, Model::BlendMode::kAdd, Model::DepthTestMode::kReadOnly);
	for (size_t i = 0; i < fireCount_; ++i) {
		const Particle& p = fire_[i];
		if (p.active && p.age >= 0.0f)
			model_->Draw(const_cast<WorldTransform&>(p.worldTransform), *camera_, textureHandle_, const_cast<ObjectColor*>(&p.objectColor));
	}
	Model::PostDraw();
}