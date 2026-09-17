#include "RiflemanBehavior.h"
#include "EnemyRifleman.h"
#include "FirearmMusketBayonet.h"
#include "GameScene.h"
#include "Player.h"

#include <cmath>

using namespace KamataEngine;

RiflemanBehavior::RiflemanBehavior(EnemyRifleman& owner) : BaseEnemyBehavior(owner), riflemanOwner_(owner) {}

void RiflemanBehavior::InitializeBehaviorState(Behavior behavior) {
	switch (behavior) {
	case Behavior::kAiming:
		BehaviorAimingInitialize();
		return;
	case Behavior::kReloading:
		BehaviorReloadingInitialize();
		return;
	case Behavior::kMelee:
		BehaviorMeleeInitialize();
		return;
	default:
		break;
	}
	BaseEnemyBehavior::InitializeBehaviorState(behavior);
}

void RiflemanBehavior::UpdateBehaviorState() {
	switch (GetBehavior()) {
	case Behavior::kAiming:
		BehaviorAimingUpdate();
		RunPhysicsAndCollisionUpdate();
		return;
	case Behavior::kReloading:
		BehaviorReloadingUpdate();
		RunPhysicsAndCollisionUpdate();
		return;
	case Behavior::kMelee:
		BehaviorMeleeUpdate();
		RunPhysicsAndCollisionUpdate();
		return;
	default:
		break;
	}
	BaseEnemyBehavior::UpdateBehaviorState();
}

void RiflemanBehavior::BehaviorRootInitialize() {
	BaseEnemyBehavior::BehaviorRootInitialize();
	OwnerSetFirearmMode(0);
	riflemanOwner_.SetGunPose(EnemyRifleman::GunPose::kNeutral); // [NEW]
}

void RiflemanBehavior::BehaviorRootUpdate() {
	BaseEnemyBehavior::BehaviorRootUpdate();

	if (!IsPlayerSpotted())
		return;

	if (!IsPlayerOnSameFloor())
		return;

	GameScene* scene = OwnerGameScene();
	if (!scene || !scene->GetPlayer())
		return;

	Vector3 diff = scene->GetPlayer()->GetWorldPosition() - owner_.GetWorldPosition();
	float distance = std::sqrtf(diff.x * diff.x + diff.y * diff.y);

	if (distance <= kMeleeEngageRange) {
		RequestBehavior(Behavior::kMelee);
		return;
	}
	if (distance <= kFiringRange) {
		float roll = sRandom_.GetRandValue(0.0f, 1.0f);
		RequestBehavior(roll < kRangedAttackChance ? Behavior::kAiming : Behavior::kMelee);
	}
}

void RiflemanBehavior::BehaviorAimingInitialize() {
	aimTimer_ = 0.0f;
	OwnerVelocity().x = 0.0f;
	OwnerSetFirearmMode(1);
	riflemanOwner_.SetGunPose(EnemyRifleman::GunPose::kAiming); // [NEW]
}

void RiflemanBehavior::BehaviorAimingUpdate() {
	if (!IsPlayerSpotted()) {
		RequestBehavior(Behavior::kRoot);
		return;
	}
	OwnerVelocity().x = 0.0f;
	riflemanOwner_.UpdateFirearmHybrid();

	aimTimer_ += 1.0f / 60.0f;
	if (aimTimer_ >= kAimDuration) {
		riflemanOwner_.ShootFirearm();
		riflemanOwner_.SetGunPose(EnemyRifleman::GunPose::kFiring); // [NEW]
		RequestBehavior(Behavior::kReloading);
	}
}

void RiflemanBehavior::BehaviorReloadingInitialize() {
	retreatTimer_ = 0.0f;
	OwnerSetFirearmMode(2);
	riflemanOwner_.SetGunPose(EnemyRifleman::GunPose::kReloading); // [NEW]
}

void RiflemanBehavior::BehaviorReloadingUpdate() {
	retreatTimer_ += 1.0f / 60.0f;

	if (GameScene* scene = OwnerGameScene()) {
		if (Player* player = scene->GetPlayer()) {
			bool playerOnRight = player->GetWorldPosition().x >= owner_.GetWorldPosition().x;
			OwnerVelocity().x = playerOnRight ? -kRetreatSpeed : kRetreatSpeed;
		}
	}

	riflemanOwner_.UpdateFirearmHybrid();

	FirearmMusketBayonet* firearm = riflemanOwner_.GetCurrentFirearm();
	bool reloadComplete = firearm && !firearm->IsReloading() && firearm->GetCurrentAmmo() > 0;

	if (reloadComplete || retreatTimer_ >= kMaxRetreatDuration) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
	}
}

void RiflemanBehavior::BehaviorMeleeInitialize() {
	meleePhase_ = MeleePhase::kAnticipate;
	meleeParameter_ = 0;
	lungeFacingLocked_ = false; // [FIX]
	OwnerVelocity().x = 0.0f;
	OwnerSetFirearmMode(3);
	riflemanOwner_.SetGunPose(EnemyRifleman::GunPose::kNeutral);
	riflemanOwner_.SetBayonetActive(false);
	riflemanOwner_.ResetBayonetHit();
	if (FirearmMusketBayonet* firearm = riflemanOwner_.GetCurrentFirearm()) {
		firearm->SetReloading(false);
	}
}

void RiflemanBehavior::BehaviorMeleeUpdate() {
	++meleeParameter_;

	// [FIX] only re-face the player before the lunge commits; once locked,
	// the facing (and therefore lunge velocity direction) can't change mid-swing.
	if (!lungeFacingLocked_) {
		if (GameScene* scene = OwnerGameScene()) {
			if (Player* player = scene->GetPlayer()) {
				OwnerFaceDirection(player->GetWorldPosition().x < owner_.GetWorldPosition().x ? 1 : 0);
			}
		}
	}

	bool facingRight = owner_.isRight();

	switch (meleePhase_) {
	case MeleePhase::kAnticipate:
		OwnerVelocity().x = 0.0f;
		riflemanOwner_.SetBayonetActive(false);
		if (static_cast<float>(meleeParameter_) >= sabreComboReference_.GetComboAnticipateDuration(kBayonetComboIndex)) {
			meleePhase_ = MeleePhase::kLunge;
			meleeParameter_ = 0;
			lungeFacingLocked_ = true; // [FIX] commit direction now
		}
		break;
	case MeleePhase::kLunge: {
		float speed = sabreComboReference_.GetComboAttackSpeed(kBayonetComboIndex);
		OwnerVelocity().x = facingRight ? speed : -speed;
		riflemanOwner_.SetBayonetActive(true);
		if (static_cast<float>(meleeParameter_) >= sabreComboReference_.GetComboAttackDuration(kBayonetComboIndex)) {
			meleePhase_ = MeleePhase::kRecovery;
			meleeParameter_ = 0;
		}
		break;
	}
	case MeleePhase::kRecovery:
	default:
		OwnerVelocity().x = 0.0f;
		riflemanOwner_.SetBayonetActive(false);
		if (static_cast<float>(meleeParameter_) >= sabreComboReference_.GetComboRecoveryDuration(kBayonetComboIndex)) {
			RequestBehavior(Behavior::kRoot);
		}
		break;
	}

	riflemanOwner_.UpdateFirearmHybrid();
}