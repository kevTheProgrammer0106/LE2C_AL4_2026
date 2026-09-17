#include "HealthBlock.h"
#include "GlobalVariables.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "HealthBlock";
}

void HealthBlock::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "HalfWidth", kHalfWidth);
	gv->AddItem(kGroupName, "HalfHeight", kHalfHeight);
	gv->AddItem(kGroupName, "HalfDepth", kHalfDepth);
	gv->AddItem(kGroupName, "BobAmplitude", kBobAmplitude);
	gv->AddItem(kGroupName, "BobSpeed", kBobSpeed);
	gv->AddItem(kGroupName, "SpinSpeed", kSpinSpeed);
	gv->AddItem(kGroupName, "RespawnInterval", kRespawnInterval);
	gv->AddItem(kGroupName, "BlinkWarningDuration", kBlinkWarningDuration);
	gv->AddItem(kGroupName, "BlinkFrequency", kBlinkFrequency);
}

void HealthBlock::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kHalfWidth = gv->GetFloatValue(kGroupName, "HalfWidth");
	kHalfHeight = gv->GetFloatValue(kGroupName, "HalfHeight");
	kHalfDepth = gv->GetFloatValue(kGroupName, "HalfDepth");
	kBobAmplitude = gv->GetFloatValue(kGroupName, "BobAmplitude");
	kBobSpeed = gv->GetFloatValue(kGroupName, "BobSpeed");
	kSpinSpeed = gv->GetFloatValue(kGroupName, "SpinSpeed");
	kRespawnInterval = gv->GetFloatValue(kGroupName, "RespawnInterval");
	kBlinkWarningDuration = gv->GetFloatValue(kGroupName, "BlinkWarningDuration");
	kBlinkFrequency = gv->GetFloatValue(kGroupName, "BlinkFrequency");
}

void HealthBlock::Initialize(Model* model, uint32_t textureHandle, Camera* camera, const Vector3& position, Mode mode) {
#ifdef _DEBUG
	assert(model);
	assert(camera);
#endif
	model_ = model;
	textureHandle_ = textureHandle;
	camera_ = camera;
	mode_ = mode;

	basePosition_ = position;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {0.6f, 0.6f, 0.6f};

	bobTimer_ = 0.0f;
	respawnTimer_ = 0.0f;
	isActive_ = true;
}

void HealthBlock::Update() {
	if (!isActive_) {
		// kLimited: 恒久的に非アクティブのまま(respawnTimer_は進めない)
		// kTimer: 再出現までカウントする
		if (mode_ == Mode::kTimer) {
			respawnTimer_ += 1.0f / 60.0f;
			if (respawnTimer_ >= kRespawnInterval) {
				isActive_ = true;
				respawnTimer_ = 0.0f;
				bobTimer_ = 0.0f;
			}
		}
		return;
	}

	bobTimer_ += 1.0f / 60.0f;
	float bob = std::sin(bobTimer_ * kBobSpeed) * kBobAmplitude;
	worldTransform_.translation_ = basePosition_ + Vector3(0.0f, bob, 0.0f);
	worldTransform_.rotation_.y += kSpinSpeed * (1.0f / 60.0f);
	WorldTransformUpdate(worldTransform_);
}

void HealthBlock::Draw() {
	if (!isActive_ || !model_) {
		return;
	}

	Model::PreDraw();
	model_->Draw(worldTransform_, *camera_, textureHandle_);
	Model::PostDraw();
}

AABB HealthBlock::GetAABB() const {
	Vector3 pos = worldTransform_.translation_;
	AABB aabb;
	aabb.min = {pos.x - kHalfWidth, pos.y - kHalfHeight, pos.z - kHalfDepth};
	aabb.max = {pos.x + kHalfWidth, pos.y + kHalfHeight, pos.z + kHalfDepth};
	return aabb;
}

void HealthBlock::Consume() {
	if (!isActive_) {
		return;
	}
	isActive_ = false;
	respawnTimer_ = 0.0f;
	if (sConsumeSoundHandle_ != 0) {
		Audio::GetInstance()->PlayWave(sConsumeSoundHandle_, false, sSfxVolume_);
	}
}