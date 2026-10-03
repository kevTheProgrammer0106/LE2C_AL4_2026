#pragma once

#include "BaseEnemyBehavior.h"
#include "FirearmMusketBayonet.h"
#include "WeaponSabre.h"

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

	// プレイヤーとのXZ/Yではなく、このゲームの2D戦闘平面上の距離を取得する。
	float GetPlayerDistance() const;
	// 射撃中の距離に応じた横移動速度を計算する。
	float CalculateRetreatSpeed(float distance) const;

		// プレイヤーが存在しない場合はfalseを返す。
	bool TryGetPlayerDistance(float& outDistance) const;

	// --- kAiming ---
	float aimTimer_ = 0.0f;

	// 狙いを定めてから発砲するまでの時間。
	static inline float kAimDuration = 1.2f;

	// --- kReloading ---
	float retreatTimer_ = 0.0f;
	float retreatMoveTimer_ = 0.0f;

	// リロード中および射撃中の最大後退速度。
	static inline float kRetreatSpeed = 0.03f;

	// リロード開始直後だけ実際に後退する時間。
	static inline float kRetreatDuration = 0.75f;

	// 後退方向の崖・段差を検査する距離。
	static inline float kRetreatLookAheadDistance = 0.75f;

	// リロードが長引いた場合の安全弁。
	static inline float kMaxRetreatDuration = 4.0f;

	// --- kMelee ---
	MeleePhase meleePhase_ = MeleePhase::kAnticipate;
	uint32_t meleeParameter_ = 0;

	// ランジ開始時に向きを固定する。
	bool lungeFacingLocked_ = false;

	// --- combat distance ---

	// プレイヤーがこの距離以内に入ると銃剣へ移行する。
	static inline float kBayonetRange = 5.0f;

	// この距離以内では、射撃しながら徐々に後退する。
	// FiringRangeの半分を初期値にする。
	static inline float kRetreatRange = 20.0f;

	// Riflemanが交戦を開始できる最大距離。
	static inline float kFiringRange = 40.0f;

	WeaponSabre sabreComboReference_;
};