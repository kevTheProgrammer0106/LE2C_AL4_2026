#include "BaseEnemy.h"
#include "BaseProjectile.h"
#include "BaseFirearm.h"
#include "GameScene.h"
#include "GlobalVariables.h"
#include "MapChipField.h"
#include "Player.h"
#include "OfficerBuff.h"

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
	gv->AddItem(kGroupName, "ModelStandLerp", kModelStandLerp);
	gv->AddItem(kGroupName, "ModelMaxOffset", kModelMaxOffset);
	gv->AddItem(kGroupName, "ModelProbeDepth", kModelProbeDepth);
	gv->AddItem(kGroupName, "ModelProbeStepUp", kModelProbeStepUp);
	gv->AddItem(kGroupName, "DropThroughDuration", kDropThroughDuration);
	gv->AddItem(kGroupName, "DropThroughVelocity", kDropThroughVelocity);
	gv->AddItem(kGroupName, "AttackEffectScale", kAttackEffectScale);
	gv->AddItem(kGroupName, "AuraInterval", kAuraInterval);
	gv->AddItem(kGroupName, "BulletKnockbackScale", kBulletKnockbackScale);
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
	kModelStandLerp = gv->GetFloatValue(kGroupName, "ModelStandLerp");
	kModelMaxOffset = gv->GetFloatValue(kGroupName, "ModelMaxOffset");
	kModelProbeDepth = gv->GetFloatValue(kGroupName, "ModelProbeDepth");
	kModelProbeStepUp = gv->GetFloatValue(kGroupName, "ModelProbeStepUp");
	kDropThroughDuration = gv->GetFloatValue(kGroupName, "DropThroughDuration");
	kDropThroughVelocity = gv->GetFloatValue(kGroupName, "DropThroughVelocity");
	kAttackEffectScale = gv->GetFloatValue(kGroupName, "AttackEffectScale");
	kAuraInterval = gv->GetFloatValue(kGroupName, "AuraInterval");
	kBulletKnockbackScale = gv->GetFloatValue(kGroupName, "BulletKnockbackScale");
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
		blockDetector_.SetUseDetailedTiles(false); // ルート: 全タイルを1.0セルとして扱う
		detectorProbe_.SetUseDetailedTiles(true);  // ディテクター: タイルの実形状・オフセット
		blockDetector_.SetIgnoreBarrier(true);
		detectorProbe_.SetIgnoreBarrier(true);
	}
	behavior_->Initialize();

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_.y = -(std::numbers::pi_v<float> / 2.0f);
	walkTimer_ = 0.0f;
	worldTransform_.translation_ = position;
	modelWorldTransform_.Initialize();
	modelWorldTransform_.translation_ = position;
	modelWorldTransform_.rotation_ = worldTransform_.rotation_;
	modelStandOffsetY_ = 0.0f;
	dropThroughTimer_ = 0.0f;
	CacheAABBMarkerSize();

	if (!pathfindingModule_)
		pathfindingModule_ = std::make_unique<PathFindingAI>();
	pathfindingModule_->Initialize(this, mapChipField_);
}

AnimationManager::AnimationContext BaseEnemy::BuildAnimationContext() const {
	AnimationManager::AnimationContext context;
	context.velocity = velocity_;
	context.onGround = onGround_;
	context.isDead = behavior_->IsDeathBehavior();
	context.isKnockedBack = behavior_->IsKnockedBack();
	context.isStunned = behavior_->IsStunned();
	context.isBlocking = behavior_->GetBehavior() == Behavior::kBlock;
	context.facingRight = isRight();
	context.isAttacking = behavior_->GetBehavior() == Behavior::kMelee;
	context.actionPose = GetActionAnimOffset();
	return context;
}

void BaseEnemy::UpdateMovementState() {
	AnimationManager::MovementEvents events = animManager_->UpdateMovementState(BuildAnimationContext());
	if (!behavior_->IsDeathBehavior()) {
		const Vector3 feet = worldTransform_.translation_ + Vector3(0.0f, -kHeight * 0.5f, 0.0f);
		if (events.justJumped) {
			PlayJumpSound();
			if (gameScene_)
				gameScene_->CreateBurstEffect(BurstType::kJump, feet, {0.0f, 1.0f});
		}
		if (events.justLanded) {
			PlayMovementSound();
			if (gameScene_)
				gameScene_->CreateBurstEffect(BurstType::kLand, feet, {0.0f, 1.0f});
		}
		if (events.footstepTick)
			PlayFootstepSound();
	}
}

void BaseEnemy::UpdateBodyParts() {
	animManager_->UpdateBodyParts(modelWorldTransform_, BuildAnimationContext(), parts_); 
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
	if (behavior_->IsDeathBehavior())
		UpdateModelCollision(); // 死亡中は物理更新を通らないのでここでモデルを追従させる
	UpdateBodyParts();
	UpdateBuffAura();
}
void BaseEnemy::UpdateBuffAura() {
	if (!OfficerBuff::gActive || !gameScene_ || isDead_ || behavior_->IsDeathBehavior()) {
		auraTimer_ = 0.0f; // the first burst appears right away when the buff lands
		return;
	}
	auraTimer_ -= 1.0f / 60.0f;
	if (auraTimer_ > 0.0f)
		return;
	auraTimer_ = kAuraInterval;

	// under the body: the model's feet, nudged up so the motes start inside the ground contact
	Vector3 feet = GetModelWorldPosition() + Vector3(0.0f, -kHeight * 0.5f + 0.05f, 0.0f);
	gameScene_->CreateBurstEffect(BurstType::kAura, feet, {0.0f, 1.0f});
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

AABB BaseEnemy::GetAABB() { return static_cast<const BaseEnemy*>(this)->GetAABB(); }
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
bool BaseEnemy::PerformJump() {
	if (!onGround_)
		return false;
	velocity_.y = kJumpAcceleration;
	return true;
}
void BaseEnemy::TakeDamage(int damage, const Vector3& hitDirection, float impactPower, bool ignoreBuff) {
	if (behavior_->IsDeathBehavior() || isDead_)
		return;

	if (!ignoreBuff)
		damage = OfficerBuff::ReduceDamage(damage);

	hp_ -= damage;
	pendingHitDirection_ = hitDirection;
	pendingImpactPower_ = impactPower;

	Vector3 effectPos = GetModelWorldPosition();
	if (gameScene_) {
		gameScene_->CreateHitEffect(effectPos);
		gameScene_->CreateBurstEffect(BurstType::kHit, effectPos, {hitDirection.x, hitDirection.y}); // 被弾の火花
	}

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
		if (TryBlockAttack(player->GetWorldPosition()))
			return; 
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
			power = melee->GetImpactPower(player->GetComboIndex(), Owner::kPlayer);
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
	if (TryBlockAttack(bullet->GetWorldPosition()))
		return;
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
	// 銃弾が当たった時の小さなノックバック(スタンなし)。倒した時は死亡の吹き飛びに任せるので付けない
	if (hp_ > 0) {
		// 弾の進行方向へ押す。速度が無い弾だけ上のdirにフォールバックする
		const Vector3 v = bullet->GetVelocity();
		const float vLen = std::sqrtf(v.x * v.x + v.y * v.y);
		const Vector3 pushDir = (vLen > 0.0001f) ? Vector3{v.x / vLen, v.y / vLen, 0.0f} : dir;
		RequestBulletKnockback(pushDir, bullet->GetImpactPower() * kBulletKnockbackScale);
	}
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

void BaseEnemy::JumpMovement(const CollisionInfo& collisionInfo) {
	if (!onGround_)
		return;
	if (behavior_->IsDebuffed() || behavior_->IsEngaged())
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
		PerformJump();
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
	if (dropThroughTimer_ > 0.0f) {
		dropThroughTimer_ = std::max(0.0f, dropThroughTimer_ - 1.0f / 60.0f);
	}

	if (ignoreMapCollision_) {
		// マップ判定を無視する移動: 速度をそのまま位置へ反映する(重力は呼び出し側のBehaviorで扱う)
		worldTransform_.translation_ += velocity_;
		onGround_ = false;
		hitWallThisFrame_ = false;
	} else {
		behavior_->ApplySeparationFromOtherEnemies();

		if (!onGround_) {
			velocity_ += Vector3(0, -kGravityAcceleration, 0);
			velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
		}

		MapChipCollisionUpdate();
	}

	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		worldTransform_.rotation_.y = EaseInOut(turnFirstRotationY_, destinationRotationY, 1.0f - (turnTimer_ / kTimeTurn));
	}
	// ターン後の回転を反映してからモデル層を更新する
	UpdateModelCollision();
}
float BaseEnemy::GetSlowTerrainScale() const {
	// 空中ではslowBlockの影響を受けない(プレイヤーと同じ扱い)
	if (!mapChipField_ || !onGround_)
		return 1.0f;
	const Vector3 footPos = worldTransform_.translation_ + Vector3(0.0f, -(kHeight * 0.5f) - 0.05f, 0.0f);
	return mapChipField_->IsSlowAtPosition(footPos) ? MapChipField::kSlowSpeedFactor : 1.0f;
}
void BaseEnemy::MapChipCollisionUpdate() {
	const Vector3 velocityBefore = velocity_; // ノックバックの跳ね返り用
	blockDetector_.SetWorldTransform(worldTransform_);
	blockDetector_.SetVelocity(velocity_);
	blockDetector_.SetOnGround(onGround_);
	blockDetector_.SetMapChipField(mapChipField_);
	blockDetector_.SetSize(kWidth, kHeight);
	blockDetector_.SetIgnoreOneWay(IsDroppingThrough()); // ドロップスルー中は足場への着地だけ無視

	CollisionInfo horizontalInfo{};
	// ノックバック/スタン中の吹き飛びはslowBlockで遅くしない(OfficerBuffのMoveScaleと同じ扱い)
	const bool suppressed = behavior_ && behavior_->IsDebuffed();
	const float slowScale = suppressed ? 1.0f : GetSlowTerrainScale();
	horizontalInfo.move.x = velocity_.x * OfficerBuff::MoveScale(suppressed) * slowScale;
	blockDetector_.MapChipCollision(horizontalInfo);
	if (behavior_) {
		BaseEnemyBehavior::LedgeGuardRequest guard = behavior_->GetLedgeGuardRequest();
		if (guard.active) {
			blockDetector_.LedgeGuard(horizontalInfo, guard.speedLimit);
		}
	}
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

	if (behavior_ && behavior_->IsKnockedBack()) {
		behavior_->ApplyKnockbackBounce(velocityBefore, horizontalInfo, verticalInfo);
	}

	JumpMovement(horizontalInfo);
}
void BaseEnemy::MapChipCollisionUpdate(CollisionInfo& collisionInfo) {
	blockDetector_.SetWorldTransform(worldTransform_);
	blockDetector_.SetVelocity(velocity_);
	blockDetector_.SetOnGround(onGround_);
	blockDetector_.SetMapChipField(mapChipField_);
	blockDetector_.SetSize(kWidth, kHeight);
	blockDetector_.SetIgnoreOneWay(IsDroppingThrough()); // ドロップスルー中は足場への着地だけ無視

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
	DropItemState state = DropItemState::kReady;
	if (GetDropInfo(category, kind)) {
		gameScene_->CreateDroppableItem(GetWorldPosition(), category, kind, state);
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
void BaseEnemy::UpdateModelCollision() {
	// ルートとディテクターは同じ位置・同じ 0.8 x 1.6 のサイズ
	const Vector3 rootPos = worldTransform_.translation_;

	float targetOffsetY = 0.0f;
	if (onGround_ && !isDead_ && !behavior_->IsDeathBehavior() && mapChipField_) {
		blockDetector_.SetMapChipField(mapChipField_);
		blockDetector_.SetSize(kWidth, kHeight);
		detectorProbe_.SetMapChipField(mapChipField_);
		detectorProbe_.SetSize(kWidth, kHeight);

		float rootTopY = 0.0f;     // ルートが立っていると思っている面(セル全体)
		float detectorTopY = 0.0f; // ディテクターが見つけた実際の面(タイル実形状)
		if (blockDetector_.ProbeGroundTop(rootPos, kModelProbeStepUp, kModelProbeDepth, rootTopY) && detectorProbe_.ProbeGroundTop(rootPos, kModelProbeStepUp, kModelProbeDepth, detectorTopY)) {
			targetOffsetY = std::clamp(detectorTopY - rootTopY, -kModelMaxOffset, kModelMaxOffset);
		}
	}
	// 空中・死亡中は目標0なので、モデルはルートへ滑らかに戻る
	modelStandOffsetY_ = Lerp(modelStandOffsetY_, targetOffsetY, kModelStandLerp);

	modelWorldTransform_.translation_ = rootPos + Vector3(0.0f, modelStandOffsetY_, 0.0f);
	modelWorldTransform_.rotation_ = worldTransform_.rotation_;
	modelWorldTransform_.scale_ = worldTransform_.scale_;
	WorldTransformUpdate(modelWorldTransform_);
}

bool BaseEnemy::IsStandingOnPlatform() const {
	if (!mapChipField_ || !onGround_) {
		return false;
	}
	const float checkOffset = 0.05f;
	Vector3 leftFoot = worldTransform_.translation_ + Vector3(-kWidth * 0.5f + kBlank, -kHeight * 0.5f - checkOffset, 0.0f);
	Vector3 rightFoot = worldTransform_.translation_ + Vector3(kWidth * 0.5f - kBlank, -kHeight * 0.5f - checkOffset, 0.0f);
	// 左右どちらかの足元が一方通行足場なら、足場に立っている
	return mapChipField_->IsOneWayAtPosition(leftFoot) || mapChipField_->IsOneWayAtPosition(rightFoot);
}

void BaseEnemy::RequestDropThrough() {
	if (!IsStandingOnPlatform()) {
		return;
	}
	dropThroughTimer_ = kDropThroughDuration;
	onGround_ = false; // 接地を解除して下へ進ませる
	velocity_.y = std::min(velocity_.y, kDropThroughVelocity);
}
bool BaseEnemy::TryBlockAttack(const Vector3& attackerPos) {
	if (isDead_ || !behavior_ || behavior_->IsDeathBehavior())
		return false;

	// only the facing side is guarded
	float dx = attackerPos.x - worldTransform_.translation_.x;
	bool attackerInFront = isRight() ? (dx >= 0.0f) : (dx <= 0.0f);
	if (!attackerInFront)
		return false;

	bool blocked = behavior_->IsBlocking() || behavior_->IsBlockPending() || behavior_->TryStartBlockOnHit();
	if (!blocked)
		return false;

	wasBlocked_ = true;
	Vector3 sparkPos = GetModelWorldPosition();
	sparkPos.x += (isRight() ? 1.0f : -1.0f) * (kWidth * 0.5f);
	if (gameScene_)
		gameScene_->CreateBlockSparkEffect(sparkPos);
	return true;
}
void BaseEnemy::DrawAttackEffect(bool active, const AABB& swingBox) {
	if (!active || !modelAttack_ || !camera_ || isDead_)
		return;

	// 向きは lrDirection_ から直接決める(ルートの回転はターン補間中にずれる)
	const bool facingRight = isRight();

	// 斬撃は当たり判定(スイングAABB)の中心に置く。リーチや高さが変わっても判定の中に収まる
	worldTransformAttack_.translation_ = {(swingBox.min.x + swingBox.max.x) * 0.5f, (swingBox.min.y + swingBox.max.y) * 0.5f, worldTransform_.translation_.z};
	worldTransformAttack_.rotation_ = {0.0f, facingRight ? 1.570796f : 4.712389f, 0.0f};
	worldTransformAttack_.scale_ = {kAttackEffectScale, kAttackEffectScale, kAttackEffectScale};
	WorldTransformUpdate(worldTransformAttack_);

	Model::PreDraw();
	modelAttack_->Draw(worldTransformAttack_, *camera_, modelAttackTex_);
	Model::PostDraw();
}
bool BaseEnemy::AttackObstacle(const Vector3& cellPos) {
	if (!behavior_ || behavior_->IsDebuffed() || behavior_->IsDeathBehavior())
		return false;
	BaseMelee* melee = GetObstacleMelee();
	if (!melee)
		return false;
	if (obstacleSwingFrame_ >= 0 || obstacleCooldown_ > 0.0f)
		return true; // swinging / recovering: hold position
	FaceDirection(cellPos.x < worldTransform_.translation_.x ? LRDirection::kLeft : LRDirection::kRight);
	melee->ResetSwingHits();
	melee->PlaySwingSound(0);
	obstacleSwingFrame_ = 0;
	return true;
}

bool BaseEnemy::TickObstacleSwing() {
	if (obstacleCooldown_ > 0.0f)
		obstacleCooldown_ = std::max(0.0f, obstacleCooldown_ - 1.0f / 60.0f);

	BaseMelee* melee = GetObstacleMelee();
	if (obstacleSwingFrame_ < 0)
		return false;
	if (!melee || !behavior_ || behavior_->IsDebuffed() || behavior_->IsDeathBehavior() || behavior_->GetBehavior() == Behavior::kMelee) {
		obstacleSwingFrame_ = -1; // interrupted
		return false;
	}

	++obstacleSwingFrame_;
	const float anticipate = melee->GetComboAnticipateDuration(0);
	const float attack = melee->GetComboAttackDuration(0);
	const float recovery = melee->GetComboRecoveryDuration(0);
	const float f = static_cast<float>(obstacleSwingFrame_);

	BaseMelee::GenericAttackAnimState state;
	state.comboIndex = 0;
	if (f < anticipate) {
		state.phase = BaseMelee::AttackPhase::kAnticipate;
		state.t = f / anticipate;
	} else if (f < anticipate + attack) {
		state.phase = BaseMelee::AttackPhase::kAttack;
		state.t = (f - anticipate) / attack;
		if (gameScene_)
			gameScene_->HitObstaclesInAABB(melee->GetSwingAABBGeneric(worldTransform_.translation_, isRight(), 0), melee);
	} else if (f < anticipate + attack + recovery) {
		state.phase = BaseMelee::AttackPhase::kRecovery;
		state.t = (f - anticipate - attack) / recovery;
	} else {
		obstacleSwingFrame_ = -1;
		obstacleCooldown_ = kObstacleCooldown;
		return false;
	}
	ApplyObstacleSwingVisual(state);
	return true;
}

bool BaseEnemy::FireAtObstacle(BaseFirearm* firearm, const Vector3& cellPos) {
	if (!firearm || !gameScene_ || !behavior_ || behavior_->IsDebuffed() || behavior_->IsDeathBehavior())
		return false;
	Vector2 dir = {cellPos.x - worldTransform_.translation_.x, cellPos.y - worldTransform_.translation_.y};
	const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	if (len < 0.0001f)
		return false;
	dir.x /= len;
	dir.y /= len;
	FaceDirection(dir.x < 0.0f ? LRDirection::kLeft : LRDirection::kRight);

	if (!firearm->TryFire())
		return true; // reloading: wait in front of it

	const Vector3 origin = GetWorldPosition();
	const int pelletCount = std::max(1, firearm->GetPelletCount());
	for (int i = 0; i < pelletCount; ++i) {
		const float angle = firearm->GetPelletAngle(i);
		const float c = std::cos(angle);
		const float s = std::sin(angle);
		const Vector2 d = {dir.x * c - dir.y * s, dir.x * s + dir.y * c};
		gameScene_->CreateFirearmProjectile(Owner::kEnemy, origin, d, firearm->GetBulletDamage(), firearm->GetProjectileLifeFrames());
	}
	gameScene_->CreateMuzzleFlash(origin + Vector3(dir.x * 0.8f, dir.y * 0.8f, 0.0f), dir, false);
	firearm->TriggerRecoil(isRight());
	BaseFirearm::PlayFireSound();
	return true;
}