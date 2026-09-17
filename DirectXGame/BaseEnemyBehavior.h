#pragma once

#define NOMINMAX

#include "Global.h"
#include "PathDetector.h"
#include "PathFindingAlgorithm.h"
#include "RandomGenerator.h"
#include "VisionSensor.h"

class BaseEnemy;
class GameScene;
class MapChipField;

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
	};

	enum class FlankPreference { kPreferShortest, kPreferFlanking, kNeutral };

	Behavior GetBehavior() const { return behavior_; }
	bool IsRootBehavior() const { return behavior_ == Behavior::kRoot; }
	bool IsDeathBehavior() const { return behavior_ == Behavior::kDeath; }
	void RequestBehavior(Behavior request) { behaviorRequest_ = request; }

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

	bool IsChasing() const;

	void ReceiveAlert(const KamataEngine::Vector3& alertedPosition);

	bool IsGroundAheadClear(int direction) const;
	bool ScanWallAhead(int direction, float maxLookAhead, float maxHeight, float& outDistance, float& outHeight) const;
	bool ScanGapAhead(int direction, float maxLookAhead, float& outGapDistance) const;
	bool CanJumpOverWall(int direction) const;
	bool HasReachableLedgeAhead(int direction) const;
	bool TryJumpIfLedgeAhead(int direction);

	void ApplySeparationFromOtherEnemies();

	void RequestKnockback(const KamataEngine::Vector3& direction, float power) {
		if (behavior_ == Behavior::kDeath || behaviorRequest_ == Behavior::kDeath)
			return;
		knockbackDirection_ = direction;
		knockbackPower_ = power;
		behaviorRequest_ = Behavior::kKnockback;
	}

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

protected:
	virtual void BehaviorRootInitialize();
	virtual void BehaviorRootUpdate();

	virtual void BehaviorIdleInitialize();
	virtual void BehaviorIdleUpdate();

	virtual void BehaviorRoamInitialize();
	virtual void BehaviorRoamUpdate();

	virtual void BehaviorDieInitialize();
	virtual void BehaviorDieUpdate();

	// [NEW]
	virtual void BehaviorKnockbackInitialize();
	virtual void BehaviorKnockbackUpdate();

	void IdleFacingUpdate();

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

	PathDetector pathSensor_;
	static inline float kWallLookAheadMargin = 0.35f;

	static inline RandomGenerator sRandom_;

	static inline bool kEndlessChase = false;

	VisionSensor visionSensor_;
	static inline float kVisionEyeHeight = 0.2f;

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
	// [NEW] knockback state
	KamataEngine::Vector3 knockbackDirection_{};
	float knockbackPower_ = 0.0f;
	uint32_t knockbackParameter_ = 0;
	static inline uint32_t kKnockbackDuration = 15;
	static inline float kKnockbackSpeedScale = 0.12f;
};