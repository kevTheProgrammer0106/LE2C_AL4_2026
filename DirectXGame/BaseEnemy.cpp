#include "BaseEnemy.h"
#include "BaseProjectile.h"
#include "GameScene.h"
#include "GlobalVariables.h"
#include "MapChipField.h"
#include "Player.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "BaseEnemy";
}

void BaseEnemy::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "TimeTurn", kTimeTurn);
	gv->AddItem(kGroupName, "WalkSpeed", kWalkSpeed);
	gv->AddItem(kGroupName, "Acceleration", kAcceleration);
	gv->AddItem(kGroupName, "Attenuation", kAttenuation);
	gv->AddItem(kGroupName, "LimitRunSpeed", kLimitRunSpeed);
	gv->AddItem(kGroupName, "WalkMotionAngleStart", kWalkMotionAngleStart);
	gv->AddItem(kGroupName, "WalkMotionAngleEnd", kWalkMotionAngleEnd);
	gv->AddItem(kGroupName, "WalkMotionTime", kWalkMotionTime);
	gv->AddItem(kGroupName, "GravityAcceleration", kGravityAcceleration);
	gv->AddItem(kGroupName, "LimitFallSpeed", kLimitFallSpeed);
	gv->AddItem(kGroupName, "JumpAcceleration", kJumpAcceleration);
	gv->AddItem(kGroupName, "Width", kWidth);
	gv->AddItem(kGroupName, "Height", kHeight);
	gv->AddItem(kGroupName, "Blank", kBlank);
	gv->AddItem(kGroupName, "AttenuationLanding", kAttenuationLanding);
	gv->AddItem(kGroupName, "AttenuationWall", kAttenuationWall);
	gv->AddItem(kGroupName, "AttenuationCeilling", kAttenuationCeilling);

	BaseEnemyBehavior::RegisterGlobalVariables();
}

void BaseEnemy::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kTimeTurn = gv->GetFloatValue(kGroupName, "TimeTurn");
	kWalkSpeed = gv->GetFloatValue(kGroupName, "WalkSpeed");
	kAcceleration = gv->GetFloatValue(kGroupName, "Acceleration");
	kAttenuation = gv->GetFloatValue(kGroupName, "Attenuation");
	kLimitRunSpeed = gv->GetFloatValue(kGroupName, "LimitRunSpeed");
	kWalkMotionAngleStart = gv->GetFloatValue(kGroupName, "WalkMotionAngleStart");
	kWalkMotionAngleEnd = gv->GetFloatValue(kGroupName, "WalkMotionAngleEnd");
	kWalkMotionTime = gv->GetFloatValue(kGroupName, "WalkMotionTime");
	kGravityAcceleration = gv->GetFloatValue(kGroupName, "GravityAcceleration");
	kLimitFallSpeed = gv->GetFloatValue(kGroupName, "LimitFallSpeed");
	kJumpAcceleration = gv->GetFloatValue(kGroupName, "JumpAcceleration");
	kWidth = gv->GetFloatValue(kGroupName, "Width");
	kHeight = gv->GetFloatValue(kGroupName, "Height");
	kBlank = gv->GetFloatValue(kGroupName, "Blank");
	kAttenuationLanding = gv->GetFloatValue(kGroupName, "AttenuationLanding");
	kAttenuationWall = gv->GetFloatValue(kGroupName, "AttenuationWall");
	kAttenuationCeilling = gv->GetFloatValue(kGroupName, "AttenuationCeilling");

	BaseEnemyBehavior::ApplyGlobalVariables();
}

void BaseEnemy::Initialize(const std::array<PartResources, kPartCount>& partResources, Camera* camera, const Vector3& position) {
	assert(camera);
	for (size_t i = 0; i < parts_.size(); ++i) {
		parts_[i].model = partResources[i].model;
		parts_[i].textureHandle = partResources[i].textureHandle;
		parts_[i].transform.Initialize();
	}

	camera_ = camera;
	velocity_ = {-kWalkSpeed, 0, 0};
	hp_ = maxHP_;
	if (!animManager_)
		animManager_ = CreateAnimationManager();
	if (!behavior_) {
		behavior_ = CreateBehaviorModule();
		behavior_->SetMapChipField(mapChipField_);
		blockDetector_.SetIgnoreBarrier(true);
	}
	behavior_->Initialize();

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_.y = -(std::numbers::pi_v<float> / 2.0f);
	walkTimer_ = 0.0f;
	worldTransform_.translation_ = position;
	CacheAABBMarkerSize();

	if (!pathfindingModule_)
		pathfindingModule_ = std::make_unique<PathFindingAI>();
	pathfindingModule_->Initialize(this, mapChipField_);
}

void BaseEnemy::UpdateMovementState() {
	AnimationManager::AnimationContext context;
	context.velocity = velocity_;
	context.onGround = onGround_;
	context.isDead = behavior_->IsDeathBehavior();
	// catch the landing transition; AnimationManager now reports all three events.
	AnimationManager::MovementEvents events = animManager_->UpdateMovementState(context);

	// Skip movement SFX during the death tumble — onGround_/velocity_ there are
	// driven by BehaviorDieUpdate's bounce logic, not real locomotion.
	if (!behavior_->IsDeathBehavior()) {
		if (events.justJumped) {
			PlayJumpSound();
		}
		if (events.justLanded) {
			PlayMovementSound(); // land sfx — Cuirassier overrides this, unaffected
		}
		if (events.footstepTick) {
			PlayFootstepSound();
		}
	}
}

SRTOffset BaseEnemy::GetMovementAnimOffset() const { return animManager_->GetMovementAnimOffset(); }

void BaseEnemy::Update() {
	ApplyGlobalVariables();
	wasBlocked_ = false;
	hitWallThisFrame_ = false;

	behavior_->UpdateVisionAndChaseRequest();
	behavior_->ResolveBehaviorRequest();

	if (isDead_)
		return;

	behavior_->UpdateCurrentBehavior();
	if (behavior_->IsDebuffed()) 
		RefreshWeaponVisual();
	UpdateMovementState();
	UpdateBodyParts();
}

void BaseEnemy::Draw() {
	for (auto& part : parts_) {
		if (!isDead_ && part.model) {
			part.model->PreDraw();
			part.model->Draw(part.transform, *camera_, part.textureHandle);
			part.model->PostDraw();
		}
	}
}

const WorldTransform& BaseEnemy::GetWorldTransform() const { return worldTransform_; }

void BaseEnemy::SetWorldTransform(const WorldTransform& worldTransform) {
	worldTransform_.translation_ = worldTransform.translation_;
	worldTransform_.rotation_ = worldTransform.rotation_;
	worldTransform_.scale_ = worldTransform.scale_;
}

Vector3 BaseEnemy::GetWorldPosition() const { return {worldTransform_.translation_.x, worldTransform_.translation_.y, worldTransform_.translation_.z}; }

AABB BaseEnemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};
	return aabb;
}
AABB BaseEnemy::GetAABB() const {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};
	return aabb;
}

void BaseEnemy::CacheAABBMarkerSize() {
	AABB aabb = GetAABB();
	float w = aabb.max.x - aabb.min.x;
	float h = aabb.max.y - aabb.min.y;
	constexpr float kVertexMarkerRatio = 0.15f;
	aabbMarkerSize_ = std::max(std::min(w, h) * kVertexMarkerRatio, 0.1f);
}

void BaseEnemy::CheckPlayerDirection() {
	Player* player = gameScene_->GetPlayer();
	if (player->GetWorldPosition().x < GetWorldPosition().x) {
		velocity_.x = -kWalkSpeed;
		FaceDirection(LRDirection::kLeft);
	} else {
		velocity_.x = kWalkSpeed;
		FaceDirection(LRDirection::kRight);
	}
}

void BaseEnemy::FaceDirection(LRDirection direction) {
	if (lrDirection_ == direction)
		return;
	lrDirection_ = direction;
	turnFirstRotationY_ = worldTransform_.rotation_.y;
	turnTimer_ = kTimeTurn;
}

void BaseEnemy::TakeDamage(int damage, const Vector3& hitDirection, float impactPower) {
	if (behavior_->IsDeathBehavior() || isDead_)
		return;

	hp_ -= damage;
	pendingHitDirection_ = hitDirection;
	pendingImpactPower_ = impactPower;

	Vector3 effectPos = GetWorldPosition();
	if (gameScene_)
		gameScene_->CreateHitEffect(effectPos);

	if (hp_ <= 0) {
		behavior_->RequestBehavior(Behavior::kDeath);
		PlayDeathSound(); 
		return;
	}

	if (behavior_->IsInAttackAnticipation()) {
		behavior_->RequestBehavior(Behavior::kRoot);
	}

	PlayHurtSound();
}

void BaseEnemy::OnCollision(const Player* player) {
	if (behavior_->IsDeathBehavior() || isDead_) {
		isCollisionDisabled_ = true;
		return;
	}
	if (player->isAttack()) {
		Vector3 dir = GetWorldPosition() - player->GetWorldPosition();
		float len = std::sqrtf(dir.x * dir.x + dir.y * dir.y);
		if (len > 0.0001f) {
			dir.x /= len;
			dir.y /= len;
		} else {
			dir = {player->isRight() ? 1.0f : -1.0f, 0.0f, 0.0f};
		}
		dir.y += 0.4f;

		int damage = 10;
		float power = 1.0f;
		if (const BaseMelee* melee = player->GetCurrentMelee()) {
			damage = melee->GetDamage(player->GetComboIndex(), Owner::kPlayer); 
			power = melee->GetImpactPower(player->GetComboIndex());
		}

		TakeDamage(damage, dir, power);
		Vector3 effectPos = (GetWorldPosition() + player->GetWorldPosition()) / 2.0f;
		gameScene_->CreateHitEffect(effectPos);
	}
}

void BaseEnemy::OnCollision(const BaseProjectile* bullet) {
	if (behavior_->IsDeathBehavior() || isDead_) {
		isCollisionDisabled_ = true;
		return;
	}
	Vector3 dir = GetWorldPosition() - bullet->GetWorldPosition();
	float len = std::sqrtf(dir.x * dir.x + dir.y * dir.y);
	if (len > 0.0001f) {
		dir.x /= len;
		dir.y /= len;
	}
	dir.y += 0.2f;

	TakeDamage(bullet->GetDamage(bullet->GetOwner()), dir, bullet->GetImpactPower());
	Vector3 effectPos = (GetWorldPosition() + bullet->GetWorldPosition()) / 2.0f;
	gameScene_->CreateHitEffect(effectPos);
}

void BaseEnemy::KillInstant() {
	if (behavior_->IsDeathBehavior() || isDead_)
		return;
	pendingHitDirection_ = {0.0f, 1.0f, 0.0f};
	pendingImpactPower_ = 1.0f;
	hp_ = 0;
	behavior_->RequestBehavior(Behavior::kDeath);
	PlayDeathSound();
}

void BaseEnemy::UpdateBodyParts() {
	AnimationManager::AnimationContext context;
	context.velocity = velocity_;
	context.onGround = onGround_;
	context.isDead = behavior_->IsDeathBehavior();
	animManager_->UpdateBodyParts(worldTransform_, context, parts_);
}

void BaseEnemy::JumpMovement(const CollisionInfo& collisionInfo) {
	if (!onGround_)
		return;

	if (behavior_->IsRootBehavior() && std::abs(velocity_.x) > 0.0001f) {
		LRDirection movingDirection = (velocity_.x < 0.0f) ? LRDirection::kLeft : LRDirection::kRight;
		if (behavior_->TryJumpIfLedgeAhead(static_cast<int>(movingDirection))) {
			lrDirection_ = movingDirection;
			return;
		}
	}

	bool shouldConsiderJump = collisionInfo.isHitWall;

	if (!shouldConsiderJump && std::abs(velocity_.x) > 0.0001f) {
		LRDirection movingDirection = (velocity_.x < 0.0f) ? LRDirection::kLeft : LRDirection::kRight;
		float lookAhead = (kWidth / 2.0f) + BaseEnemyBehavior::GetWallLookAheadMargin();
		float wallDistance = 0.0f;
		float wallHeight = 0.0f;
		if (behavior_->ScanWallAhead(static_cast<int>(movingDirection), lookAhead, kHeight, wallDistance, wallHeight)) {
			shouldConsiderJump = true;
			lrDirection_ = movingDirection;
		}
	}

	if (!shouldConsiderJump)
		return;

	if (behavior_->CanJumpOverWall(static_cast<int>(lrDirection_))) {
		velocity_ += Vector3(0, kJumpAcceleration, 0);
	}
}

#pragma region MapChip / TileMap Collision Detection

void BaseEnemy::PositionUpdate(const CollisionInfo& collisionInfo) { worldTransform_.translation_ += collisionInfo.move; }

void BaseEnemy::CeillingCollision(const CollisionInfo& collisionInfo) {
	if (collisionInfo.isHitTop)
		velocity_.y *= (1.0f - kAttenuationCeilling);
}

void BaseEnemy::WallCollision(const CollisionInfo& collisionInfo) {
	if (collisionInfo.isHitWall) {
		velocity_.x *= (1.0f - kAttenuationWall);
		hitWallThisFrame_ = true;
	}
}

void BaseEnemy::PhysicsAndCollisionUpdate() {
	behavior_->ApplySeparationFromOtherEnemies();

	if (!onGround_) {
		velocity_ += Vector3(0, -kGravityAcceleration, 0);
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}

	MapChipCollisionUpdate();

	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		worldTransform_.rotation_.y = EaseInOut(turnFirstRotationY_, destinationRotationY, 1.0f - (turnTimer_ / kTimeTurn));
	}
}
void BaseEnemy::MapChipCollisionUpdate() {
	blockDetector_.SetWorldTransform(worldTransform_);
	blockDetector_.SetVelocity(velocity_);
	blockDetector_.SetOnGround(onGround_);
	blockDetector_.SetMapChipField(mapChipField_);

	CollisionInfo horizontalInfo{};
	horizontalInfo.move.x = velocity_.x;
	blockDetector_.MapChipCollision(horizontalInfo);
	worldTransform_.translation_ += horizontalInfo.move;
	WallCollision(horizontalInfo);

	CollisionInfo verticalInfo{};
	verticalInfo.move.y = velocity_.y;
	blockDetector_.MapChipCollision(verticalInfo);
	worldTransform_.translation_ += verticalInfo.move;
	CeillingCollision(verticalInfo);

	blockDetector_.GroundSwitch(verticalInfo);
	onGround_ = blockDetector_.IsOnGround();
	velocity_ = blockDetector_.GetVelocity();

	JumpMovement(horizontalInfo);
}
void BaseEnemy::MapChipCollisionUpdate(CollisionInfo& collisionInfo) {
	blockDetector_.SetWorldTransform(worldTransform_);
	blockDetector_.SetVelocity(velocity_);
	blockDetector_.SetOnGround(onGround_);
	blockDetector_.SetMapChipField(mapChipField_);

	blockDetector_.MapChipCollision(collisionInfo);
	PositionUpdate(collisionInfo);
	WallCollision(collisionInfo);
	CeillingCollision(collisionInfo);
	blockDetector_.GroundSwitch(collisionInfo);
	onGround_ = blockDetector_.IsOnGround();
	velocity_ = blockDetector_.GetVelocity();
}
#pragma endregion

void BaseEnemy::SpawnDropItem() {
	if (!gameScene_)
		return;
	DropItemCategory category;
	DropWeaponKind kind;
	if (GetDropInfo(category, kind)) {
		gameScene_->CreateDroppableItem(GetWorldPosition(), category, kind);
	}
}
bool BaseEnemy::ShouldWeaponBeEquipped() const {
	if (!behavior_)
		return false;
	Behavior current = behavior_->GetBehavior();
	if (current == Behavior::kMelee || current == Behavior::kAiming || current == Behavior::kReloading || current == Behavior::kBlock)
		return true;
	// IsChasing(): spotted, or still within the chase timeout / alerted by a nearby enemy.
	return behavior_->IsChasing() || behavior_->IsEngaged();
}

void BaseEnemy::UpdateWeaponEquipState() {
	if (ignoreEquipStates_) {
		weaponEquipped_ = true;
		equipLockTimer_ = 0.0f;
		return;
	}

	if (equipLockTimer_ > 0.0f)
		equipLockTimer_ = std::max(0.0f, equipLockTimer_ - 1.0f / 60.0f);

	bool want = ShouldWeaponBeEquipped();
	if (want != weaponEquipped_) {
		weaponEquipped_ = want;
		if (GetEquipStyle() == EquipStyle::kHeavy)
			equipLockTimer_ = kHeavyEquipLockDuration;
		ApplyWeaponEquipState(weaponEquipped_);
	}
}

void BaseEnemy::SetIgnoreEquipStates(bool ignore) {
	ignoreEquipStates_ = ignore;
	equipLockTimer_ = 0.0f;
	if (ignore) {
		weaponEquipped_ = true;
		SnapWeaponEquipState(true);
	}
}