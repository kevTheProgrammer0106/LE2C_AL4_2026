#include "CuirassierBehavior.h"
#include "EnemyCuirassier.h"
#include "GameScene.h"
#include "Player.h"

#include <algorithm>
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

void CuirassierBehavior::TickTimers() {
	constexpr float kDt = 1.0f / 60.0f;
	lungeCooldownTimer_ = std::max(0.0f, lungeCooldownTimer_ - kDt);
	comboCooldownTimer_ = std::max(0.0f, comboCooldownTimer_ - kDt);
}

void CuirassierBehavior::UpdateBehaviorState() {
	TickTimers(); // tick in every state, so the lunge cooldown also runs while blocking/stunned/etc.

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

	noAttackTimer_ += 1.0f / 60.0f;
	if (noAttackTimer_ >= kComboResetDuration) {
		comboIndex_ = 0;
	}

	GameScene* scene = OwnerGameScene();
	if (!scene || !scene->GetPlayer()) {
		return;
	}
	if (!IsPlayerSpotted() || !IsPlayerOnSameFloor()) {
		return;
	}
	if (comboCooldownTimer_ > 0.0f) {
		return; // short pause after the last engagement
	}

	Vector3 diff = scene->GetPlayer()->GetWorldPosition() - owner_.GetWorldPosition();
	float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

	if (distance <= kComboEngageRange) {
		startWithLunge_ = false; // close: straight into the two swings
		RequestBehavior(Behavior::kMelee);
	} else if (distance <= kMeleeEngageRange && lungeCooldownTimer_ <= 0.0f) {
		startWithLunge_ = true; // mid range: lunge in, then swings
		RequestBehavior(Behavior::kMelee);
	}
	// otherwise keep chasing until in range / lunge is ready
}

void CuirassierBehavior::BehaviorMeleeInitialize() {
	attackParameter_ = 0;
	swingsRemaining_ = kSwingsPerEngagement;
	OwnerVelocity().x = 0.0f;

	if (startWithLunge_) {
		meleeStage_ = MeleeStage::kLunge;
		lungePhase_ = LungePhase::kAnticipate;
		lungeCooldownTimer_ = kLungeCooldown; // starts the moment the lunge is launched
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
	noAttackTimer_ = 0.0f;

	GameScene* scene = OwnerGameScene();
	Player* player = scene ? scene->GetPlayer() : nullptr;
	WeaponHeavySabre* weapon = cuirassierOwner_.GetCurrentMelee();

	// Only track the player while winding up; once the attack starts, facing is committed.
	const bool anticipating = (meleeStage_ == MeleeStage::kLunge) ? (lungePhase_ == LungePhase::kAnticipate) : (comboPhase_ == ComboPhase::kAnticipate);
	if (anticipating) {
		FacePlayer();
	}

	// Stop pushing forward once the weapon has connected (or been blocked) this attack.
	const bool alreadyHit = (weapon && player && weapon->HasHit(player));

	// ---------------- Lunge ----------------
	if (meleeStage_ == MeleeStage::kLunge) {
		float t = 0.0f;
		switch (lungePhase_) {
		case LungePhase::kAnticipate: {
			OwnerVelocity().x = 0.0f;
			const float duration = kLungeAnticipateDuration * 60.0f;
			t = std::clamp(static_cast<float>(attackParameter_) / duration, 0.0f, 1.0f);
			if (static_cast<float>(attackParameter_) >= duration) {
				lungePhase_ = LungePhase::kAttack;
				attackParameter_ = 0;
				WeaponHeavySabre::PlayLungeSound();
			}
			break;
		}
		case LungePhase::kAttack: {
			if (alreadyHit) {
				OwnerVelocity().x = 0.0f;
			} else {
				AdvanceTowardFacing(kLungeSpeed);
			}
			const float duration = kLungeAttackDuration * 60.0f;
			t = std::clamp(static_cast<float>(attackParameter_) / duration, 0.0f, 1.0f);
			if (static_cast<float>(attackParameter_) >= duration) {
				lungePhase_ = LungePhase::kRecovery;
				attackParameter_ = 0;
			}
			break;
		}
		case LungePhase::kRecovery:
		default: {
			OwnerVelocity().x = 0.0f;
			const float duration = kLungeRecoveryDuration * 60.0f;
			t = std::clamp(static_cast<float>(attackParameter_) / duration, 0.0f, 1.0f);
			if (static_cast<float>(attackParameter_) >= duration) {
				// Lunge done -> unleash the two swings
				meleeStage_ = MeleeStage::kCombo;
				comboPhase_ = ComboPhase::kAnticipate;
				attackParameter_ = 0;
				if (weapon) {
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

	// ---------------- Swings (kSwingsPerEngagement per engagement) ----------------
	BaseMelee::GenericAttackAnimState animState;
	animState.comboIndex = comboIndex_;

	switch (comboPhase_) {
	case ComboPhase::kAnticipate: {
		OwnerVelocity().x = 0.0f;
		animState.phase = BaseMelee::AttackPhase::kAnticipate;
		const float duration = 10.0f;
		animState.t = std::clamp(static_cast<float>(attackParameter_) / duration, 0.0f, 1.0f);
		if (static_cast<float>(attackParameter_) >= duration) {
			comboPhase_ = ComboPhase::kAttack;
			attackParameter_ = 0;
			if (weapon) {
				static_cast<BaseMelee*>(weapon)->PlayDefaultSwingSound();
			}
		}
		break;
	}
	case ComboPhase::kAttack: {
		// small forward step during the swing (like the player / Fighter)
		if (alreadyHit) {
			OwnerVelocity().x = 0.0f;
		} else {
			AdvanceTowardFacing(kSwingAdvanceSpeed);
		}
		animState.phase = BaseMelee::AttackPhase::kAttack;
		const float duration = 10.0f;
		animState.t = std::clamp(static_cast<float>(attackParameter_) / duration, 0.0f, 1.0f);
		if (static_cast<float>(attackParameter_) >= duration) {
			comboPhase_ = ComboPhase::kRecovery;
			attackParameter_ = 0;
		}
		break;
	}
	case ComboPhase::kRecovery:
	default: {
		OwnerVelocity().x = 0.0f;
		animState.phase = BaseMelee::AttackPhase::kRecovery;
		const float duration = 18.0f;
		animState.t = std::clamp(static_cast<float>(attackParameter_) / duration, 0.0f, 1.0f);
		if (static_cast<float>(attackParameter_) >= duration) {
			comboIndex_ = (comboIndex_ + 1) % 3; // rotation 0 -> 1 -> 2 -> 0
			--swingsRemaining_;

			if (swingsRemaining_ > 0) {
				comboPhase_ = ComboPhase::kAnticipate;
				attackParameter_ = 0;
				if (weapon) {
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

//WeaponHeavySabre& CuirassierBehavior::Weapon() { return *cuirassierOwner_.GetCurrentMelee(); }