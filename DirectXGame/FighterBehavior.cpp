#include "FighterBehavior.h"
#include "BaseMelee.h"
#include "EnemyFighter.h"
#include "GameScene.h"
#include "Player.h"

#include <cmath>

using namespace KamataEngine;

FighterBehavior::FighterBehavior(EnemyFighter& owner) : BaseEnemyBehavior(owner), fighterOwner_(owner) {}

void FighterBehavior::InitializeBehaviorState(Behavior behavior) {
	if (behavior == Behavior::kMelee) {
		if (BaseMelee* melee = fighterOwner_.GetCurrentMelee()) {
			melee->SetBehavior(BaseMelee::MeleeBehavior::kAttacking);
			melee->PlaySwingSound(comboIndex_); 
		}
		BehaviorUseMeleeInitialize();
		return;
	}

	if (BaseMelee* melee = fighterOwner_.GetCurrentMelee()) {
		melee->SetBehavior(BaseMelee::MeleeBehavior::kIdle);
	}
	BaseEnemyBehavior::InitializeBehaviorState(behavior);
}

void FighterBehavior::UpdateBehaviorState() {
	if (GetBehavior() == Behavior::kMelee) {
		BehaviorUseMeleeUpdate();
		RunPhysicsAndCollisionUpdate();
		return;
	}
	BaseEnemyBehavior::UpdateBehaviorState();
}

void FighterBehavior::BehaviorRootInitialize() { BaseEnemyBehavior::BehaviorRootInitialize(); }

void FighterBehavior::BehaviorRootUpdate() {
	BaseEnemyBehavior::BehaviorRootUpdate();

	if (cooldownTimer_ > 0.0f) {
		cooldownTimer_ -= 1.0f / 60.0f;
	}
	noAttackTimer_ += 1.0f / 60.0f;
	if (noAttackTimer_ >= kComboResetDuration) {
		comboIndex_ = 0;
	}

	if (!IsPlayerSpotted() || cooldownTimer_ > 0.0f) {
		return;
	}
	if (!IsPlayerOnSameFloor())
		return;

	GameScene* scene = OwnerGameScene();
	if (!scene || !scene->GetPlayer()) {
		return;
	}

	Vector3 diff = scene->GetPlayer()->GetWorldPosition() - owner_.GetWorldPosition();
	float distance = std::sqrtf(diff.x * diff.x + diff.y * diff.y);

	if (distance <= kMeleeEngageRange) {
		RequestBehavior(Behavior::kMelee);
	}
}

void FighterBehavior::BehaviorUseMeleeInitialize() {
	meleePhase_ = MeleePhase::kAnticipate;
	meleeAttackParameter_ = 0;
	OwnerVelocity().x = 0.0f;
	if (BaseMelee* melee = fighterOwner_.GetCurrentMelee()) {
		melee->ResetSwingHits();
	}
}

void FighterBehavior::BehaviorUseMeleeUpdate() {
	++meleeAttackParameter_;

	if (GameScene* scene = OwnerGameScene()) {
		if (Player* player = scene->GetPlayer()) {
			OwnerFaceDirection(player->GetWorldPosition().x < owner_.GetWorldPosition().x ? 1 : 0);
		}
	}

	BaseMelee::GenericAttackAnimState animState;
	animState.comboIndex = comboIndex_;

	switch (meleePhase_) {
	case MeleePhase::kAnticipate: {
		OwnerVelocity().x = 0.0f;
		float duration = sabreComboReference_.GetComboAnticipateDuration(comboIndex_);
		animState.phase = BaseMelee::AttackPhase::kAnticipate;
		animState.t = std::clamp(static_cast<float>(meleeAttackParameter_) / duration, 0.0f, 1.0f);
		if (static_cast<float>(meleeAttackParameter_) >= duration) {
			meleePhase_ = MeleePhase::kAttack;
			meleeAttackParameter_ = 0;
			if (BaseMelee* melee = fighterOwner_.GetCurrentMelee()) {
				fighterOwner_.GetCurrentMelee()->PlaySwingSound(comboIndex_);
			}
		}
		break;
	}
	case MeleePhase::kAttack: {
		float duration = sabreComboReference_.GetComboAttackDuration(comboIndex_);
		animState.phase = BaseMelee::AttackPhase::kAttack;
		animState.t = std::clamp(static_cast<float>(meleeAttackParameter_) / duration, 0.0f, 1.0f);
		if (static_cast<float>(meleeAttackParameter_) >= duration) {
			meleePhase_ = MeleePhase::kRecovery;
			meleeAttackParameter_ = 0;
		}
		break;
	}
	case MeleePhase::kRecovery:
	default: {
		float duration = sabreComboReference_.GetComboRecoveryDuration(comboIndex_);
		animState.phase = BaseMelee::AttackPhase::kRecovery;
		animState.t = std::clamp(static_cast<float>(meleeAttackParameter_) / duration, 0.0f, 1.0f);
		if (static_cast<float>(meleeAttackParameter_) >= duration) {
			comboIndex_ = (comboIndex_ + 1) % fighterOwner_.GetCurrentMelee()->GetMaxCombo();
			noAttackTimer_ = 0.0f;
			cooldownTimer_ = kCooldownDuration;
			RequestBehavior(Behavior::kRoot);
		}
		break;
	}
	}

	fighterOwner_.UpdateMeleeVisual(&animState);
}