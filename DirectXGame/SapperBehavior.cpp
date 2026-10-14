#include "SapperBehavior.h"
#include "EnemySapper.h"
#include "GameScene.h"
#include "GlobalVariables.h"
#include "Player.h"
#include "WeaponAxe.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "SapperBehavior";
}

void SapperBehavior::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "MeleeEngageRange", kMeleeEngageRange);
	gv->AddItem(kGroupName, "MeleeCooldown", kMeleeCooldown);
	gv->AddItem(kGroupName, "ComboResetDuration", kComboResetDuration);
	gv->AddItem(kGroupName, "AttackMoveSpeed", kAttackMoveSpeed);
	gv->AddItem(kGroupName, "LeapRiseSpeed", kLeapRiseSpeed);
	gv->AddItem(kGroupName, "LeapGravity", kLeapGravity);
	gv->AddItem(kGroupName, "LeapMaxHorizontalSpeed", kLeapMaxHorizontalSpeed);
	gv->AddItem(kGroupName, "LeapDiveSpeed", kLeapDiveSpeed);
	gv->AddItem(kGroupName, "LeapMinRange", kLeapMinRange);
	gv->AddItem(kGroupName, "LeapMaxRange", kLeapMaxRange);
	gv->AddItem(kGroupName, "LeapApexMargin", kLeapApexMargin);
	gv->AddItem(kGroupName, "LeapCooldown", kLeapCooldown);
	gv->AddItem(kGroupName, "LeapMaxDuration", kLeapMaxDuration);
	gv->AddItem(kGroupName, "NearPlayerDistance", kNearPlayerDistance);
	gv->AddItem(kGroupName, "LandStunDuration", kLandStunDuration);
	gv->AddItem(kGroupName, "LeapDiveEase", kLeapDiveEase);
}

void SapperBehavior::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kMeleeEngageRange = gv->GetFloatValue(kGroupName, "MeleeEngageRange");
	kMeleeCooldown = gv->GetFloatValue(kGroupName, "MeleeCooldown");
	kComboResetDuration = gv->GetFloatValue(kGroupName, "ComboResetDuration");
	kAttackMoveSpeed = gv->GetFloatValue(kGroupName, "AttackMoveSpeed");
	kLeapRiseSpeed = gv->GetFloatValue(kGroupName, "LeapRiseSpeed");
	kLeapGravity = std::max(gv->GetFloatValue(kGroupName, "LeapGravity"), 0.001f);
	kLeapMaxHorizontalSpeed = gv->GetFloatValue(kGroupName, "LeapMaxHorizontalSpeed");
	kLeapDiveSpeed = gv->GetFloatValue(kGroupName, "LeapDiveSpeed");
	kLeapMinRange = gv->GetFloatValue(kGroupName, "LeapMinRange");
	kLeapMaxRange = gv->GetFloatValue(kGroupName, "LeapMaxRange");
	kLeapApexMargin = gv->GetFloatValue(kGroupName, "LeapApexMargin");
	kLeapCooldown = gv->GetFloatValue(kGroupName, "LeapCooldown");
	kLeapMaxDuration = gv->GetFloatValue(kGroupName, "LeapMaxDuration");
	kNearPlayerDistance = gv->GetFloatValue(kGroupName, "NearPlayerDistance");
	kLandStunDuration = gv->GetFloatValue(kGroupName, "LandStunDuration");
	kLeapDiveEase = std::clamp(gv->GetFloatValue(kGroupName, "LeapDiveEase"), 0.01f, 1.0f);
}

SapperBehavior::SapperBehavior(EnemySapper& owner) : BaseEnemyBehavior(owner), sapperOwner_(owner) {}

void SapperBehavior::TickTimers() {
	constexpr float kDt = 1.0f / 60.0f;
	meleeCooldownTimer_ = std::max(0.0f, meleeCooldownTimer_ - kDt);
	leapCooldownTimer_ = std::max(0.0f, leapCooldownTimer_ - kDt);
}

void SapperBehavior::InitializeBehaviorState(Behavior behavior) {
	// ジャンプ以外の状態へ入る時は必ずマップ判定を戻す(ノックバック/スタン/死亡で判定無視のまま残らないように)
	if (behavior != Behavior::kLeap) {
		sapperOwner_.SetPhasing(false);
	}

	switch (behavior) {
	case Behavior::kMelee:
		BehaviorMeleeInitialize();
		return;
	case Behavior::kLeap:
		BehaviorLeapInitialize();
		return;
	default:
		break;
	}
	BaseEnemyBehavior::InitializeBehaviorState(behavior);
}

void SapperBehavior::UpdateBehaviorState() {
	TickTimers(); // どの状態でもクールダウンは進める

	switch (GetBehavior()) {
	case Behavior::kMelee:
		BehaviorMeleeUpdate();
		RunPhysicsAndCollisionUpdate();
		return;
	case Behavior::kLeap:
		BehaviorLeapUpdate();
		RunPhysicsAndCollisionUpdate();
		return;
	default:
		break;
	}
	BaseEnemyBehavior::UpdateBehaviorState();
}

// ------------------------------------------------------------------ Root
void SapperBehavior::BehaviorRootUpdate() {
	constexpr float kDt = 1.0f / 60.0f;

	noAttackTimer_ += kDt;
	if (noAttackTimer_ >= kComboResetDuration) {
		comboIndex_ = 0;
	}

	GameScene* scene = OwnerGameScene();
	Player* player = scene ? scene->GetPlayer() : nullptr;

	// アリーナ/全知の敵は視野から外れても戦い続ける
	const bool aware = IsPlayerSpotted() || HasPersistentAwareness();
	if (!player || !aware) {
		BaseEnemyBehavior::BehaviorRootUpdate(); // 最後に見た位置へ向かう
		return;
	}

	const Vector3 playerPos = player->GetWorldPosition();
	const Vector3 myPos = owner_.GetWorldPosition();
	const float dx = playerPos.x - myPos.x;
	const float dy = playerPos.y - myPos.y;
	const float distance = std::sqrt(dx * dx + dy * dy);

	// 1) 近距離: 斧のコンボ
	if (distance <= kMeleeEngageRange && IsPlayerOnSameFloor() && meleeCooldownTimer_ <= 0.0f) {
		RequestBehavior(Behavior::kMelee);
		return;
	}

	// 2) 届くならホーミングジャンプ
	if (leapCooldownTimer_ <= 0.0f && owner_.IsOnGround() && CanLeapTo(myPos, playerPos)) {
		RequestBehavior(Behavior::kLeap);
		return;
	}

	// 3) それ以外は通常の経路追跡
	BaseEnemyBehavior::BehaviorRootUpdate();
}

// ------------------------------------------------------------------ Melee
void SapperBehavior::BehaviorMeleeInitialize() {
	meleePhase_ = MeleePhase::kAnticipate;
	attackParameter_ = 0;
	if (WeaponAxe* weapon = sapperOwner_.GetCurrentMelee()) {
		weapon->ResetSwingHits();
	}
}

// 攻撃中も止まらない: 向いている方向へ歩き続ける(プレイヤーに重なる手前では止まる)
void SapperBehavior::AdvanceWhileAttacking() {
	GameScene* scene = OwnerGameScene();
	Player* player = scene ? scene->GetPlayer() : nullptr;
	if (!player) {
		OwnerVelocity().x = 0.0f;
		return;
	}
	const float dx = player->GetWorldPosition().x - owner_.GetWorldPosition().x;
	if (std::abs(dx) <= kMeleeAdvanceStopGap) {
		OwnerVelocity().x = 0.0f;
		return;
	}
	const int dir = owner_.isRight() ? static_cast<int>(BaseEnemy::LRDirection::kRight) : static_cast<int>(BaseEnemy::LRDirection::kLeft);
	MoveAlongGround(dir, kAttackMoveSpeed, MoveKind::kApproach);
}

void SapperBehavior::BehaviorMeleeUpdate() {
	++attackParameter_;
	noAttackTimer_ = 0.0f;

	WeaponAxe* weapon = sapperOwner_.GetCurrentMelee();
	if (!weapon) {
		OwnerVelocity().x = 0.0f;
		RequestBehavior(Behavior::kRoot);
		return;
	}

	// 振りかぶり中だけプレイヤーを追って向きを合わせる。振り始めたら向きは固定
	if (meleePhase_ == MeleePhase::kAnticipate) {
		FacePlayer();
	}
	AdvanceWhileAttacking();

	BaseMelee::GenericAttackAnimState animState;
	animState.comboIndex = comboIndex_;

	switch (meleePhase_) {
	case MeleePhase::kAnticipate: {
		const float duration = weapon->GetComboAnticipateDuration(comboIndex_);
		animState.phase = BaseMelee::AttackPhase::kAnticipate;
		animState.t = std::clamp(static_cast<float>(attackParameter_) / duration, 0.0f, 1.0f);
		if (static_cast<float>(attackParameter_) >= duration) {
			meleePhase_ = MeleePhase::kAttack;
			attackParameter_ = 0;
			weapon->PlaySwingSound(comboIndex_);
		}
		break;
	}
	case MeleePhase::kAttack: {
		const float duration = weapon->GetComboAttackDuration(comboIndex_);
		animState.phase = BaseMelee::AttackPhase::kAttack;
		animState.t = std::clamp(static_cast<float>(attackParameter_) / duration, 0.0f, 1.0f);
		if (static_cast<float>(attackParameter_) >= duration) {
			meleePhase_ = MeleePhase::kRecovery;
			attackParameter_ = 0;
		}
		break;
	}
	case MeleePhase::kRecovery:
	default: {
		const float duration = weapon->GetComboRecoveryDuration(comboIndex_);
		animState.phase = BaseMelee::AttackPhase::kRecovery;
		animState.t = std::clamp(static_cast<float>(attackParameter_) / duration, 0.0f, 1.0f);
		if (static_cast<float>(attackParameter_) >= duration) {
			comboIndex_ = (comboIndex_ + 1) % weapon->GetMaxCombo();
			noAttackTimer_ = 0.0f;
			meleeCooldownTimer_ = kMeleeCooldown;
			ClearMoveCommit(); // 攻撃中のホップ状態を引きずらない
			RequestBehavior(Behavior::kRoot);
		}
		break;
	}
	}

	sapperOwner_.UpdateMeleeVisual(&animState);
}

// ------------------------------------------------------------------ Leap (ホーミングジャンプ)
bool SapperBehavior::CanLeapTo(const Vector3& from, const Vector3& to) const {
	const float dx = to.x - from.x;
	const float dy = to.y - from.y;
	const float distance = std::sqrt(dx * dx + dy * dy);
	if (distance < kLeapMinRange || distance > kLeapMaxRange) {
		return false;
	}
	// 頂点の高さ(v^2 / 2g)より高い所へは届かない
	const float apexHeight = (kLeapRiseSpeed * kLeapRiseSpeed) / (2.0f * kLeapGravity);
	return dy <= apexHeight - kLeapApexMargin;
}

void SapperBehavior::BehaviorLeapInitialize() {
	GameScene* scene = OwnerGameScene();
	Player* player = scene ? scene->GetPlayer() : nullptr;
	const Vector3 myPos = owner_.GetWorldPosition();

	// 1. この瞬間のプレイヤーの位置から水平速度を決める
	leapTarget_ = player ? player->GetWorldPosition() : myPos;
	sapperOwner_.SetLeapTarget(leapTarget_);
	const float distanceX = leapTarget_.x - myPos.x;

	// 2. 総飛翔時間: 初速と重力から、元の高さへ戻るまでのフレーム数 t = 2v / g
	const float flightTime = 2.0f * kLeapRiseSpeed / kLeapGravity;

	// 3. 必要な水平速度 = 距離 / 時間
	const float requiredVelocityX = std::clamp(distanceX / flightTime, -kLeapMaxHorizontalSpeed, kLeapMaxHorizontalSpeed);

	Vector3& velocity = OwnerVelocity();
	velocity.x = requiredVelocityX;
	velocity.y = kLeapRiseSpeed;
	velocity.z = 0.0f;

	leapPhase_ = LeapPhase::kRise;
	leapTime_ = 0.0f;
	diveStartDistance_ = 1.0f;
	leapAnticipateTime_ = std::max((kLeapRiseSpeed / kLeapGravity) / 60.0f, 0.01f);

	if (WeaponAxe* weapon = sapperOwner_.GetCurrentMelee()) {
		weapon->ResetSwingHits();
	}
	// 上昇から判定を無視する(壁・天井を抜けられる)
	sapperOwner_.SetPhasing(true);
}

void SapperBehavior::BehaviorLeapUpdate() {
	constexpr float kDt = 1.0f / 60.0f;
	leapTime_ += kDt;

	GameScene* scene = OwnerGameScene();
	Player* player = scene ? scene->GetPlayer() : nullptr;
	const Vector3 pos = owner_.GetWorldPosition();

	// 安全弁: 長引いたら目標へ強制的に降ろす
	if (leapTime_ >= kLeapMaxDuration) {
		sapperOwner_.SnapTo(leapTarget_);
		FinishLeap();
		return;
	}

	switch (leapPhase_) {
	case LeapPhase::kRise: {
		// 判定無視中は重力もBaseEnemy側で掛からないので、上昇中の重力はここで掛ける
		Vector3& velocity = OwnerVelocity();
		velocity.y -= kLeapGravity;
		FacePlayer();
		if (velocity.y <= 0.0f) {
			// 頂点: 止めずにそのまま急降下フェーズへ。速度は下のイージングで滑らかに目標方向へ向かう
			leapPhase_ = LeapPhase::kDive;
			if (player) {
				leapTarget_ = player->GetWorldPosition();
				sapperOwner_.SetLeapTarget(leapTarget_);
			}
			const float tx = leapTarget_.x - pos.x;
			const float ty = leapTarget_.y - pos.y;
			diveStartDistance_ = std::max(std::sqrt(tx * tx + ty * ty), 0.01f);
			if (WeaponAxe* weapon = sapperOwner_.GetCurrentMelee()) {
				weapon->ResetSwingHits();
			}
		}
		sapperOwner_.UpdateSmashVisual(BaseMelee::AttackPhase::kAnticipate, std::clamp(leapTime_ / leapAnticipateTime_, 0.0f, 1.0f));
		break;
	}
	case LeapPhase::kDive: {
		// 目標は常に現在のプレイヤー位置(降下しながらプレイヤーへ寄っていく)
		if (player) {
			leapTarget_ = player->GetWorldPosition();
			sapperOwner_.SetLeapTarget(leapTarget_);
		}
		const float tx = leapTarget_.x - pos.x;
		const float ty = leapTarget_.y - pos.y;
		const float distance = std::sqrt(tx * tx + ty * ty);

		// プレイヤーの近くまで来て、体が地形に埋まっていなければ判定を戻す
		bool nearPlayer = false;
		if (player) {
			const Vector3 pp = player->GetWorldPosition();
			const float px = pp.x - pos.x;
			const float py = pp.y - pos.y;
			nearPlayer = std::sqrt(px * px + py * py) <= kNearPlayerDistance && !sapperOwner_.IsOverlappingTerrain();
		}
		// 1フレームで届く距離なら目標位置へ吸着して終了
		const bool arrived = distance <= kLeapDiveSpeed;

		sapperOwner_.UpdateSmashVisual(BaseMelee::AttackPhase::kAttack, std::clamp(1.0f - distance / diveStartDistance_, 0.0f, 1.0f));

		if (arrived) {
			sapperOwner_.SnapTo(leapTarget_);
			FinishLeap();
			return;
		}
		if (nearPlayer) {
			FinishLeap();
			return;
		}

		// 現在の速度から、プレイヤーへ向かう降下速度へ滑らかに近づける(止まらない)
		const float inv = (distance > 0.0001f) ? kLeapDiveSpeed / distance : 0.0f;
		const Vector3 desired = {tx * inv, ty * inv, 0.0f};
		Vector3& velocity = OwnerVelocity();
		velocity = Lerp(velocity, desired, kLeapDiveEase);
		break;
	}
	}
}

void SapperBehavior::FinishLeap() {
	sapperOwner_.SetPhasing(false); // 判定を戻す(万一埋まっていたら目標位置へ戻す)
	OwnerVelocity() = {0.0f, 0.0f, 0.0f};
	leapCooldownTimer_ = kLeapCooldown;

	// 叩きつけの着弾演出
	if (GameScene* scene = OwnerGameScene()) {
		Vector3 feet = owner_.GetWorldPosition() + Vector3(0.0f, -BaseEnemy::GetBodyHeight() * 0.5f, 0.0f);
		scene->CreateBurstEffect(BurstType::kLand, feet, {0.0f, 1.0f});
		scene->CreateBurstEffect(BurstType::kHit, feet, {0.0f, 1.0f});
	}

	// 自分でスタン(+6秒のボーナスは外部要因のスタンだけなので、ここは直接指定する)
	stunDuration_ = kLandStunDuration;
	behaviorRequest_ = Behavior::kStunned;
}