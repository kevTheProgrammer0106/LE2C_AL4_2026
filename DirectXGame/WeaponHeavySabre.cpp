#include "WeaponHeavySabre.h"
#include "BaseEnemy.h"
#include "GlobalVariables.h"

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
	kLungeDamage = gv->GetIntValue(kGroupName, "LungeDamage");             // [NEW]
	kLungeImpactPower = gv->GetFloatValue(kGroupName, "LungeImpactPower"); // [NEW]
}
void WeaponHeavySabre::SetWorldPosition(Vector3& TargetPosition) { worldTransform_.translation_ = TargetPosition; }

AABB WeaponHeavySabre::GetAABB() const {
	Vector3 pos = worldTransform_.translation_;
	AABB aabb;
	aabb.min = {pos.x - kHalfWidthMin / 2.0f, pos.y - kHalfHeightMin / 2.0f, pos.z - kHalfDepthMin / 2.0f};
	aabb.max = {pos.x + kHalfWidthMax / 2.0f, pos.y + kHalfHeightMax / 2.0f, pos.z + kHalfDepthMax / 2.0f};
	return aabb;
}

AABB WeaponHeavySabre::GetSwingAABBGeneric(const Vector3& ownerPosition, bool facingRight, int comboIndex) const {
	constexpr float kOverlapMargin = 0.1f;
	const float ownerHalfWidth = BaseEnemy::GetBodyWidth() * 0.5f;

	float reach = kComboReach[comboIndex];
	float swingHeight = kComboSwingHeight[comboIndex];

	AABB aabb;
	if (facingRight) {
		aabb.min.x = ownerPosition.x + ownerHalfWidth - kOverlapMargin;
		aabb.max.x = ownerPosition.x + ownerHalfWidth + reach;
	} else {
		aabb.min.x = ownerPosition.x - ownerHalfWidth - reach;
		aabb.max.x = ownerPosition.x - ownerHalfWidth + kOverlapMargin;
	}
	aabb.min.y = ownerPosition.y - swingHeight * 0.5f;
	aabb.max.y = ownerPosition.y + swingHeight * 0.5f;
	aabb.min.z = ownerPosition.z - ownerHalfWidth;
	aabb.max.z = ownerPosition.z + ownerHalfWidth;
	return aabb;
}

void WeaponHeavySabre::SetWeaponState(WeaponState state) {
	if (weaponState_ == state) {
		return;
	}
	weaponState_ = state;
	equipTimer_ = 0.0f;
	isTransitioning_ = true;
}

void WeaponHeavySabre::UpdateWeaponState() {
	if (!isTransitioning_) {
		return;
	}
	equipTimer_ += (1.0f / 60.0f) / equipDuration_;
	if (equipTimer_ >= 1.0f) {
		equipTimer_ = 1.0f;
		isTransitioning_ = false;
	}
}

void WeaponHeavySabre::SetMeleePositionGeneric(const Vector3& ownerPosition, const Vector3& ownerRotation, bool facingRight, const GenericAttackAnimState* attackState) {
	SRTOffset pose;
	switch (weaponState_) {
	case WeaponState::kHolstered:
		pose = LerpSRT(readyStanceOffset_, holsterOffset_, equipTimer_);
		break;
	case WeaponState::kEquipped:
		pose = LerpSRT(holsterOffset_, readyStanceOffset_, equipTimer_);
		break;
	}

	if (attackState) {
		const ComboAttackPattern& pattern = comboPatterns_[attackState->comboIndex];
		switch (attackState->phase) {
		case AttackPhase::kAnticipate:
			pose = LerpSRT(pattern.anticipateStart, pattern.anticipateEnd, attackState->t);
			break;
		case AttackPhase::kAttack:
			pose = LerpSRT(pattern.anticipateEnd, pattern.attackEnd, attackState->t);
			break;
		case AttackPhase::kRecovery:
			pose = LerpSRT(pattern.attackEnd, pattern.recoveryEnd, attackState->t);
			break;
		}
	}

	Vector3 translation = pose.translation;
	Vector3 rotation = pose.rotation;
	Vector3 baseRotation = ownerRotation;
	constexpr float kFacingRotY[] = {1.570796f, 4.712389f};
	baseRotation.y = kFacingRotY[facingRight ? 0 : 1];

	if (!facingRight) {
		translation.x = -translation.x;
		translation.z = -translation.z;
		rotation.y = -rotation.y;
		rotation.z = -rotation.z;
	}

	Vector3 target = ownerPosition + translation;
	SetWorldPosition(target);
	worldTransform_.rotation_ = baseRotation + rotation;
	worldTransform_.scale_ = pose.scale;
	WorldTransformUpdate(worldTransform_);
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

	Vector3 translation = pose.translation;
	Vector3 rotation = pose.rotation;
	Vector3 baseRotation = ownerRotation;
	constexpr float kFacingRotY[] = {1.570796f, 4.712389f};
	baseRotation.y = kFacingRotY[facingRight ? 0 : 1];

	if (!facingRight) {
		translation.x = -translation.x;
		translation.z = -translation.z;
		rotation.y = -rotation.y;
		rotation.z = -rotation.z;
	}

	Vector3 target = ownerPosition + translation;
	SetWorldPosition(target);
	worldTransform_.rotation_ = baseRotation + rotation;
	worldTransform_.scale_ = pose.scale;
	WorldTransformUpdate(worldTransform_);
}

// WeaponHeavySabre.cpp
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