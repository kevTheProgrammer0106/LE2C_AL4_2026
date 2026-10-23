#pragma once

#define NOMINMAX

#include "Global.h"

class Player;

class PlayerBehavior {
public:
	explicit PlayerBehavior(Player& owner) : owner_(owner) {}

	enum class Behavior { kUnknown, kRoot, kAttack, kKnockback, kStunned, kBlock, kDeath };
	enum class AttackPhase { kAnticipate, kAttack, kRecovery };
	enum class AttackDirection { kSide, kDown };

	struct AttackAnimState {
		AttackPhase phase = AttackPhase::kAnticipate;
		float t = 0.0f;
		int comboIndex = 0;
	};

	Behavior GetBehavior() const { return behavior_; }
	bool IsAttacking() const { return behavior_ == Behavior::kAttack; }
	bool IsKnockback() const { return behavior_ == Behavior::kKnockback; }

	void RequestBehavior(Behavior request) { behaviorRequest_ = request; }

	void ResolveBehaviorRequest();
	void UpdateCurrentBehavior();

	void RequestAttack(AttackDirection direction);
	void RequestKnockback(float pushDirectionSign);

	int GetComboIndex() const { return comboIndex_; }
	AttackDirection GetAttackDirection() const { return attackDirection_; }
	AttackAnimState GetAttackAnimState() const;
	SRTOffset GetAttackPoseOffset() const;

	void RequestStun(float duration);
	// hop=true: 被弾ノックバック時に小さく跳ね上がる。ガード成功時の押し戻しはhop=falseで呼ぶ。
	void RequestHitKnockback(const KamataEngine::Vector3& direction, float power, bool hop = true);
	void RequestDeath(const KamataEngine::Vector3& direction, float power);
	void SetBlocking(bool blocking);
	bool IsBlocking() const { return isBlocking_; }
	bool IsStunned() const { return behavior_ == Behavior::kStunned; }
	bool IsDeathBehavior() const { return behavior_ == Behavior::kDeath; }

	float GetStunRemaining() const { return (behavior_ == Behavior::kStunned && stunDuration_ > stunTimer_) ? (stunDuration_ - stunTimer_) : 0.0f; }

	void ClearStun() {
		if (behavior_ == Behavior::kStunned)
			behaviorRequest_ = Behavior::kRoot;
		else if (behaviorRequest_ == Behavior::kStunned)
			behaviorRequest_ = Behavior::kUnknown;
	}
	void ClearKnockback() {
		if (behavior_ == Behavior::kKnockback)
			behaviorRequest_ = Behavior::kRoot;
		else if (behaviorRequest_ == Behavior::kKnockback)
			behaviorRequest_ = Behavior::kUnknown;
	}

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	enum class AttackKind { kCombo, kLunge };

	bool IsLunge() const { return behavior_ == Behavior::kAttack && attackKind_ == AttackKind::kLunge; }
	AttackKind GetAttackKind() const { return attackKind_; }
	LungeSource GetLungeSource() const { return lungeSource_; }
	void RequestLunge(LungeSource source);

	// --- SFX ---
	// GameSceneがロード後にセットする。プレイヤーの被弾/死亡/ブロック開始SFX。
	static void SetHurtSoundHandle(uint32_t handle) { sHurtSoundHandle_ = handle; }
	static void SetDeathSoundHandle(uint32_t handle) { sDeathSoundHandle_ = handle; }
	static void SetBlockStartSoundHandle(uint32_t handle) { sBlockStartSoundHandle_ = handle; }
	static void SetBlockImpactSoundHandle(uint32_t handle) { sBlockImpactSoundHandle_ = handle; }
	static void SetSfxVolume(float volume) { sSfxVolume_ = volume; }
	// ガード成功時(パリィ的な被弾)のSFX。Player::TakeDamage内、ブロック分岐で呼ぶ。
	static void PlayBlockImpactSound() { PlaySfxHandle(sBlockImpactSoundHandle_); }
	static void PlayHurtSound() { PlaySfxHandle(sHurtSoundHandle_); }
	static void PlayDeathSound() { PlaySfxHandle(sDeathSoundHandle_); }

private:
	void BehaviorRootInitialize();
	void BehaviorRootUpdate();
	void BehaviorAttackInitialize();
	void BehaviorAttackUpdate();
	void BehaviorKnockbackInitialize();
	void BehaviorKnockbackUpdate();
	void BehaviorStunnedInitialize();
	void BehaviorStunnedUpdate();
	void BehaviorBlockInitialize();
	void BehaviorBlockUpdate();
	void BehaviorDeathInitialize();
	void BehaviorDeathUpdate();

	bool isBlocking_ = false;
	float stunTimer_ = 0.0f;
	float stunDuration_ = 0.0f;

	KamataEngine::Vector3 deathDirection_{};
	float deathPower_ = 0.0f;
	uint32_t deathParameter_ = 0;
	float deathSpinSpeed_ = 0.0f;
	int deathBounceCount_ = 0;

	static inline uint32_t kDeathDuration = 100;
	static inline int kDeathMaxBounces = 3;
	static inline float kDeathBounceDamping = 0.45f;
	static inline float kDeathFriction = 0.25f;
	static inline float kDeathMaxFallSpeed = 0.75f;
	static inline float kDeathFlingSpeedScale = 0.22f;
	static inline float kDeathFlingUpBoost = 0.3f;
	static inline float kHitKnockbackSpeedScale = 0.1f;

	// [NEW] ノックバック/スタンに入った瞬間の跳ね上がり初速
	static inline float kHitKnockbackHopSpeed = 0.3f;
	static inline float kStunHopSpeed = 0.2f;

	Player& owner_;

	Behavior behavior_ = Behavior::kRoot;
	Behavior behaviorRequest_ = Behavior::kUnknown;

	bool isAttacking_ = false;
	AttackPhase attackPhase_ = AttackPhase::kAnticipate;
	int comboIndex_ = 0;
	bool comboInputBuffered_ = false;
	AttackDirection attackDirection_ = AttackDirection::kSide;
	uint32_t attackParameter_ = 0;
	KamataEngine::Vector3 attackVelocity_ = {0.0f, 0.0f, 0.0f};

	static inline float kHitEffectOffset = 1.0f;

	uint32_t knockBackParameter_ = 0;
	static inline float kKnockBackSpeedX = 0.5f;
	static inline float kKnockBackSpeedY = 0.35f;
	static inline uint32_t kKnockBackDuration = 25;

	static inline float kKnockbackAirDrag = 0.97f;
	static inline float kKnockbackGroundFriction = 0.88f;
	static inline float kKnockbackRestitution = 0.45f;
	static inline float kKnockbackBounceMinSpeed = 0.08f;
	static inline uint32_t kKnockBackMaxDuration = 60;

static inline BodyComboPoseSet kBodyCombo = {
	    {MakeBodyPose(-0.1f, -0.7f, 0.0f), MakeBodyPose(-0.1f, 1.0f, 0.0f), MakeBodyPose(-0.4f, -0.7f, 0.0f)},
	    {MakeBodyPose(0.2f, 0.2f, 0.0f), MakeBodyPose(0.3f, 0.0f, 0.0f, {1.1f, 1.0f, 1.0f}), MakeBodyPose(0.4f, -0.1f, 0.0f)},
	    {MakeBodyPose(0.3f, 1.0f, 0.0f), MakeBodyPose(0.4f, 0.0f, 0.0f), MakeBodyPose(0.5f, 0.0f, 0.0f)},
	};
	static inline SRTOffset kNeutralPose = {};

		void BehaviorLungeUpdate();

	AttackKind attackKind_ = AttackKind::kCombo;
	LungeSource lungeSource_ = LungeSource::kNone;

	static inline BodyLungePoseSet kBodyLungeHeavy = {
	    MakeBodyPose(-0.3f, -0.5f, 0.0f, {0.97f, 1.03f, 1.0f}),
	    MakeBodyPose(0.55f, 0.35f, 0.0f, {1.05f, 0.95f, 1.0f}, {0.0f, -0.05f, 0.0f}),
	};
	static inline BodyLungePoseSet kBodyLungeBayonet = {
	    MakeBodyPose(-0.2f, -0.3f, 0.0f, {1.0f, 0.97f, 1.0f}, {0.0f, -0.05f, 0.0f}),
	    MakeBodyPose(0.35f, 0.25f, 0.0f, {1.02f, 0.98f, 1.0f}),
	};

	static inline float kDeathSpinPerSpeed = 1.25f;
	static inline float kDeathMinSpin = 0.05f;
	static inline float kDeathMaxSpin = 0.5f;

	// --- SFX ---
	static inline uint32_t sHurtSoundHandle_ = 0;
	static inline uint32_t sDeathSoundHandle_ = 0;
	static inline uint32_t sBlockStartSoundHandle_ = 0;
	static inline uint32_t sBlockImpactSoundHandle_ = 0;
	static inline float sSfxVolume_ = 1.0f;
	static void PlaySfxHandle(uint32_t handle) {
		if (handle != 0) {
			AudioSettings::PlaySfx(handle, false, sSfxVolume_);
		}
	}
};