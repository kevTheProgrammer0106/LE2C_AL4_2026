#pragma once
#include "BaseEnemyBehavior.h"
#include "WeaponSabre.h"

class EnemyFighter;

/// <summary>
/// EnemyFighter専用のBehaviorモジュール。共通のIdle/Roam/Root/Deathは基底クラスに任せ、
/// 白兵戦(kMelee)状態の初期化・更新だけをここに追加する。
///
/// 仕様: 接敵するとWeaponSabreのコンボを「1段だけ」振る → クールダウン → 次に攻撃する時は
/// 次のコンボ段(0→1→2→0...)を振る、というローテーションにする。しばらく(kComboResetDuration)
/// 攻撃せずにいるとコンボ段が0段目にリセットされる(コンボが繋がっていた体で扱わない)。
/// </summary>
class FighterBehavior final : public BaseEnemyBehavior {
public:
	explicit FighterBehavior(EnemyFighter& owner);

	FlankPreference GetDefaultFlankPreference() const override { return FlankPreference::kPreferShortest; }

	bool IsEngaged() const override {
		Behavior b = GetBehavior();
		return b == Behavior::kMelee || b == Behavior::kBlock;
	}

	void InitializeBehaviorState(Behavior behavior) override;
	void UpdateBehaviorState() override;
	
	
	bool IsInAttackAnticipation() const override { return GetBehavior() == Behavior::kMelee && meleePhase_ == MeleePhase::kAnticipate; }

	bool IsInMeleeRecovery() const override { return GetBehavior() == Behavior::kMelee && meleePhase_ == MeleePhase::kRecovery; }

protected:
	// 接敵距離まで詰まったらkMeleeへ、クールダウン中は距離を保つ、といった
	// 交戦判断をkRoot中に行う。
	void BehaviorRootInitialize() override;
	void BehaviorRootUpdate() override;

	// 10% chance to guard whenever it gets hit from the front (shared block: 1.5s, then 10s cooldown)
	float GetBlockChanceWhenHit() const override { return kBlockChanceWhenHit; }

private:
	friend class DebugMenu;
	enum class MeleePhase { kAnticipate, kAttack, kRecovery };

	void BehaviorUseMeleeInitialize();
	void BehaviorUseMeleeUpdate();

	EnemyFighter& fighterOwner_;

	MeleePhase meleePhase_ = MeleePhase::kAnticipate;
	uint32_t meleeAttackParameter_ = 0;

	// --- コンボローテーション/クールダウン ---
	int comboIndex_ = 0;
	float cooldownTimer_ = 0.0f;
	float noAttackTimer_ = 0.0f; // 最後に攻撃してからの経過時間。これがkComboResetDurationを超えるとコンボを0段目に戻す

	// WeaponSabreのコンボ間合い/速度/フェーズ長を問い合わせるためだけのダミー(Initialize/Drawはしない)
	WeaponSabre sabreComboReference_;

	static inline float kMeleeEngageRange = 2.2f;   // これ以下ならkMeleeへ突入
	static inline float kCooldownDuration = 0.8f;   // 1段振った後、次に振れるようになるまでの秒数
	static inline float kComboResetDuration = 3.0f; // これだけ攻撃せずにいるとコンボ段を0に戻す

	static inline float kBlockChanceWhenHit = 0.10f;
};