#define NOMINMAX
#include "Grenade.h"
#include "GlobalVariables.h"
#include "MapChipField.h"
#include "ObjectBounce.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "Grenade";
constexpr float kDt = 1.0f / 60.0f;
} // namespace

void Grenade::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "FuseTime", kFuseTime);
	gv->AddItem(kGroupName, "FlashStartTime", kFlashStartTime);
	gv->AddItem(kGroupName, "FlashFreqStart", kFlashFreqStart);
	gv->AddItem(kGroupName, "FlashFreqEnd", kFlashFreqEnd);
	gv->AddItem(kGroupName, "EnlargeDuration", kEnlargeDuration);
	gv->AddItem(kGroupName, "EnlargeScale", kEnlargeScale);
	gv->AddItem(kGroupName, "BaseScale", kBaseScale);
	gv->AddItem(kGroupName, "ThrowSpeedX", kThrowSpeedX);
	gv->AddItem(kGroupName, "ThrowSpeedY", kThrowSpeedY);
	gv->AddItem(kGroupName, "Gravity", kGravity);
	gv->AddItem(kGroupName, "Restitution", kRestitution);
	gv->AddItem(kGroupName, "Friction", kFriction);
	gv->AddItem(kGroupName, "FatalRadius", kFatalRadius);
	gv->AddItem(kGroupName, "NormalRadius", kNormalRadius);
	gv->AddItem(kGroupName, "NormalDamage", kNormalDamage);
	gv->AddItem(kGroupName, "Impact", kImpact);
}

void Grenade::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kFuseTime = gv->GetFloatValue(kGroupName, "FuseTime");
	kFlashStartTime = gv->GetFloatValue(kGroupName, "FlashStartTime");
	kFlashFreqStart = gv->GetFloatValue(kGroupName, "FlashFreqStart");
	kFlashFreqEnd = gv->GetFloatValue(kGroupName, "FlashFreqEnd");
	kEnlargeDuration = gv->GetFloatValue(kGroupName, "EnlargeDuration");
	kEnlargeScale = gv->GetFloatValue(kGroupName, "EnlargeScale");
	kBaseScale = gv->GetFloatValue(kGroupName, "BaseScale");
	kThrowSpeedX = gv->GetFloatValue(kGroupName, "ThrowSpeedX");
	kThrowSpeedY = gv->GetFloatValue(kGroupName, "ThrowSpeedY");
	kGravity = gv->GetFloatValue(kGroupName, "Gravity");
	kRestitution = gv->GetFloatValue(kGroupName, "Restitution");
	kFriction = gv->GetFloatValue(kGroupName, "Friction");
	kFatalRadius = gv->GetFloatValue(kGroupName, "FatalRadius");
	kNormalRadius = gv->GetFloatValue(kGroupName, "NormalRadius");
	kNormalDamage = gv->GetIntValue(kGroupName, "NormalDamage");
	kImpact = gv->GetFloatValue(kGroupName, "Impact");
}

Vector3 Grenade::MakeThrowVelocity(bool facingRight) { return {facingRight ? kThrowSpeedX : -kThrowSpeedX, kThrowSpeedY, 0.0f}; }

void Grenade::Initialize(Model* model, uint32_t textureHandle, Camera* camera, MapChipField* field, const Vector3& position, const Vector3& velocity, Owner owner) {
#ifdef _DEBUG
	assert(model);
	assert(camera);
#endif
	model_ = model;
	textureHandle_ = textureHandle;
	camera_ = camera;
	mapChipField_ = field;
	velocity_ = velocity;
	owner_ = owner;

	fuseTimer_ = kFuseTime;
	flashPhase_ = 0.0f;
	readyToExplode_ = false;
	isDead_ = false;

	objectColor_.Initialize();
	objectColor_.SetColor(Vector4{1.0f, 1.0f, 1.0f, 1.0f});

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {kBaseScale, kBaseScale, kBaseScale};
}

void Grenade::Update() {
	if (isDead_ || readyToExplode_) {
		return;
	}

	fuseTimer_ -= kDt;
	if (fuseTimer_ <= 0.0f) {
		fuseTimer_ = 0.0f;
		readyToExplode_ = true;
		return;
	}

	// ---- flight: gravity + bounce (same stepping as DroppableItem) ----
	velocity_.y -= kGravity;
	velocity_.y = std::max(velocity_.y, -kMaxFallSpeed);

	bool landed = false;
	ObjectBounce::Step(mapChipField_, worldTransform_.translation_, velocity_, kBodySize, kBodySize, kRestitution, kFriction, false, landed);
	if (landed && std::abs(velocity_.y) < kSettleSpeed) {
		velocity_.y = 0.0f; // stops the micro-bounce once it comes to rest
	}

	// fell into a pit / kill block: it is simply lost, no blast
	const Vector3 pos = worldTransform_.translation_;
	if (pos.y < kKillY || (mapChipField_ && mapChipField_->IsKillAtPosition(pos))) {
		isDead_ = true;
		return;
	}

	worldTransform_.rotation_.z -= velocity_.x / (kBodySize * 0.5f); // rolls as it travels

	// ---- fuse visuals ----
	const float remaining = fuseTimer_;

	bool flashOn = false;
	if (remaining <= kFlashStartTime) {
		const float progress = 1.0f - remaining / std::max(kFlashStartTime, 0.001f);
		flashPhase_ += Lerp(kFlashFreqStart, kFlashFreqEnd, progress) * kDt; // blinks faster as it nears the end
		flashOn = std::fmod(flashPhase_, 1.0f) < 0.5f;
	}

	float scale = kBaseScale;
	const bool enlarging = (kEnlargeDuration > 0.0f && remaining <= kEnlargeDuration);
	if (enlarging) {
		const float t = 1.0f - remaining / kEnlargeDuration;
		scale = Lerp(kBaseScale, kBaseScale * kEnlargeScale, EaseIn(0.0f, 1.0f, t)); // slow start, sudden swell
	}
	worldTransform_.scale_ = {scale, scale, scale};

	// holds solid red while swelling so the last moments read as "about to blow"
	objectColor_.SetColor((flashOn || enlarging) ? Vector4{1.0f, 0.15f, 0.1f, 1.0f} : Vector4{1.0f, 1.0f, 1.0f, 1.0f});

	WorldTransformUpdate(worldTransform_);
}

void Grenade::Draw() {
	if (isDead_ || !model_ || !camera_) {
		return;
	}
	Model::PreDraw();
	model_->Draw(worldTransform_, *camera_, textureHandle_, &objectColor_);
	Model::PostDraw();
}