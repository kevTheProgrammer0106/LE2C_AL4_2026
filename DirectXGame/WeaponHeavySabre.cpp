#define NOMINMAX
#include "WeaponHeavySabre.h"
#include "BaseEnemy.h"
#include "GlobalVariables.h"
#include "Player.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "WeaponHeavySabre";
}

void WeaponHeavySabre::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "HalfWidthMin", kHalfWidthMin);
	gv->AddItem(kGroupName, "HalfWidthMax", kHalfWidthMax);
	gv->AddItem(kGroupName, "HalfHeightMin", kHalfHeightMin);
	gv->AddItem(kGroupName, "HalfHeightMax", kHalfHeightMax);
	gv->AddItem(kGroupName, "HalfDepthMin", kHalfDepthMin);
	gv->AddItem(kGroupName, "HalfDepthMax", kHalfDepthMax);
	gv->AddItem(kGroupName, "LungeReach", kLungeReach);
	gv->AddItem(kGroupName, "HolsterOffsetTranslation", holsterOffset_.translation);
	gv->AddItem(kGroupName, "HolsterOffsetRotation", holsterOffset_.rotation);
	gv->AddItem(kGroupName, "LungeDamage", kLungeDamage);
	gv->AddItem(kGroupName, "LungeImpactPower", kLungeImpactPower);
	gv->AddItem(kGroupName, "PlayerLungeAnticipate", kPlayerLungeAnticipate);
	gv->AddItem(kGroupName, "PlayerLungeAttack", kPlayerLungeAttack);
	gv->AddItem(kGroupName, "PlayerLungeRecovery", kPlayerLungeRecovery);
	gv->AddItem(kGroupName, "PlayerLungeSpeed", kPlayerLungeSpeed);
	gv->AddItem(kGroupName, "PlayerLungeDamage", kPlayerLungeDamage);
	gv->AddItem(kGroupName, "PlayerLungeImpact", kPlayerLungeImpact);
	gv->AddItem(kGroupName, "PlayerLungeCooldown", kPlayerLungeCooldown);
	gv->AddItem(kGroupName, "PlayerLungeReach", kPlayerLungeReach);
	gv->AddItem(kGroupName, "PlayerLungeHeight", kPlayerLungeHeight);
}

void WeaponHeavySabre::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kHalfWidthMin = gv->GetFloatValue(kGroupName, "HalfWidthMin");
	kHalfWidthMax = gv->GetFloatValue(kGroupName, "HalfWidthMax");
	kHalfHeightMin = gv->GetFloatValue(kGroupName, "HalfHeightMin");
	kHalfHeightMax = gv->GetFloatValue(kGroupName, "HalfHeightMax");
	kHalfDepthMin = gv->GetFloatValue(kGroupName, "HalfDepthMin");
	kHalfDepthMax = gv->GetFloatValue(kGroupName, "HalfDepthMax");
	kLungeReach = gv->GetFloatValue(kGroupName, "LungeReach");
	holsterOffset_.translation = gv->GetVector3Value(kGroupName, "HolsterOffsetTranslation");
	holsterOffset_.rotation = gv->GetVector3Value(kGroupName, "HolsterOffsetRotation");
	kLungeDamage = gv->GetIntValue(kGroupName, "LungeDamage");
	kLungeImpactPower = gv->GetFloatValue(kGroupName, "LungeImpactPower");
	kPlayerLungeAnticipate = gv->GetFloatValue(kGroupName, "PlayerLungeAnticipate");
	kPlayerLungeAttack = gv->GetFloatValue(kGroupName, "PlayerLungeAttack");
	kPlayerLungeRecovery = gv->GetFloatValue(kGroupName, "PlayerLungeRecovery");
	kPlayerLungeSpeed = gv->GetFloatValue(kGroupName, "PlayerLungeSpeed");
	kPlayerLungeDamage = gv->GetIntValue(kGroupName, "PlayerLungeDamage");
	kPlayerLungeImpact = gv->GetFloatValue(kGroupName, "PlayerLungeImpact");
	kPlayerLungeCooldown = gv->GetFloatValue(kGroupName, "PlayerLungeCooldown");
	kPlayerLungeReach = gv->GetFloatValue(kGroupName, "PlayerLungeReach");
	kPlayerLungeHeight = gv->GetFloatValue(kGroupName, "PlayerLungeHeight");
}

AABB WeaponHeavySabre::GetAABB() const {
	Vector3 pos = worldTransform_.translation_;
	AABB aabb;
	aabb.min = {pos.x - kHalfWidthMin / 2.0f, pos.y - kHalfHeightMin / 2.0f, pos.z - kHalfDepthMin / 2.0f};
	aabb.max = {pos.x + kHalfWidthMax / 2.0f, pos.y + kHalfHeightMax / 2.0f, pos.z + kHalfDepthMax / 2.0f};
	return aabb;
}

void WeaponHeavySabre::SetLungePositionGeneric(const Vector3& ownerPosition, const Vector3& ownerRotation, bool facingRight, AttackPhase lungePhase, float t) {
	SRTOffset pose;
	switch (lungePhase) {
	case AttackPhase::kAnticipate:
		pose = LerpSRT(readyStanceOffset_, kLungeAnticipate, t);
		break;
	case AttackPhase::kAttack:
		pose = LerpSRT(kLungeAnticipate, kLungeExtend, t);
		break;
	case AttackPhase::kRecovery:
	default:
		pose = LerpSRT(kLungeExtend, readyStanceOffset_, t);
		break;
	}
	ApplyPoseToWorld(pose, ownerPosition, ownerRotation, facingRight, true);
}

AABB WeaponHeavySabre::GetLungeAABBGeneric(const Vector3& ownerPosition, bool facingRight) const {
	constexpr float kOverlapMargin = 0.1f;
	// 1.4 / 1.6 with the default body height: same value as before, but now scales with the body.
	constexpr float kLungeHeightRatio = 0.875f;
	const float ownerHalfWidth = BaseEnemy::GetBodyWidth() * 0.5f;
	const float lungeHeight = BaseEnemy::GetBodyHeight() * kLungeHeightRatio;

	AABB aabb;
	if (facingRight) {
		aabb.min.x = ownerPosition.x + ownerHalfWidth - kOverlapMargin;
		aabb.max.x = ownerPosition.x + ownerHalfWidth + kLungeReach;
	} else {
		aabb.min.x = ownerPosition.x - ownerHalfWidth - kLungeReach;
		aabb.max.x = ownerPosition.x - ownerHalfWidth + kOverlapMargin;
	}
	aabb.min.y = ownerPosition.y - lungeHeight * 0.5f;
	aabb.max.y = ownerPosition.y + lungeHeight * 0.5f;
	aabb.min.z = ownerPosition.z - ownerHalfWidth;
	aabb.max.z = ownerPosition.z + ownerHalfWidth;
	return aabb;
}
void WeaponHeavySabre::SetPlayerLungePosition(const Player& player, AttackPhase phase, float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	SRTOffset pose;
	switch (phase) {
	case AttackPhase::kAnticipate:
		pose = LerpSRT(readyStanceOffset_, kPlayerLungeAnticipatePose, t);
		break;
	case AttackPhase::kAttack:
		pose = LerpSRT(kPlayerLungeAnticipatePose, kPlayerLungeExtendPose, t);
		break;
	case AttackPhase::kRecovery:
	default:
		pose = LerpSRT(kPlayerLungeExtendPose, readyStanceOffset_, t);
		break;
	}
	const bool facingRight = (player.GetLRDirection() == Player::LRDirection::kRight);
	ApplyPoseToWorld(pose, player.GetModelWorldPosition(), player.GetModelWorldTransform().rotation_, facingRight, true);
}

AABB WeaponHeavySabre::GetPlayerLungeAABB(const AABB& body, bool facingRight) const {
	constexpr float kOverlapMargin = 0.1f;
	const float centerY = (body.min.y + body.max.y) * 0.5f;

	AABB aabb;
	if (facingRight) {
		aabb.min.x = body.max.x - kOverlapMargin;
		aabb.max.x = body.max.x + kPlayerLungeReach;
	} else {
		aabb.min.x = body.min.x - kPlayerLungeReach;
		aabb.max.x = body.min.x + kOverlapMargin;
	}
	aabb.min.y = centerY - kPlayerLungeHeight * 0.5f;
	aabb.max.y = centerY + kPlayerLungeHeight * 0.5f;
	aabb.min.z = body.min.z;
	aabb.max.z = body.max.z;
	return aabb;
}