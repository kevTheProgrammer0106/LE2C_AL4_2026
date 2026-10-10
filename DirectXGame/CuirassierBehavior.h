#pragma once
#include "BaseEnemyBehavior.h"
#include "WeaponHeavySabre.h"

class EnemyCuirassier;

/// <summary>
/// EnemyCuirassier(盾役)専用のBehaviorモジュール。
/// - フランクルートを好んで回り込む
/// - 中距離(kComboEngageRange超〜kMeleeEngageRange以内)ではランジ突撃から入る(クールダウンあり)
/// - 近距離では通常の2連撃から入る。ランジの後も2連撃に繋がる
/// - 2連撃・ランジ中は前方へ踏み込む(プレイヤー/Fighterと同様)
/// </summary>
class CuirassierBehavior final : public BaseEnemyBehavior {
public:
	explicit CuirassierBehavior(EnemyCuirassier& owner);

	FlankPreference GetDefaultFlankPreference() const override { return FlankPreference::kPreferFlanking; }

	bool IsEngaged() const override {
		Behavior b = GetBehavior();
		return b == Behavior::kMelee || b == Behavior::kBlock;
	}

	void InitializeBehaviorState(Behavior behavior) override;
	void UpdateBehaviorState() override;

	bool IsInAttackAnticipation() const override {
		if (GetBehavior() != Behavior::kMelee)
			return false;
		return (meleeStage_ == MeleeStage::kLunge) ? (lungePhase_ == LungePhase::kAnticipate) : (comboPhase_ == ComboPhase::kAnticipate);
	}
	bool IsInMeleeRecovery() const override {
		if (GetBehavior() != Behavior::kMelee)
			return false;
		return (meleeStage_ == MeleeStage::kLunge) ? (lungePhase_ == LungePhase::kRecovery) : (comboPhase_ == ComboPhase::kRecovery);
	}

protected:
	void BehaviorRootInitialize() override;
	void BehaviorRootUpdate() override;

	bool ShouldBlockAfterStun() const override { return true; }

private:
	friend class DebugMenu;
	enum class MeleeStage { kLunge, kCombo };
	enum class LungePhase { kAnticipate, kAttack, kRecovery };
	enum class ComboPhase { kAnticipate, kAttack, kRecovery };

	void BehaviorMeleeInitialize();
	void BehaviorMeleeUpdate();
	void TickTimers();

	EnemyCuirassier& cuirassierOwner_;

	// --- kMelee: [lunge] -> swings (Fighter style, twice per engagement) ---
	MeleeStage meleeStage_ = MeleeStage::kLunge;
	LungePhase lungePhase_ = LungePhase::kAnticipate;
	ComboPhase comboPhase_ = ComboPhase::kAnticipate;
	uint32_t attackParameter_ = 0;
	int comboIndex_ = 0; // persists between engagements (rotates 0->1->2->0)

	bool startWithLunge_ = false; // set by BehaviorRootUpdate right before requesting kMelee
	int swingsRemaining_ = 0;     // swings left in the current engagement

	float lungeCooldownTimer_ = 0.0f;
	float comboCooldownTimer_ = 0.0f;
	float noAttackTimer_ = 0.0f;

	static inline float kLungeAnticipateDuration = 0.35f;
	static inline float kLungeAttackDuration = 0.30f;
	static inline float kLungeRecoveryDuration = 0.25f;
	static inline float kLungeSpeed = 0.35f;

	static inline float kMeleeEngageRange = 6.0f;   // lunge range (outer)
	static inline float kComboEngageRange = 2.6f;   // inside this: swings instead of a lunge
	static inline float kLungeCooldown = 5.0f;      // seconds before another lunge is allowed
	static inline float kCooldownDuration = 0.8f;   // pause after a 2-swing engagement
	static inline float kComboResetDuration = 3.0f; // idle time before combo index resets to 0
	static inline int kSwingsPerEngagement = 2;
	static inline float kSwingAdvanceSpeed = 0.12f; // forward step during each swing's attack phase
};