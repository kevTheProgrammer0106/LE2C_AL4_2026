#include "BaseEnemyBehavior.h"
#include "BaseEnemy.h"
#include "GameScene.h"
#include "GlobalVariables.h"
#include "MapChipField.h"
#include "Player.h"
#include "ObjectBounce.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "BaseEnemyBehavior";
}

namespace {
// Walls for ground movement: anything solid that blocks sideways. One-way platforms never do.
bool IsMoveWall(MapChipType type) { return MapChipField::IsGroundType(type) && type != MapChipType::kPlatform; }
} // namespace


void BaseEnemyBehavior::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);

	gv->AddItem(kGroupName, "xIdleDurationMin", kIdleDurationMin);
	gv->AddItem(kGroupName, "xIdleDurationMax", kIdleDurationMax);
	gv->AddItem(kGroupName, "xRoamWalkDurationMin", kRoamWalkDurationMin);
	gv->AddItem(kGroupName, "xRoamWalkDurationMax", kRoamWalkDurationMax);
	gv->AddItem(kGroupName, "xChaseTimeoutDuration", kChaseTimeoutDuration);
	gv->AddItem(kGroupName, "xAlertRadius", kAlertRadius);
	gv->AddItem(kGroupName, "xSeparationRadius", kSeparationRadius);
	gv->AddItem(kGroupName, "xSeparationStrength", kSeparationStrength);
	gv->AddItem(kGroupName, "xFloorLevelTolerance", kFloorLevelTolerance);
	gv->AddItem(kGroupName, "xKnockbackDuration", static_cast<int32_t>(kKnockbackDuration));
	gv->AddItem(kGroupName, "xKnockbackSpeedScale", kKnockbackSpeedScale);
	gv->AddItem(kGroupName, "xHitStunDuration", kHitStunDuration);
	gv->AddItem(kGroupName, "xBlockDuration", kBlockDuration);
	gv->AddItem(kGroupName, "xBlockCooldown", kBlockCooldown);
	gv->AddItem(kGroupName, "xMoveLookAheadDistance", kMoveLookAheadDistance);
	gv->AddItem(kGroupName, "xMoveMaxStepDown", kMoveMaxStepDown);
	gv->AddItem(kGroupName, "xMoveHopForwardSpeed", kMoveHopForwardSpeed);
	gv->AddItem(kGroupName, "xMoveDecisionCooldown", kMoveDecisionCooldown);
	gv->AddItem(kGroupName, "xRetreatJumpableWallBlocks", kRetreatJumpableWallBlocks);
	gv->AddItem(kGroupName, "xRetreatHopSpeed", kRetreatHopSpeed);
	gv->AddItem(kGroupName, "xLungeLedgeGuardSpeed", kLungeLedgeGuardSpeed);
}

void BaseEnemyBehavior::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();

	kIdleDurationMin = gv->GetFloatValue(kGroupName, "xIdleDurationMin");
	kIdleDurationMax = gv->GetFloatValue(kGroupName, "xIdleDurationMax");
	kRoamWalkDurationMin = gv->GetFloatValue(kGroupName, "xRoamWalkDurationMin");
	kRoamWalkDurationMax = gv->GetFloatValue(kGroupName, "xRoamWalkDurationMax");
	kChaseTimeoutDuration = gv->GetFloatValue(kGroupName, "xChaseTimeoutDuration");
	kAlertRadius = gv->GetFloatValue(kGroupName, "xAlertRadius");
	kSeparationRadius = gv->GetFloatValue(kGroupName, "xSeparationRadius");
	kSeparationStrength = gv->GetFloatValue(kGroupName, "xSeparationStrength");
	kFloorLevelTolerance = gv->GetFloatValue(kGroupName, "xFloorLevelTolerance");
	kKnockbackDuration = static_cast<uint32_t>(gv->GetIntValue(kGroupName, "xKnockbackDuration"));
	kKnockbackSpeedScale = gv->GetFloatValue(kGroupName, "xKnockbackSpeedScale");
	kHitStunDuration = gv->GetFloatValue(kGroupName, "xHitStunDuration");
	kBlockDuration = gv->GetFloatValue(kGroupName, "xBlockDuration");
	kBlockCooldown = gv->GetFloatValue(kGroupName, "xBlockCooldown");
	kMoveLookAheadDistance = gv->GetFloatValue(kGroupName, "xMoveLookAheadDistance");
	kMoveMaxStepDown = gv->GetIntValue(kGroupName, "xMoveMaxStepDown");
	kMoveHopForwardSpeed = gv->GetFloatValue(kGroupName, "xMoveHopForwardSpeed");
	kMoveDecisionCooldown = gv->GetFloatValue(kGroupName, "xMoveDecisionCooldown");
	kRetreatJumpableWallBlocks = gv->GetIntValue(kGroupName, "xRetreatJumpableWallBlocks");
	kRetreatHopSpeed = gv->GetFloatValue(kGroupName, "xRetreatHopSpeed");
	kLungeLedgeGuardSpeed = gv->GetFloatValue(kGroupName, "xLungeLedgeGuardSpeed");
}

void BaseEnemyBehavior::Initialize() {
	sRandom_.Initialize();
	chaseTimeoutTimer_ = kChaseTimeoutDuration;
	hasLastKnownPosition_ = false;
	flankPreference_ = GetDefaultFlankPreference();
	blockTimer_ = 0.0f;
	blockCooldownTimer_ = 0.0f;
	behavior_ = Behavior::kIdle;
	ClearMoveCommit();
	BehaviorIdleInitialize();
}

PathFindingAlgorithm::PathCostProfile BaseEnemyBehavior::BuildPathCostProfile() const {
	PathFindingAlgorithm::PathCostProfile profile = PathFindingAlgorithm::GetDefaultPathCostProfile();
	switch (flankPreference_) {
	case FlankPreference::kPreferFlanking:
		std::reverse(profile.pathNodeCosts.begin(), profile.pathNodeCosts.end());
		break;
	case FlankPreference::kNeutral:
		profile.pathNodeCosts.fill(1.0f);
		break;
	default:
		break;
	}
	return profile;
}

void BaseEnemyBehavior::UpdateVisionAndChaseRequest() {
	if (kDebugDecisionFreeze_) {
		return;
	}

	if (!owner_.isDead_ && behavior_ != Behavior::kDeath && behaviorRequest_ != Behavior::kDeath) {

		if (owner_.gameScene_ && owner_.gameScene_->GetPlayer() && owner_.gameScene_->GetPlayer()->IsDefeated()) {
			Vector3 eyePos = owner_.GetWorldPosition() + Vector3(0.0f, kVisionEyeHeight, 0.0f);
			visionSensor_.CheckPlayerSpotted(eyePos, Vector3(1.0f, 0.0f, 0.0f), nullptr); // null player clears "spotted"
			hasLastKnownPosition_ = false;
			chaseTimeoutTimer_ = kChaseTimeoutDuration;
			hasBroadcastAlertThisSighting_ = false;

			bool fighting = behavior_ == Behavior::kRoot || behavior_ == Behavior::kAiming || behavior_ == Behavior::kReloading || behavior_ == Behavior::kMelee || behavior_ == Behavior::kBlock;
			bool debuffPending = (behaviorRequest_ == Behavior::kKnockback || behaviorRequest_ == Behavior::kStunned);
			if (fighting && !debuffPending) {
				behaviorRequest_ = Behavior::kIdle;
			}
			return;
		}
		Vector3 eyePos = owner_.GetWorldPosition() + Vector3(0.0f, kVisionEyeHeight, 0.0f);
		Vector3 baseFacingDir = owner_.isRight() ? Vector3(1.0f, 0.0f, 0.0f) : Vector3(-1.0f, 0.0f, 0.0f);
		Vector3 facingDir = baseFacingDir;

		if (visionSensor_.IsPlayerSpotted() && owner_.gameScene_ && owner_.gameScene_->GetPlayer()) {
			Vector3 toPlayer = owner_.gameScene_->GetPlayer()->GetWorldPosition() - eyePos;
			float horizontalDist = std::sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
			if (horizontalDist > 0.0001f) {
				Vector3 toPlayerDir = toPlayer / horizontalDist;
				float forwardSign = toPlayerDir.x * baseFacingDir.x;
				if (forwardSign >= 0.0f)
					facingDir = toPlayerDir;
			}
		}

		if (owner_.gameScene_) {
			visionSensor_.CheckPlayerSpotted(eyePos, facingDir, owner_.gameScene_->GetPlayer());

			if (visionSensor_.IsPlayerSpotted()) {
				chaseTimeoutTimer_ = 0.0f;
				if (owner_.gameScene_->GetPlayer()) {
					lastKnownPlayerPos_ = owner_.gameScene_->GetPlayer()->GetWorldPosition();
					hasLastKnownPosition_ = true;
				}
				if (!hasBroadcastAlertThisSighting_) {
					hasBroadcastAlertThisSighting_ = true;
					owner_.gameScene_->AlertNearbyEnemies(&owner_, lastKnownPlayerPos_, kAlertRadius);
				}
			} else {
				chaseTimeoutTimer_ += 1.0f / 60.0f;
				hasBroadcastAlertThisSighting_ = false;
				if ((kEndlessChase || persistentAwareness_) && owner_.gameScene_->GetPlayer()) {
					if (persistentAwareness_)
						hasLastKnownPosition_ = true;
					if (hasLastKnownPosition_)
						lastKnownPlayerPos_ = owner_.gameScene_->GetPlayer()->GetWorldPosition();
				}
			}
		} else {
			chaseTimeoutTimer_ += 1.0f / 60.0f;
			hasBroadcastAlertThisSighting_ = false;
		}
		bool shouldChase = IsChasing();
		bool debuffPending = (behaviorRequest_ == Behavior::kKnockback || behaviorRequest_ == Behavior::kStunned);
		bool blockPending = (behaviorRequest_ == Behavior::kBlock);
		if (shouldChase) {
			if (!IsDebuffed() && !debuffPending && !blockPending && !IsEngaged() && (behavior_ != Behavior::kRoot || behaviorRequest_ == Behavior::kRoam || behaviorRequest_ == Behavior::kIdle)) {
				behaviorRequest_ = Behavior::kRoot;
			}
		} else if (behavior_ == Behavior::kRoot && behaviorRequest_ == Behavior::kUnknown) {
			behaviorRequest_ = Behavior::kIdle;
		}
	}
}

void BaseEnemyBehavior::SetPostMode(PostMode mode) {
	postMode_ = mode;
	switch (postMode_) {
	case PostMode::kHoldLeft:
		SnapFacing(static_cast<int>(BaseEnemy::LRDirection::kLeft));
		owner_.velocity_.x = 0.0f;
		break;
	case PostMode::kHoldRight:
		SnapFacing(static_cast<int>(BaseEnemy::LRDirection::kRight));
		owner_.velocity_.x = 0.0f;
		break;
	case PostMode::kWander:
	default:
		break;
	}
}

void BaseEnemyBehavior::SnapFacing(int direction) {
	BaseEnemy::LRDirection dir = static_cast<BaseEnemy::LRDirection>(direction);
	// Same destination table BaseEnemy::PhysicsAndCollisionUpdate lerps toward: {right, left}.
	const float destinationRotationY = (dir == BaseEnemy::LRDirection::kRight) ? std::numbers::pi_v<float> / 2.0f : std::numbers::pi_v<float> * 3.0f / 2.0f;
	owner_.lrDirection_ = dir;
	owner_.turnTimer_ = 0.0f;
	owner_.turnFirstRotationY_ = destinationRotationY;
	owner_.worldTransform_.rotation_.y = destinationRotationY;
}

void BaseEnemyBehavior::ResolveBehaviorRequest() {
	// A holding enemy never wanders: anything that would send it to Roam (idle timeout, or the
	// pathfinder giving up on an unreachable player) drops it back to Idle at its post instead.
	if (postMode_ != PostMode::kWander && behaviorRequest_ == Behavior::kRoam) {
		behaviorRequest_ = Behavior::kIdle;
	}
	if (behaviorRequest_ != Behavior::kUnknown) {
		if (behaviorRequest_ == Behavior::kKnockback || behaviorRequest_ == Behavior::kStunned || behaviorRequest_ == Behavior::kDeath) {
			ClearMoveCommit();
		}
		behavior_ = behaviorRequest_;
		InitializeBehaviorState(behavior_);
		behaviorRequest_ = Behavior::kUnknown;
	}

}

void BaseEnemyBehavior::UpdateCurrentBehavior() {
	if (blockCooldownTimer_ > 0.0f) {
		blockCooldownTimer_ = std::max(0.0f, blockCooldownTimer_ - 1.0f / 60.0f);
	}
	if (moveDecisionCooldownTimer_ > 0.0f) {
		moveDecisionCooldownTimer_ = std::max(0.0f, moveDecisionCooldownTimer_ - 1.0f / 60.0f);
	}
	UpdateBehaviorState();
}

void BaseEnemyBehavior::InitializeBehaviorState(Behavior behavior) {
	switch (behavior) {
	case Behavior::kRoot:
		BehaviorRootInitialize();
		break;
	case Behavior::kKnockback:
		BehaviorKnockbackInitialize();
		break;
	case Behavior::kStunned:
		BehaviorStunnedInitialize();
		break;
	case Behavior::kBlock:
		BehaviorBlockInitialize();
		break;
	case Behavior::kIdle:
	default:
		BehaviorIdleInitialize();
		break;
	case Behavior::kRoam:
		BehaviorRoamInitialize();
		break;
	case Behavior::kDeath:
		BehaviorDieInitialize();
		break;
	}
}

void BaseEnemyBehavior::UpdateBehaviorState() {
	switch (behavior_) {
	case Behavior::kRoot:
		BehaviorRootUpdate();
		RunPhysicsAndCollisionUpdate();
		break;
	case Behavior::kKnockback:
		BehaviorKnockbackUpdate();
		RunPhysicsAndCollisionUpdate();
		break;
	case Behavior::kStunned:
		BehaviorStunnedUpdate();
		RunPhysicsAndCollisionUpdate();
		break;
	case Behavior::kBlock:
		BehaviorBlockUpdate();
		RunPhysicsAndCollisionUpdate();
		break;
	case Behavior::kIdle:
	default:
		BehaviorIdleUpdate();
		RunPhysicsAndCollisionUpdate();
		break;
	case Behavior::kRoam:
		BehaviorRoamUpdate();
		RunPhysicsAndCollisionUpdate();
		break;
	case Behavior::kDeath:
		BehaviorDieUpdate();
		break;
	}
}

bool BaseEnemyBehavior::IsChasing() const {
	if (kEndlessChase || persistentAwareness_)
		return visionSensor_.IsPlayerSpotted() || hasLastKnownPosition_;
	return visionSensor_.IsPlayerSpotted() || (chaseTimeoutTimer_ < kChaseTimeoutDuration);
}
bool BaseEnemyBehavior::IsPlayerOnSameFloor() const {
	if (!owner_.gameScene_ || !owner_.gameScene_->GetPlayer())
		return false;
	float deltaY = std::abs(owner_.gameScene_->GetPlayer()->GetWorldPosition().y - owner_.GetWorldPosition().y);
	return deltaY <= kFloorLevelTolerance;
}

void BaseEnemyBehavior::ReceiveAlert(const Vector3& alertedPosition) {
	if (owner_.isDead_ || behavior_ == Behavior::kDeath)
		return;
	if (visionSensor_.IsPlayerSpotted())
		return;
	chaseTimeoutTimer_ = 0.0f;
	lastKnownPlayerPos_ = alertedPosition;
	hasLastKnownPosition_ = true;
}

void BaseEnemyBehavior::ApplySeparationFromOtherEnemies() {
	if (!owner_.gameScene_)
		return;

	Vector3 myPos = owner_.GetWorldPosition();
	float pushX = 0.0f;

	for (const std::unique_ptr<BaseEnemy>& other : owner_.gameScene_->GetEnemies()) {
		BaseEnemy* otherEnemy = other.get();
		if (!otherEnemy || otherEnemy == &owner_ || otherEnemy->IsDead())
			continue;

		Vector3 otherPos = otherEnemy->GetWorldPosition();
		if (std::abs(myPos.y - otherPos.y) > owner_.kHeight)
			continue;

		float dx = myPos.x - otherPos.x;
		float distance = std::abs(dx);
		if (distance < 0.0001f) {
			dx = (&owner_ < otherEnemy) ? -0.01f : 0.01f;
			distance = 0.01f;
		}
		if (distance < kSeparationRadius) {
			float t = 1.0f - (distance / kSeparationRadius);
			float direction = (dx > 0.0f) ? 1.0f : -1.0f;
			pushX += direction * kSeparationStrength * t;
		}
	}

	if (pushX == 0.0f)
		return;
	const float speedBefore = std::abs(owner_.velocity_.x);
	owner_.velocity_.x += pushX;
	float maxSeparatedSpeed = std::max(owner_.kChaseSpeed + kSeparationStrength, speedBefore);
	owner_.velocity_.x = std::clamp(owner_.velocity_.x, -maxSeparatedSpeed, maxSeparatedSpeed);
}

void BaseEnemyBehavior::IdleFacingUpdate() {
	idleScanTimer_ += 1.0f / 60.0f;
	if (idleScanTimer_ < kIdleScanInterval)
		return;
	idleScanTimer_ = 0.0f;
	BaseEnemy::LRDirection newDirection = (owner_.lrDirection_ == BaseEnemy::LRDirection::kRight) ? BaseEnemy::LRDirection::kLeft : BaseEnemy::LRDirection::kRight;
	owner_.FaceDirection(newDirection);
}
void BaseEnemyBehavior::ApproachPosition(const Vector3& target, float speed, float stopDistance) {
	const Vector3 pos = owner_.GetWorldPosition();
	const float dx = target.x - pos.x;

	// Mid-hop: keep going, ignore stopDistance until we land
	if (moveHopping_ && !owner_.onGround_) {
		MoveAlongGround(lastMoveDirection_, speed, MoveKind::kApproach);
		return;
	}

	// Platform: target is well below and we stand on a platform -> drop through (only if the landing is safe).
	if (owner_.onGround_ && owner_.IsStandingOnPlatform() && pos.y - target.y > PathFindingAI::kDropPlayerBelowThreshold && std::abs(dx) < 2.0f && owner_.pathfindingModule_ &&
	    owner_.pathfindingModule_->IsDropLandingSafe(pos)) {
		owner_.velocity_.x = 0.0f;
		owner_.RequestDropThrough();
		return;
	}

	if (std::abs(dx) <= stopDistance) {
		owner_.velocity_.x = 0.0f;
		return;
	}

	const int dir = (dx < 0.0f) ? static_cast<int>(BaseEnemy::LRDirection::kLeft) : static_cast<int>(BaseEnemy::LRDirection::kRight);
	MoveAlongGround(dir, speed, MoveKind::kApproach);
}

void BaseEnemyBehavior::OwnerFaceDirection(int direction) { owner_.FaceDirection(static_cast<BaseEnemy::LRDirection>(direction)); }
void BaseEnemyBehavior::OwnerSetFirearmMode(int mode) { owner_.firearmMode_ = static_cast<BaseEnemy::FirearmMode>(mode); }
void BaseEnemyBehavior::FacePlayer() {
	GameScene* scene = OwnerGameScene();
	if (!scene || !scene->GetPlayer())
		return;
	OwnerFaceDirection(scene->GetPlayer()->GetWorldPosition().x < owner_.GetWorldPosition().x ? 1 : 0);
}

void BaseEnemyBehavior::AdvanceTowardFacing(float speed) {
	const bool right = owner_.isRight();

	if (GameScene* scene = OwnerGameScene()) {
		if (Player* player = scene->GetPlayer()) {
			float dx = player->GetWorldPosition().x - owner_.GetWorldPosition().x;
			bool playerAhead = right ? (dx > 0.0f) : (dx < 0.0f);
			if (playerAhead && std::abs(dx) <= kMeleeAdvanceStopGap) {
				OwnerVelocity().x = 0.0f;
				return;
			}
		}
	}
	if (!IsGroundAheadClear(right ? 0 : 1)) {
		OwnerVelocity().x = 0.0f;
		return;
	}
	OwnerVelocity().x = right ? speed : -speed;
}



BaseEnemyBehavior::MoveProfile BaseEnemyBehavior::GetMoveProfile(MoveKind kind) const {
	MoveProfile p;
	p.maxStepDown = kMoveMaxStepDown;
	p.hopForwardSpeed = kMoveHopForwardSpeed;
	if (kind == MoveKind::kRetreat) {
		p.maxWallBlocks = kRetreatJumpableWallBlocks;
		p.hopSpeed = kRetreatHopSpeed;
		p.allowGapJump = false;
	} else {
		// Approach uses the enemy's normal jump (same height formula as CanJumpOverWall).
		float maxJumpHeight = (owner_.kJumpAcceleration * owner_.kJumpAcceleration) / (2.0f * owner_.kGravityAcceleration) + kJumpHeightMargin;
		p.maxWallBlocks = std::max(0, static_cast<int32_t>(std::floor(maxJumpHeight)));
		p.hopSpeed = owner_.kJumpAcceleration;
		p.allowGapJump = true;
	}
	return p;
}

BaseEnemyBehavior::MoveTerrain BaseEnemyBehavior::ClassifyMoveTerrain(int direction, const MoveProfile& profile) const {
	MapChipField* field = owner_.mapChipField_;
	if (!field) {
		return MoveTerrain::kClear;
	}

	const float sign = (direction == static_cast<int>(BaseEnemy::LRDirection::kRight)) ? 1.0f : -1.0f;
	const Vector3 pos = owner_.worldTransform_.translation_;
	const float footY = pos.y - BaseEnemy::GetBodyHeight() * 0.5f;

	// the column we are moving into, and the row the feet are in (cell just above the floor)
	MapChipField::IndexSet feet = field->GetMapChipIndexSetByPosition(Vector3(pos.x + sign * kMoveLookAheadDistance, footY + kMoveFootSampleOffset, 0.0f));
	const int col = static_cast<int>(feet.xIndex);
	const int row = static_cast<int>(feet.yIndex); // yIndex grows downward
	const int rows = static_cast<int>(field->GetNumBlockVertical());

	auto typeAt = [&](int r) -> MapChipType {
		if (r < 0 || r >= rows) {
			return MapChipType::kBlank;
		}
		return field->GetMapChipTypeByIndex(static_cast<uint32_t>(col), static_cast<uint32_t>(r));
	};

	const float blockHeight = (rows > 0) ? field->GetWorldHeight() / static_cast<float>(rows) : 1.0f;
	const int bodyCells = std::max(1, static_cast<int>(std::ceil(BaseEnemy::GetBodyHeight() / blockHeight - 0.05f)));

	// --- wall: count its height (stop one past the limit) ---
	int wallBlocks = 0;
	while (wallBlocks <= profile.maxWallBlocks && IsMoveWall(typeAt(row - wallBlocks))) {
		++wallBlocks;
	}
	if (wallBlocks > profile.maxWallBlocks) {
		return MoveTerrain::kBlocked; // too tall to hop
	}
	if (wallBlocks > 0) {
		for (int k = 0; k < bodyCells; ++k) {
			if (IsMoveWall(typeAt(row - wallBlocks - k))) {
				return MoveTerrain::kBlocked; // no headroom on top of the wall
			}
		}
		return MoveTerrain::kJumpable;
	}

	// --- overhang that would hit the head ---
	for (int k = 1; k < bodyCells; ++k) {
		if (IsMoveWall(typeAt(row - k))) {
			return MoveTerrain::kBlocked;
		}
	}

	// --- floor: flat, staircase down, or gap (blocks, half blocks, slow blocks and platforms all count as floor) ---
	for (int depth = 0; depth <= profile.maxStepDown; ++depth) {
		MapChipType type = typeAt(row + 1 + depth);
		if (type == MapChipType::kKillBlock) {
			return MoveTerrain::kGap;
		}
		if (MapChipField::IsGroundType(type)) {
			return (depth == 0) ? MoveTerrain::kClear : MoveTerrain::kStepDown;
		}
	}
	return MoveTerrain::kGap;
}

void BaseEnemyBehavior::LaunchMoveHop(int direction, float forwardSpeed, float hopSpeed) {
	const float sign = (direction == static_cast<int>(BaseEnemy::LRDirection::kLeft)) ? -1.0f : 1.0f;
	moveHopping_ = true;
	moveHopDirection_ = direction;
	moveHopSpeed_ = forwardSpeed;
	moveDecisionCooldownTimer_ = kMoveDecisionCooldown;
	owner_.velocity_.x = sign * forwardSpeed;
	owner_.velocity_.y = hopSpeed; // onGround_ is cleared by the ground check this frame
}

void BaseEnemyBehavior::ClearMoveCommit() {
	moveHopping_ = false;
	moveDecisionCooldownTimer_ = 0.0f;
}

bool BaseEnemyBehavior::IsMoveCommitted() const { return (moveHopping_ && !owner_.onGround_) || moveDecisionCooldownTimer_ > 0.0f; }

void BaseEnemyBehavior::ContinueCommittedMove() { MoveAlongGround(lastMoveDirection_, lastMoveSpeed_, lastMoveKind_); }

bool BaseEnemyBehavior::MoveAlongGround(int direction, float speed, MoveKind kind) {
	lastMoveKind_ = kind;
	lastMoveDirection_ = direction;
	lastMoveSpeed_ = speed;

	const float sign = (direction == static_cast<int>(BaseEnemy::LRDirection::kLeft)) ? -1.0f : 1.0f;

	// Airborne: never re-evaluate. During our own hop keep the velocity locked forward/back
	// and hold the decision cooldown at full so it only starts counting after landing.
	if (!owner_.onGround_) {
		if (moveHopping_) {
			const float hopSign = (moveHopDirection_ == static_cast<int>(BaseEnemy::LRDirection::kLeft)) ? -1.0f : 1.0f;
			owner_.velocity_.x = hopSign * moveHopSpeed_;
			moveDecisionCooldownTimer_ = kMoveDecisionCooldown;
		} else {
			owner_.velocity_.x = sign * speed; // walked off a step: keep going
		}
		return true;
	}
	moveHopping_ = false; // landed

	if (speed <= 0.0001f) {
		owner_.velocity_.x = 0.0f;
		return false;
	}

	// slow terrain (same rule as chase / roam)
	float speedFactor = 1.0f;
	if (owner_.mapChipField_) {
		Vector3 footPos = owner_.worldTransform_.translation_ + Vector3(0.0f, -(BaseEnemy::GetBodyHeight() * 0.5f) - 0.05f, 0.0f);
		if (owner_.mapChipField_->IsSlowAtPosition(footPos)) {
			speedFactor = MapChipField::kSlowSpeedFactor;
		}
	}
	const float walkSpeed = speed * speedFactor;

	const MoveProfile profile = GetMoveProfile(kind);
	switch (ClassifyMoveTerrain(direction, profile)) {
	case MoveTerrain::kClear:
	case MoveTerrain::kStepDown:
		owner_.velocity_.x = sign * walkSpeed;
		return true;

	case MoveTerrain::kJumpable:
		LaunchMoveHop(direction, std::max(walkSpeed, profile.hopForwardSpeed), profile.hopSpeed);
		return true;

	case MoveTerrain::kGap:
		if (profile.allowGapJump && HasReachableLedgeAhead(direction, profile.hopForwardSpeed)) {
			LaunchMoveHop(direction, profile.hopForwardSpeed, profile.hopSpeed);
			return true;
		}
		owner_.velocity_.x = 0.0f; // corner lock
		return false;

	case MoveTerrain::kBlocked:
	default:
		owner_.velocity_.x = 0.0f; // wall too tall: stand still
		return false;
	}
}

bool BaseEnemyBehavior::RetreatInDirection(int direction, float speed) { return MoveAlongGround(direction, speed, MoveKind::kRetreat); }

bool BaseEnemyBehavior::RetreatAwayFrom(const Vector3& threatPos, float speed) {
	const bool threatOnRight = threatPos.x >= owner_.GetWorldPosition().x;
	const int direction = threatOnRight ? static_cast<int>(BaseEnemy::LRDirection::kLeft) : static_cast<int>(BaseEnemy::LRDirection::kRight);
	return RetreatInDirection(direction, speed);
}

bool BaseEnemyBehavior::TimedRetreatAwayFrom(const Vector3& threatPos, float speed, float duration, float& timer) {
	if (timer >= duration) {
		return false;
	}
	if (!RetreatAwayFrom(threatPos, speed)) {
		return false;
	}
	timer += 1.0f / 60.0f;
	return true;
}

float BaseEnemyBehavior::ScaleRetreatSpeed(float distance, float outerRange, float innerRange, float maxSpeed) {
	const float span = outerRange - innerRange;
	if (span <= 0.0f) {
		return maxSpeed;
	}
	const float amount = std::clamp((outerRange - distance) / span, 0.0f, 1.0f);
	return maxSpeed * amount;
}
void BaseEnemyBehavior::BehaviorRootInitialize() {}
void BaseEnemyBehavior::BehaviorRootUpdate() { owner_.pathfindingModule_->Update(); }

void BaseEnemyBehavior::BehaviorIdleInitialize() {
	owner_.velocity_.x = 0.0f;
	idleTimer_ = 0.0f;
	idleDuration_ = sRandom_.GetRandValue(kIdleDurationMin, kIdleDurationMax);

	if (postMode_ == PostMode::kHoldLeft)
		owner_.FaceDirection(BaseEnemy::LRDirection::kLeft);
	else if (postMode_ == PostMode::kHoldRight)
		owner_.FaceDirection(BaseEnemy::LRDirection::kRight);
}
void BaseEnemyBehavior::BehaviorIdleUpdate() {
	owner_.velocity_.x = 0.0f;
	if (postMode_ != PostMode::kWander)
		return; // holding: no look-around flip, no Idle -> Roam transition
	IdleFacingUpdate();
	idleTimer_ += 1.0f / 60.0f;
	if (idleTimer_ >= idleDuration_)
		behaviorRequest_ = Behavior::kRoam;
}

void BaseEnemyBehavior::BehaviorRoamInitialize() {
	roamTimer_ = 0.0f;
	roamWalkDuration_ = sRandom_.GetRandValue(kRoamWalkDurationMin, kRoamWalkDurationMax);
	int candidate = (sRandom_.GetRandValue(0.0f, 1.0f) < 0.5f) ? static_cast<int>(BaseEnemy::LRDirection::kLeft) : static_cast<int>(BaseEnemy::LRDirection::kRight);
	if (!IsGroundAheadClear(candidate) && !HasReachableLedgeAhead(candidate)) {
		candidate = (candidate == static_cast<int>(BaseEnemy::LRDirection::kLeft)) ? static_cast<int>(BaseEnemy::LRDirection::kRight) : static_cast<int>(BaseEnemy::LRDirection::kLeft);
	}
	roamDirection_ = candidate;
}
void BaseEnemyBehavior::BehaviorRoamUpdate() {
	if (owner_.hitWallThisFrame_) {
		owner_.velocity_.x = 0.0f;
		behaviorRequest_ = Behavior::kIdle;
		return;
	}
	if (!IsGroundAheadClear(roamDirection_)) {
		if (!TryJumpIfLedgeAhead(roamDirection_)) {
			owner_.velocity_.x = 0.0f;
			behaviorRequest_ = Behavior::kIdle;
			return;
		}
	}

	float speedFactor = 1.0f;
	if (owner_.mapChipField_) {
		Vector3 footPos = owner_.worldTransform_.translation_ + Vector3(0.0f, -(owner_.kHeight / 2.0f) - 0.05f, 0.0f);
		if (owner_.mapChipField_->IsSlowAtPosition(footPos))
			speedFactor = MapChipField::kSlowSpeedFactor;
	}

	BaseEnemy::LRDirection roamLR = static_cast<BaseEnemy::LRDirection>(roamDirection_);
	owner_.velocity_.x = (roamLR == BaseEnemy::LRDirection::kLeft) ? -owner_.kWalkSpeed * speedFactor : owner_.kWalkSpeed * speedFactor;
	owner_.FaceDirection(roamLR);

	roamTimer_ += 1.0f / 60.0f;
	if (roamTimer_ >= roamWalkDuration_) {
		owner_.velocity_.x = 0.0f;
		behaviorRequest_ = Behavior::kIdle;
	}
}

// Knockback
void BaseEnemyBehavior::BehaviorKnockbackInitialize() {
	knockbackParameter_ = 0;
	owner_.velocity_ = {knockbackDirection_.x * (knockbackPower_ * kKnockbackSpeedScale), knockbackDirection_.y * (knockbackPower_ * kKnockbackSpeedScale) + 0.15f, 0.0f};
	owner_.onGround_ = false;
}
void BaseEnemyBehavior::BehaviorKnockbackUpdate() {
	++knockbackParameter_;
	owner_.velocity_ += Vector3(0, -BaseEnemy::kGravityAcceleration, 0);
	owner_.velocity_.y = std::max(owner_.velocity_.y, -BaseEnemy::kLimitFallSpeed);
	if (knockbackParameter_ >= kKnockbackDuration) {
		if (pendingStunAfterKnockback_ > 0.0f) {
			stunDuration_ = pendingStunAfterKnockback_;
			pendingStunAfterKnockback_ = 0.0f;
			behaviorRequest_ = Behavior::kStunned;
		} else {
			behaviorRequest_ = Behavior::kRoot;
		}
	}
}
void BaseEnemyBehavior::BehaviorStunnedInitialize() {
	stunTimer_ = 0.0f;
	owner_.velocity_.x = 0.0f;
}
void BaseEnemyBehavior::BehaviorStunnedUpdate() {
	stunTimer_ += 1.0f / 60.0f;
	owner_.velocity_.x = 0.0f; 
	if (stunTimer_ >= stunDuration_) {
		behaviorRequest_ = (ShouldBlockAfterStun() && IsBlockReady()) ? Behavior::kBlock : Behavior::kRoot;
	}
}
void BaseEnemyBehavior::BehaviorDieInitialize() {
	deathParameter_ = 0;
	hasSpawnedDrop_ = false;
	deathBounceCount_ = 0;
	owner_.velocity_ = {
	    owner_.pendingHitDirection_.x * (owner_.pendingImpactPower_ * kDeathFlingSpeedScale) + 0.0f,
	    owner_.pendingHitDirection_.y * (owner_.pendingImpactPower_ * kDeathFlingSpeedScale) + kDeathFlingUpBoost,
	    owner_.pendingHitDirection_.z * (owner_.pendingImpactPower_ * kDeathFlingSpeedScale) + 0.0f};
	deathSpinSpeed_ = sRandom_.GetRandValue(-kDeathMaxSpin, kDeathMaxSpin);
	owner_.onGround_ = false;
}

void BaseEnemyBehavior::BehaviorDieUpdate() {
	++deathParameter_;

	// stop rolling once settled, ease into a flat "lying down" pose instead of freezing mid-tumble
	bool settled = (deathBounceCount_ >= kDeathMaxBounces) || (owner_.velocity_.x == 0.0f && owner_.velocity_.y == 0.0f && deathBounceCount_ > 0);

	if (!settled) {
		owner_.worldTransform_.rotation_.x += deathSpinSpeed_ * 0.6f;
		owner_.worldTransform_.rotation_.z += deathSpinSpeed_;
	} else {
		float targetX = std::numbers::pi_v<float> / 2.0f;
		owner_.worldTransform_.rotation_.x = EaseOut(owner_.worldTransform_.rotation_.x, targetX, 0.15f);
		float nearestFlatZ = std::round(owner_.worldTransform_.rotation_.z / std::numbers::pi_v<float>) * std::numbers::pi_v<float>;
		owner_.worldTransform_.rotation_.z = EaseOut(owner_.worldTransform_.rotation_.z, nearestFlatZ, 0.15f);
	}

	owner_.velocity_.y -= BaseEnemy::kGravityAcceleration;
	owner_.velocity_.y = std::max(owner_.velocity_.y, -kDeathMaxFallSpeed);

	bool landed = false;
	ObjectBounce::Step(owner_.mapChipField_, owner_.worldTransform_.translation_, owner_.velocity_, BaseEnemy::kWidth, BaseEnemy::kHeight, kDeathBounceDamping, kDeathFriction, true, landed);
	if (landed) {
		++deathBounceCount_;
		if (deathBounceCount_ >= kDeathMaxBounces || std::abs(owner_.velocity_.y) < 0.02f) {
			owner_.velocity_ = {};
		}
	}

	if (static_cast<uint32_t>(deathParameter_) >= kDeathDuration) {
		if (!hasSpawnedDrop_) {
			owner_.SpawnDropItem();
			hasSpawnedDrop_ = true;
		}
		if (owner_.gameScene_) {
			owner_.gameScene_->CreateEnemyDeathParticles(owner_.GetModelWorldPosition());
		}
		owner_.isDead_ = true;
	}
}

#pragma region PathNavigator 先読み判定
bool BaseEnemyBehavior::ScanWallAhead(int direction, float maxLookAhead, float maxHeight, float& outDistance, float& outHeight) const {
	if (!owner_.mapChipField_)
		return false;
	bool directionIsRight = (direction == static_cast<int>(BaseEnemy::LRDirection::kRight));
	Vector3 footPos = owner_.worldTransform_.translation_ + Vector3(0.0f, -(owner_.kHeight / 2.0f), 0.0f);
	PathDetector::ScanParams params;
	params.stepDistance = 0.1f;
	params.maxDistance = maxLookAhead;
	params.bodyHeight = maxHeight;
	std::vector<PathDetector::Segment> segments = pathSensor_.Scan(footPos, directionIsRight, params);
	return PathDetector::FirstWallInfo(segments, outDistance, outHeight);
}
bool BaseEnemyBehavior::ScanGapAhead(int direction, float maxLookAhead, float& outGapDistance) const {
	if (!owner_.mapChipField_)
		return false;
	bool directionIsRight = (direction == static_cast<int>(BaseEnemy::LRDirection::kRight));
	Vector3 footPos = owner_.worldTransform_.translation_ + Vector3(0.0f, -(owner_.kHeight / 2.0f), 0.0f);
	PathDetector::ScanParams params;
	params.stepDistance = kLedgeCheckAheadDistance * 0.5f;
	params.maxDistance = maxLookAhead;
	params.bodyHeight = owner_.kHeight;
	params.groundProbeDown = kLedgeCheckDownOffset;
	std::vector<PathDetector::Segment> segments = pathSensor_.Scan(footPos, directionIsRight, params);
	return PathDetector::FirstGapInfo(segments, outGapDistance);
}
bool BaseEnemyBehavior::IsGroundAheadClear(int direction) const {
	if (!owner_.mapChipField_)
		return true;
	bool directionIsRight = (direction == static_cast<int>(BaseEnemy::LRDirection::kRight));
	Vector3 footPos = owner_.worldTransform_.translation_ + Vector3(0.0f, -(owner_.kHeight / 2.0f), 0.0f);
	PathDetector::ScanParams params;
	params.stepDistance = kLedgeCheckAheadDistance;
	params.maxDistance = kLedgeCheckAheadDistance;
	params.bodyHeight = owner_.kHeight;
	params.groundProbeDown = kLedgeCheckDownOffset;
	std::vector<PathDetector::Segment> segments = pathSensor_.Scan(footPos, directionIsRight, params);
	if (segments.empty())
		return true;
	return segments.front().hasGroundBelow;
}
bool BaseEnemyBehavior::CanJumpOverWall(int direction) const {
	if (!owner_.mapChipField_)
		return false;
	float maxJumpHeight = (owner_.kJumpAcceleration * owner_.kJumpAcceleration) / (2.0f * owner_.kGravityAcceleration) + kJumpHeightMargin;
	float wallDistance = 0.0f;
	float wallHeight = 0.0f;
	if (!ScanWallAhead(direction, owner_.kWidth / 2.0f + 0.1f, maxJumpHeight + 0.1f, wallDistance, wallHeight))
		return false;
	return wallHeight <= maxJumpHeight;
}
bool BaseEnemyBehavior::HasReachableLedgeAhead(int direction, float travelSpeedOverride) const {
	if (!owner_.mapChipField_)
		return false;
	float maxJumpHeight = (owner_.kJumpAcceleration * owner_.kJumpAcceleration) / (2.0f * owner_.kGravityAcceleration) + kJumpHeightMargin;
	float airTime = (2.0f * owner_.kJumpAcceleration) / owner_.kGravityAcceleration;
	float travelSpeed = (travelSpeedOverride > 0.0f) ? travelSpeedOverride : ((behavior_ == Behavior::kRoot) ? owner_.kChaseSpeed : owner_.kWalkSpeed);
	float maxJumpDistance = travelSpeed * airTime;
	bool directionIsRight = (direction == static_cast<int>(BaseEnemy::LRDirection::kRight));
	Vector3 footPos = owner_.worldTransform_.translation_ + Vector3(0.0f, -(owner_.kHeight / 2.0f), 0.0f);
	PathDetector::ScanParams params;
	params.stepDistance = kLedgeCheckAheadDistance * 0.5f;
	params.maxDistance = std::min(kMaxCrossableGapDistance, maxJumpDistance + kLedgeCheckAheadDistance);
	params.bodyHeight = maxJumpHeight + 0.1f;
	params.groundProbeDown = kLedgeCheckDownOffset;
	std::vector<PathDetector::Segment> segments = pathSensor_.Scan(footPos, directionIsRight, params);
	if (segments.empty())
		return false;
	if (segments.front().hasGroundBelow)
		return false;
	for (const PathDetector::Segment& segment : segments) {
		if (segment.isWall)
			return segment.wallTopHeight <= maxJumpHeight && segment.distance <= maxJumpDistance;
		if (segment.hasGroundBelow)
			return segment.distance <= maxJumpDistance;
	}
	return false;
}
bool BaseEnemyBehavior::TryJumpIfLedgeAhead(int direction) {
	if (!owner_.onGround_)
		return false;
	if (!HasReachableLedgeAhead(direction))
		return false;
	owner_.velocity_ += Vector3(0.0f, owner_.kJumpAcceleration, 0.0f);
	return true;
}
#pragma endregion

Vector3& BaseEnemyBehavior::OwnerVelocity() { return owner_.velocity_; }
const Vector3& BaseEnemyBehavior::OwnerVelocity() const { return owner_.velocity_; }
void BaseEnemyBehavior::RunPhysicsAndCollisionUpdate() {
	// デバッグ中はBehavior自体を更新しつつ、敵本体の物理移動だけ停止する。
	// これにより武器のポーズ・アニメーション・当たり判定をその場で確認できる。
	if (kDebugMovementFreeze_) {
		owner_.velocity_.x = 0.0f;
		owner_.velocity_.y = 0.0f;
		return;
	}

	if (owner_.IsEquipLocked() && (behavior_ == Behavior::kRoot || behavior_ == Behavior::kIdle || behavior_ == Behavior::kRoam)) {
		owner_.velocity_.x = 0.0f;
	}

	owner_.PhysicsAndCollisionUpdate();
}
GameScene* BaseEnemyBehavior::OwnerGameScene() const { return owner_.gameScene_; }
bool BaseEnemyBehavior::TryStartBlockOnHit() {
	float chance = GetBlockChanceWhenHit();
	if (chance <= 0.0f || !CanBlockNow())
		return false;
	if (sRandom_.GetRandValue(0.0f, 1.0f) >= chance)
		return false;
	// Hit reaction, not an AI decision, so (like RequestKnockback) this bypasses the debug decision freeze.
	behaviorRequest_ = Behavior::kBlock;
	return true;
}

void BaseEnemyBehavior::BehaviorBlockInitialize() {
	blockTimer_ = 0.0f;
	owner_.velocity_.x = 0.0f;
	// Block time + cooldown, so the cooldown always ends kBlockCooldown after the block ends.
	blockCooldownTimer_ = kBlockDuration + kBlockCooldown;
}

void BaseEnemyBehavior::BehaviorBlockUpdate() {
	blockTimer_ += 1.0f / 60.0f;
	owner_.velocity_.x = 0.0f;

	// keep facing the player so the guard stays up on the correct side
	if (owner_.gameScene_) {
		if (Player* player = owner_.gameScene_->GetPlayer()) {
			OwnerFaceDirection(player->GetWorldPosition().x < owner_.GetWorldPosition().x ? static_cast<int>(BaseEnemy::LRDirection::kLeft) : static_cast<int>(BaseEnemy::LRDirection::kRight));
		}
	}

	if (blockTimer_ >= kBlockDuration) {
		behaviorRequest_ = Behavior::kRoot;
	}
}
BaseEnemyBehavior::LedgeGuardRequest BaseEnemyBehavior::GetLedgeGuardRequest() const {
	if (IsKnockedBack()) {
		return {true, BaseEnemy::kChaseSpeed}; // same rule as before: let strong launches fly
	}
	if (behavior_ == Behavior::kMelee) {
		if (IsInMeleeRecovery()) {
			return {true, std::numeric_limits<float>::max()}; // recovery: always stop at the edge
		}
		return {true, kLungeLedgeGuardSpeed}; // otherwise only once slow enough
	}
	return {};
}