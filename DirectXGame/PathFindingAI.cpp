// PathFindingAI.cpp
#define NOMINMAX

#include "PathFindingAI.h"
#include "BaseEnemy.h"
#include "GameScene.h"
#include "Player.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

using LRDirection = BaseEnemy::LRDirection;
using Behavior = BaseEnemy::Behavior;

void PathFindingAI::Initialize(BaseEnemy* owner, MapChipField* mapChipField) {
	owner_ = owner;
	mapChipField_ = mapChipField;
}

void PathFindingAI::Update() {
	if (!owner_) {
		return;
	}
	UpdatePathToPlayer();
	ChaseMovement();
}

void PathFindingAI::UpdatePathToPlayer() {
	if (!mapChipField_ || !owner_->GetBehaviorModule().HasLastKnownPosition()) {
		currentPath_.clear();
		currentPathIndex_ = 0;
		isDirectChaseActive_ = false;
		previousPlayerSide_ = 0;
		isWaitingForPathRestart_ = false;
		return;
	}

	CheckPlayerCrossedOverhead();

	if (!owner_->onGround_) {
		return;
	}

	if (isWaitingForPathRestart_) {
		pathRestartTimer_ -= 1.0f / 60.0f;
		if (pathRestartTimer_ > 0.0f) {
			return;
		}
		isWaitingForPathRestart_ = false;
	}

	Player* livePlayer = owner_->gameScene_ ? owner_->gameScene_->GetPlayer() : nullptr;

	if (livePlayer && owner_->IsPlayerSpotted() && CanReachPlayerDirectly(livePlayer->GetWorldPosition())) {
		currentPath_.clear();
		currentPathIndex_ = 0;
		isDirectChaseActive_ = true;
		return;
	}
	isDirectChaseActive_ = false;

	MapChipField::IndexSet startIndex = mapChipField_->GetMapChipIndexSetByPosition(owner_->GetWorldPosition());
	MapChipField::IndexSet goalIndex =
	    livePlayer ? PredictPlayerLandingIndex(livePlayer->GetWorldPosition(), livePlayer->GetVelocity()) : GetGroundedGoalIndex(owner_->GetBehaviorModule().GetLastKnownPlayerPos());

	PathFindingAlgorithm::PathCostProfile costProfile = owner_->BuildPathCostProfile();
	currentPath_ = PathFindingAlgorithm::FindPath(mapChipField_, startIndex, goalIndex, 2000, &costProfile);
	currentPathIndex_ = 0;
}

void PathFindingAI::ChaseMovement() {
	if (!owner_ || !mapChipField_) {
		return;
	}

	Vector3 currentPos = owner_->GetWorldPosition();
	Player* livePlayer = owner_->gameScene_ ? owner_->gameScene_->GetPlayer() : nullptr;

	if (livePlayer) {
		owner_->FaceDirection(livePlayer->GetWorldPosition().x < currentPos.x ? LRDirection::kLeft : LRDirection::kRight);
	}

	if (currentPath_.empty()) {
		if (!livePlayer) {
			owner_->velocity_.x = 0.0f;
			return;
		}

		float diff = livePlayer->GetWorldPosition().x - currentPos.x;
		if (std::abs(diff) < BaseEnemyBehavior::GetWaypointReachThreshold()) {
			owner_->velocity_.x *= (1.0f - BaseEnemy::kAttenuation);
		} else {
			ApplyChaseAcceleration((diff < 0.0f) ? -1.0f : 1.0f);
		}

		if (isDirectChaseActive_) {
			owner_->GetBehaviorModule().SetUnreachableTimer(0.0f);
			return;
		}

		float unreachable = owner_->GetBehaviorModule().GetUnreachableTimer() + 1.0f / 60.0f;
		owner_->GetBehaviorModule().SetUnreachableTimer(unreachable);
		if (unreachable >= BaseEnemyBehavior::GetUnreachableGiveUpDuration()) {
			owner_->velocity_.x = 0.0f;
			owner_->GetBehaviorModule().SetHasLastKnownPosition(false);
			owner_->GetBehaviorModule().SetChaseTimeoutTimer(BaseEnemyBehavior::GetChaseTimeoutDuration());
			owner_->GetBehaviorModule().SetUnreachableTimer(0.0f);
			owner_->GetBehaviorModule().RequestBehavior(Behavior::kRoam);
		}
		return;
	}

	owner_->GetBehaviorModule().SetUnreachableTimer(0.0f);

	MapChipField::IndexSet currentIndex = mapChipField_->GetMapChipIndexSetByPosition(currentPos);
	while (currentPathIndex_ < currentPath_.size()) {
		const MapChipField::IndexSet& waypointIndex = currentPath_[currentPathIndex_];
		Vector3 waypointPos = mapChipField_->GetMapChipPositionByIndex(waypointIndex.xIndex, waypointIndex.yIndex);

		bool sameRow = (currentIndex.yIndex == waypointIndex.yIndex);
		bool closeX = std::abs(waypointPos.x - currentPos.x) < BaseEnemyBehavior::GetWaypointReachThreshold();
		if (sameRow && closeX) {
			++currentPathIndex_;
		} else {
			break;
		}
	}

	if (currentPathIndex_ >= currentPath_.size()) {
		if (livePlayer) {
			float diff = livePlayer->GetWorldPosition().x - currentPos.x;
			if (std::abs(diff) < BaseEnemyBehavior::GetWaypointReachThreshold()) {
				owner_->velocity_.x *= (1.0f - BaseEnemy::kAttenuation);
			} else {
				ApplyChaseAcceleration((diff < 0.0f) ? -1.0f : 1.0f);
			}
		} else {
			owner_->velocity_.x *= (1.0f - BaseEnemy::kAttenuation);
		}
		return;
	}

	MapChipField::IndexSet waypoint = currentPath_[currentPathIndex_];
	Vector3 waypointPos = mapChipField_->GetMapChipPositionByIndex(waypoint.xIndex, waypoint.yIndex);

	bool jumpNodeTriggered = false;

	if (owner_->onGround_) {
		MapChipType currentTileType = mapChipField_->GetMapChipTypeByIndex(currentIndex.xIndex, currentIndex.yIndex);
		if (currentTileType == MapChipType::kJumpNode) {
			uint8_t jumpTier = mapChipField_->GetMapChipLinkIDByIndex(currentIndex.xIndex, currentIndex.yIndex); // linkID=優先度tier(旧subID) / J2ではランディング先とのペアID
			uint8_t jumpKind = mapChipField_->GetMapChipSubIDByIndex(currentIndex.xIndex, currentIndex.yIndex);  // [NEW] subID: kTargetedJumpSubID(2) = J2, ランディング指定ジャンプ
			bool isAlwaysJump = PathFindingAlgorithm::IsAlwaysJumpSubID(jumpTier);

			bool playerVeryNear = false;
			if (!isAlwaysJump && livePlayer) {
				float distToPlayerX = std::abs(livePlayer->GetWorldPosition().x - currentPos.x);
				playerVeryNear = distToPlayerX < BaseEnemyBehavior::GetJumpNodeIgnoreDistance();
			}

			if (isAlwaysJump || !playerVeryNear) {
				// [NEW] J2: linkID経由でペアのkJumpLandTarget('L')を探し、そこへ正確に着地する初速を解いて設定する。
				// 見つからない/水平距離がほぼ0で解けない場合は従来通りの単純な垂直加速にフォールバックする。
				bool usedTargetedJump = false;
				if (jumpKind == kTargetedJumpSubID) {
					Vector3 landingPos{};
					if (FindJumpLandingTarget(jumpTier, landingPos)) {
						Vector3 solvedVelocity{};
						if (TryComputeTargetedJumpVelocity(currentPos, landingPos, solvedVelocity)) {
							owner_->velocity_ = solvedVelocity;
							usedTargetedJump = true;
						}
					}
				}
				if (!usedTargetedJump) {
					owner_->velocity_.y += BaseEnemy::kJumpAcceleration;
				}
				jumpNodeTriggered = true;
			}
		}
	}

	if (!jumpNodeTriggered && owner_->onGround_ && currentIndex.yIndex > waypoint.yIndex && (currentIndex.yIndex - waypoint.yIndex) == 1) {
		owner_->velocity_.y += BaseEnemy::kJumpAcceleration;
	}

	float desiredDir = 0.0f;
	if (waypointPos.x < currentPos.x - 0.01f) {
		desiredDir = -1.0f;
	} else if (waypointPos.x > currentPos.x + 0.01f) {
		desiredDir = 1.0f;
	}

	ApplyChaseAcceleration(desiredDir);
}

bool PathFindingAI::FindJumpLandingTarget(uint8_t linkID, Vector3& outPosition) const {
	if (!mapChipField_) {
		return false;
	}
	uint32_t width = mapChipField_->GetNumBlockHorizontal();
	uint32_t height = mapChipField_->GetNumBlockVertical();
	for (uint32_t y = 0; y < height; ++y) {
		for (uint32_t x = 0; x < width; ++x) {
			if (mapChipField_->GetMapChipTypeByIndex(x, y) != MapChipType::kJumpLandTarget) {
				continue;
			}
			if (mapChipField_->GetMapChipLinkIDByIndex(x, y) != linkID) {
				continue;
			}
			outPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
			return true;
		}
	}
	return false;
}

bool PathFindingAI::TryComputeTargetedJumpVelocity(const Vector3& startPos, const Vector3& targetPos, Vector3& outVelocity) const {
	float dx = targetPos.x - startPos.x;
	float dy = targetPos.y - startPos.y;

	if (std::abs(dx) < 0.0001f) {
		return false; // no horizontal distance to time the arc against
	}

	// [FIX] Softer, consistent jump: fix the vertical launch speed to the same
	// value every other enemy jump uses, instead of solving it from distance /
	// current horizontal speed. Solving vy0 from a short or near-zero horizontal
	// speed produced huge, "accelerated"-looking launches. With vy0 fixed, the
	// arc's height and hang-time are always the same familiar jump — only the
	// horizontal speed adapts to cover whatever distance is needed, mirroring
	// how the reference logic keeps velocity.y constant (30.0) and only solves
	// requiredVelocityX from distance / a known flight time.
	float vy0 = BaseEnemy::kJumpAcceleration;
	float g = BaseEnemy::kGravityAcceleration;

	// Solve flight time t from: dy = vy0*t - 0.5*g*t^2
	//   =>  0.5*g*t^2 - vy0*t + dy = 0
	// (The reference's constant flightTime=75 is the special case dy=0, i.e.
	// t = 2*vy0/g; here we solve the general quadratic so a landing tile above
	// or below the launch tile still gets a correct — not just same-height — time.)
	float a = 0.5f * g;
	float b = -vy0;
	float c = dy;
	float discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0.0f) {
		return false; // landing tile sits higher than this launch speed can reach
	}
	float sqrtDisc = std::sqrt(discriminant);
	// Larger root = falling back through the target height after the apex,
	// i.e. landing on the way down (not clipping it on the way up).
	float t = (-b + sqrtDisc) / (2.0f * a);
	if (t <= 0.0001f) {
		return false;
	}

	float vx0 = dx / t;
	if (std::abs(vx0) > kMaxTargetedJumpHorizontalSpeed) {
		return false; // unreasonably fast hop — let the caller fall back to a normal jump
	}

	outVelocity = {vx0, vy0, 0.0f};
	return true;
}

MapChipField::IndexSet PathFindingAI::GetGroundedGoalIndex(const Vector3& targetPos) const {
	if (!mapChipField_) {
		return MapChipField::IndexSet{0, 0};
	}

	MapChipField::IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(targetPos);

	uint32_t numRows = mapChipField_->GetNumBlockVertical();
	for (uint32_t step = 0; step < numRows; ++step) {
		uint32_t belowY = index.yIndex + 1;
		if (belowY >= numRows) {
			break;
		}
		if (MapChipField::IsGroundType(mapChipField_->GetMapChipTypeByIndex(index.xIndex, belowY))) {
			break;
		}
		index.yIndex = belowY;
	}
	return index;
}

MapChipField::IndexSet PathFindingAI::PredictPlayerLandingIndex(const Vector3& playerPos, const Vector3& playerVelocity) const {
	if (!mapChipField_) {
		return MapChipField::IndexSet{0, 0};
	}

	Vector3 simPos = playerPos;
	Vector3 simVel = playerVelocity;

	constexpr uint32_t kMaxSimSteps = 180;
	for (uint32_t step = 0; step < kMaxSimSteps; ++step) {
		simVel.y -= BaseEnemy::kGravityAcceleration;
		simVel.y = std::max(simVel.y, -BaseEnemy::kLimitFallSpeed);
		simPos += simVel;

		MapChipField::IndexSet posIndex = mapChipField_->GetMapChipIndexSetByPosition(simPos);
		uint32_t belowY = posIndex.yIndex + 1;
		if (belowY < mapChipField_->GetNumBlockVertical() && MapChipField::IsGroundType(mapChipField_->GetMapChipTypeByIndex(posIndex.xIndex, belowY))) {
			return posIndex;
		}
	}

	return GetGroundedGoalIndex(simPos);
}

bool PathFindingAI::CanReachPlayerDirectly(const Vector3& playerPos) const {
	if (!mapChipField_ || !owner_) {
		return false;
	}

	Vector3 currentPos = owner_->GetWorldPosition();

	static constexpr float kDirectReachHeightTolerance = 0.5f;
	if (std::abs(playerPos.y - currentPos.y) > kDirectReachHeightTolerance) {
		return false;
	}

	float horizontalDistance = std::abs(playerPos.x - currentPos.x);
	if (horizontalDistance < 0.0001f) {
		return true;
	}

	LRDirection direction = (playerPos.x < currentPos.x) ? LRDirection::kLeft : LRDirection::kRight;

	float gapDistance = 0.0f;
	if (owner_->GetBehaviorModule().ScanGapAhead(static_cast<int>(direction), horizontalDistance, gapDistance)) {
		return false;
	}

	float wallDistance = 0.0f;
	float wallHeight = 0.0f;
	if (owner_->GetBehaviorModule().ScanWallAhead(static_cast<int>(direction), horizontalDistance, BaseEnemy::kHeight, wallDistance, wallHeight)) {
		return false;
	}

	return true;
}
void PathFindingAI::CheckPlayerCrossedOverhead() {
	if (!owner_) {
		return;
	}
	Player* livePlayer = owner_->gameScene_ ? owner_->gameScene_->GetPlayer() : nullptr;
	if (!livePlayer) {
		return;
	}

	float diff = livePlayer->GetWorldPosition().x - owner_->GetWorldPosition().x;

	int newSide = previousPlayerSide_;
	if (diff > BaseEnemyBehavior::GetPlayerSideDeadZone()) {
		newSide = 1;
	} else if (diff < -BaseEnemyBehavior::GetPlayerSideDeadZone()) {
		newSide = -1;
	}

	if (previousPlayerSide_ != 0 && newSide != 0 && newSide != previousPlayerSide_) {
		RestartChasePath();
	}

	previousPlayerSide_ = newSide;
}

void PathFindingAI::RestartChasePath() {
	currentPath_.clear();
	currentPathIndex_ = 0;
	isDirectChaseActive_ = false;
	isWaitingForPathRestart_ = true;
	pathRestartTimer_ = BaseEnemyBehavior::GetPathRestartWaitDuration();

	if (owner_) {
		owner_->GetBehaviorModule().SetUnreachableTimer(0.0f);
		owner_->velocity_.x = 0.0f;
	}
}

void PathFindingAI::ApplyChaseAcceleration(float desiredDir) {
	if (!owner_) {
		return;
	}

	float speedFactor = 1.0f;
	if (owner_->mapChipField_) {
		Vector3 footPos = owner_->GetWorldPosition() + Vector3(0.0f, -(BaseEnemy::kHeight / 2.0f) - 0.05f, 0.0f);
		if (owner_->mapChipField_->IsSlowAtPosition(footPos)) {
			speedFactor = MapChipField::kSlowSpeedFactor;
		}
	}

	float targetSpeed = BaseEnemy::kChaseSpeed * speedFactor;

	if (desiredDir == 0.0f) {
		owner_->velocity_.x *= (1.0f - BaseEnemy::kAttenuation);
		if (std::abs(owner_->velocity_.x) < 0.001f) {
			owner_->velocity_.x = 0.0f;
		}
		return;
	}

	bool movingOppositeDir = (desiredDir > 0.0f && owner_->velocity_.x < 0.0f) || (desiredDir < 0.0f && owner_->velocity_.x > 0.0f);
	if (movingOppositeDir) {
		owner_->velocity_.x *= (1.0f - BaseEnemy::kAttenuation);
		if (std::abs(owner_->velocity_.x) < 0.001f) {
			owner_->velocity_.x = 0.0f;
			movingOppositeDir = false;
		}
	}

	if (!movingOppositeDir) {
		owner_->velocity_.x += desiredDir * BaseEnemy::kAcceleration * speedFactor;
		owner_->velocity_.x = std::clamp(owner_->velocity_.x, -targetSpeed, targetSpeed);
	}
}