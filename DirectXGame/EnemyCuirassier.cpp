#include "EnemyCuirassier.h"
#include "GameScene.h"
#include "GlobalVariables.h"
#include "Player.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "EnemyCuirassier";
}

void EnemyCuirassier::RegisterGlobalVariables() {
	GlobalVariables::GetInstance()->CreateGroup(kGroupName);
	RegisterBodyComboPoseSet(kGroupName, "BodyCombo", kBodyCombo);
	RegisterBodyLungePoseSet(kGroupName, "BodyLunge", kBodyLunge);
}
void EnemyCuirassier::ApplyGlobalVariables() {
	ApplyBodyComboPoseSet(kGroupName, "BodyCombo", kBodyCombo);
	ApplyBodyLungePoseSet(kGroupName, "BodyLunge", kBodyLunge);
}

void EnemyCuirassier::Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, Camera* camera, const Vector3& position) {
	BaseEnemy::Initialize(partResources, camera, position);
	SetMaxHP(350);
	SetHP(350);
}

void EnemyCuirassier::Update() {
	BaseEnemy::Update();

	if (isDead_ || !currentMelee_)
		return;
	if (GetBehaviorModule().IsDeathBehavior())
		return;

	UpdateWeaponEquipState();

	if (GetBehaviorModule().GetBehavior() == BaseEnemyBehavior::Behavior::kMelee)
		return;
	if (TickObstacleSwing())
		return;
	UpdateMeleeVisual(nullptr);
}


void EnemyCuirassier::Draw() {
	BaseEnemy::Draw();
	if (!isDead_ && !GetBehaviorModule().IsDeathBehavior() && currentMelee_) {
		currentMelee_->Draw();
		DrawAttackEffect(isSwingActive_ || isLungeActive_, isLungeActive_ ? GetLungeAABB() : GetSwingAABB());
	}
}

void EnemyCuirassier::SetMeleeModel(Model* model, uint32_t textureHandle) {
	currentMelee_ = std::make_unique<WeaponHeavySabre>();
	currentMelee_->Initialize(model, textureHandle, camera_, worldTransform_.translation_);
	SnapWeaponEquipState(ignoreEquipStates_);
}

void EnemyCuirassier::UpdateMeleeVisual(const BaseMelee::GenericAttackAnimState* attackState) {
	if (!currentMelee_) {
		return;
	}
	currentMelee_->UpdateWeaponState();
	bool facingRight = isRight();
	currentMelee_->SetReactionPose(GetReactionPose());
	currentMelee_->SetMeleePositionGeneric(modelWorldTransform_.translation_, modelWorldTransform_.rotation_, facingRight, attackState);

	isLungeActive_ = false;
	isSwingActive_ = (attackState != nullptr && attackState->phase == BaseMelee::AttackPhase::kAttack);
	if (attackState) {
		currentComboIndex_ = attackState->comboIndex;
		attackBodyPose_ = EvaluateBodyComboPose(kBodyCombo, static_cast<BodyAttackPhase>(static_cast<int>(attackState->phase)), attackState->t, attackState->comboIndex);
	} else {
		attackBodyPose_ = SRTOffset{};
	}
}

void EnemyCuirassier::UpdateLungeVisual(WeaponHeavySabre::AttackPhase lungePhase, float t) {
	if (!currentMelee_) {
		return;
	}
	currentMelee_->UpdateWeaponState();
	bool facingRight = isRight();
	currentMelee_->SetReactionPose(GetReactionPose());
	currentMelee_->SetLungePositionGeneric(modelWorldTransform_.translation_, modelWorldTransform_.rotation_, facingRight, lungePhase, t);

	isSwingActive_ = false;
	isLungeActive_ = (lungePhase == WeaponHeavySabre::AttackPhase::kAttack);
	attackBodyPose_ = EvaluateBodyLungePose(kBodyLunge, static_cast<BodyAttackPhase>(static_cast<int>(lungePhase)), t);
}

AABB EnemyCuirassier::GetSwingAABB() const { return currentMelee_->GetSwingAABBGeneric(worldTransform_.translation_, isRight(), currentComboIndex_); }
AABB EnemyCuirassier::GetLungeAABB() const { return currentMelee_->GetLungeAABBGeneric(worldTransform_.translation_, isRight()); }

void EnemyCuirassier::CheckPlayerDirection() {
	constexpr float deltaTime = 1.0f / 60.0f;
	Player* player = gameScene_->GetPlayer();

	if (!isTurning_) {
		if (player->GetWorldPosition().x < GetWorldPosition().x) {
			velocity_.x = -kWalkSpeed;
			if (lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
				isTurning_ = true;
			}
		} else {
			velocity_.x = kWalkSpeed;
			if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
				isTurning_ = true;
			}
		}
	}

	if (isTurning_) {
		turnTimer_ -= deltaTime;
		float destinationRotationY = (lrDirection_ == LRDirection::kRight) ? std::numbers::pi_v<float> / 2.0f : std::numbers::pi_v<float> * 3.0f / 2.0f;
		float t = std::clamp(1.0f - (turnTimer_ / kTimeTurn), 0.0f, 1.0f);
		worldTransform_.rotation_.y = EaseInOut(turnFirstRotationY_, destinationRotationY, t);
		if (turnTimer_ <= 0.0f) {
			worldTransform_.rotation_.y = destinationRotationY;
			turnTimer_ = 0.0f;
			isTurning_ = false;
		}
	} else {
		float baseRotation = (lrDirection_ == LRDirection::kLeft) ? -(std::numbers::pi_v<float> / 2.0f) : -(std::numbers::pi_v<float> * 3.0f / 2.0f);
		float angle = std::sin(walkTimer_ * (2.0f * std::numbers::pi_v<float> / kWalkMotionTime)) * 0.2f;
		if (lrDirection_ == LRDirection::kLeft) {
			angle = -angle;
		}
		worldTransform_.rotation_.y = baseRotation + angle;
	}
}