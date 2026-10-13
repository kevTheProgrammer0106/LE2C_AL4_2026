#pragma once

#define NOMINMAX

#include "Global.h"
#include "PathDetector.h"
#include "PathFindingAlgorithm.h"
#include "RandomGenerator.h"
#include "VisionSensor.h"
#include "OfficerBuff.h"

class BaseEnemy;
class GameScene;
class MapChipField;

/// Gun-aim tracker. Holds the world aim ANGLE that feeds the gun rotation.
/// - Spotted: angle == exact angle to the player.
/// - Lost: angle stays on the LAST SEEN position (recomputed from the moving origin).
/// - Re-spotted: angle lerps (shortest way round) to the live player, then returns to exact tracking.
struct AimTracker {
	void Reset(const KamataEngine::Vector3& origin, const KamataEngine::Vector3& livePos) {
		lastSeenPos_ = livePos;
		aimAngle_ = AngleTo(origin, livePos);
		lostTimer_ = 0.0f;
		wasSpotted_ = true;
		justReacquired_ = false;
		catchingUp_ = false;
		active_ = true;
	}

	void Update(bool spotted, const KamataEngine::Vector3& origin, const KamataEngine::Vector3& livePos, float catchUpRate, float dt = 1.0f / 60.0f) {
		if (!active_)
			Reset(origin, livePos);

		if (spotted) {
			if (!wasSpotted_) {
				justReacquired_ = true;
				catchingUp_ = true; // start lerping back to the player
			}
			lastSeenPos_ = livePos;
			lostTimer_ = 0.0f;
		} else {
			lostTimer_ += dt;
		}

		const float target = AngleTo(origin, lastSeenPos_);
		if (catchingUp_) {
			aimAngle_ += WrapAngle(target - aimAngle_) * std::clamp(catchUpRate, 0.0f, 1.0f);
			if (std::abs(WrapAngle(target - aimAngle_)) < 0.02f)
				catchingUp_ = false; // arrived: back to exact tracking
		} else {
			aimAngle_ = target;
		}
		wasSpotted_ = spotted;
	}
	const KamataEngine::Vector3& GetLastSeenPos() const { return lastSeenPos_; }
	bool ConsumeReacquired() {
		bool r = justReacquired_;
		justReacquired_ = false;
		return r;
	}
	void Deactivate() { active_ = false; }
	bool IsActive() const { return active_; }
	bool IsSightLost() const { return !wasSpotted_; }
	float GetLostTime() const { return lostTimer_; }
	KamataEngine::Vector2 GetAimDir() const { return {std::cos(aimAngle_), std::sin(aimAngle_)}; }

private:
	static float AngleTo(const KamataEngine::Vector3& from, const KamataEngine::Vector3& to) { return std::atan2(to.y - from.y, to.x - from.x); }
	static float WrapAngle(float a) {
		constexpr float kPi = std::numbers::pi_v<float>;
		while (a > kPi)
			a -= 2.0f * kPi;
		while (a <= -kPi)
			a += 2.0f * kPi;
		return a;
	}

	KamataEngine::Vector3 lastSeenPos_{};
	float aimAngle_ = 0.0f;
	float lostTimer_ = 0.0f;
	bool wasSpotted_ = true;
	bool justReacquired_ = false;
	bool catchingUp_ = false;
	bool active_ = false;
};

class BaseEnemyBehavior {
public:
	explicit BaseEnemyBehavior(BaseEnemy& owner) : owner_(owner) {}
	virtual ~BaseEnemyBehavior() = default;

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	enum class Behavior {
		kUnknown,
		kRoot,
		kIdle,
		kRoam,
		kDeath,
		kAiming,
		kReloading,
		kMelee,
		kDuel,
		kKnockback,
		kStunned,
		kBlock, 
		kCommand,
		kLeap
	};

	enum class FlankPreference { kPreferShortest, kPreferFlanking, kNeutral };

	Behavior GetBehavior() const { return behavior_; }
	bool IsRootBehavior() const { return behavior_ == Behavior::kRoot; }
	bool IsDeathBehavior() const { return behavior_ == Behavior::kDeath; }
	void RequestBehavior(Behavior request) {
		// デバッグ中は新しいAI状態への遷移だけを止める。
		// 現在のBehavior自体はUpdateされるため、武器アニメーションや
		// 現在の攻撃モーションを継続して確認できる。
		if (kDebugDecisionFreeze_) {
			return;
		}

		behaviorRequest_ = request;
	}

	virtual bool IsEngaged() const { return false; }

	virtual bool IsInAttackAnticipation() const { return false; }

	void SetFlankPreference(FlankPreference preference) { flankPreference_ = preference; }
	FlankPreference GetFlankPreference() const { return flankPreference_; }
	virtual FlankPreference GetDefaultFlankPreference() const { return FlankPreference::kPreferShortest; }

	PathFindingAlgorithm::PathCostProfile BuildPathCostProfile() const;

	void Initialize();

	void SetMapChipField(MapChipField* mapChipField) {
		visionSensor_.SetMapChipField(mapChipField);
		pathSensor_.SetMapChipField(mapChipField);
	}

	bool IsPlayerOnSameFloor() const;

	void UpdateVisionAndChaseRequest();
	void ResolveBehaviorRequest();
	void UpdateCurrentBehavior();

	virtual void InitializeBehaviorState(Behavior behavior);
	virtual void UpdateBehaviorState();

	bool IsPlayerSpotted() const { return visionSensor_.IsPlayerSpotted(); }
	const VisionSensor& GetVisionSensor() const { return visionSensor_; }
	VisionSensor& GetVisionSensor() { return visionSensor_; }
	const AimTracker& GetAimTracker() const { return aimTracker_; }

	bool IsChasing() const;

	void ReceiveAlert(const KamataEngine::Vector3& alertedPosition);

	bool IsGroundAheadClear(int direction) const;
	bool ScanWallAhead(int direction, float maxLookAhead, float maxHeight, float& outDistance, float& outHeight) const;
	bool ScanGapAhead(int direction, float maxLookAhead, float& outGapDistance) const;
	bool CanJumpOverWall(int direction) const;
	bool HasReachableLedgeAhead(int direction, float travelSpeedOverride = 0.0f) const;
	bool TryJumpIfLedgeAhead(int direction);

	void ApplySeparationFromOtherEnemies();

	void RequestKnockback(const KamataEngine::Vector3& direction, float power) {
		if (behavior_ == Behavior::kDeath || behaviorRequest_ == Behavior::kDeath)
			return;
		knockbackDirection_ = direction;
		knockbackPower_ = power;
		pendingStunAfterKnockback_ = kHitStunDuration + GetStunDurationBonus();
		behaviorRequest_ = Behavior::kKnockback;
	}
	void RequestStun(float duration) {
		if (behavior_ == Behavior::kDeath || behaviorRequest_ == Behavior::kDeath)
			return;
		stunDuration_ = duration + GetStunDurationBonus();
		behaviorRequest_ = Behavior::kStunned;
	}
	void ClearDebuffs() {
		pendingStunAfterKnockback_ = 0.0f; 
		if (behavior_ == Behavior::kStunned || behavior_ == Behavior::kKnockback)
			behaviorRequest_ = Behavior::kRoot;
		else if (behaviorRequest_ == Behavior::kStunned || behaviorRequest_ == Behavior::kKnockback)
			behaviorRequest_ = Behavior::kUnknown;
	}
	bool IsStunned() const { return behavior_ == Behavior::kStunned; }
	bool IsKnockedBack() const { return behavior_ == Behavior::kKnockback; }
	bool IsDebuffed() const { return IsStunned() || IsKnockedBack(); }

	
	enum class PostMode { kWander, kHoldLeft, kHoldRight };
	void SetPostMode(PostMode mode);
	PostMode GetPostMode() const { return postMode_; }

	// Instantly faces direction (0 = right, 1 = left, same as BaseEnemy::LRDirection): sets lrDirection_,
	// cancels any turn animation and writes the matching body rotation, so body and weapons agree on frame 1.
	void SnapFacing(int direction);

	bool HasLastKnownPosition() const { return hasLastKnownPosition_; }
	void SetHasLastKnownPosition(bool value) { hasLastKnownPosition_ = value; }
	const KamataEngine::Vector3& GetLastKnownPlayerPos() const { return lastKnownPlayerPos_; }
	void SetLastKnownPlayerPos(const KamataEngine::Vector3& pos) { lastKnownPlayerPos_ = pos; }
	float GetChaseTimeoutTimer() const { return chaseTimeoutTimer_; }
	void SetChaseTimeoutTimer(float value) { chaseTimeoutTimer_ = value; }
	float GetUnreachableTimer() const { return unreachableTimer_; }
	void SetUnreachableTimer(float value) { unreachableTimer_ = value; }

	static void SetEndlessChaseEnabled(bool enabled) { kEndlessChase = enabled; }
	static bool IsEndlessChaseEnabled() { return kEndlessChase; }

	uint32_t GetDeathParameter() const { return deathParameter_; }

	static float GetWallLookAheadMargin() { return kWallLookAheadMargin; }
	static float GetWaypointReachThreshold() { return kWaypointReachThreshold; }
	static float GetJumpNodeIgnoreDistance() { return kJumpNodeIgnoreDistance; }
	static float GetUnreachableGiveUpDuration() { return kUnreachableGiveUpDuration; }
	static float GetChaseTimeoutDuration() { return kChaseTimeoutDuration; }
	static float GetPlayerSideDeadZone() { return kPlayerSideDeadZone; }
	static float GetPathRestartWaitDuration() { return kPathRestartWaitDuration; }

	void SetPersistentAwareness(bool value) { persistentAwareness_ = value; }
	bool HasPersistentAwareness() const { return persistentAwareness_; }

	//デバッグツール
	static void SetDebugDecisionFreeze(bool frozen) { kDebugDecisionFreeze_ = frozen; }
	static bool IsDebugDecisionFreeze() { return kDebugDecisionFreeze_; }
	static void SetDebugMovementFreeze(bool frozen) { kDebugMovementFreeze_ = frozen; }
	static bool IsDebugMovementFreeze() { return kDebugMovementFreeze_; }

		bool IsBlocking() const { return behavior_ == Behavior::kBlock; }
	bool IsBlockPending() const { return behaviorRequest_ == Behavior::kBlock; }
	// Off cooldown and not dying (usable even while stunned, e.g. "block when stun ends")
	bool IsBlockReady() const { return !IsDeathBehavior() && behaviorRequest_ != Behavior::kDeath && blockCooldownTimer_ <= 0.0f; }
	bool CanBlockNow() const { return IsBlockReady() && !IsBlocking() && !IsBlockPending() && !IsDebuffed(); }
	float GetBlockCooldownRemaining() const { return blockCooldownTimer_; }
	// Rolls GetBlockChanceWhenHit(); on success queues kBlock. Called by BaseEnemy::TryBlockAttack.
	bool TryStartBlockOnHit();
	// 外部要因のスタンに足す秒数(通常は0。Sapperだけ+6秒)
	virtual float GetStunDurationBonus() const { return 0.0f; }

	static inline float kBlockDuration = 1.5f;
	static inline float kBlockCooldown = 10.0f; // counted from the END of the block

	bool RetreatAwayFrom(const KamataEngine::Vector3& threatPos, float speed);
	bool RetreatInDirection(int direction, float speed);
	bool TimedRetreatAwayFrom(const KamataEngine::Vector3& threatPos, float speed, float duration, float& timer);
	static float ScaleRetreatSpeed(float distance, float outerRange, float innerRange, float maxSpeed);

	// True while airborne from a hop, and for kMoveDecisionCooldown seconds after landing.
	// While true, AI must not make decisions (aim / fire / melee / state changes): just call ContinueCommittedMove().
	bool IsMoveCommitted() const;
	bool CanMakeDecision() const { return !IsMoveCommitted(); }
	// Repeats the last approach/retreat request (keeps the hop velocity forward/back in the air).
	void ContinueCommittedMove();

	// Ledge guard request, read by BaseEnemy::MapChipCollisionUpdate every frame.
	// active: guard on. speedLimit: guard only applies while |velocity.x| <= speedLimit (BlockDetector::LedgeGuard).
	struct LedgeGuardRequest {
		bool active = false;
		float speedLimit = 0.0f;
	};
	LedgeGuardRequest GetLedgeGuardRequest() const;
	// Override in melee behaviors: true while the current melee action is in its recovery phase.
	virtual bool IsInMeleeRecovery() const { return false; }

		// True when a chasing officer is nearby and this enemy has run ahead of it: it waits so the group moves together.
	bool ShouldHoldForEscort() const;
protected:
	PostMode postMode_ = PostMode::kWander;

	static inline float kEscortJoinRadius = 14.0f; // enemies this close to a chasing officer are "escorting" it
	static inline float kEscortLeash = 5.0f;       // max distance an escort may get ahead of the officer
	static inline float kEscortFreeRange = 8.0f;   // player this close: ignore the leash and fight

	float pendingStunAfterKnockback_ = 0.0f;
	static inline float kHitStunDuration = 0.6f; 

	static inline bool kDebugDecisionFreeze_ = false;
	static inline bool kDebugMovementFreeze_ = false;

	virtual void BehaviorRootInitialize();
	virtual void BehaviorRootUpdate();

	virtual void BehaviorIdleInitialize();
	virtual void BehaviorIdleUpdate();

	virtual void BehaviorRoamInitialize();
	virtual void BehaviorRoamUpdate();

	virtual void BehaviorDieInitialize();
	virtual void BehaviorDieUpdate();
	virtual void BehaviorKnockbackInitialize();
	virtual void BehaviorKnockbackUpdate();
	virtual void BehaviorStunnedInitialize();
	virtual void BehaviorStunnedUpdate();

	void IdleFacingUpdate();
	// Faces the player (no-op if there is none).
	void FacePlayer();
	// Sets forward (facing-side) horizontal velocity for a melee step. Stops short of the player
	// (kMeleeAdvanceStopGap) and before ledges, so lunges/swings never run through or off the map.
	void AdvanceTowardFacing(float speed);
	// Slowly walks toward target. Stops at ledges it can't cross, walls too tall to jump, and within stopDistance.
	// Small wall hops are handled by BaseEnemy::JumpMovement; ledge hops and platform drop-through are handled here.
	void ApproachPosition(const KamataEngine::Vector3& target, float speed, float stopDistance);

	KamataEngine::Vector3& OwnerVelocity();
	const KamataEngine::Vector3& OwnerVelocity() const;
	void RunPhysicsAndCollisionUpdate();
	GameScene* OwnerGameScene() const;
	void OwnerFaceDirection(int direction);
	void OwnerSetFirearmMode(int mode);

	BaseEnemy& owner_;

	Behavior behavior_ = Behavior::kRoam;
	Behavior behaviorRequest_ = Behavior::kUnknown;
	uint32_t deathParameter_ = 0;

	bool hasSpawnedDrop_ = false;

	FlankPreference flankPreference_ = FlankPreference::kPreferShortest;

	float idleTimer_ = 0.0f;
	float idleDuration_ = 0.0f;
	static inline float kIdleDurationMin = 1.5f;
	static inline float kIdleDurationMax = 3.5f;

	float idleScanTimer_ = 0.0f;
	static inline float kIdleScanInterval = 2.0f;

	float roamTimer_ = 0.0f;
	float roamWalkDuration_ = 0.0f;
	int roamDirection_ = 0;
	static inline float kRoamWalkDurationMin = 1.0f;
	static inline float kRoamWalkDurationMax = 2.5f;

	static inline float kLedgeCheckAheadDistance = 0.5f;
	static inline float kLedgeCheckDownOffset = 0.3f;
	static inline float kMaxCrossableGapDistance = 1.5f;

	// ---- Ground movement: approach and retreat share ONE calculation ----
	enum class MoveKind { kApproach, kRetreat };
	enum class MoveTerrain {
		kClear,    // flat ground behind/ahead: just walk
		kStepDown, // empty cell(s) but ground within maxStepDown below: staircase, walk off
		kJumpable, // wall of <= maxWallBlocks with headroom on top: hop over it
		kGap,      // real gap (or kill block in the drop)
		kBlocked,  // wall too tall or low ceiling
	};
	struct MoveProfile {
		int32_t maxWallBlocks = 1;
		int32_t maxStepDown = 2;
		float hopSpeed = 0.45f;       // vertical launch speed
		float hopForwardSpeed = 0.1f; // horizontal speed held in the air
		bool allowGapJump = false;
	};
	MoveProfile GetMoveProfile(MoveKind kind) const;
	MoveTerrain ClassifyMoveTerrain(int direction, const MoveProfile& profile) const;
	bool MoveAlongGround(int direction, float speed, MoveKind kind);
	void LaunchMoveHop(int direction, float forwardSpeed, float hopSpeed);
	void ClearMoveCommit();

	static inline float kMoveLookAheadDistance = 0.75f;   // how far ahead/behind the terrain column is sampled
	static inline int32_t kMoveMaxStepDown = 2;           // empty cells below the feet row that still count as a staircase
	static inline float kMoveHopForwardSpeed = 0.1f;      // horizontal speed during a hop
	static inline float kMoveFootSampleOffset = 0.1f;     // sample this far above the feet to find the feet row
	static inline float kMoveDecisionCooldown = 1.0f;     // seconds after landing before AI may decide again
	static inline int32_t kRetreatJumpableWallBlocks = 1; // retreat hops walls up to this height, taller ones stop it
	static inline float kRetreatHopSpeed = 0.45f;         // retreat hop launch speed (apex about 2.2 blocks)

	bool moveHopping_ = false;
	int moveHopDirection_ = 0;
	float moveHopSpeed_ = 0.0f;
	float moveDecisionCooldownTimer_ = 0.0f;
	MoveKind lastMoveKind_ = MoveKind::kRetreat;
	int lastMoveDirection_ = 0;
	float lastMoveSpeed_ = 0.0f;


	PathDetector pathSensor_;
	static inline float kWallLookAheadMargin = 0.35f;

	static inline RandomGenerator sRandom_;

	static inline bool kEndlessChase = false;

	VisionSensor visionSensor_;
	static inline float kVisionEyeHeight = 0.2f;

	AimTracker aimTracker_;

	static inline float kWaypointReachThreshold = 0.3f;
	static inline float kJumpNodeIgnoreDistance = 1.0f;
	static inline float kJumpHeightMargin = 0.15f;

	static inline float kAlertRadius = 8.0f;
	bool hasBroadcastAlertThisSighting_ = false;

	static inline float kSeparationRadius = 1.2f;
	static inline float kSeparationStrength = 0.03f;

	static inline float kPathRestartWaitDuration = 0.10f;
	static inline float kPlayerSideDeadZone = 0.15f;

	KamataEngine::Vector3 lastKnownPlayerPos_ = {};
	bool hasLastKnownPosition_ = false;
	bool persistentAwareness_ = false;

	float chaseTimeoutTimer_ = 0.0f;
	static inline float kChaseTimeoutDuration = 5.0f;

	float unreachableTimer_ = 0.0f;
	static inline float kUnreachableGiveUpDuration = 2.0f;

	float deathSpinSpeed_ = 0.0f;
	int deathBounceCount_ = 0;

	static inline float kDeathFlingSpeedScale = 0.22f;
	static inline float kDeathFlingUpBoost = 0.25f;
	static inline float kDeathMaxSpin = 0.5f;
	static inline uint32_t kDeathDuration = 90;
	static inline int kDeathMaxBounces = 3;
	static inline float kDeathBounceDamping = 0.45f;
	static inline float kDeathFriction = 0.25f;
	static inline float kDeathMaxFallSpeed = 0.75f;

	static inline float kFloorLevelTolerance = 0.6f;

	KamataEngine::Vector3 knockbackDirection_{};
	float knockbackPower_ = 0.0f;
	uint32_t knockbackParameter_ = 0;
	static inline uint32_t kKnockbackDuration = 15;
	static inline float kKnockbackSpeedScale = 0.12f;

	float stunTimer_ = 0.0f;
	float stunDuration_ = 0.0f;

	virtual float GetBlockChanceWhenHit() const { return 0.0f; } // 0 = never blocks on hit
	virtual bool ShouldBlockAfterStun() const { return false; }  // true = guaranteed block when stun ends
	virtual void BehaviorBlockInitialize();
	virtual void BehaviorBlockUpdate();

	float blockTimer_ = 0.0f;
	float blockCooldownTimer_ = 0.0f;

	static inline float kMeleeAdvanceStopGap = 0.9f;

	// During kMelee (outside recovery), the ledge guard only applies once |velocity.x| is at or below this.
	static inline float kLungeLedgeGuardSpeed = 0.1f;
};