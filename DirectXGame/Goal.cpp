#include "Goal.h"
#include "GlobalVariables.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "Goal";
}

void Goal::Initialize(Model* model, uint32_t textureHandle, Camera* camera, const Vector3& position) {
#ifdef _DEBUG
	assert(model);
	assert(camera);
#endif
	model_ = model;
	textureHandle_ = textureHandle;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	basePosition_ = position;
	baseScale_ = worldTransform_.scale_;

	isCleared_ = false;
	animTimer_ = 0.0f;
}

void Goal::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);

	gv->AddItem(kGroupName, "HalfWidth", kHalfWidth);
	gv->AddItem(kGroupName, "HalfHeight", kHalfHeight);
	gv->AddItem(kGroupName, "HalfDepth", kHalfDepth);

	gv->AddItem(kGroupName, "BobAmplitude", kBobAmplitude);
	gv->AddItem(kGroupName, "BobSpeed", kBobSpeed);

	gv->AddItem(kGroupName, "ClearPulseScale", kClearPulseScale);
	gv->AddItem(kGroupName, "ClearPulseDuration", kClearPulseDuration);
}

void Goal::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();

	kHalfWidth = gv->GetFloatValue(kGroupName, "HalfWidth");
	kHalfHeight = gv->GetFloatValue(kGroupName, "HalfHeight");
	kHalfDepth = gv->GetFloatValue(kGroupName, "HalfDepth");

	kBobAmplitude = gv->GetFloatValue(kGroupName, "BobAmplitude");
	kBobSpeed = gv->GetFloatValue(kGroupName, "BobSpeed");

	kClearPulseScale = gv->GetFloatValue(kGroupName, "ClearPulseScale");
	kClearPulseDuration = gv->GetFloatValue(kGroupName, "ClearPulseDuration");
}

void Goal::Update() {
	animTimer_ += 1.0f / 60.0f;

	if (isCleared_) {
		UpdateClearedAnimation();
	} else {
		UpdateIdleAnimation();
	}

	WorldTransformUpdate(worldTransform_);
}

void Goal::UpdateIdleAnimation() {
	float bob = std::sin(animTimer_ * kBobSpeed) * kBobAmplitude;
	worldTransform_.translation_ = basePosition_ + Vector3(0.0f, bob, 0.0f);
	worldTransform_.scale_ = baseScale_;
}

void Goal::UpdateClearedAnimation() {
	float t = std::clamp(animTimer_ / kClearPulseDuration, 0.0f, 1.0f);
	float pulse = (t < 0.5f) ? (t / 0.5f) : (1.0f - (t - 0.5f) / 0.5f);
	float scaleFactor = 1.0f + (kClearPulseScale - 1.0f) * pulse;

	worldTransform_.translation_ = basePosition_;
	worldTransform_.scale_ = {baseScale_.x * scaleFactor, baseScale_.y * scaleFactor, baseScale_.z * scaleFactor};
}

bool Goal::CheckCollision(const AABB& playerAABB) {
	if (isCleared_) {
		return true;
	}

	if (IsCollisionAABB(GetAABB(), playerAABB)) {
		isCleared_ = true;
		animTimer_ = 0.0f; // クリア演出を最初から再生
	}

	return isCleared_;
}

AABB Goal::GetAABB() const {
	Vector3 pos = worldTransform_.translation_;
	AABB aabb;
	aabb.min = {pos.x - kHalfWidth, pos.y - kHalfHeight, pos.z - kHalfDepth};
	aabb.max = {pos.x + kHalfWidth, pos.y + kHalfHeight, pos.z + kHalfDepth};
	return aabb;
}

void Goal::Draw() {
	if (!model_ || isCleared_) { 
		return;
	}
	Model::PreDraw();
	model_->Draw(worldTransform_, *camera_, textureHandle_);
	Model::PostDraw();
}