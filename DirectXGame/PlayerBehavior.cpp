#include "PlayerBehavior.h"
#include "BaseMelee.h"
#include "GlobalVariables.h"
#include "Player.h"
#include "ObjectBounce.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "PlayerBehavior";
}

void PlayerBehavior::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "DeathDuration", static_cast<int32_t>(kDeathDuration));
	gv->AddItem(kGroupName, "DeathMaxBounces", kDeathMaxBounces);
	gv->AddItem(kGroupName, "DeathBounceDamping", kDeathBounceDamping);
	gv->AddItem(kGroupName, "DeathFriction", kDeathFriction);
	gv->AddItem(kGroupName, "DeathMaxFallSpeed", kDeathMaxFallSpeed);
	gv->AddItem(kGroupName, "DeathFlingSpeedScale", kDeathFlingSpeedScale);
	gv->AddItem(kGroupName, "DeathFlingUpBoost", kDeathFlingUpBoost);
	gv->AddItem(kGroupName, "HitKnockbackSpeedScale", kHitKnockbackSpeedScale);
	gv->AddItem(kGroupName, "KnockBackSpeedX", kKnockBackSpeedX);
	gv->AddItem(kGroupName, "KnockBackSpeedY", kKnockBackSpeedY);
	gv->AddItem(kGroupName, "KnockBackDuration", static_cast<int32_t>(kKnockBackDuration));
	gv->AddItem(kGroupName, "HitEffectOffset", kHitEffectOffset);
	gv->AddItem(kGroupName, "HitKnockbackHopSpeed", kHitKnockbackHopSpeed); // [NEW]
	gv->AddItem(kGroupName, "StunHopSpeed", kStunHopSpeed);                 // [NEW]
}
void PlayerBehavior::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kDeathDuration = static_cast<uint32_t>(gv->GetIntValue(kGroupName, "DeathDuration"));
	kDeathMaxBounces = gv->GetIntValue(kGroupName, "DeathMaxBounces");
	kDeathBounceDamping = gv->GetFloatValue(kGroupName, "DeathBounceDamping");
	kDeathFriction = gv->GetFloatValue(kGroupName, "DeathFriction");
	kDeathMaxFallSpeed = gv->GetFloatValue(kGroupName, "DeathMaxFallSpeed");
	kDeathFlingSpeedScale = gv->GetFloatValue(kGroupName, "DeathFlingSpeedScale");
	kDeathFlingUpBoost = gv->GetFloatValue(kGroupName, "DeathFlingUpBoost");
	kHitKnockbackSpeedScale = gv->GetFloatValue(kGroupName, "HitKnockbackSpeedScale");
	kKnockBackSpeedX = gv->GetFloatValue(kGroupName, "KnockBackSpeedX");
	kKnockBackSpeedY = gv->GetFloatValue(kGroupName, "KnockBackSpeedY");
	kKnockBackDuration = static_cast<uint32_t>(gv->GetIntValue(kGroupName, "KnockBackDuration"));
	kHitEffectOffset = gv->GetFloatValue(kGroupName, "HitEffectOffset");
	kHitKnockbackHopSpeed = gv->GetFloatValue(kGroupName, "HitKnockbackHopSpeed"); // [NEW]
	kStunHopSpeed = gv->GetFloatValue(kGroupName, "StunHopSpeed");                 // [NEW]
}

void PlayerBehavior::ResolveBehaviorRequest() {
	if (behaviorRequest_ == Behavior::kUnknown)
		return;
	behavior_ = behaviorRequest_;
	switch (behavior_) {
	case Behavior::kRoot:
	default:
		BehaviorRootInitialize();
		if (owner_.GetCurrentMelee())
			owner_.GetCurrentMelee()->SetBehavior(BaseMelee::MeleeBehavior::kIdle);
		break;
	case Behavior::kAttack:
		BehaviorAttackInitialize();
		if (owner_.GetCurrentMelee())
			owner_.GetCurrentMelee()->SetBehavior(BaseMelee::MeleeBehavior::kAttacking);
		break;
	case Behavior::kKnockback:
		BehaviorKnockbackInitialize();
		break;
	case Behavior::kStunned:
		BehaviorStunnedInitialize();
		break;
	case Behavior::kBlock:
		BehaviorBlockInitialize();
		break;
	case Behavior::kDeath:
		BehaviorDeathInitialize();
		break;
	}
	behaviorRequest_ = Behavior::kUnknown;
}

void PlayerBehavior::UpdateCurrentBehavior() {
	switch (behavior_) {
	case Behavior::kRoot:
	default:
		BehaviorRootUpdate();
		if (!owner_.IsControlLocked()) {
			owner_.GroundMovement();
			owner_.JumpMovement();
		}
		owner_.ApplyScrollPush();
		owner_.MapChipCollisionUpdate();
		break;
	case Behavior::kAttack:
		BehaviorAttackUpdate();
		break;
	case Behavior::kKnockback:
		BehaviorKnockbackUpdate();
		break;
	case Behavior::kStunned:
		BehaviorStunnedUpdate();
		break;
	case Behavior::kBlock:
		BehaviorBlockUpdate();
		break;
	case Behavior::kDeath:
		BehaviorDeathUpdate();
		break;
	}
}

void PlayerBehavior::RequestAttack(AttackDirection direction) {
	if (owner_.IsControlLocked()) {
		return;
	}
	// [FIX] ノックバック/スタン/死亡中(およびそれらの要求が保留中)は攻撃で上書きさせない。
	// 以前はkStunnedが抜けていて、スタン中でも攻撃でスタンを破れた。
	if (behavior_ == Behavior::kKnockback || behavior_ == Behavior::kStunned || behavior_ == Behavior::kDeath) {
		return;
	}
	if (behaviorRequest_ == Behavior::kKnockback || behaviorRequest_ == Behavior::kStunned || behaviorRequest_ == Behavior::kDeath) {
		return;
	}
	if (!isAttacking_) {
		comboIndex_ = 0;
		isAttacking_ = true;
		attackDirection_ = direction;
		behaviorRequest_ = Behavior::kAttack;
	} else if (attackPhase_ == AttackPhase::kAnticipate) {
		comboIndex_ = 0;
		comboInputBuffered_ = false;
		attackDirection_ = direction;
		behaviorRequest_ = Behavior::kAttack;
	} else if (attackPhase_ == AttackPhase::kRecovery) {
		comboInputBuffered_ = true;
	}
}

void PlayerBehavior::RequestStun(float duration) {
	if (behavior_ == Behavior::kDeath || behaviorRequest_ == Behavior::kDeath)
		return;
	stunDuration_ = duration;
	behaviorRequest_ = Behavior::kStunned;
}

void PlayerBehavior::RequestHitKnockback(const Vector3& direction, float power, bool hop) {
	if (behavior_ == Behavior::kDeath || behaviorRequest_ == Behavior::kDeath)
		return;
	owner_.velocity_ = {direction.x * (power * kHitKnockbackSpeedScale), direction.y * (power * kHitKnockbackSpeedScale), direction.z * (power * kHitKnockbackSpeedScale)};
	// [NEW] 被弾ノックバックでは小さく跳ね上がる(ガード成功時の押し戻しは対象外)
	if (hop) {
		owner_.velocity_.y = std::max(owner_.velocity_.y, kHitKnockbackHopSpeed);
	}
	isAttacking_ = false;
	behaviorRequest_ = Behavior::kKnockback;
}

void PlayerBehavior::RequestDeath(const Vector3& direction, float power) {
	deathDirection_ = direction;
	deathPower_ = power;
	behaviorRequest_ = Behavior::kDeath;
}

void PlayerBehavior::SetBlocking(bool blocking) {
	isBlocking_ = blocking;
	if (blocking && behavior_ == Behavior::kRoot) {
		behaviorRequest_ = Behavior::kBlock;
		PlaySfxHandle(sBlockStartSoundHandle_);
	} else if (!blocking && behavior_ == Behavior::kBlock)
		behaviorRequest_ = Behavior::kRoot;
}

void PlayerBehavior::BehaviorStunnedInitialize() {
	stunTimer_ = 0.0f;
	owner_.velocity_.x = 0.0f;

	// スタンに入った瞬間に跳ね上がる
	owner_.velocity_.y = std::max(owner_.velocity_.y, kStunHopSpeed);
	owner_.onGround_ = false;

	//  攻撃を中断されたまま攻撃状態が残らないようにする
	isAttacking_ = false;
	comboIndex_ = 0;
	comboInputBuffered_ = false;
}
void PlayerBehavior::BehaviorStunnedUpdate() {
	stunTimer_ += 1.0f / 60.0f;
	if (!owner_.onGround_) {
		owner_.velocity_ += Vector3(0, -Player::kGravityAcceleration, 0);
		owner_.velocity_.y = std::max(owner_.velocity_.y, -Player::kLimitFallSpeed);
	}
	owner_.MapChipCollisionUpdate();
	if (stunTimer_ >= stunDuration_)
		behaviorRequest_ = Behavior::kRoot;
}

void PlayerBehavior::BehaviorBlockInitialize() { owner_.velocity_.x = 0.0f; }
void PlayerBehavior::BehaviorBlockUpdate() {
	owner_.UpdateTurnRotation();
	owner_.velocity_.x *= 0.8f;
	if (!owner_.onGround_) {
		owner_.velocity_ += Vector3(0, -Player::kGravityAcceleration, 0);
		owner_.velocity_.y = std::max(owner_.velocity_.y, -Player::kLimitFallSpeed);
	}
	owner_.MapChipCollisionUpdate();
	if (!isBlocking_)
		behaviorRequest_ = Behavior::kRoot;
}

void PlayerBehavior::BehaviorDeathInitialize() {
	owner_.worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	deathParameter_ = 0;
	deathBounceCount_ = 0;
	owner_.velocity_ = {
	    deathDirection_.x * (deathPower_ * kDeathFlingSpeedScale) + 0.0f, deathDirection_.y * (deathPower_ * kDeathFlingSpeedScale) + kDeathFlingUpBoost,
	    deathDirection_.z * (deathPower_ * kDeathFlingSpeedScale) + 0.0f};
	deathSpinSpeed_ = (deathDirection_.x < 0.0f) ? -0.35f : 0.35f;
	owner_.onGround_ = false;
	PlayDeathSound();
}

void PlayerBehavior::BehaviorDeathUpdate() {
	++deathParameter_;

	bool settled = (deathBounceCount_ >= kDeathMaxBounces) || (owner_.velocity_.x == 0.0f && owner_.velocity_.y == 0.0f && deathBounceCount_ > 0);

	if (!settled) {
		owner_.worldTransform_.rotation_.x += deathSpinSpeed_ * 0.6f;
		owner_.worldTransform_.rotation_.z += deathSpinSpeed_;
	} else {
		float targetX = std::numbers::pi_v<float> / 2.0f;
		owner_.worldTransform_.rotation_.x = EaseOut(owner_.worldTransform_.rotation_.x, targetX, 0.15f);
		float nearestFlatZ = std::round(owner_.worldTransform_.rotation_.z / std::numbers::pi_v<float>) * std::numbers::pi_v<float>;
		owner_.worldTransform_.rotation_.z = EaseOut(owner_.worldTransform_.rotation_.z, nearestFlatZ, 0.15f);
	}

	owner_.velocity_.y -= Player::kGravityAcceleration;
	owner_.velocity_.y = std::max(owner_.velocity_.y, -kDeathMaxFallSpeed);

	bool landed = false;
	ObjectBounce::Step(owner_.mapChipField_, owner_.worldTransform_.translation_, owner_.velocity_, Player::kWidth, Player::kHeight, kDeathBounceDamping, kDeathFriction, false, landed);
	if (landed) {
		++deathBounceCount_;
		if (deathBounceCount_ >= kDeathMaxBounces || std::abs(owner_.velocity_.y) < 0.02f)
			owner_.velocity_ = {};
	}

	if (deathParameter_ >= kDeathDuration)
		owner_.isDead_ = true;
}

void PlayerBehavior::RequestKnockback(float pushDirectionSign) {
	if (behavior_ == Behavior::kDeath || behaviorRequest_ == Behavior::kDeath)
		return;
	owner_.velocity_.x = pushDirectionSign * kKnockBackSpeedX;
	owner_.velocity_.y = kKnockBackSpeedY;
	isAttacking_ = false;
	behaviorRequest_ = Behavior::kKnockback;
}

PlayerBehavior::AttackAnimState PlayerBehavior::GetAttackAnimState() const {
	float duration = 1.0f;
	const BaseMelee* melee = owner_.GetCurrentMelee();
	switch (attackPhase_) {
	case AttackPhase::kAnticipate:
		duration = melee ? melee->GetComboAnticipateDuration(comboIndex_) : duration;
		break;
	case AttackPhase::kAttack:
		duration = melee ? melee->GetComboAttackDuration(comboIndex_) : duration;
		break;
	case AttackPhase::kRecovery:
		duration = melee ? melee->GetComboRecoveryDuration(comboIndex_) : duration;
		break;
	}
	float t = (duration > 0.0f) ? static_cast<float>(attackParameter_) / duration : 0.0f;
	return {attackPhase_, std::clamp(t, 0.0f, 1.0f), comboIndex_};
}

SRTOffset PlayerBehavior::GetAttackPoseOffset() const {
	AttackAnimState state = GetAttackAnimState();
	int idx = std::clamp(state.comboIndex, 0, static_cast<int>(kComboPosePeak.size()) - 1);
	switch (state.phase) {
	case AttackPhase::kAnticipate: {
		float te = EaseIn(0.0f, 1.0f, state.t);
		return LerpSRT(kNeutralPose, kComboPoseAnticipate[idx], te);
	}
	case AttackPhase::kAttack: {
		float te = EaseOut(0.0f, 1.0f, state.t);
		return LerpSRT(kComboPoseAnticipate[idx], kComboPosePeak[idx], te);
	}
	case AttackPhase::kRecovery: {
		float te = EaseInOut(0.0f, 1.0f, state.t);
		return LerpSRT(kComboPoseRecovery[idx], kNeutralPose, te);
	}
	default:
		return kNeutralPose;
	}
}

#pragma region BehaviorStates
void PlayerBehavior::BehaviorRootInitialize() { owner_.worldTransform_.scale_ = {1.0f, 1.0f, 1.0f}; }

void PlayerBehavior::BehaviorRootUpdate() {
	if (owner_.IsControlLocked()) {
		owner_.velocity_.x *= (1.0f - Player::kAttenuation);
	}
}

void PlayerBehavior::BehaviorAttackInitialize() {
	attackParameter_ = 0;
	attackPhase_ = AttackPhase::kAnticipate;
	attackVelocity_ = {0.0f, 0.0f, 0.0f};
	if (BaseMelee* melee = owner_.GetCurrentMelee()) {
		melee->ResetSwingHits();
		melee->PlaySwingSound(comboIndex_);
	}
}
void PlayerBehavior::BehaviorAttackUpdate() {
	++attackParameter_;

	BaseMelee* melee = owner_.GetCurrentMelee();
	int maxCombo = melee ? melee->GetMaxCombo() : 1;

	if (attackDirection_ == AttackDirection::kSide) {
		owner_.worldTransform_.rotation_.y = (owner_.lrDirection_ == Player::LRDirection::kRight) ? 1.570796f : 4.712389f;
	}

	switch (attackPhase_) {
	case AttackPhase::kAnticipate:
	default: {
		attackVelocity_ = {0.0f, 0.0f, 0.0f};
		float anticipateDuration = melee ? melee->GetComboAnticipateDuration(comboIndex_) : 10.0f;
		if (static_cast<float>(attackParameter_) >= anticipateDuration) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
		}
		break;
	}
	case AttackPhase::kAttack: {
		if (attackDirection_ == AttackDirection::kSide) {
			float speed = melee ? melee->GetComboAttackSpeed(comboIndex_) : 0.25f;
			attackVelocity_.x = (owner_.lrDirection_ == Player::LRDirection::kRight) ? speed : -speed;
		} else {
			attackVelocity_ = {0.0f, 0.0f, 0.0f};
		}

		Vector3 facingOffset;
		if (owner_.lrDirection_ == Player::LRDirection::kRight)
			facingOffset = {kHitEffectOffset, 0.0f, 0.0f};
		else
			facingOffset = {-kHitEffectOffset, 0.0f, 0.0f};
		owner_.worldTransformAttack_.translation_ = owner_.worldTransform_.translation_ + facingOffset;
		owner_.worldTransformAttack_.rotation_ = owner_.worldTransform_.rotation_;

		float attackDuration = melee ? melee->GetComboAttackDuration(comboIndex_) : 8.0f;
		if (static_cast<float>(attackParameter_) >= attackDuration) {
			attackPhase_ = AttackPhase::kRecovery;
			attackParameter_ = 0;
		}
		break;
	}
	case AttackPhase::kRecovery: {
		attackVelocity_ = {0.0f, 0.0f, 0.0f};
		float recoveryDuration = melee ? melee->GetComboRecoveryDuration(comboIndex_) : 15.0f;
		if (static_cast<float>(attackParameter_) >= recoveryDuration) {
			if (comboInputBuffered_ && comboIndex_ + 1 < maxCombo) {
				comboIndex_++;
				comboInputBuffered_ = false;
				attackPhase_ = AttackPhase::kAnticipate;
				attackParameter_ = 0;
				if (melee == owner_.GetCurrentMelee()) {
					melee->ResetSwingHits();
					melee->PlaySwingSound(comboIndex_);
				}
			} else {
				behaviorRequest_ = Behavior::kRoot;
				isAttacking_ = false;
				comboIndex_ = 0;
				comboInputBuffered_ = false;
			}
		}
		break;
	}
	}

	Vector3 finalMove;
	if (attackDirection_ == AttackDirection::kDown) {
		if (!owner_.IsControlLocked()) {
			owner_.GroundMovement();
			owner_.JumpMovement();
		}
		finalMove = owner_.velocity_;
	} else {
		if (!owner_.onGround_) {
			owner_.velocity_.y -= Player::kGravityAcceleration;
			owner_.velocity_.y = std::max(owner_.velocity_.y, -Player::kLimitFallSpeed);
		}
		finalMove = {attackVelocity_.x, owner_.velocity_.y, 0.0f};
	}

	CollisionInfo collisionMapInfo;
	collisionMapInfo.move = finalMove;
	owner_.MapChipCollisionUpdate(collisionMapInfo);

	WorldTransformUpdate(owner_.worldTransform_);
	WorldTransformUpdate(owner_.worldTransformAttack_);
}

void PlayerBehavior::BehaviorKnockbackInitialize() {
	owner_.worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	knockBackParameter_ = 0;
	owner_.onGround_ = false;

	// [FIX] 攻撃を中断されたコンボ状態を残さない
	isAttacking_ = false;
	comboIndex_ = 0;
	comboInputBuffered_ = false;
}
void PlayerBehavior::BehaviorKnockbackUpdate() {
	++knockBackParameter_;
	owner_.velocity_ += Vector3(0, -Player::kGravityAcceleration, 0);
	owner_.velocity_.y = std::max(owner_.velocity_.y, -Player::kLimitFallSpeed);

	CollisionInfo collisionMapInfo;
	collisionMapInfo.move = owner_.velocity_;
	owner_.MapChipCollisionUpdate(collisionMapInfo);

	if (knockBackParameter_ >= kKnockBackDuration)
		behaviorRequest_ = Behavior::kRoot;
}
#pragma endregion