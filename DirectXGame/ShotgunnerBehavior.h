#pragma once

#include "BaseEnemyBehavior.h"

class EnemyShotgunner;

/// <summary>
/// EnemyShotgunner専用のBehaviorモジュール。RiflemanBehaviorから近接(銃剣)を除いたもの。
///
/// 未発見時はBaseEnemyBehaviorの通常AIを使用する。発見後は自分から追いかけず、射撃可能距離でその場から撃つ。
/// - プレイヤーが近づいたら、照準中でもリロード中でも常に後退する。
/// - 2回目以降にプレイヤーを視認し直した時は、照準タイマーが即座に満了して撃つ(kReacquireAimDuration = 0)。
/// - リロードはFirearmShotgunの敵用リロード時間(長い)が終わるまで抜けない。
/// </summary>
class ShotgunnerBehavior final : public BaseEnemyBehavior {
public:
	explicit ShotgunnerBehavior(EnemyShotgunner& owner);

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	bool IsEngaged() const override {
		Behavior b = GetBehavior();
		return b == Behavior::kAiming || b == Behavior::kReloading;
	}

	void InitializeBehaviorState(Behavior behavior) override;
	void UpdateBehaviorState() override;

protected:
	void BehaviorRootInitialize() override;
	void BehaviorRootUpdate() override;

private:
	void BehaviorAimingInitialize();
	void BehaviorAimingUpdate();
	void BehaviorReloadingInitialize();
	void BehaviorReloadingUpdate();

	// プレイヤーとの2D距離。プレイヤーがいなければfalse。
	bool TryGetPlayerDistance(float& outDistance) const;
	float GetPlayerDistance() const;

	EnemyShotgunner& shotgunnerOwner_;

	// --- kAiming ---
	float aimTimer_ = 0.0f;
	float currentAimDuration_ = 1.2f;

	static inline float kAimDuration = 1.2f;          // 初回発見時の照準時間
	static inline float kReacquireAimDuration = 0.0f; // 見失った後の再発見時: 残り照準時間の上限(0 = 即発砲)
	static inline float kLostSightChaseDelay = 3.0f;  // 見失ってから追跡に戻るまでの秒数
	static inline float kAimTrackSpeed = 0.15f;       // 再発見時に銃がプレイヤーへ追いつく速さ(毎フレームの補間率)

	// --- kReloading ---
	float retreatTimer_ = 0.0f;
	float retreatMoveTimer_ = 0.0f;

	static inline float kRetreatSpeed = 0.05f;      // 後退速度(近距離時は常にこの速度)
	static inline float kRetreatDuration = 0.75f;   // 発砲直後の「遠くても下がる」時間
	static inline float kMaxReloadDuration = 12.0f; // リロードが終わらない場合の安全弁

	// --- 距離 ---
	static inline float kCloseRange = 8.0f;   // これより近いと照準中/リロード中を問わず後退する
	static inline float kFiringRange = 25.0f; // 交戦開始距離(ペレットの射程に合わせて短め)

	// --- 見失い時 ---
	static inline float kApproachSpeed = 0.03f;
	static inline float kApproachStopDistance = 1.5f;
};