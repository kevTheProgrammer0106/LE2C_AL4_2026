#pragma once

#include "BaseEnemyBehavior.h"
#include "FirearmMusketBayonet.h"

class EnemyRifleman;

/// <summary>
/// EnemyRifleman専用のBehaviorモジュール。
///
/// 未発見時はBaseEnemyBehaviorの通常AIを使用する。
/// プレイヤーを発見した後は自分から追いかけず、射撃可能な距離までその場で待機する。
///
/// 近距離では銃剣突撃、中距離では射撃を行う。
/// リロード中はプレイヤーと反対方向へ短時間だけ後退するが、
/// 後方が崖・壁の場合はそれ以上下がらない。
/// </summary>
class RiflemanBehavior final : public BaseEnemyBehavior {
public:
	explicit RiflemanBehavior(EnemyRifleman& owner);

	bool IsEngaged() const override {
		Behavior b = GetBehavior();
		return b == Behavior::kAiming || b == Behavior::kReloading || b == Behavior::kMelee;
	}

	void InitializeBehaviorState(Behavior behavior) override;
	void UpdateBehaviorState() override;

	bool IsInAttackAnticipation() const override { return GetBehavior() == Behavior::kMelee && meleePhase_ == MeleePhase::kAnticipate; }

protected:
	void BehaviorRootInitialize() override;
	void BehaviorRootUpdate() override;

private:
	friend class DebugMenu;
	enum class MeleePhase { kAnticipate, kLunge, kRecovery };

	void BehaviorAimingInitialize();
	void BehaviorAimingUpdate();

	void BehaviorReloadingInitialize();
	void BehaviorReloadingUpdate();

	void BehaviorMeleeInitialize();
	void BehaviorMeleeUpdate();

	EnemyRifleman& riflemanOwner_;

	// --- kAiming ---
	float aimTimer_ = 0.0f;
	static inline float kAimDuration = 1.0f;

	// --- kReloading ---
	float retreatTimer_ = 0.0f;
	float retreatMoveTimer_ = 0.0f;

	static inline float kRetreatSpeed = 0.03f;

	// リロード中に実際に後退する時間。
	static inline float kRetreatDuration = 0.75f;

	// 崖・段差を検査する距離。
	static inline float kRetreatLookAheadDistance = 0.75f;

	// リロードが長引いた場合の安全弁。
	static inline float kMaxRetreatDuration = 4.0f;

	// --- kMelee ---
	MeleePhase meleePhase_ = MeleePhase::kAnticipate;
	uint32_t meleeParameter_ = 0;

	// ランジ開始時に向きを固定する。
	bool lungeFacingLocked_ = false;

	// --- combat distance ---
	static inline float kMeleeEngageRange = 5.0f;
	static inline float kFiringRange = 40.0f;
};