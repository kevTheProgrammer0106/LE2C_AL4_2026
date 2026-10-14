#include "ShotgunnerBehavior.h"
#include "EnemyShotgunner.h"
#include "FirearmShotgun.h"
#include "GameScene.h"
#include "GlobalVariables.h"
#include "Player.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "ShotgunnerBehavior";
}

void ShotgunnerBehavior::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "AimDuration", kAimDuration);
	gv->AddItem(kGroupName, "ReacquireAimDuration", kReacquireAimDuration);
	gv->AddItem(kGroupName, "LostSightChaseDelay", kLostSightChaseDelay);
	gv->AddItem(kGroupName, "AimTrackSpeed", kAimTrackSpeed);
	gv->AddItem(kGroupName, "RetreatSpeed", kRetreatSpeed);
	gv->AddItem(kGroupName, "RetreatDuration", kRetreatDuration);
	gv->AddItem(kGroupName, "MaxReloadDuration", kMaxReloadDuration);
	gv->AddItem(kGroupName, "CloseRange", kCloseRange);
	gv->AddItem(kGroupName, "FiringRange", kFiringRange);
	gv->AddItem(kGroupName, "ApproachSpeed", kApproachSpeed);
	gv->AddItem(kGroupName, "ApproachStopDistance", kApproachStopDistance);
}

void ShotgunnerBehavior::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kAimDuration = gv->GetFloatValue(kGroupName, "AimDuration");
	kReacquireAimDuration = gv->GetFloatValue(kGroupName, "ReacquireAimDuration");
	kLostSightChaseDelay = gv->GetFloatValue(kGroupName, "LostSightChaseDelay");
	kAimTrackSpeed = gv->GetFloatValue(kGroupName, "AimTrackSpeed");
	kRetreatSpeed = gv->GetFloatValue(kGroupName, "RetreatSpeed");
	kRetreatDuration = gv->GetFloatValue(kGroupName, "RetreatDuration");
	kMaxReloadDuration = gv->GetFloatValue(kGroupName, "MaxReloadDuration");
	kCloseRange = gv->GetFloatValue(kGroupName, "CloseRange");
	kFiringRange = gv->GetFloatValue(kGroupName, "FiringRange");
	kApproachSpeed = gv->GetFloatValue(kGroupName, "ApproachSpeed");
	kApproachStopDistance = gv->GetFloatValue(kGroupName, "ApproachStopDistance");
}

ShotgunnerBehavior::ShotgunnerBehavior(EnemyShotgunner& owner) : BaseEnemyBehavior(owner), shotgunnerOwner_(owner) {}

void ShotgunnerBehavior::InitializeBehaviorState(Behavior behavior) {
	switch (behavior) {
	case Behavior::kAiming:
		BehaviorAimingInitialize();
		return;
	case Behavior::kReloading:
		BehaviorReloadingInitialize();
		return;
	case Behavior::kIdle:
	case Behavior::kKnockback:
	case Behavior::kStunned:
	case Behavior::kDeath:
		// 交戦状態から抜ける時は銃を構え解除に戻す(リロード自体はBaseFirearm側で進み続ける)
		OwnerSetFirearmMode(0);
		shotgunnerOwner_.SetGunPose(EnemyShotgunner::GunPose::kNeutral);
		if (FirearmShotgun* firearm = shotgunnerOwner_.GetCurrentFirearm()) {
			firearm->SetReloading(false);
		}
		break;
	default:
		break;
	}
	BaseEnemyBehavior::InitializeBehaviorState(behavior);
}

void ShotgunnerBehavior::UpdateBehaviorState() {
	switch (GetBehavior()) {
	case Behavior::kAiming:
		BehaviorAimingUpdate();
		RunPhysicsAndCollisionUpdate();
		return;
	case Behavior::kReloading:
		BehaviorReloadingUpdate();
		RunPhysicsAndCollisionUpdate();
		return;
	default:
		break;
	}
	BaseEnemyBehavior::UpdateBehaviorState();
}

bool ShotgunnerBehavior::TryGetPlayerDistance(float& outDistance) const {
	GameScene* scene = OwnerGameScene();
	if (!scene || !scene->GetPlayer()) {
		return false;
	}
	const Vector3 p = scene->GetPlayer()->GetWorldPosition();
	const Vector3 e = owner_.GetWorldPosition();
	const float dx = p.x - e.x;
	const float dy = p.y - e.y;
	outDistance = std::sqrt(dx * dx + dy * dy);
	return true;
}

float ShotgunnerBehavior::GetPlayerDistance() const {
	float d = 0.0f;
	TryGetPlayerDistance(d);
	return d;
}

// ------------------------------------------------------------------ Root
void ShotgunnerBehavior::BehaviorRootInitialize() {
	BaseEnemyBehavior::BehaviorRootInitialize();

	OwnerSetFirearmMode(0);
	shotgunnerOwner_.SetGunPose(EnemyShotgunner::GunPose::kNeutral);
	if (FirearmShotgun* firearm = shotgunnerOwner_.GetCurrentFirearm()) {
		firearm->SetReloading(false);
	}
}

void ShotgunnerBehavior::BehaviorRootUpdate() {
	GameScene* scene = OwnerGameScene();
	if (!scene || !scene->GetPlayer()) {
		BaseEnemyBehavior::BehaviorRootUpdate();
		return;
	}
	Player* player = scene->GetPlayer();

	// 未発見の間だけ通常の追跡AIを動かす
	if (!IsPlayerSpotted()) {
		BaseEnemyBehavior::BehaviorRootUpdate();
		return;
	}

	// 発見後は追いかけず、その場でプレイヤーの方を向く
	OwnerFaceDirection(player->GetWorldPosition().x < owner_.GetWorldPosition().x ? 1 : 0);
	if (!CanMakeDecision()) {
		ContinueCommittedMove();
		return;
	}
	OwnerVelocity().x = 0.0f;

	const float distance = GetPlayerDistance();
	if (distance <= kFiringRange) {
		// 未装填(長いリロード中)なら照準せずリロード状態へ。装填済みなら照準へ。
		FirearmShotgun* firearm = shotgunnerOwner_.GetCurrentFirearm();
		const bool loaded = firearm && firearm->CanFire();
		RequestBehavior(loaded ? Behavior::kAiming : Behavior::kReloading);
	}
}

// ------------------------------------------------------------------ Aiming
void ShotgunnerBehavior::BehaviorAimingInitialize() {
	aimTimer_ = 0.0f;
	currentAimDuration_ = kAimDuration; // 初回発見時はフル照準時間

	OwnerVelocity().x = 0.0f;
	OwnerSetFirearmMode(1);

	GameScene* scene = OwnerGameScene();
	if (scene && scene->GetPlayer()) {
		aimTracker_.Reset(owner_.GetModelWorldPosition(), scene->GetPlayer()->GetWorldPosition());
	}

	shotgunnerOwner_.SetGunPose(EnemyShotgunner::GunPose::kAiming);
	if (FirearmShotgun* firearm = shotgunnerOwner_.GetCurrentFirearm()) {
		firearm->SetReloading(false);
	}
}

void ShotgunnerBehavior::BehaviorAimingUpdate() {
	constexpr float kDeltaTime = 1.0f / 60.0f;

	GameScene* scene = OwnerGameScene();
	Player* player = scene ? scene->GetPlayer() : nullptr;

	if (!player) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
		return;
	}

	// 照準中に弾が無くなっていたらリロードへ(ノックバック等で撃てないまま戻ってきた場合)
	FirearmShotgun* firearm = shotgunnerOwner_.GetCurrentFirearm();
	if (!firearm || !firearm->CanFire()) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kReloading);
		return;
	}

	const bool spotted = IsPlayerSpotted();
	aimTracker_.Update(spotted, owner_.GetModelWorldPosition(), player->GetWorldPosition(), kAimTrackSpeed, kDeltaTime);

	// 見失った後にもう一度プレイヤーが見えた: 残り照準時間をkReacquireAimDurationまでに切り詰める。
	// 既定は0なので、照準タイマーは即座に満了して撃つ。
	if (aimTracker_.ConsumeReacquired()) {
		currentAimDuration_ = std::min(currentAimDuration_, aimTimer_ + kReacquireAimDuration);
	}

	if (IsMoveCommitted()) {
		ContinueCommittedMove();
		shotgunnerOwner_.UpdateFirearmHybrid();
		return;
	}

	// 見失い中: 最後に見た位置へ銃を向けたまま少しずつ近づく。発砲せず、照準タイマーも止める
	if (!spotted) {
		if (aimTracker_.GetLostTime() >= kLostSightChaseDelay) {
			OwnerVelocity().x = 0.0f;
			RequestBehavior(Behavior::kRoot); // 最後に見た位置を追う
			return;
		}
		ApproachPosition(aimTracker_.GetLastSeenPos(), kApproachSpeed, kApproachStopDistance);
		shotgunnerOwner_.UpdateFirearmHybrid();
		return;
	}

	const float distance = GetPlayerDistance();
	if (distance > kFiringRange) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
		return;
	}

	// プレイヤーが近い間は、照準中でも常に後退する
	if (distance < kCloseRange) {
		RetreatAwayFrom(player->GetWorldPosition(), kRetreatSpeed);
	} else {
		OwnerVelocity().x = 0.0f;
	}

	shotgunnerOwner_.UpdateFirearmHybrid();

	aimTimer_ += kDeltaTime;
	if (aimTimer_ >= currentAimDuration_) {
		shotgunnerOwner_.ShootFirearm();
		shotgunnerOwner_.SetGunPose(EnemyShotgunner::GunPose::kFiring);
		RequestBehavior(Behavior::kReloading);

		// recoil: pushed away from the player. No extra hit-stun for a self-inflicted recoil.
		if (FirearmShotgun* gun = shotgunnerOwner_.GetCurrentFirearm()) {
			const float kb = gun->GetFireKnockbackPower();
			if (kb > 0.0f) {
				const float away = (player->GetWorldPosition().x < owner_.GetWorldPosition().x) ? 1.0f : -1.0f;
				RequestKnockback({away, 0.1f, 0.0f}, kb);
				pendingStunAfterKnockback_ = 0.0f;
			}
		}
	}
}

// ------------------------------------------------------------------ Reloading
void ShotgunnerBehavior::BehaviorReloadingInitialize() {
	retreatTimer_ = 0.0f;
	retreatMoveTimer_ = 0.0f;

	OwnerSetFirearmMode(2);
	shotgunnerOwner_.SetGunPose(EnemyShotgunner::GunPose::kReloading);
	if (FirearmShotgun* firearm = shotgunnerOwner_.GetCurrentFirearm()) {
		firearm->SetReloading(true);
	}
}

void ShotgunnerBehavior::BehaviorReloadingUpdate() {
	constexpr float kDeltaTime = 1.0f / 60.0f;

	retreatTimer_ += kDeltaTime;
	OwnerVelocity().x = 0.0f;

	GameScene* scene = OwnerGameScene();
	Player* player = scene ? scene->GetPlayer() : nullptr;

	if (player) {
		const Vector3 playerPos = player->GetWorldPosition();
		const bool playerOnRight = playerPos.x >= owner_.GetWorldPosition().x;

		// 後退中もプレイヤーの方を向き続ける
		OwnerFaceDirection(playerOnRight ? static_cast<int>(BaseEnemy::LRDirection::kRight) : static_cast<int>(BaseEnemy::LRDirection::kLeft));

		if (IsMoveCommitted()) {
			ContinueCommittedMove();
		} else if (GetPlayerDistance() < kCloseRange) {
			// プレイヤーが近い間は、リロード中ずっと後退し続ける
			RetreatAwayFrom(playerPos, kRetreatSpeed);
		} else {
			// 遠い時は、発砲直後の短時間だけ下がる
			TimedRetreatAwayFrom(playerPos, kRetreatSpeed, kRetreatDuration, retreatMoveTimer_);
		}
	}

	shotgunnerOwner_.UpdateFirearmHybrid();

	// 装填が終わるまでは抜けない(安全弁のkMaxReloadDurationだけ例外)
	FirearmShotgun* firearm = shotgunnerOwner_.GetCurrentFirearm();
	const bool reloadComplete = firearm && !firearm->IsReloading() && firearm->GetCurrentAmmo() > 0;

	if (CanMakeDecision() && (reloadComplete || retreatTimer_ >= kMaxReloadDuration)) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
	}
}