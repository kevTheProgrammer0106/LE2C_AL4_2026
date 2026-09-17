#include "SwitchBlock.h"

using namespace KamataEngine;

void SwitchBlock::Initialize(Model* model, uint32_t textureHandle, Camera* camera, const Vector3& position, uint8_t buttonID) {
#ifdef _DEBUG
	assert(model);
	assert(camera);
#endif
	model_ = model;
	textureHandle_ = textureHandle;
	camera_ = camera;
	buttonID_ = buttonID;
	isPressed_ = false;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	objectColor_.Initialize();
	objectColor_.SetColor(kUnpressedColor);
}

void SwitchBlock::Update() {
	WorldTransformUpdate(worldTransform_);
	objectColor_.SetColor(isPressed_ ? kPressedColor : kUnpressedColor);
}

AABB SwitchBlock::GetAABB() const {
	Vector3 pos = worldTransform_.translation_;
	AABB aabb;
	aabb.min = {pos.x - kHalfWidth, pos.y - kHalfHeight, pos.z - kHalfDepth};
	aabb.max = {pos.x + kHalfWidth, pos.y + kHalfHeight, pos.z + kHalfDepth};
	return aabb;
}

void SwitchBlock::Draw() {
	if (model_) {
		Model::PreDraw();
		model_->Draw(worldTransform_, *camera_, textureHandle_, &objectColor_);
		Model::PostDraw();
	}
}