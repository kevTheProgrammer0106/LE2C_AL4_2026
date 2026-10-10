#include "BaseMelee.h"
#include "BaseEnemy.h"
#include "GlobalVariables.h"
#include "Player.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "BaseMelee";
}

void BaseMelee::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	// Register内(旧 BaseImpactPower / ImpactPowerStep の2行と置き換え)
	gv->AddItem(kGroupName, "BaseImpactPowerPlayer", kBaseImpactPowerPlayer);
	gv->AddItem(kGroupName, "ImpactPowerStepPlayer", kImpactPowerStepPlayer);
	gv->AddItem(kGroupName, "BaseImpactPowerEnemy", kBaseImpactPowerEnemy);
	gv->AddItem(kGroupName, "ImpactPowerStepEnemy", kImpactPowerStepEnemy);
	gv->AddItem(kGroupName, "ComboDamagePlayer0", kComboDamagePlayer[0]);
	gv->AddItem(kGroupName, "ComboDamagePlayer1", kComboDamagePlayer[1]);
	gv->AddItem(kGroupName, "ComboDamagePlayer2", kComboDamagePlayer[2]);
	gv->AddItem(kGroupName, "ComboDamageEnemy0", kComboDamageEnemy[0]);
	gv->AddItem(kGroupName, "ComboDamageEnemy1", kComboDamageEnemy[1]);
	gv->AddItem(kGroupName, "ComboDamageEnemy2", kComboDamageEnemy[2]);
}
void BaseMelee::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	// Apply内(同様に置き換え)
	kBaseImpactPowerPlayer = gv->GetFloatValue(kGroupName, "BaseImpactPowerPlayer");
	kImpactPowerStepPlayer = gv->GetFloatValue(kGroupName, "ImpactPowerStepPlayer");
	kBaseImpactPowerEnemy = gv->GetFloatValue(kGroupName, "BaseImpactPowerEnemy");
	kImpactPowerStepEnemy = gv->GetFloatValue(kGroupName, "ImpactPowerStepEnemy");
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

#pragma region Equip state
void BaseMelee::SetWeaponState(WeaponState state) {
	if (weaponState_ == state) {
		return;
	}
	weaponState_ = state;
	equipTimer_ = 0.0f;
	isTransitioning_ = true;
}

void BaseMelee::UpdateWeaponState() {
	if (!isTransitioning_) {
		return;
	}
	equipTimer_ += (1.0f / 60.0f) / equipDuration_;
	if (equipTimer_ >= 1.0f) {
		equipTimer_ = 1.0f;
		isTransitioning_ = false;
	}
}

void BaseMelee::SnapWeaponState(WeaponState state) {
	weaponState_ = state;
	equipTimer_ = 1.0f;
	isTransitioning_ = false;
}

void BaseMelee::SetEquipDuration(float seconds) { equipDuration_ = std::max(seconds, 0.01f); }
#pragma endregion

#pragma region Pose
SRTOffset BaseMelee::ComputeEquipPose() {
	const SRTOffset& holster = GetHolsterOffset();
	const SRTOffset& ready = GetReadyStanceOffset();

	SRTOffset pose = (weaponState_ == WeaponState::kHolstered) ? LerpSRT(ready, holster, equipTimer_) : LerpSRT(holster, ready, equipTimer_);
	float equipWeight = (weaponState_ == WeaponState::kEquipped) ? equipTimer_ : (1.0f - equipTimer_);
	return ApplyReactionPose(pose, equipWeight);
}

SRTOffset BaseMelee::BlendComboPose(const ComboAttackPattern& pattern, AttackPhase phase, float t) {
	switch (phase) {
	case AttackPhase::kAnticipate:
		return LerpSRT(pattern.anticipateStart, pattern.anticipateEnd, t);
	case AttackPhase::kAttack:
		return LerpSRT(pattern.anticipateEnd, pattern.attackEnd, t);
	case AttackPhase::kRecovery:
	default:
		return LerpSRT(pattern.attackEnd, pattern.recoveryEnd, t);
	}
}

void BaseMelee::ApplyPoseToWorld(const SRTOffset& pose, const Vector3& basePos, const Vector3& baseRot, bool facingRight, bool mirrorFull) {
	Vector3 translation = pose.translation;
	Vector3 rotation = pose.rotation;
	Vector3 baseRotation = baseRot;
	constexpr float kFacingRotY[] = {1.570796f, 4.712389f};
	baseRotation.y = kFacingRotY[facingRight ? 0 : 1];

	if (!facingRight) {
		translation.x = -translation.x;
		if (mirrorFull) {
			translation.z = -translation.z;
			rotation.y = -rotation.y;
			rotation.z = -rotation.z;
		}
	}

	worldTransform_.translation_ = basePos + translation;
	worldTransform_.rotation_ = baseRotation + rotation;
	worldTransform_.scale_ = pose.scale;
	WorldTransformUpdate(worldTransform_);
}

void BaseMelee::SetMeleePosition(const Player& player) {
	const bool facingRight = (player.GetLRDirection() == Player::LRDirection::kRight);
	const bool swinging = player.IsMeleeSwingActive(); // lunges don't use the combo poses
	const Player::AttackDirection dir = swinging ? player.GetAttackDirection() : Player::AttackDirection::kSide;

	// Always evaluated so the reaction blend ticks once per frame, even while attacking.
	SRTOffset pose = ComputeEquipPose();

	if (swinging) {
		Player::AttackAnimState anim = player.GetAttackAnimState();
		AttackPhase phase = static_cast<AttackPhase>(static_cast<int>(anim.phase));

		if (dir == Player::AttackDirection::kSide) {
			int idx = std::clamp(anim.comboIndex, 0, 2);
			pose = BlendComboPose(GetComboPatterns()[idx], phase, anim.t);
		} else {
			const SRTOffset& downPose = GetDownAttackOffset();
			const SRTOffset& holster = GetHolsterOffset();
			switch (phase) {
			case AttackPhase::kAnticipate:
				pose = LerpSRT(holster, downPose, anim.t);
				break;
			case AttackPhase::kAttack:
				pose = downPose;
				break;
			case AttackPhase::kRecovery:
				pose = LerpSRT(downPose, holster, anim.t);
				break;
			}
		}
	}

	ApplyPoseToWorld(pose, player.GetModelWorldPosition(), player.GetModelWorldTransform().rotation_, facingRight, dir == Player::AttackDirection::kSide);
}

void BaseMelee::SetMeleePositionGeneric(const Vector3& ownerPosition, const Vector3& ownerRotation, bool facingRight, const GenericAttackAnimState* attackState) {
	SRTOffset pose = ComputeEquipPose();
	if (attackState) {
		int idx = std::clamp(attackState->comboIndex, 0, 2);
		pose = BlendComboPose(GetComboPatterns()[idx], attackState->phase, attackState->t);
	}
	ApplyPoseToWorld(pose, ownerPosition, ownerRotation, facingRight, true);
}
#pragma endregion

#pragma region Hitboxes
AABB BaseMelee::GetSwingAABB(const Player& player, int comboIndex) const {
	comboIndex = std::clamp(comboIndex, 0, 2);
	AABB playerAABB = const_cast<Player&>(player).GetAABB();
	const bool facingRight = (player.GetLRDirection() == Player::LRDirection::kRight);
	constexpr float kOverlapMargin = 0.1f;

	AABB aabb;
	if (player.GetAttackDirection() == Player::AttackDirection::kDown) {
		float reach = GetUpDownReach()[comboIndex];
		float width = GetUpDownWidth()[comboIndex];
		float centerX = (playerAABB.min.x + playerAABB.max.x) * 0.5f;
		aabb.min.x = centerX - width * 0.5f;
		aabb.max.x = centerX + width * 0.5f;
		aabb.min.y = playerAABB.min.y - reach;
		aabb.max.y = playerAABB.min.y + kOverlapMargin;
	} else {
		float reach = GetComboReach()[comboIndex];
		float swingHeight = GetComboSwingHeight()[comboIndex];
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
	}
	aabb.min.z = playerAABB.min.z;
	aabb.max.z = playerAABB.max.z;
	return aabb;
}

AABB BaseMelee::GetSwingAABBGeneric(const Vector3& ownerPosition, bool facingRight, int comboIndex) const {
	comboIndex = std::clamp(comboIndex, 0, 2);
	constexpr float kOverlapMargin = 0.1f;
	const float ownerHalfWidth = BaseEnemy::GetBodyWidth() * 0.5f;

	float reach = GetComboReach()[comboIndex];
	float swingHeight = GetComboSwingHeight()[comboIndex];

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
#pragma endregion
