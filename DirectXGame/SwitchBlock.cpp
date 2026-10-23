#include "SwitchBlock.h"

using namespace KamataEngine;


void SwitchBlock::Initialize(Model* leverModel, uint32_t leverTex, Model* supportModel, uint32_t supportTex, Camera* camera, const Vector3& position, uint8_t buttonID) {
#ifdef _DEBUG
	assert(leverModel);
	assert(camera);
#endif
	model_ = leverModel;
	textureHandle_ = leverTex;
	supportModel_ = supportModel;
	supportTex_ = supportTex;
	camera_ = camera;
	buttonID_ = buttonID;
	isPressed_ = false;
	pressTimer_ = 0.0f;
	position_ = position;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position + Vector3(0.0f, kPivotOffsetY, 0.0f);
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	WorldTransformUpdate(worldTransform_);

	supportTransform_.Initialize();
	supportTransform_.translation_ = position + Vector3(0.0f, kPivotOffsetY, 0.0f);
	WorldTransformUpdate(supportTransform_);
}

void SwitchBlock::Update() {
	float tilt = 0.0f;
	if (isPressed_) {
		pressTimer_ += 1.0f / 60.0f;
		const float decay = std::exp(-kBounceDamping * pressTimer_);
		tilt = kTiltAngle * (1.0f - decay * std::cos(kBounceFrequency * pressTimer_));
	}
	worldTransform_.translation_ = position_ + Vector3(0.0f, kPivotOffsetY, 0.0f);
	worldTransform_.rotation_ = {0.0f, 0.0f, tilt};
	WorldTransformUpdate(worldTransform_);

	supportTransform_.translation_ = position_ + Vector3(0.0f, kPivotOffsetY, 0.0f);
	WorldTransformUpdate(supportTransform_);
}

void SwitchBlock::Draw() {
	if (!model_ || !camera_)
		return;
	Model::PreDraw();
	if (supportModel_)
		supportModel_->Draw(supportTransform_, *camera_, supportTex_);
	model_->Draw(worldTransform_, *camera_, textureHandle_);
	Model::PostDraw();
}

//void SwitchBlock::SetPressed(bool pressed, bool snap) {
//	isPressed_ = pressed;
//	if (snap)
//		leverAngle_ = pressed ? kLeverOnAngle : kLeverOffAngle;
//}
void SwitchBlock::SetPressed(bool pressed) {
	if (pressed == isPressed_)
		return;
	isPressed_ = pressed;
	pressTimer_ = 0.0f;
}

void SwitchBlock::SnapPressed() {
	isPressed_ = true;
	pressTimer_ = 100.0f; // 減衰しきった状態 = 倒れた位置で静止
}

AABB SwitchBlock::GetAABB() const {
	Vector3 pos = worldTransform_.translation_;
	AABB aabb;
	aabb.min = {pos.x - kHalfWidth, pos.y - kHalfHeight, pos.z - kHalfDepth};
	aabb.max = {pos.x + kHalfWidth, pos.y + kHalfHeight, pos.z + kHalfDepth};
	return aabb;
}