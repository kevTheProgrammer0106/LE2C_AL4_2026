#include "GuardEffect.h"
#include "GlobalVariables.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "GuardEffect";
}

void GuardEffect::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);

	gv->AddItem(kGroupName, "ExpandDuration", kExpandDuration_);
	gv->AddItem(kGroupName, "FadeDuration", kFadeDuration_);
	gv->AddItem(kGroupName, "RingScaleStart", kRingScaleStart_);
	gv->AddItem(kGroupName, "RingScaleEnd", kRingScaleEnd_);
	gv->AddItem(kGroupName, "RotationSpeed", kRotationSpeed_);
}

void GuardEffect::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();

	kExpandDuration_ = gv->GetFloatValue(kGroupName, "ExpandDuration");
	kFadeDuration_ = gv->GetFloatValue(kGroupName, "FadeDuration");
	kRingScaleStart_ = gv->GetVector3Value(kGroupName, "RingScaleStart");
	kRingScaleEnd_ = gv->GetVector3Value(kGroupName, "RingScaleEnd");
	kRotationSpeed_ = gv->GetFloatValue(kGroupName, "RotationSpeed");
}
Camera* GuardEffect::camera_ = nullptr;
Model* GuardEffect::model_ = nullptr;

GuardEffect::~GuardEffect() {}

void GuardEffect::Initialize(uint32_t textureHandle, Vector3& spawnLocation) {
	BaseInitialize(textureHandle, 0.15f, 0.25f);

	ringWorldTransform_.Initialize();
	ringWorldTransform_.translation_ = spawnLocation;
	ringWorldTransform_.scale_ = kRingScaleStart_;
}

GuardEffect* GuardEffect::Create(uint32_t textureHandle, Vector3& spawnLocation) {
	GuardEffect* instance = new GuardEffect();
	assert(instance);
	instance->Initialize(textureHandle, spawnLocation);
	return instance;
}

void GuardEffect::OnExpandUpdate(float t) {
	ringWorldTransform_.scale_.x = kRingScaleStart_.x + (kRingScaleEnd_.x - kRingScaleStart_.x) * t;
	ringWorldTransform_.scale_.y = kRingScaleStart_.y + (kRingScaleEnd_.y - kRingScaleStart_.y) * t;
	ringWorldTransform_.scale_.z = kRingScaleStart_.z + (kRingScaleEnd_.z - kRingScaleStart_.z) * t;
}

void GuardEffect::OnCommonUpdate() {
	// リングを回転させる演出（拡大・フェードアウト問わず毎フレーム）
	ringWorldTransform_.rotation_.z += kRotationSpeed_;
	WorldTransformUpdate(ringWorldTransform_);
}

void GuardEffect::Draw() {
	Model::PreDraw();
	model_->Draw(ringWorldTransform_, *camera_, textureHandle_, &objectColor_);
	Model::PostDraw();
}