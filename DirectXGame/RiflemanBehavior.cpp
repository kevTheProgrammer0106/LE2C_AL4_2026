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
	riflemanOwner_.SetGunPose(EnemyRifleman::GunPose::kNeutral);
	if (FirearmMusketBayonet* firearm = riflemanOwner_.GetCurrentFirearm()) {
		firearm->SetReloading(false); // back to the normal holster pose
	}
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

	// プレイヤーを発見した後は自分から接近せず、その場で停止する。
	// 射撃手としてプレイヤーとの距離を維持する。
	OwnerVelocity().x = 0.0f;

	// 移動せず、その場でプレイヤーの方向だけを向く。
	OwnerFaceDirection(player->GetWorldPosition().x < owner_.GetWorldPosition().x ? 1 : 0);

	if (!IsPlayerOnSameFloor())
		return;

	Vector3 diff = player->GetWorldPosition() - owner_.GetWorldPosition();

	float distance = std::sqrtf(diff.x * diff.x + diff.y * diff.y);

	// 至近距離では必ず銃剣へ切り替える。
	if (distance <= kMeleeEngageRange) {
		RequestBehavior(Behavior::kMelee);
		return;
	}

	// 射撃可能距離なら射撃か銃剣を抽選する。
	if (distance <= kFiringRange) {
		RequestBehavior(Behavior::kAiming);
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
	if (!IsPlayerSpotted() || !IsPlayerOnSameFloor()) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
		return;
	}

	Vector3 diff = player->GetWorldPosition() - owner_.GetWorldPosition();

	float distance = std::sqrtf(diff.x * diff.x + diff.y * diff.y);

	// 狙っている途中で近づかれたら射撃せず銃剣へ切り替える。
	if (distance <= kMeleeEngageRange) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kMelee);
		return;
	}

	// 射程外まで離れたらRootへ戻して再判定する。
	if (distance > kFiringRange) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
		return;
	}

	OwnerVelocity().x = 0.0f;

	// 射撃中も常にプレイヤーの方向を向く。
	OwnerFaceDirection(player->GetWorldPosition().x < owner_.GetWorldPosition().x ? 1 : 0);

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
		// 銃剣突撃へ入るため、銃器側のリロード状態を解除する。
		firearm->SetReloading(false);
	}
}

void RiflemanBehavior::BehaviorMeleeUpdate() {
	++meleeParameter_;

	FirearmMusketBayonet* firearm = riflemanOwner_.GetCurrentFirearm();

	if (!firearm) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
		return;
	}

	// ランジ開始まではプレイヤーを追従し、攻撃開始時に向きを固定する。
	if (!lungeFacingLocked_) {
		if (GameScene* scene = OwnerGameScene()) {
			if (Player* player = scene->GetPlayer()) {
				OwnerFaceDirection(player->GetWorldPosition().x < owner_.GetWorldPosition().x ? 1 : 0);
			}
		}
	}

	bool facingRight = owner_.isRight();

	const float anticipateDuration = firearm->GetBayonetAnticipateDuration();

	const float attackDuration = firearm->GetBayonetAttackDuration();

	const float recoveryDuration = firearm->GetBayonetRecoveryDuration();

	switch (meleePhase_) {
	case MeleePhase::kAnticipate:
		OwnerVelocity().x = 0.0f;
		riflemanOwner_.SetBayonetActive(false);

		if (static_cast<float>(meleeParameter_) >= anticipateDuration) {

			meleePhase_ = MeleePhase::kLunge;
			meleeParameter_ = 0;

			// 攻撃開始時に初めて向きを固定する。
			lungeFacingLocked_ = true;
		}
		break;

	case MeleePhase::kLunge: {
		float speed = firearm->GetBayonetAttackSpeed();

		OwnerVelocity().x = facingRight ? speed : -speed;

		riflemanOwner_.SetBayonetActive(true);

		if (static_cast<float>(meleeParameter_) >= attackDuration) {

			meleePhase_ = MeleePhase::kRecovery;
			meleeParameter_ = 0;

			riflemanOwner_.SetBayonetActive(false);
		}
		break;
	}

	case MeleePhase::kRecovery:
	default:
		OwnerVelocity().x = 0.0f;
		riflemanOwner_.SetBayonetActive(false);

		if (static_cast<float>(meleeParameter_) >= recoveryDuration) {

			RequestBehavior(Behavior::kRoot);
		}
		break;
	}

	riflemanOwner_.UpdateFirearmHybrid();
}