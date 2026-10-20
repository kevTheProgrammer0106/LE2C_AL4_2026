#include "PlayerBehavior.h"
#include "BaseMelee.h"
#include "GlobalVariables.h"
#include "GameScene.h"
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
	gv->AddItem(kGroupName, "HitKnockbackHopSpeed", kHitKnockbackHopSpeed); 
	gv->AddItem(kGroupName, "StunHopSpeed", kStunHopSpeed);
	RegisterBodyComboPoseSet(kGroupName, "BodyCombo", kBodyCombo);
	RegisterBodyLungePoseSet(kGroupName, "BodyLungeHeavy", kBodyLungeHeavy);
	RegisterBodyLungePoseSet(kGroupName, "BodyLungeBayonet", kBodyLungeBayonet);
	gv->AddItem(kGroupName, "KnockbackAirDrag", kKnockbackAirDrag);
	gv->AddItem(kGroupName, "KnockbackGroundFriction", kKnockbackGroundFriction);
	gv->AddItem(kGroupName, "KnockbackRestitution", kKnockbackRestitution);
	gv->AddItem(kGroupName, "KnockbackBounceMinSpeed", kKnockbackBounceMinSpeed);
	gv->AddItem(kGroupName, "KnockBackMaxDuration", static_cast<int32_t>(kKnockBackMaxDuration));
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
	kHitKnockbackHopSpeed = gv->GetFloatValue(kGroupName, "HitKnockbackHopSpeed"); 
	kStunHopSpeed = gv->GetFloatValue(kGroupName, "StunHopSpeed"); 
	ApplyBodyComboPoseSet(kGroupName, "BodyCombo", kBodyCombo);
	ApplyBodyLungePoseSet(kGroupName, "BodyLungeHeavy", kBodyLungeHeavy);
	ApplyBodyLungePoseSet(kGroupName, "BodyLungeBayonet", kBodyLungeBayonet);
	kKnockbackAirDrag = gv->GetFloatValue(kGroupName, "KnockbackAirDrag");
	kKnockbackGroundFriction = gv->GetFloatValue(kGroupName, "KnockbackGroundFriction");
	kKnockbackRestitution = gv->GetFloatValue(kGroupName, "KnockbackRestitution");
	kKnockbackBounceMinSpeed = gv->GetFloatValue(kGroupName, "KnockbackBounceMinSpeed");
	kKnockBackMaxDuration = static_cast<uint32_t>(gv->GetIntValue(kGroupName, "KnockBackMaxDuration"));
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
		// Combo and heavy-sabre lunge drive the melee weapon's pose; the bayonet lunge drives the firearm instead.
		if (owner_.GetCurrentMelee() && (attackKind_ == AttackKind::kCombo || lungeSource_ == LungeSource::kHeavySabre))
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
	if (behavior_ != Behavior::kAttack) {
		if (BaseMelee* melee = owner_.GetCurrentMelee())
			melee->SetBehavior(BaseMelee::MeleeBehavior::kIdle);
		attackKind_ = AttackKind::kCombo;
		lungeSource_ = LungeSource::kNone;
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
		attackKind_ = AttackKind::kCombo;
		lungeSource_ = LungeSource::kNone;
		behaviorRequest_ = Behavior::kAttack;
	} else if (attackPhase_ != AttackPhase::kAnticipate) {
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
void PlayerBehavior::RequestLunge(LungeSource source) {
	if (owner_.IsControlLocked())
		return;
	if (behavior_ == Behavior::kKnockback || behavior_ == Behavior::kStunned || behavior_ == Behavior::kDeath)
		return;
	if (behaviorRequest_ == Behavior::kKnockback || behaviorRequest_ == Behavior::kStunned || behaviorRequest_ == Behavior::kDeath)
		return;
	if (isAttacking_)
		return;
	comboIndex_ = 0;
	comboInputBuffered_ = false;
	isAttacking_ = true;
	attackDirection_ = AttackDirection::kSide;
	attackKind_ = AttackKind::kLunge;
	lungeSource_ = source;
	behaviorRequest_ = Behavior::kAttack;
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
	if (owner_.gameScene_)
		owner_.gameScene_->TriggerCameraFx(GameScene::CameraFx::kDeath); // covers enemy hits, explosions, kill blocks, KillInstant
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
	if (attackKind_ == AttackKind::kLunge) {
		const LungeSpec spec = owner_.GetLungeSpec(lungeSource_);
		duration = (attackPhase_ == AttackPhase::kAnticipate) ? spec.anticipateFrames : (attackPhase_ == AttackPhase::kAttack) ? spec.attackFrames : spec.recoveryFrames;
	} else {
		const BaseMelee* melee = owner_.GetCurrentMelee();
		switch (attackPhase_) {
		case AttackPhase::kAnticipate:
			duration = melee ? melee->GetComboAnticipateDuration(comboIndex_, Owner::kPlayer) : duration;
			break;
		case AttackPhase::kAttack:
			duration = melee ? melee->GetComboAttackDuration(comboIndex_, Owner::kPlayer) : duration;
			break;
		case AttackPhase::kRecovery:
			duration = melee ? melee->GetComboRecoveryDuration(comboIndex_, Owner::kPlayer) : duration;
			break;
		}
	}
	float t = (duration > 0.0f) ? static_cast<float>(attackParameter_) / duration : 0.0f;
	return {attackPhase_, std::clamp(t, 0.0f, 1.0f), comboIndex_};
}

SRTOffset PlayerBehavior::GetAttackPoseOffset() const {
	AttackAnimState s = GetAttackAnimState();
	BodyAttackPhase phase = static_cast<BodyAttackPhase>(static_cast<int>(s.phase));
	if (attackKind_ == AttackKind::kLunge) {
		const BodyLungePoseSet& set = (lungeSource_ == LungeSource::kBayonet) ? kBodyLungeBayonet : kBodyLungeHeavy;
		return EvaluateBodyLungePose(set, phase, s.t);
	}
	return EvaluateBodyComboPose(kBodyCombo, phase, s.t, s.comboIndex);
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

	if (attackKind_ == AttackKind::kLunge) {
		owner_.ResetLungeHits();
		owner_.BeginLungeCooldown(lungeSource_); // cooldown starts the moment the lunge is launched
		return;
	}
	if (BaseMelee* melee = owner_.GetCurrentMelee()) {
		melee->ResetSwingHits();
		melee->PlaySwingSound(comboIndex_);
	}
}
void PlayerBehavior::BehaviorAttackUpdate() {
	if (attackKind_ == AttackKind::kLunge) {
		BehaviorLungeUpdate();
		return;
	}
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
		float anticipateDuration = melee ? melee->GetComboAnticipateDuration(comboIndex_, Owner::kPlayer) : 10.0f;
		if (static_cast<float>(attackParameter_) >= anticipateDuration) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			if (owner_.gameScene_)
				owner_.gameScene_->TriggerCameraFx(GameScene::CameraFx::kSwing, 1.0f + 0.5f * static_cast<float>(comboIndex_));
		}
		break;
	}
	case AttackPhase::kAttack: {
		if (attackDirection_ == AttackDirection::kSide) {
			float speed = melee ? melee->GetComboAttackSpeed(comboIndex_, Owner::kPlayer) : 0.25f;
			attackVelocity_.x = (owner_.lrDirection_ == Player::LRDirection::kRight) ? speed : -speed;
		} else {
			attackVelocity_ = {0.0f, 0.0f, 0.0f};
		}

		float attackDuration = melee ? melee->GetComboAttackDuration(comboIndex_, Owner::kPlayer) : 8.0f;
		if (static_cast<float>(attackParameter_) >= attackDuration) {
			attackPhase_ = AttackPhase::kRecovery;
			attackParameter_ = 0;
		}
		break;
	}
	case AttackPhase::kRecovery: {
		attackVelocity_ = {0.0f, 0.0f, 0.0f};
		float recoveryDuration = melee ? melee->GetComboRecoveryDuration(comboIndex_, Owner::kPlayer) : 15.0f;
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
	if (attackDirection_ == AttackDirection::kSide) {
		finalMove.x = owner_.LimitAttackMoveByEnemies(finalMove.x); // 敵の体を通り抜けない
	}

	CollisionInfo collisionMapInfo;
	collisionMapInfo.move = finalMove;
	owner_.MapChipCollisionUpdate(collisionMapInfo);

	// 斬撃モデルは移動後の位置と現在の向き(lrDirection_)に毎フレーム合わせる
	// ルートの回転はターン補間中にずれるので使わない
	{
		const bool facingRight = (owner_.lrDirection_ == Player::LRDirection::kRight);
		const float dir = facingRight ? 1.0f : -1.0f;
		owner_.worldTransformAttack_.translation_ = owner_.worldTransform_.translation_ + Vector3(dir * kHitEffectOffset, 0.0f, 0.0f);
		owner_.worldTransformAttack_.rotation_ = {0.0f, facingRight ? 1.570796f : 4.712389f, 0.0f};
	}

	WorldTransformUpdate(owner_.worldTransform_);
	WorldTransformUpdate(owner_.worldTransformAttack_);
}
void PlayerBehavior::BehaviorLungeUpdate() {
	++attackParameter_; 

	const LungeSpec spec = owner_.GetLungeSpec(lungeSource_);
	const bool right = (owner_.lrDirection_ == Player::LRDirection::kRight);
	owner_.worldTransform_.rotation_.y = right ? 1.570796f : 4.712389f;

	switch (attackPhase_) {
	case AttackPhase::kAnticipate:
	default:
		attackVelocity_ = {0.0f, 0.0f, 0.0f};
		if (static_cast<float>(attackParameter_) >= spec.anticipateFrames) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0;
			owner_.PlayLungeSound(lungeSource_);
			if (owner_.gameScene_)
				owner_.gameScene_->TriggerCameraFx(GameScene::CameraFx::kSpecial);
		}
		break;
	case AttackPhase::kAttack:
		// stop pushing forward once the lunge has connected
		if (owner_.HasLungeHits())
			attackVelocity_ = {0.0f, 0.0f, 0.0f};
		else
			attackVelocity_ = {right ? spec.speed : -spec.speed, 0.0f, 0.0f};
		if (static_cast<float>(attackParameter_) >= spec.attackFrames) {
			attackPhase_ = AttackPhase::kRecovery;
			attackParameter_ = 0;
		}
		break;
	case AttackPhase::kRecovery:
		attackVelocity_ = {0.0f, 0.0f, 0.0f};
		if (static_cast<float>(attackParameter_) >= spec.recoveryFrames) {
			behaviorRequest_ = Behavior::kRoot;
			isAttacking_ = false;
			comboIndex_ = 0;
			comboInputBuffered_ = false;
		}
		break;
	}

	if (!owner_.onGround_) {
		owner_.velocity_.y -= Player::kGravityAcceleration;
		owner_.velocity_.y = std::max(owner_.velocity_.y, -Player::kLimitFallSpeed);
	}
	CollisionInfo collisionMapInfo;
	collisionMapInfo.move = {owner_.LimitAttackMoveByEnemies(attackVelocity_.x), owner_.velocity_.y, 0.0f};
	owner_.MapChipCollisionUpdate(collisionMapInfo);
	WorldTransformUpdate(owner_.worldTransform_);
}
void PlayerBehavior::BehaviorKnockbackInitialize() {
	owner_.worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	knockBackParameter_ = 0;
	owner_.onGround_ = false;

	// 攻撃を中断されたコンボ状態を残さない
	isAttacking_ = false;
	comboIndex_ = 0;
	comboInputBuffered_ = false;
}
void PlayerBehavior::BehaviorKnockbackUpdate() {
	++knockBackParameter_;
	owner_.velocity_ += Vector3(0, -Player::kGravityAcceleration, 0);
	owner_.velocity_.y = std::max(owner_.velocity_.y, -Player::kLimitFallSpeed);
	owner_.velocity_.x *= owner_.onGround_ ? kKnockbackGroundFriction : kKnockbackAirDrag;

	const Vector3 before = owner_.velocity_;
	CollisionInfo collisionMapInfo;
	collisionMapInfo.move = before;
	owner_.MapChipCollisionUpdate(collisionMapInfo, true);

	// 壁/床/天井で跳ね返る(グレネードと同じ考え方)
	if (collisionMapInfo.isHitWall && std::abs(before.x) > kKnockbackBounceMinSpeed) {
		owner_.velocity_.x = -before.x * kKnockbackRestitution;
	}
	if (collisionMapInfo.isHitGround && before.y < -kKnockbackBounceMinSpeed) {
		owner_.velocity_.y = -before.y * kKnockbackRestitution;
		owner_.onGround_ = false;
	} else if (collisionMapInfo.isHitTop && before.y > kKnockbackBounceMinSpeed) {
		owner_.velocity_.y = -before.y * kKnockbackRestitution;
	}

	// 固定時間で打ち切らず、空中で跳ねている間は続ける(着地するか最大時間で終了)
	const bool settled = owner_.onGround_ || knockBackParameter_ >= kKnockBackMaxDuration;
	if (knockBackParameter_ >= kKnockBackDuration && settled)
		behaviorRequest_ = Behavior::kRoot;
}
#pragma endregion