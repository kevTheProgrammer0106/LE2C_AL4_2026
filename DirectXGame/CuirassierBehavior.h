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

private:
	friend class DebugMenu;
	enum class MeleeStage { kLunge, kCombo };
	enum class LungePhase { kAnticipate, kAttack, kRecovery };
	enum class ComboPhase { kAnticipate, kAttack, kRecovery };

	void BehaviorMeleeInitialize();
	void BehaviorMeleeUpdate();

	void BehaviorBlockInitialize();
	void BehaviorBlockUpdate();

	EnemyCuirassier& cuirassierOwner_;

	// --- kMelee: ランジ(kLunge) -> コンボ(kCombo) ---
	MeleeStage meleeStage_ = MeleeStage::kLunge;
	LungePhase lungePhase_ = LungePhase::kAnticipate;
	ComboPhase comboPhase_ = ComboPhase::kAnticipate;
	uint32_t attackParameter_ = 0;
	int comboIndex_ = 0;

	// ランジ専用の予備動作/踏み込み/硬直の秒数
	static inline float kLungeAnticipateDuration = 0.35f;
	static inline float kLungeAttackDuration = 0.30f; // ここで大きく前進する
	static inline float kLungeRecoveryDuration = 0.25f;
	static inline float kLungeSpeed = 0.35f; // 踏み込み速度(riflemanの銃剣突撃より速い)

	// WeaponSabreと同じ枠組みのコンボ参照用ダミー(間合い等はWeaponHeavySabre側が独自の値を持つ)
	WeaponHeavySabre& Weapon();

	static inline float kMeleeEngageRange = 6.0f; // これ以下ならkMeleeへ突入(ランジ間合いを含む)

	// --- kBlock: プレイヤーの攻撃に反応してランダムでガード ---
	static inline float kBlockChance = 0.75f;    // プレイヤーが攻撃を開始した瞬間にガードへ入る確率
	static inline float kBlockCheckRange = 5.0f; // この距離以内でのみガード判定を行う
	static inline float kBlockDuration = 1.0f;   // ガード継続時間
	float blockTimer_ = 0.0f;
	bool playerWasAttackingLastFrame_ = false;
};