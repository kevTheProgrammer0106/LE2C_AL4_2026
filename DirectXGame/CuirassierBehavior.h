#pragma once
#include "BaseEnemyBehavior.h"
#include "WeaponHeavySabre.h"

class EnemyCuirassier;

/// <summary>
/// EnemyCuirassier(盾役)専用のBehaviorモジュール。
/// - フランクルートを好んで回り込む(既存)
/// - 接敵時は必ず「ランジ突撃」(rifleman銃剣より遥かに長い間合い)から入る
/// - ランジの後はWeaponSabreと同じコンボパターンで連撃する
/// - プレイヤーが攻撃中はランダムにガード(kBlock)する
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

	EnemyCuirassier& cuirassierOwner_;

	// --- kMelee: [lunge] -> swings (Fighter style, twice per engagement) ---
	MeleeStage meleeStage_ = MeleeStage::kLunge;
	LungePhase lungePhase_ = LungePhase::kAnticipate;
	ComboPhase comboPhase_ = ComboPhase::kAnticipate;
	uint32_t attackParameter_ = 0;
	int comboIndex_ = 0; // persists between engagements (rotates 0->1->2->0)

	// [NEW] set by BehaviorRootUpdate right before requesting kMelee
	bool startWithLunge_ = false;
	// [NEW] swings left in the current engagement
	int swingsRemaining_ = 0;

	// [NEW] timers
	float lungeCooldownTimer_ = 0.0f;
	float comboCooldownTimer_ = 0.0f;
	float noAttackTimer_ = 0.0f;

	static inline float kLungeAnticipateDuration = 0.35f;
	static inline float kLungeAttackDuration = 0.30f;
	static inline float kLungeRecoveryDuration = 0.25f;
	static inline float kLungeSpeed = 0.35f;

	WeaponHeavySabre& Weapon();

	static inline float kMeleeEngageRange = 6.0f; // lunge range

	// [NEW]
	static inline float kLungeCooldown = 5.0f;      // seconds before another lunge is allowed
	static inline float kComboEngageRange = 2.6f;   // Fighter-style swing range
	static inline float kCooldownDuration = 0.8f;   // pause after a 2-swing engagement
	static inline float kComboResetDuration = 3.0f; // idle time before combo index resets to 0
	static inline int kSwingsPerEngagement = 2;     // swings per engagement ("twice")

	// --- kBlock ---
	static inline float kBlockChance = 0.75f;
	static inline float kBlockCheckRange = 5.0f;
	static inline float kBlockDuration = 1.0f;
	float blockTimer_ = 0.0f;
	bool playerWasAttackingLastFrame_ = false;
};