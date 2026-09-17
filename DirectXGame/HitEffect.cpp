#include "HitEffect.h"
#include "GlobalVariables.h"
#include "RandomGenerator.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "HitEffect";
}

void HitEffect::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "ExpandDuration", kExpandDuration_);
	gv->AddItem(kGroupName, "FadeDuration", kFadeDuration_);
	gv->AddItem(kGroupName, "CircleScaleStart", kCircleScaleStart_);
	gv->AddItem(kGroupName, "CircleScaleEnd", kCircleScaleEnd_);
	gv->AddItem(kGroupName, "EllipseScaleStart", kEllipseScaleStart_);
	gv->AddItem(kGroupName, "EllipseScaleEnd", kEllipseScaleEnd_);
}

void HitEffect::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kExpandDuration_ = gv->GetFloatValue(kGroupName, "ExpandDuration");
	kFadeDuration_ = gv->GetFloatValue(kGroupName, "FadeDuration");
	kCircleScaleStart_ = gv->GetVector3Value(kGroupName, "CircleScaleStart");
	kCircleScaleEnd_ = gv->GetVector3Value(kGroupName, "CircleScaleEnd");
	kEllipseScaleStart_ = gv->GetVector3Value(kGroupName, "EllipseScaleStart");
	kEllipseScaleEnd_ = gv->GetVector3Value(kGroupName, "EllipseScaleEnd");
}

Camera* HitEffect::camera_ = nullptr;
Model* HitEffect::model_ = nullptr;

HitEffect::~HitEffect() { delete randomizer; }

void HitEffect::Initialize(uint32_t textureHandle, Vector3& spawnLocation, float rotationZ) {
	randomizer = new RandomGenerator();
	randomizer->Initialize();

	BaseInitialize(textureHandle, 0.15f, 0.25f);

	circleWorldTransform_.Initialize();
	circleWorldTransform_.translation_ = spawnLocation;
	circleWorldTransform_.rotation_.z = rotationZ;

	for (WorldTransform& worldTransform : ellipseWorldTransforms_) {
		worldTransform.Initialize();
		worldTransform.scale_ = {1.5f, 0.04f, 1.0f};
		worldTransform.rotation_ = {0.0f, 0.0f, rotationZ + randomizer->GetRandValue()};
		worldTransform.translation_ = circleWorldTransform_.translation_;
	}
}

HitEffect* HitEffect::Create(uint32_t textureHandle, Vector3& spawnLocation, float rotationZ) {
	HitEffect* instance = new HitEffect();
	assert(instance);
	instance->Initialize(textureHandle, spawnLocation, rotationZ);
	return instance;
}

void HitEffect::OnExpandUpdate(float t) {
	circleWorldTransform_.scale_.x = kCircleScaleStart_.x + (kCircleScaleEnd_.x - kCircleScaleStart_.x) * t;
	circleWorldTransform_.scale_.y = kCircleScaleStart_.y + (kCircleScaleEnd_.y - kCircleScaleStart_.y) * t;
	circleWorldTransform_.scale_.z = kCircleScaleStart_.z + (kCircleScaleEnd_.z - kCircleScaleStart_.z) * t;

	for (WorldTransform& worldTransform : ellipseWorldTransforms_) {
		worldTransform.scale_.x = kEllipseScaleStart_.x + (kEllipseScaleEnd_.x - kEllipseScaleStart_.x) * t;
		worldTransform.scale_.y = kEllipseScaleStart_.y + (kEllipseScaleEnd_.y - kEllipseScaleStart_.y) * t;
		worldTransform.scale_.z = kEllipseScaleStart_.z + (kEllipseScaleEnd_.z - kEllipseScaleStart_.z) * t;
	}
}

void HitEffect::OnCommonUpdate() {
	WorldTransformUpdate(circleWorldTransform_);
	for (WorldTransform& worldTransform : ellipseWorldTransforms_) {
		WorldTransformUpdate(worldTransform);
	}
}

void HitEffect::Draw() {
	Model::PreDraw(Model::CullingMode::kBack, Model::BlendMode::kNormal, Model::DepthTestMode::kOff);
	model_->Draw(circleWorldTransform_, *camera_, textureHandle_, &objectColor_);
	for (WorldTransform& worldTransform : ellipseWorldTransforms_) {
		model_->Draw(worldTransform, *camera_, textureHandle_, &objectColor_);
	}
	Model::PostDraw();
}