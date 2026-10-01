#include "WeaponSabre.h"
#include "BaseEnemy.h"
#include "GlobalVariables.h"
#include "Player.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "MeleeSabre";
} // namespace

void WeaponSabre::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);

	gv->AddItem(kGroupName, "HalfWidthMin", WeaponSabre::kHalfWidthMin);
	gv->AddItem(kGroupName, "HalfWidthMax", WeaponSabre::kHalfWidthMax);
	gv->AddItem(kGroupName, "HalfHeightMin", WeaponSabre::kHalfHeightMin);
	gv->AddItem(kGroupName, "HalfHeightMax", WeaponSabre::kHalfHeightMax);
	gv->AddItem(kGroupName, "HalfDepthMin", WeaponSabre::kHalfDepthMin);
	gv->AddItem(kGroupName, "HalfDepthMax", WeaponSabre::kHalfDepthMax);

	gv->AddItem(kGroupName, "HolsterOffsetTranslation", holsterOffset_.translation);
	gv->AddItem(kGroupName, "HolsterOffsetRotation", holsterOffset_.rotation);
}

void WeaponSabre::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();

	kHalfWidthMin = gv->GetFloatValue(kGroupName, "HalfWidthMin");
	kHalfWidthMax = gv->GetFloatValue(kGroupName, "HalfWidthMax");
	kHalfHeightMin = gv->GetFloatValue(kGroupName, "HalfHeightMin");
	kHalfHeightMax = gv->GetFloatValue(kGroupName, "HalfHeightMax");
	kHalfDepthMin = gv->GetFloatValue(kGroupName, "HalfDepthMin");
	kHalfDepthMax = gv->GetFloatValue(kGroupName, "HalfDepthMax");

	holsterOffset_.translation = gv->GetVector3Value(kGroupName, "HolsterOffsetTranslation");
	holsterOffset_.rotation = gv->GetVector3Value(kGroupName, "HolsterOffsetRotation");
}

void WeaponSabre::SetWorldPosition(KamataEngine::Vector3& TargetPosition) { worldTransform_.translation_ = TargetPosition; }

AABB WeaponSabre::GetSwingAABB(const Player& player, int comboIndex) const {
	AABB playerAABB = const_cast<Player&>(player).GetAABB();
	Player::AttackDirection dir = player.GetAttackDirection();
	bool facingRight = (player.GetLRDirection() == Player::LRDirection::kRight);
	constexpr float kOverlapMargin = 0.1f;

	AABB aabb;
	switch (dir) {
	case Player::AttackDirection::kDown: {
		float reach = kUpDownReach[comboIndex];
		float width = kUpDownWidth[comboIndex];
		float centerX = (playerAABB.min.x + playerAABB.max.x) * 0.5f;
		aabb.min.x = centerX - width * 0.5f;
		aabb.max.x = centerX + width * 0.5f;
		aabb.min.y = playerAABB.min.y - reach;
		aabb.max.y = playerAABB.min.y + kOverlapMargin;
		aabb.min.z = playerAABB.min.z;
		aabb.max.z = playerAABB.max.z;
		break;
	}
	case Player::AttackDirection::kSide:
	default: {
		float reach = kComboReach[comboIndex];
		float swingHeight = kComboSwingHeight[comboIndex];
		if (facingRight) {
			aabb.min.x = playerAABB.max.x - kOverlapMargin;
			aabb.max.x = playerAABB.max.x + reach;
		} else {
			aabb.min.x = playerAABB.min.x - reach;
			aabb.max.x = playerAABB.min.x + kOverlapMargin;
		}
		float centerY = (playerAABB.min.y + playerAABB.max.y) * 0.5f;
		aabb.min.y = centerY - swingHeight * 0.5f;
		aabb.max.y = centerY + swingHeight * 0.5f;
		aabb.min.z = playerAABB.min.z;
		aabb.max.z = playerAABB.max.z;
		break;
	}
	}
	return aabb;
}


// 横振りAABBとして扱う(Fighter/Cuirassierどちらも横薙ぎ/突きなのでこれで十分)。
AABB WeaponSabre::GetSwingAABBGeneric(const Vector3& ownerPosition, bool facingRight, int comboIndex) const {
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

void WeaponSabre::SetWeaponState(WeaponState state) {
	if (weaponState_ == state) {
		return;
	}

	weaponState_ = state;
	equipTimer_ = 0.0f;
	isTransitioning_ = true;
}
void WeaponSabre::UpdateWeaponState() {
	if (!isTransitioning_) {
		return;
	}

	equipTimer_ += (1.0f / 60.0f) / equipDuration_;

	if (equipTimer_ >= 1.0f) {
		equipTimer_ = 1.0f;
		isTransitioning_ = false;
	}
}
void WeaponSabre::SetMeleePosition(const Player& player) {
	bool facingRight = (player.GetLRDirection() == Player::LRDirection::kRight);
	Player::AttackDirection dir = player.isAttack() ? player.GetAttackDirection() : Player::AttackDirection::kSide;

	SRTOffset pose;
	switch (weaponState_) {
	case WeaponState::kHolstered:
		pose = LerpSRT(readyStanceOffset_, holsterOffset_, equipTimer_);
		break;
	case WeaponState::kEquipped:
		pose = LerpSRT(holsterOffset_, readyStanceOffset_, equipTimer_);
		break;
	}

	if (player.isAttack()) {
		Player::AttackAnimState anim = player.GetAttackAnimState();

		if (dir == Player::AttackDirection::kSide) {
			const ComboAttackPattern& pattern = comboPatterns_[anim.comboIndex];
			switch (anim.phase) {
			case Player::AttackPhase::kAnticipate:
				pose = LerpSRT(pattern.anticipateStart, pattern.anticipateEnd, anim.t);
				break;
			case Player::AttackPhase::kAttack:
				pose = LerpSRT(pattern.anticipateEnd, pattern.attackEnd, anim.t);
				break;
			case Player::AttackPhase::kRecovery:
				pose = LerpSRT(pattern.attackEnd, pattern.recoveryEnd, anim.t);
				break;
			}
		} else {
			const SRTOffset& directionPose = kDownAttackOffset;
			switch (anim.phase) {
			case Player::AttackPhase::kAnticipate:
				pose = LerpSRT(holsterOffset_, directionPose, anim.t);
				break;
			case Player::AttackPhase::kAttack:
				pose = directionPose;
				break;
			case Player::AttackPhase::kRecovery:
				pose = LerpSRT(directionPose, holsterOffset_, anim.t);
				break;
			}
		}
	}

	Vector3 translation = pose.translation;
	Vector3 rotation = pose.rotation;
	Vector3 baseRotation = player.GetWorldTransform().rotation_;
	constexpr float kFacingRotY[] = {1.570796f, 4.712389f};
	baseRotation.y = kFacingRotY[facingRight ? 0 : 1];

	if (dir == Player::AttackDirection::kSide) {
		if (!facingRight) {
			translation.x = -translation.x;
			translation.z = -translation.z;
			rotation.y = -rotation.y;
			rotation.z = -rotation.z;
		}
	} else {
		if (!facingRight) {
			translation.x = -translation.x;
		}
	}

	Vector3 target = player.GetWorldPosition() + translation;
	SetWorldPosition(target);
	worldTransform_.rotation_ = baseRotation + rotation;
	worldTransform_.scale_ = pose.scale;
	WorldTransformUpdate(worldTransform_);
}

void WeaponSabre::SetMeleePositionGeneric(const Vector3& ownerPosition, const Vector3& ownerRotation, bool facingRight, const GenericAttackAnimState* attackState) {
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

AABB WeaponSabre::GetAABB() const {
	Vector3 pos = worldTransform_.translation_;
	AABB aabb;
	aabb.min = {pos.x - kHalfWidthMin / 2.0f, pos.y - kHalfHeightMin / 2.0f, pos.z - kHalfDepthMin / 2.0f};
	aabb.max = {pos.x + kHalfWidthMax / 2.0f, pos.y + kHalfHeightMax / 2.0f, pos.z + kHalfDepthMax / 2.0f};
	return aabb;
}