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

float RiflemanBehavior::GetPlayerDistance() const {
	GameScene* scene = OwnerGameScene();
	if (!scene || !scene->GetPlayer()) {
		return FLT_MAX;
	}

	const Vector3 playerPosition = scene->GetPlayer()->GetWorldPosition();
	const Vector3 enemyPosition = owner_.GetWorldPosition();

	const float dx = playerPosition.x - enemyPosition.x;
	const float dy = playerPosition.y - enemyPosition.y;

	return std::sqrtf(dx * dx + dy * dy);
}
bool RiflemanBehavior::TryGetPlayerDistance(float& outDistance) const {
	GameScene* scene = OwnerGameScene();
	if (!scene || !scene->GetPlayer()) {
		outDistance = 0.0f;
		return false;
	}

	const Vector3 playerPosition = scene->GetPlayer()->GetWorldPosition();

	const Vector3 enemyPosition = owner_.GetWorldPosition();

	const float dx = playerPosition.x - enemyPosition.x;
	const float dy = playerPosition.y - enemyPosition.y;

	outDistance = std::sqrtf(dx * dx + dy * dy);
	return true;
}
float RiflemanBehavior::CalculateRetreatSpeed(float distance) const {
	// RetreatRangeからBayonetRangeへ近づくほど後退速度を上げる。
	const float retreatSpan = kRetreatRange - kBayonetRange;

	if (retreatSpan <= 0.0f) {
		return kRetreatSpeed;
	}

	const float retreatAmount = std::clamp((kRetreatRange - distance) / retreatSpan, 0.0f, 1.0f);

	return kRetreatSpeed * retreatAmount;
}

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
	// [FIX] Do NOT gate on IsPlayerOnSameFloor() here: a ranged enemy must engage even while the
	// player is jumping / on another height. (The check made the rifleman stand idle mid-jump.)

	// 移動せず、その場でプレイヤーの方向だけを向く。
	OwnerFaceDirection(player->GetWorldPosition().x < owner_.GetWorldPosition().x ? 1 : 0);

	const float distance = GetPlayerDistance();

	// 最も近い距離では必ず銃剣へ移行する。
	// [FIX] bayonet only when the player is on the same floor; an airborne player gets shot instead.
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

	OwnerVelocity().x = 0.0f;
	OwnerSetFirearmMode(1);

	riflemanOwner_.SetGunPose(EnemyRifleman::GunPose::kAiming);
	if (FirearmMusketBayonet* firearm = riflemanOwner_.GetCurrentFirearm()) {
		firearm->SetReloading(false);
	}
}

void RiflemanBehavior::BehaviorAimingUpdate() {
	GameScene* scene = OwnerGameScene();
	Player* player = scene ? scene->GetPlayer() : nullptr;

	if (!player) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
		return;
	}

	// 狙っている最中も、プレイヤーが射撃条件を満たしているか再確認する。
	// [FIX] only line of sight cancels the aim. Player height / jumping no longer does.
	if (!IsPlayerSpotted()) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
		return;
	}

	const float distance = GetPlayerDistance();

	// 射撃可能距離より外へ逃げられたら、再び追跡へ戻る。
	if (distance > kFiringRange) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
		return;
	}

	// 至近距離なら射撃をやめて銃剣へ切り替える。
	if (distance <= kBayonetRange && IsPlayerOnSameFloor()) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kMelee);
		return;
	}

	// [FIX] Facing while aiming is decided by UpdateFirearmHybrid() via ComputeAimFacing (with hysteresis).
	// Facing by raw dx here fought with it and made the rifleman flip every frame when the player was above/below.

	if (distance < kRetreatRange) {
		// RetreatRangeを切ると徐々に後退を開始する。
		const bool playerOnRight = player->GetWorldPosition().x >= owner_.GetWorldPosition().x;

		const int retreatDirection = playerOnRight ? static_cast<int>(BaseEnemy::LRDirection::kLeft) : static_cast<int>(BaseEnemy::LRDirection::kRight);

		const float retreatSpeed = CalculateRetreatSpeed(distance);

		OwnerVelocity().x = retreatDirection == static_cast<int>(BaseEnemy::LRDirection::kLeft) ? -retreatSpeed : retreatSpeed;
	} else {
		// 十分離れている間はその場に止まって射撃する。
		OwnerVelocity().x = 0.0f;
	}

	riflemanOwner_.UpdateFirearmHybrid();

	aimTimer_ += 1.0f / 60.0f;

	if (aimTimer_ >= kAimDuration) {
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
		float playerX = player->GetWorldPosition().x;
		float enemyX = owner_.GetWorldPosition().x;

		bool playerOnRight = playerX >= enemyX;

		// プレイヤーを向いたまま、プレイヤーと反対方向へ後退する。
		int facingDirection = playerOnRight ? static_cast<int>(BaseEnemy::LRDirection::kRight) : static_cast<int>(BaseEnemy::LRDirection::kLeft);

		int retreatDirection = playerOnRight ? static_cast<int>(BaseEnemy::LRDirection::kLeft) : static_cast<int>(BaseEnemy::LRDirection::kRight);

		OwnerFaceDirection(facingDirection);

		// リロード開始直後の短時間だけ後退する。
		if (retreatMoveTimer_ < kRetreatDuration) {
			float gapDistance = 0.0f;

			bool hasGap = ScanGapAhead(retreatDirection, kRetreatLookAheadDistance, gapDistance);

			float wallDistance = 0.0f;
			float wallHeight = 0.0f;

			bool hasWall = ScanWallAhead(retreatDirection, kRetreatLookAheadDistance, owner_.GetBodyHeight(), wallDistance, wallHeight);

			// 後方に崖や壁がある場合は、それ以上下がらない。
			if (!hasGap && !hasWall) {
				OwnerVelocity().x = (retreatDirection == static_cast<int>(BaseEnemy::LRDirection::kLeft)) ? -kRetreatSpeed : kRetreatSpeed;

				retreatMoveTimer_ += kDeltaTime;
			}
		}
	}

	riflemanOwner_.UpdateFirearmHybrid();

	FirearmMusketBayonet* firearm = riflemanOwner_.GetCurrentFirearm();

	bool reloadComplete = firearm && !firearm->IsReloading() && firearm->GetCurrentAmmo() > 0;

	// リロード完了後の判断は今はRootに戻すだけにして、後で調整する。
	if (reloadComplete || retreatTimer_ >= kMaxRetreatDuration) {
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