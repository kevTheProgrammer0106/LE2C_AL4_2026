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
	case Behavior::kIdle:
	case Behavior::kKnockback:
	case Behavior::kStunned:
	case Behavior::kDeath:
		OwnerSetFirearmMode(0);
		riflemanOwner_.SetGunPose(EnemyRifleman::GunPose::kNeutral);
		riflemanOwner_.SetBayonetActive(false);
		riflemanOwner_.ResetBayonetHit();
		if (FirearmMusketBayonet* firearm = riflemanOwner_.GetCurrentFirearm()) {
			firearm->SetReloading(false);
		}
		break;
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
	riflemanOwner_.SetGunPose(EnemyRifleman::GunPose::kNeutral);
	if (FirearmMusketBayonet* firearm = riflemanOwner_.GetCurrentFirearm()) {
		firearm->SetReloading(false); // back to the normal holster pose
	}
}

bool RiflemanBehavior::TryGetPlayerDistance(float& outDistance) const {
	GameScene* scene = OwnerGameScene();
	if (!scene || !scene->GetPlayer()) {
		return false;
	}
	const Vector3 p = scene->GetPlayer()->GetWorldPosition();
	const Vector3 e = owner_.GetWorldPosition();
	const float dx = p.x - e.x;
	const float dy = p.y - e.y;
	outDistance = std::sqrtf(dx * dx + dy * dy);
	return true;
}

float RiflemanBehavior::GetPlayerDistance() const {
	float d = 0.0f;
	TryGetPlayerDistance(d);
	return d;
}
float RiflemanBehavior::CalculateRetreatSpeed(float distance) const { return ScaleRetreatSpeed(distance, kRetreatRange, kBayonetRange, kRetreatSpeed); }

void RiflemanBehavior::BehaviorRootUpdate() {
	GameScene* scene = OwnerGameScene();
	if (!scene || !scene->GetPlayer()) {
		BaseEnemyBehavior::BehaviorRootUpdate();
		return;
	}

	Player* player = scene->GetPlayer();

	// プレイヤーをまだ発見していない間だけ、通常の追跡AIを動かす。
	if (!IsPlayerSpotted()) {
		BaseEnemyBehavior::BehaviorRootUpdate();
		return;
	}

	// 移動せず、その場でプレイヤーの方向だけを向く。
	OwnerFaceDirection(player->GetWorldPosition().x < owner_.GetWorldPosition().x ? 1 : 0);
	if (!CanMakeDecision()) {
		ContinueCommittedMove(); // still hopping / just landed
		return;
	}
	const float distance = GetPlayerDistance();

	if (distance <= kBayonetRange && IsPlayerOnSameFloor()) {
		RequestBehavior(Behavior::kMelee);
		return;
	}
	// 射撃可能距離へ入ったら射撃状態へ移行する。
	if (distance <= kFiringRange) {
		RequestBehavior(Behavior::kAiming);
		return;
	}
}

void RiflemanBehavior::BehaviorAimingInitialize() {
	aimTimer_ = 0.0f;
	currentAimDuration_ = kAimDuration * riflemanOwner_.GetAimTimeScale();


	OwnerVelocity().x = 0.0f;
	OwnerSetFirearmMode(1);

	GameScene* scene = OwnerGameScene();
	if (scene && scene->GetPlayer()) {
		aimTracker_.Reset(owner_.GetModelWorldPosition(), scene->GetPlayer()->GetWorldPosition());
	}

	riflemanOwner_.SetGunPose(EnemyRifleman::GunPose::kAiming);
	if (FirearmMusketBayonet* firearm = riflemanOwner_.GetCurrentFirearm()) {
		firearm->SetReloading(false);
	}
}

void RiflemanBehavior::BehaviorAimingUpdate() {
	constexpr float kDeltaTime = 1.0f / 60.0f;

	GameScene* scene = OwnerGameScene();
	Player* player = scene ? scene->GetPlayer() : nullptr;

	if (!player) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
		return;
	}

	const bool spotted = IsPlayerSpotted();
	aimTracker_.Update(spotted, owner_.GetModelWorldPosition(), player->GetWorldPosition(), kAimTrackSpeed, kDeltaTime);

	// Player is back in sight: gun lerps to them, and the remaining aim time is capped at 0.5s.
	if (aimTracker_.ConsumeReacquired()) {
		currentAimDuration_ = std::min(currentAimDuration_, aimTimer_ + kReacquireAimDuration);
	}

	if (IsMoveCommitted()) {
		ContinueCommittedMove();
		riflemanOwner_.UpdateFirearmHybrid();
		return;
	}


	// Sight lost: hold aim on the last seen position. No firing, aim timer paused.
	// Sight lost: keep the gun on the last seen position, creep toward it, no firing, aim timer paused.
	if (!spotted) {
		if (aimTracker_.GetLostTime() >= kLostSightChaseDelay) {
			OwnerVelocity().x = 0.0f;
			RequestBehavior(Behavior::kRoot); // chase the last known position
			return;
		}
		ApproachPosition(aimTracker_.GetLastSeenPos(), kApproachSpeed, kApproachStopDistance);
		riflemanOwner_.UpdateFirearmHybrid();
		return;
	}
	const float distance = GetPlayerDistance();

	if (distance > kFiringRange) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
		return;
	}

	if (distance <= kBayonetRange && IsPlayerOnSameFloor()) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kMelee);
		return;
	}

	if (distance < kRetreatRange) {
		RetreatAwayFrom(player->GetWorldPosition(), CalculateRetreatSpeed(distance));
	} else {
		OwnerVelocity().x = 0.0f; 
	}

	riflemanOwner_.UpdateFirearmHybrid();

	aimTimer_ += kDeltaTime;
	if (aimTimer_ >= currentAimDuration_) {
		riflemanOwner_.ShootFirearm();
		riflemanOwner_.SetGunPose(EnemyRifleman::GunPose::kFiring);
		RequestBehavior(Behavior::kReloading);
	}
}

void RiflemanBehavior::BehaviorReloadingInitialize() {
	retreatTimer_ = 0.0f;
	retreatMoveTimer_ = 0.0f;

	OwnerSetFirearmMode(2);
	riflemanOwner_.SetGunPose(EnemyRifleman::GunPose::kReloading);
	if (FirearmMusketBayonet* firearm = riflemanOwner_.GetCurrentFirearm()) {
		firearm->SetReloading(true);
	}
}
void RiflemanBehavior::BehaviorReloadingUpdate() {
	constexpr float kDeltaTime = 1.0f / 60.0f;

	retreatTimer_ += kDeltaTime;
	OwnerVelocity().x = 0.0f;

	GameScene* scene = OwnerGameScene();
	Player* player = scene ? scene->GetPlayer() : nullptr;

	if (player) {
		const Vector3 playerPos = player->GetWorldPosition();
		const bool playerOnRight = playerPos.x >= owner_.GetWorldPosition().x;

		// keep facing the player while backing away
		OwnerFaceDirection(playerOnRight ? static_cast<int>(BaseEnemy::LRDirection::kRight) : static_cast<int>(BaseEnemy::LRDirection::kLeft));

		if (IsMoveCommitted()) {
			ContinueCommittedMove();
		} else {
			TimedRetreatAwayFrom(playerPos, kRetreatSpeed, kRetreatDuration, retreatMoveTimer_);
		}
	}

	riflemanOwner_.UpdateFirearmHybrid();

	FirearmMusketBayonet* firearm = riflemanOwner_.GetCurrentFirearm();
	bool reloadComplete = firearm && !firearm->IsReloading() && firearm->GetCurrentAmmo() > 0;

	if (CanMakeDecision() && (reloadComplete || retreatTimer_ >= kMaxRetreatDuration)) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
	}
}

void RiflemanBehavior::BehaviorMeleeInitialize() {
	meleePhase_ = MeleePhase::kAnticipate;
	meleeParameter_ = 0;
	lungeFacingLocked_ = false;

	OwnerVelocity().x = 0.0f;
	OwnerSetFirearmMode(3);

	riflemanOwner_.SetGunPose(EnemyRifleman::GunPose::kNeutral);
	riflemanOwner_.SetBayonetActive(false);
	riflemanOwner_.ResetBayonetHit();

	if (FirearmMusketBayonet* firearm = riflemanOwner_.GetCurrentFirearm()) {
		firearm->SetReloading(false);
		firearm->SetLungePose(FirearmMusketBayonet::LungePhase::kAnticipate, 0.0f);
	}
}

void RiflemanBehavior::BehaviorMeleeUpdate() {
	GameScene* scene = OwnerGameScene();
	Player* player = scene ? scene->GetPlayer() : nullptr;

	if (!player) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
		return;
	}

	const float distance = GetPlayerDistance();

	// player got far away: back to aiming, or chase if even farther
	if (distance > kRetreatRange) {
		OwnerVelocity().x = 0.0f;
		riflemanOwner_.SetBayonetActive(false);
		if (distance <= kFiringRange) {
			RequestBehavior(Behavior::kAiming);
		} else {
			RequestBehavior(Behavior::kRoot);
		}
		return;
	}

	++meleeParameter_;

	FirearmMusketBayonet* firearm = riflemanOwner_.GetCurrentFirearm();
	if (!firearm) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
		return;
	}

	if (!lungeFacingLocked_) {
		OwnerFaceDirection(player->GetWorldPosition().x < owner_.GetWorldPosition().x ? 1 : 0);
	}
	const bool facingRight = owner_.isRight();

	const float anticipateDuration = std::max(firearm->GetBayonetAnticipateDuration(), 1.0f);
	const float attackDuration = std::max(firearm->GetBayonetAttackDuration(), 1.0f);
	const float recoveryDuration = std::max(firearm->GetBayonetRecoveryDuration(), 1.0f);

	switch (meleePhase_) {
	case MeleePhase::kAnticipate: {
		OwnerVelocity().x = 0.0f;
		riflemanOwner_.SetBayonetActive(false);

		firearm->SetLungePose(FirearmMusketBayonet::LungePhase::kAnticipate, static_cast<float>(meleeParameter_) / anticipateDuration);

		if (static_cast<float>(meleeParameter_) >= anticipateDuration) {
			meleePhase_ = MeleePhase::kLunge;
			meleeParameter_ = 0;
			lungeFacingLocked_ = true; // lock facing when the thrust starts
		}
		break;
	}

	case MeleePhase::kLunge: {
		const float speed = firearm->GetBayonetAttackSpeed();
		OwnerVelocity().x = facingRight ? speed : -speed;
		riflemanOwner_.SetBayonetActive(true);

		firearm->SetLungePose(FirearmMusketBayonet::LungePhase::kAttack, static_cast<float>(meleeParameter_) / attackDuration);

		if (static_cast<float>(meleeParameter_) >= attackDuration) {
			meleePhase_ = MeleePhase::kRecovery;
			meleeParameter_ = 0;
			riflemanOwner_.SetBayonetActive(false);
		}
		break;
	}

	case MeleePhase::kRecovery:
	default: {
		OwnerVelocity().x = 0.0f;
		riflemanOwner_.SetBayonetActive(false);

		firearm->SetLungePose(FirearmMusketBayonet::LungePhase::kRecovery, static_cast<float>(meleeParameter_) / recoveryDuration);

		if (static_cast<float>(meleeParameter_) >= recoveryDuration) {
			firearm->ClearLungePose();
			RequestBehavior(Behavior::kRoot);
		}
		break;
	}
	}

	riflemanOwner_.UpdateFirearmHybrid();
}