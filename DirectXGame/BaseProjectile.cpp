#include "BaseProjectile.h"

using namespace KamataEngine;

void BaseProjectile::Initialize(Model* model, uint32_t textureHandle, Camera* camera, const Vector3& position, const Vector3& velocity, Owner owner) {
#ifdef _DEBUG
	assert(model);
#endif
	model_ = model;
	textureHandle_ = textureHandle;
	camera_ = camera;
	velocity_ = velocity;
	owner_ = owner; 
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
}

void BaseProjectile::Update() {
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}
	worldTransform_.translation_ += velocity_;
	WorldTransformUpdate(worldTransform_);
}

void BaseProjectile::Draw() {
	Model::PreDraw();
	model_->Draw(worldTransform_, *camera_, textureHandle_);
	Model::PostDraw();
}

AABB BaseProjectile::GetAABB() const {
	Vector3 pos = worldTransform_.translation_;
	AABB aabb;
	aabb.min = {pos.x - kWidth / 2.0f, pos.y - kHeight / 2.0f, pos.z - kWidth / 2.0f};
	aabb.max = {pos.x + kWidth / 2.0f, pos.y + kHeight / 2.0f, pos.z + kWidth / 2.0f};
	return aabb;
}