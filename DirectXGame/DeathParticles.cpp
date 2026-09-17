#include "DeathParticles.h"
#include "GlobalVariables.h"

using namespace KamataEngine;
namespace {
constexpr const char* kGroupName = "DeathParticles";
}
void DeathParticles::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);

	gv->AddItem(kGroupName, "Duration", kDuration_);
	gv->AddItem(kGroupName, "Speed", kSpeed_);
	gv->AddItem(kGroupName, "AngleUnit", kAngleUnit_);
}
void DeathParticles::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();

	kDuration_ = gv->GetFloatValue(kGroupName, "Duration");
	kSpeed_ = gv->GetFloatValue(kGroupName, "Speed");
	kAngleUnit_ = gv->GetFloatValue(kGroupName, "AngleUnit");
}
void DeathParticles::Initialize(Model* model, uint32_t textureHandle, Camera* camera, const Vector3& position) {
	assert(model);
	assert(camera);
	model_ = model;
	camera_ = camera;

	BaseInitialize(textureHandle, 0.0f, kDuration_); // expandDuration_ unused here, fadeDuration_ reused as total duration
	duration_ = kDuration_;
	status_ = Status::kFadeOut; // skip the expand phase entirely; go straight into the fade timer

	for (WorldTransform& worldTransform : worldTransforms_) {
		worldTransform.Initialize();
		worldTransform.scale_ = {1.0f, 1.0f, 1.0f};
		worldTransform.rotation_ = {0.0f, 0.0f, 0.0f};
		worldTransform.translation_ = position;
	}
}

void DeathParticles::Update() {
	if (isFinished_) {
		return;
	}

	for (uint32_t i = 0; i < kNumParticles_; ++i) {
		Vector3 velocity = {kSpeed_, 0, 0};
		float angle = kAngleUnit_ * i;
		Matrix4x4 matrixRotation = MakeRotateZMatrix(angle);
		velocity = TransformCoordinate(velocity, matrixRotation);
		worldTransforms_[i].translation_ += velocity;
	}

	color_.w = std::clamp(1.0f - counter_ / kDuration_, 0.0f, 1.0f);
	objectColor_.SetColor(color_);

	counter_ += 1.0f / 60.0f;

	if (counter_ >= kDuration_) {
		counter_ = kDuration_;
		isFinished_ = true;
	}

	for (WorldTransform& worldTransform : worldTransforms_) {
		WorldTransformUpdate(worldTransform);
	}
}

void DeathParticles::Draw() {
	model_->PreDraw();
	if (isFinished_) {
		model_->PostDraw();
		return;
	}
	for (WorldTransform& worldTransform : worldTransforms_) {
		model_->Draw(worldTransform, *camera_, textureHandle_, &objectColor_);
	}
	model_->PostDraw();
}