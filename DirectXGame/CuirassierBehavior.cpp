#include "CuirassierBehavior.h"
#include "EnemyCuirassier.h"
#include "GameScene.h"
#include "Player.h"

#include <cmath>

using namespace KamataEngine;

CuirassierBehavior::CuirassierBehavior(EnemyCuirassier& owner) : BaseEnemyBehavior(owner), cuirassierOwner_(owner) {}

void CuirassierBehavior::InitializeBehaviorState(Behavior behavior) {
	if (behavior == Behavior::kMelee) {
		BehaviorMeleeInitialize();
		return;
	}
	BaseEnemyBehavior::InitializeBehaviorState(behavior); // kBlock handled by the base
}

void CuirassierBehavior::UpdateBehaviorState() {
	if (GetBehavior() == Behavior::kMelee) {
		BehaviorMeleeUpdate();
		RunPhysicsAndCollisionUpdate();
		return;
	}
	BaseEnemyBehavior::UpdateBehaviorState();
}

void CuirassierBehavior::BehaviorRootInitialize() { BaseEnemyBehavior::BehaviorRootInitialize(); }

void CuirassierBehavior::BehaviorRootUpdate() {
	BaseEnemyBehavior::BehaviorRootUpdate();

	GameScene* scene = OwnerGameScene();
	if (!scene || !scene->GetPlayer()) {
		return;
	}
	Player* player = scene->GetPlayer();

	if (!IsPlayerSpotted()) {
		return;
	}
	if (!IsPlayerOnSameFloor()) {
		return;
	}

	Vector3 diff = player->GetWorldPosition() - owner_.GetWorldPosition();
	float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
	if (distance <= kMeleeEngageRange) {
		RequestBehavior(Behavior::kMelee);
	}
}

void CuirassierBehavior::BehaviorMeleeInitialize() {
	attackParameter_ = 0;
	swingsRemaining_ = kSwingsPerEngagement;
	OwnerVelocity().x = 0.0f;

	if (startWithLunge_) {
		meleeStage_ = MeleeStage::kLunge;
		lungePhase_ = LungePhase::kAnticipate;
		lungeCooldownTimer_ = kLungeCooldown; // 5s starts the moment the lunge is launched
	} else {
		meleeStage_ = MeleeStage::kCombo;
		comboPhase_ = ComboPhase::kAnticipate;
	}
	startWithLunge_ = false;

	if (WeaponHeavySabre* weapon = cuirassierOwner_.GetCurrentMelee()) {
		weapon->ResetSwingHits();
	}
}

void CuirassierBehavior::BehaviorMeleeUpdate() {
	++attackParameter_;

	GameScene* scene = OwnerGameScene();
	Player* player = scene ? scene->GetPlayer() : nullptr;
	if (player) {
		OwnerFaceDirection(player->GetWorldPosition().x < owner_.GetWorldPosition().x ? 1 /*kLeft*/ : 0 /*kRight*/);
	}
	bool facingRight = owner_.isRight();

	noAttackTimer_ = 0.0f;

	// ---------------- Lunge ----------------
	if (meleeStage_ == MeleeStage::kLunge) {
		float t = 0.0f;
		switch (lungePhase_) {
		case LungePhase::kAnticipate: {
			OwnerVelocity().x = 0.0f;
			t = std::clamp(static_cast<float>(attackParameter_) / (kLungeAnticipateDuration * 60.0f), 0.0f, 1.0f);
			if (static_cast<float>(attackParameter_) >= kLungeAnticipateDuration * 60.0f) {
				lungePhase_ = LungePhase::kAttack;
				attackParameter_ = 0;
			}
			break;
		}
		case LungePhase::kAttack: {
			OwnerVelocity().x = facingRight ? kLungeSpeed : -kLungeSpeed;
			t = std::clamp(static_cast<float>(attackParameter_) / (kLungeAttackDuration * 60.0f), 0.0f, 1.0f);
			if (static_cast<float>(attackParameter_) >= kLungeAttackDuration * 60.0f) {
				lungePhase_ = LungePhase::kRecovery;
				attackParameter_ = 0;
			}
			break;
		}
		case LungePhase::kRecovery:
		default: {
			OwnerVelocity().x = 0.0f;
			t = std::clamp(static_cast<float>(attackParameter_) / (kLungeRecoveryDuration * 60.0f), 0.0f, 1.0f);
			if (static_cast<float>(attackParameter_) >= kLungeRecoveryDuration * 60.0f) {
				// Lunge done -> unleash the two swings (Fighter rotation continues from comboIndex_)
				meleeStage_ = MeleeStage::kCombo;
				comboPhase_ = ComboPhase::kAnticipate;
				attackParameter_ = 0;
				if (WeaponHeavySabre* weapon = cuirassierOwner_.GetCurrentMelee()) {
					weapon->ResetSwingHits();
				}
			}
			break;
		}
		}

		WeaponHeavySabre::AttackPhase weaponPhase = (lungePhase_ == LungePhase::kAnticipate) ? WeaponHeavySabre::AttackPhase::kAnticipate
		                                            : (lungePhase_ == LungePhase::kAttack)   ? WeaponHeavySabre::AttackPhase::kAttack
		                                                                                     : WeaponHeavySabre::AttackPhase::kRecovery;
		cuirassierOwner_.UpdateLungeVisual(weaponPhase, t);
		return;
	}

	// ---------------- Swings (Fighter style, kSwingsPerEngagement per engagement) ----------------
	BaseMelee::GenericAttackAnimState animState;
	animState.comboIndex = comboIndex_;

	switch (comboPhase_) {
	case ComboPhase::kAnticipate: {
		OwnerVelocity().x = 0.0f;
		animState.phase = BaseMelee::AttackPhase::kAnticipate;
		float duration = 10.0f;
		animState.t = std::clamp(static_cast<float>(attackParameter_) / duration, 0.0f, 1.0f);
		if (static_cast<float>(attackParameter_) >= duration) {
			comboPhase_ = ComboPhase::kAttack;
			attackParameter_ = 0;
			WeaponHeavySabre::PlayLungeSound();
		}
		break;
	}
	case ComboPhase::kAttack: {
		animState.phase = BaseMelee::AttackPhase::kAttack;
		float duration = 10.0f;
		animState.t = std::clamp(static_cast<float>(attackParameter_) / duration, 0.0f, 1.0f);
		if (static_cast<float>(attackParameter_) >= duration) {
			comboPhase_ = ComboPhase::kRecovery;
			attackParameter_ = 0;
		}
		break;
	}
	case ComboPhase::kRecovery:
	default: {
		animState.phase = BaseMelee::AttackPhase::kRecovery;
		float duration = 18.0f;
		animState.t = std::clamp(static_cast<float>(attackParameter_) / duration, 0.0f, 1.0f);
		if (static_cast<float>(attackParameter_) >= duration) {
			comboIndex_ = (comboIndex_ + 1) % 3; // Fighter-style rotation 0 -> 1 -> 2 -> 0
			--swingsRemaining_;

			if (swingsRemaining_ > 0) {
				// second swing of this engagement
				comboPhase_ = ComboPhase::kAnticipate;
				attackParameter_ = 0;
				if (WeaponHeavySabre* weapon = cuirassierOwner_.GetCurrentMelee()) {
					weapon->ResetSwingHits(); // each swing may hit again
				}
			} else {
				noAttackTimer_ = 0.0f;
				comboCooldownTimer_ = kCooldownDuration;
				RequestBehavior(Behavior::kRoot);
			}
		}
		break;
	}
	}
	cuirassierOwner_.UpdateMeleeVisual(&animState);
}

WeaponHeavySabre& CuirassierBehavior::Weapon() { return *cuirassierOwner_.GetCurrentMelee(); }