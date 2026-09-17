#include "BaseMelee.h"
#include "GlobalVariables.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "BaseMelee";
}

void BaseMelee::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "BaseImpactPower", kBaseImpactPower);
	gv->AddItem(kGroupName, "ImpactPowerStep", kImpactPowerStep);
	gv->AddItem(kGroupName, "ComboDamagePlayer0", kComboDamagePlayer[0]);
	gv->AddItem(kGroupName, "ComboDamagePlayer1", kComboDamagePlayer[1]);
	gv->AddItem(kGroupName, "ComboDamagePlayer2", kComboDamagePlayer[2]);
	gv->AddItem(kGroupName, "ComboDamageEnemy0", kComboDamageEnemy[0]);
	gv->AddItem(kGroupName, "ComboDamageEnemy1", kComboDamageEnemy[1]);
	gv->AddItem(kGroupName, "ComboDamageEnemy2", kComboDamageEnemy[2]);
}
void BaseMelee::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kBaseImpactPower = gv->GetFloatValue(kGroupName, "BaseImpactPower");
	kImpactPowerStep = gv->GetFloatValue(kGroupName, "ImpactPowerStep");
	kComboDamagePlayer[0] = gv->GetIntValue(kGroupName, "ComboDamagePlayer0");
	kComboDamagePlayer[1] = gv->GetIntValue(kGroupName, "ComboDamagePlayer1");
	kComboDamagePlayer[2] = gv->GetIntValue(kGroupName, "ComboDamagePlayer2");
	kComboDamageEnemy[0] = gv->GetIntValue(kGroupName, "ComboDamageEnemy0");
	kComboDamageEnemy[1] = gv->GetIntValue(kGroupName, "ComboDamageEnemy1");
	kComboDamageEnemy[2] = gv->GetIntValue(kGroupName, "ComboDamageEnemy2");
}

void BaseMelee::Initialize(Model* model, uint32_t textureHandle, Camera* camera, const Vector3& position) {
#ifdef _DEBUG
	assert(model);
#endif
	model_ = model;
	textureHandle_ = textureHandle;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.translation_ = position;
}

void BaseMelee::Update() {
	switch (behavior_) {
	case MeleeBehavior::kIdle:
		BehaviorIdleUpdate();
		break;
	case MeleeBehavior::kAttacking:
		BehaviorAttackingUpdate();
		break;
	}
	WorldTransformUpdate(worldTransform_);
}

void BaseMelee::Draw() {
	Model::PreDraw();
	model_->Draw(worldTransform_, *camera_, textureHandle_);
	Model::PostDraw();
}

AABB BaseMelee::GetAABB() const {
	Vector3 pos = worldTransform_.translation_;
	AABB aabb;
	aabb.min = {pos.x - kHalfWidthMin / 2.0f, pos.y - kHalfHeightMin / 2.0f, pos.z - kHalfDepthMin / 2.0f};
	aabb.max = {pos.x + kHalfWidthMax / 2.0f, pos.y + kHalfHeightMax / 2.0f, pos.z + kHalfDepthMax / 2.0f};
	return aabb;
}