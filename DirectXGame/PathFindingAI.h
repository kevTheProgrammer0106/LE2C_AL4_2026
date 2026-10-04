// PathFindingAI.h
#pragma once

#include "MapChipField.h"
#include <KamataEngine.h>
#include <vector>

class BaseEnemy;

class PathFindingAI {
public:
	// owner: このAIを動かす敵本体。速度・接地状態・向き・索敵結果・GameSceneへの
	//        参照など、実体はすべてBaseEnemy側にあるため、それらを読み書きするために保持する。
	// mapChipField: パス探索に使うマップ情報（後からSetMapChipFieldで差し替え可）。
	void Initialize(BaseEnemy* owner, MapChipField* mapChipField);
	void Update();
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	const std::vector<MapChipField::IndexSet>& GetCurrentPath() const { return currentPath_; }

	// --- パスファインディング関連 ---
	void ChaseMovement();      // パスに沿ってプレイヤーを追跡する
	void UpdatePathToPlayer(); // 一定間隔でパスを再計算する
	// ChaseMovement用の加減速処理。プレイヤーの現在速度を目標速度にし、
	// 逆方向へ切り返す場合は瞬時反転ではなく一度減速させる。
	void ApplyChaseAcceleration(float desiredDir);

	// targetPosが空中(ジャンプ中など)にある場合でも、そのX位置で実際に立てる床のタイルを
	// ゴールとして返す。真下方向にブロックが見つかるまで探索し、その1つ上を返す。
	MapChipField::IndexSet GetGroundedGoalIndex(const KamataEngine::Vector3& targetPos) const;

	// プレイヤーが空中にいる場合でも、現在の位置と速度から弾道を前方シミュレーションし、
	// 実際に着地するタイルを予測する。
	MapChipField::IndexSet PredictPlayerLandingIndex(const KamataEngine::Vector3& playerPos, const KamataEngine::Vector3& playerVelocity) const;

	// プレイヤーが敵の頭上を越えて反対側へ移動したか判定する
	void CheckPlayerCrossedOverhead();

	// 古い追跡情報を破棄し、短時間停止して経路探索を再開始する
	void RestartChasePath();

	// 直接視認できていて、かつ水平移動だけでプレイヤーへ到達できるか判定する
	bool CanReachPlayerDirectly(const KamataEngine::Vector3& playerPos) const;

		// 現在「直接追跡」モード(同じ足場でプレイヤーに届く)かどうかを外部から参照する
	bool IsDirectChaseActive() const { return isDirectChaseActive_; }
	// BaseEnemy::JumpMovementが、経路探索に任せるべきか自分で先読みジャンプしてよいかを
	// 判断するために必要。private化されたisDirectChaseActive_をそのまま外に晒さず、
	// 読み取り専用のgetter経由でのみ渡す。

	// J2 targeted-landing jump support.
	// Scans the loaded map for a kJumpLandTarget ('L') tile whose linkID matches the
	// J2 launch tile's linkID, mirroring how SlidingGate/kSwitch pair via buttonID.
	bool FindJumpLandingTarget(uint8_t linkID, KamataEngine::Vector3& outPosition) const;
	// Solves an initial velocity (vx constant, vy0) that — under the same per-frame
	// gravity integration BaseEnemy already uses (velocity.y -= g each frame, then
	// position += velocity) — lands approximately on targetPos starting from startPos.
	// Returns false if there's no meaningful horizontal distance to solve against.
	bool TryComputeTargetedJumpVelocity(const KamataEngine::Vector3& startPos, const KamataEngine::Vector3& targetPos, KamataEngine::Vector3& outVelocity) const;

	static constexpr uint8_t kTargetedJumpSubID = 2; // J2

	// sanity cap on the solved horizontal speed — if hitting the target
	// tile in the fixed-vy0 flight time would require an absurd horizontal
	// speed (e.g. a very short/steep hop), fall back to a normal untargeted
	// jump instead of firing the enemy sideways like a cannon.
	static inline float kMaxTargetedJumpHorizontalSpeed = 0.6f;

	// Player is well below and we're standing on a platform: drop through instead of walking around.
	bool ShouldDropThroughToPlayer(const KamataEngine::Vector3& playerPos) const;
	// Something safe (not a kill block / void) to land on directly below this x.
	bool IsDropLandingSafe(const KamataEngine::Vector3& pos) const;

	static inline float kDropPlayerBelowThreshold = 1.2f;  // how far below the enemy the player must be
	static inline float kDropMaxHorizontalDistance = 8.0f; // only drop if the player is roughly beneath us
private:
	// このAIの所有者(敵本体)。velocity_/onGround_/FaceDirection等のprotectedメンバへは
	// BaseEnemy側の "friend class PathFindingAI;" を通じて直接アクセスする。
	BaseEnemy* owner_ = nullptr;

	std::vector<MapChipField::IndexSet> currentPath_;
	size_t currentPathIndex_ = 0;

	MapChipField::IndexSet previousGoalIndex_{};
	bool hasPreviousGoalIndex_ = false; // 現状未使用（将来ゴール不変時の再計算スキップ用に予約）

	MapChipField* mapChipField_ = nullptr;

	bool isDirectChaseActive_ = false; // 現在「直接追跡」モードで経路探索をスキップしているか

	// 前フレームでプレイヤーが敵のどちら側にいたか
	// -1 = 左側、0 =ほぼ真上、1 = 右側
	int previousPlayerSide_ = 0;

	// 頭上通過を検出した後、短時間停止してから経路を再計算する
	float pathRestartTimer_ = 0.0f;

	// trueの間は古い経路を使わず、プレイヤー位置の安定を待つ
	bool isWaitingForPathRestart_ = false;


	// 注: hasLastKnownPosition_ / lastKnownPlayerPos_ / chaseTimeoutTimer_ / unreachableTimer_ は
	// ここでは複製しない。索敵の生死に関する状態はBaseEnemy側が単一の真実の情報源であり、
	// ここで別コピーを持つと同期漏れ(片方だけ更新されて片方は永遠にfalseのまま等)の原因になる。
	// 必ず owner_->hasLastKnownPosition_ 等、owner_経由で参照/更新する。
};