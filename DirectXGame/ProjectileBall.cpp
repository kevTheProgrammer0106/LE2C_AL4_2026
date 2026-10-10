#include "ProjectileBall.h"
#include "GlobalVariables.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "ProjectileBall";
}

void ProjectileBall::InitializeAimed(Model* model, uint32_t textureHandle, Camera* camera, const Vector3& position, const Vector2& aimDirection, Owner owner) {
	Vector3 dir = {aimDirection.x, aimDirection.y, 0.0f};
	float length = std::sqrtf(dir.x * dir.x + dir.y * dir.y);
	if (length > 0.0001f) {
		dir.x /= length;
		dir.y /= length;
	}
	Vector3 velocity = {dir.x * kSpeed, dir.y * kSpeed, 0.0f};

	Initialize(model, textureHandle, camera, position, velocity, owner); 
	worldTransform_.rotation_.z = std::atan2(dir.y, dir.x);
}

void ProjectileBall::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "Speed", kSpeed);
	gv->AddItem(kGroupName, "Width", kWidth);
	gv->AddItem(kGroupName, "Height", kHeight);
	gv->AddItem(kGroupName, "DamageToEnemy", kDamageToEnemy);
	gv->AddItem(kGroupName, "DamageToPlayer", kDamageToPlayer);
	gv->AddItem(kGroupName, "ImpactPowerToEnemy", kImpactPowerToEnemy);
	gv->AddItem(kGroupName, "ImpactPowerToPlayer", kImpactPowerToPlayer);
}

void ProjectileBall::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kSpeed = gv->GetFloatValue(kGroupName, "Speed");
	kWidth = gv->GetFloatValue(kGroupName, "Width");
	kHeight = gv->GetFloatValue(kGroupName, "Height");
	kDamageToEnemy = gv->GetIntValue(kGroupName, "DamageToEnemy");
	kDamageToPlayer = gv->GetIntValue(kGroupName, "DamageToPlayer");
	kImpactPowerToEnemy = gv->GetFloatValue(kGroupName, "ImpactPowerToEnemy");
	kImpactPowerToPlayer = gv->GetFloatValue(kGroupName, "ImpactPowerToPlayer");
}