#pragma once

#include "BaseEnemyBehavior.h"

class EnemySapper;

/// <summary>
/// EnemySapper専用のBehavior。
/// - 近距離: 斧のコンボ。攻撃中も歩き続ける(止まらない)。
/// - 中距離: ホーミングジャンプ(kLeap)。上昇中はマップ判定を無視し、頂点で速度を止め、
///   目標位置へ真っ直ぐ急降下(地面叩きつけ)。プレイヤーの近くまで来たら判定を戻し、
///   1.5秒自分でスタンしてから追跡へ戻る。
/// - プレイヤーの攻撃ではスタン/ノックバックしない(EnemySapper側)。ガードで弾かれた時だけスタンし、その時間は +6秒。
/// </summary>
class SapperBehavior final : public BaseEnemyBehavior {
public:
	explicit SapperBehavior(EnemySapper& owner);

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	bool IsEngaged() const override {
		Behavior b = GetBehavior();
		return b == Behavior::kMelee || b == Behavior::kLeap;
	}
	bool IsInMeleeRecovery() const override { return GetBehavior() == Behavior::kMelee && meleePhase_ == MeleePhase::kRecovery; }

	void InitializeBehaviorState(Behavior behavior) override;
	void UpdateBehaviorState() override;

protected:
	void BehaviorRootUpdate() override;

	// 外部要因(ガード成功のノックバック→スタン)のスタン時間に足す秒数
	float GetStunDurationBonus() const override { return kStunDurationBonus; }

private:
	enum class MeleePhase { kAnticipate, kAttack, kRecovery };
	enum class LeapPhase { kRise, kHang, kDive };

	void TickTimers();

	void BehaviorMeleeInitialize();
	void BehaviorMeleeUpdate();
	void AdvanceWhileAttacking();

	bool CanLeapTo(const KamataEngine::Vector3& from, const KamataEngine::Vector3& to) const;
	void BehaviorLeapInitialize();
	void BehaviorLeapUpdate();
	void FinishLeap();

	EnemySapper& sapperOwner_;

	// --- 近接 ---
	MeleePhase meleePhase_ = MeleePhase::kAnticipate;
	uint32_t attackParameter_ = 0;
	int comboIndex_ = 0;
	float meleeCooldownTimer_ = 0.0f;
	float noAttackTimer_ = 0.0f;

	// --- ホーミングジャンプ ---
	LeapPhase leapPhase_ = LeapPhase::kRise;
	KamataEngine::Vector3 leapTarget_{};
	float leapTime_ = 0.0f;
	float leapAnticipateTime_ = 0.1f; // 上昇+ホバーの合計(振りかぶりの進行度に使う)
	float hangTimer_ = 0.0f;
	float diveStartDistance_ = 1.0f;
	float leapCooldownTimer_ = 0.0f;

	// ---- 調整値(GlobalVariables) ----
	static inline float kMeleeEngageRange = 2.6f;
	static inline float kMeleeCooldown = 0.8f;
	static inline float kComboResetDuration = 3.0f;
	static inline float kAttackMoveSpeed = 0.05f; // 攻撃中の歩行速度

	static inline float kLeapRiseSpeed = 0.65f;         // 初速(BaseEnemyのジャンプ初速と同じ)
	static inline float kLeapGravity = 0.05f;           // 上昇中の重力(BaseEnemyと同じ)
	static inline float kLeapMaxHorizontalSpeed = 0.6f; // 上昇中の水平速度の上限
	static inline float kLeapHangDuration = 0.15f;      // 頂点で止まって振りかぶる時間(秒)
	static inline float kLeapDiveSpeed = 0.8f;          // 急降下の速度(1フレームあたり)
	static inline float kLeapMinRange = 5.0f;           // これより近いと近接/通常追跡
	static inline float kLeapMaxRange = 14.0f;          // これより遠いと届かない
	static inline float kLeapApexMargin = 0.5f;         // 頂点の高さからこれだけ低い所まで届く
	static inline float kLeapCooldown = 5.0f;
	static inline float kLeapMaxDuration = 3.0f;    // 安全弁
	static inline float kNearPlayerDistance = 1.5f; // この距離に入ったら判定を戻す
	static inline float kLandStunDuration = 1.5f;   // 着地後の自己スタン
	static inline float kStunDurationBonus = 6.0f;  // ガード成功時などのスタンに追加される秒数
};