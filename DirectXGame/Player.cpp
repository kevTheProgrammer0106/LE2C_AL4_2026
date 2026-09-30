#define NOMINMAX
#include <algorithm>
#include <array>
#include <cassert>
#include <numbers>

#include "EnemyCuirassier.h"
#include "EnemyRifleman.h"
#include "GameScene.h"
#include "GlobalVariables.h"
#include "MapChipField.h"
#include "Player.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "Player";
}

void Player::ApplyScrollPush() {
	if (!scrollPushActive_ || isDead_) {
		pushVelocityX_ = 0.0f;
		isBeingPushed_ = false;
		return;
	}
	float leftEdge = worldTransform_.translation_.x - kWidth / 2.0f;
	if (leftEdge >= scrollBoundaryX_) {
		pushVelocityX_ = std::max(0.0f, pushVelocityX_ - kPushAcceleration * 2.0f);
		isBeingPushed_ = pushVelocityX_ > 0.0f;
	} else {
		pushVelocityX_ += kPushAcceleration;
		pushVelocityX_ = std::clamp(pushVelocityX_, 0.0f, kMaxPushSpeed);
		isBeingPushed_ = true;
	}
	if (pushVelocityX_ > 0.0f)
		velocity_.x = std::max(velocity_.x, pushVelocityX_);
}

#pragma region GlobalVariables
void Player::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "TimeTurn", kTimeTurn);
	gv->AddItem(kGroupName, "Acceleration", kAcceleration);
	gv->AddItem(kGroupName, "Attenuation", kAttenuation);
	gv->AddItem(kGroupName, "LimitRunSpeed", kLimitRunSpeed);
	gv->AddItem(kGroupName, "GravityAcceleration", kGravityAcceleration);
	gv->AddItem(kGroupName, "LimitFallSpeed", kLimitFallSpeed);
	gv->AddItem(kGroupName, "JumpAcceleration", kJumpAcceleration);
	gv->AddItem(kGroupName, "Width", kWidth);
	gv->AddItem(kGroupName, "Height", kHeight);
	gv->AddItem(kGroupName, "Blank", kBlank);
	gv->AddItem(kGroupName, "AttenuationLanding", kAttenuationLanding);
	gv->AddItem(kGroupName, "AttenuationWall", kAttenuationWall);
	gv->AddItem(kGroupName, "AttenuationCeilling", kAttenuationCeilling);
	gv->AddItem(kGroupName, "BodyOffsetTranslation", bodyOffset_.translation);
	gv->AddItem(kGroupName, "BodyOffsetRotation", bodyOffset_.rotation);
	gv->AddItem(kGroupName, "BodyOffsetScale", bodyOffset_.scale);
	gv->AddItem(kGroupName, "HeadOffsetTranslation", headOffset_.translation);
	gv->AddItem(kGroupName, "HeadOffsetRotation", headOffset_.rotation);
	gv->AddItem(kGroupName, "HeadOffsetScale", headOffset_.scale);
	gv->AddItem(kGroupName, "HatOffsetTranslation", hatOffset_.translation);
	gv->AddItem(kGroupName, "HatOffsetRotation", hatOffset_.rotation);
	gv->AddItem(kGroupName, "HatOffsetScale", hatOffset_.scale);
	gv->AddItem(kGroupName, "FirearmOffsetTranslation", firearmOffset_.translation);
	gv->AddItem(kGroupName, "FirearmOffsetRotation", firearmOffset_.rotation);
	gv->AddItem(kGroupName, "FirearmOffsetScale", firearmOffset_.scale);
	gv->AddItem(kGroupName, "BlockAttackerKnockbackScale", kBlockAttackerKnockbackScale);
}
void Player::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kTimeTurn = gv->GetFloatValue(kGroupName, "TimeTurn");
	kAcceleration = gv->GetFloatValue(kGroupName, "Acceleration");
	kAttenuation = gv->GetFloatValue(kGroupName, "Attenuation");
	kLimitRunSpeed = gv->GetFloatValue(kGroupName, "LimitRunSpeed");
	kGravityAcceleration = gv->GetFloatValue(kGroupName, "GravityAcceleration");
	kLimitFallSpeed = gv->GetFloatValue(kGroupName, "LimitFallSpeed");
	kJumpAcceleration = gv->GetFloatValue(kGroupName, "JumpAcceleration");
	kWidth = gv->GetFloatValue(kGroupName, "Width");
	kHeight = gv->GetFloatValue(kGroupName, "Height");
	kBlank = gv->GetFloatValue(kGroupName, "Blank");
	kAttenuationLanding = gv->GetFloatValue(kGroupName, "AttenuationLanding");
	kAttenuationWall = gv->GetFloatValue(kGroupName, "AttenuationWall");
	kAttenuationCeilling = gv->GetFloatValue(kGroupName, "AttenuationCeilling");
	bodyOffset_.translation = gv->GetVector3Value(kGroupName, "BodyOffsetTranslation");
	bodyOffset_.rotation = gv->GetVector3Value(kGroupName, "BodyOffsetRotation");
	bodyOffset_.scale = gv->GetVector3Value(kGroupName, "BodyOffsetScale");
	headOffset_.translation = gv->GetVector3Value(kGroupName, "HeadOffsetTranslation");
	headOffset_.rotation = gv->GetVector3Value(kGroupName, "HeadOffsetRotation");
	headOffset_.scale = gv->GetVector3Value(kGroupName, "HeadOffsetScale");
	hatOffset_.translation = gv->GetVector3Value(kGroupName, "HatOffsetTranslation");
	hatOffset_.rotation = gv->GetVector3Value(kGroupName, "HatOffsetRotation");
	hatOffset_.scale = gv->GetVector3Value(kGroupName, "HatOffsetScale");
	firearmOffset_.translation = gv->GetVector3Value(kGroupName, "FirearmOffsetTranslation");
	firearmOffset_.rotation = gv->GetVector3Value(kGroupName, "FirearmOffsetRotation");
	firearmOffset_.scale = gv->GetVector3Value(kGroupName, "FirearmOffsetScale");

	kBlockAttackerKnockbackScale = gv->GetFloatValue(kGroupName, "BlockAttackerKnockbackScale");
}
#pragma endregion

#pragma region Debuffs 
bool Player::IsOnSlowTerrain() const {
	if (!mapChipField_)
		return false;
	Vector3 footPos = worldTransform_.translation_ + Vector3(0.0f, -(kHeight / 2.0f) - 0.05f, 0.0f);
	return mapChipField_->IsSlowAtPosition(footPos);
}

void Player::TickDebuffs() {
	if (!slowActive_ || slowUntilCleared_)
		return;
	slowTimer_ -= 1.0f / 60.0f;
	if (slowTimer_ <= 0.0f)
		ClearSlow();
}

void Player::ApplySlow(float factor, float duration) {
	slowActive_ = true;
	slowFactor_ = std::clamp(factor, 0.05f, 1.0f);
	slowUntilCleared_ = (duration <= 0.0f);
	slowTimer_ = std::max(duration, 0.0f);
}

void Player::ClearSlow() {
	slowActive_ = false;
	slowUntilCleared_ = false;
	slowFactor_ = 1.0f;
	slowTimer_ = 0.0f;
}

void Player::ClearAllDebuffs() {
	ClearSlow();
	ClearStun();
	ClearKnockback();
	controlLocked_ = false;
	squishTimer_ = 0;
}

Player::DebuffStatus Player::GetDebuffStatus() const {
	DebuffStatus s;
	s.stunned = behavior_.IsStunned();
	s.stunRemaining = behavior_.GetStunRemaining();
	s.knockedBack = behavior_.IsKnockback();
	s.slowedByTerrain = IsOnSlowTerrain();
	s.slowedByEffect = slowActive_;
	s.slowFactor = slowFactor_;
	s.slowRemaining = slowTimer_;
	s.slowUntilCleared = slowUntilCleared_;
	s.controlLocked = controlLocked_;
	s.beingPushed = isBeingPushed_;
	s.pushVelocityX = pushVelocityX_;
	s.squishTimer = squishTimer_;
	return s;
}

bool Player::HasAnyDebuff() const {
	DebuffStatus s = GetDebuffStatus();
	return s.stunned || s.knockedBack || s.slowedByTerrain || s.slowedByEffect || s.controlLocked || s.beingPushed;
}
#pragma endregion

Player::~Player() {}

void Player::Initialize(
    const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, KamataEngine::Model* modelAttack, uint32_t textureHandleAttack, KamataEngine::Camera* camera,
    const KamataEngine::Vector3& position) {
#ifdef _DEBUG
	assert(camera);
	assert(modelAttack);
#endif
	for (size_t i = 0; i < parts_.size(); ++i) {
		parts_[i].model = partResources[i].model;
		parts_[i].textureHandle = partResources[i].textureHandle;
		parts_[i].transform.Initialize();
	}
	modelAttack_ = modelAttack;
	modelAttackTex_ = textureHandleAttack;
	camera_ = camera;
	velocity_ = {0, 0, 0};

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	worldTransformAttack_.Initialize();
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;

	worldTransform_.translation_ = position;
}

void Player::SetMusketModel(Model* model, uint32_t textureHandle) {
	currentFirearm_ = std::make_unique<FirearmMusket>();
	currentFirearm_->Initialize(model, textureHandle, camera_);
}

void Player::SetMeleeModel(Model* model, uint32_t textureHandle) {
	currentMelee_ = std::make_unique<WeaponSabre>();
	currentMelee_->Initialize(model, textureHandle, camera_, worldTransform_.translation_);
}
void Player::SetWeaponMode(WeaponMode mode) {
	weaponMode_ = mode;
	if (currentMelee_)
		currentMelee_->SetWeaponState(weaponMode_ == WeaponMode::kMelee ? WeaponState::kEquipped : WeaponState::kHolstered);
}
Vector2 Player::ComputeMouseAimDir() const {
	auto mouse = Input::GetInstance()->GetMousePosition();
	Vector2 mouseScreen = {static_cast<float>(mouse.x), static_cast<float>(mouse.y)};
	Vector3 origin = GetWorldPosition();
	Matrix4x4 matViewport = MakeViewportMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight), 0.0f, 1.0f);
	Matrix4x4 matVPV = Multiply(camera_->matView, Multiply(camera_->matProjection, matViewport));
	Vector3 originScreen = TransformCoordinate(origin, matVPV);
	return {mouseScreen.x - originScreen.x, -(mouseScreen.y - originScreen.y)};
}
Vector2 Player::GetEffectiveAimDir() const {
	if (IsMovingHorizontally())
		return (lrDirection_ == LRDirection::kRight) ? Vector2{1.0f, 0.0f} : Vector2{-1.0f, 0.0f};
	return ComputeMouseAimDir();
}
void Player::UpdateAimAndFirearm() {
	if (!currentFirearm_)
		return;
	Vector2 aimDir = GetEffectiveAimDir();
	AimFacing aim = ComputeAimFacing(aimDir, lrDirection_ == LRDirection::kRight);
	if (!behavior_.IsAttacking() && aim.facingRight != (lrDirection_ == LRDirection::kRight)) {
		lrDirection_ = aim.facingRight ? LRDirection::kRight : LRDirection::kLeft;
		turnFirstRotationY_ = worldTransform_.rotation_.y;
		turnTimer_ = kTimeTurn;
	}
	currentFirearm_->SetEquipped(weaponMode_ == WeaponMode::kFirearm);
	currentFirearm_->UpdateFollow(worldTransform_, aim.localAngle, aim.facingRight);
}
void Player::UpdateMovementState() {
	AnimationManager::AnimationContext context;
	context.velocity = velocity_;
	context.onGround = onGround_;
	context.isDead = isDead_;
	animManager_.UpdateMovementState(context);
}
void Player::KillInstant() {
	if (isDead_ || behavior_.IsDeathBehavior())
		return;
	behavior_.RequestDeath({0.0f, 1.0f, 0.0f}, 1.0f);
}

void Player::UseFirearm() {
	if (!currentFirearm_ || weaponMode_ != WeaponMode::kFirearm)
		return;
	if (!Input::GetInstance()->IsTriggerMouse(0))
		return;
	if (!currentFirearm_->TryFire())
		return;

	Vector3 origin = GetWorldPosition();
	Vector2 aimDir = GetEffectiveAimDir();

	if (gameScene_) {
		gameScene_->CreateFirearmProjectile(Owner::kPlayer, origin, aimDir);
		Vector3 muzzlePos = origin;
		gameScene_->CreateMuzzleFlash(muzzlePos);
	}
	currentFirearm_->TriggerRecoil(lrDirection_ == LRDirection::kRight);
	BaseFirearm::PlayFireSound();
}
void Player::UseMeleeWeapon() {
	if (weaponMode_ != WeaponMode::kMelee)
		return;
	if (Input::GetInstance()->IsTriggerMouse(0)) {
		currentMelee_->SetWeaponState(WeaponState::kEquipped);
		PlayerBehavior::AttackDirection direction = PlayerBehavior::AttackDirection::kSide;
		if (Input::GetInstance()->PushKey(DIK_W))
			direction = PlayerBehavior::AttackDirection::kUp;
		else if (Input::GetInstance()->PushKey(DIK_S))
			direction = PlayerBehavior::AttackDirection::kDown;
		behavior_.RequestAttack(direction);
	}
}
void Player::Update() {
	ApplyGlobalVariables();
	TickDebuffs(); // [NEW]
	if (currentFirearm_)
		currentFirearm_->TickRecoil();

	if (!controlLocked_) {
		if (Input::GetInstance()->TriggerKey(DIK_Q)) {
			SetWeaponMode(weaponMode_ == WeaponMode::kMelee ? WeaponMode::kFirearm : WeaponMode::kMelee);
		}
		bool isBlockingNow = Input::GetInstance()->IsPressMouse(1);
		if (isBlockingNow && !behavior_.IsAttacking()) {
			Vector2 aimDir = ComputeMouseAimDir();
			AimFacing aim = ComputeAimFacing(aimDir, lrDirection_ == LRDirection::kRight);
			if (aim.facingRight != (lrDirection_ == LRDirection::kRight)) {
				lrDirection_ = aim.facingRight ? LRDirection::kRight : LRDirection::kLeft;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
			}
		}
		behavior_.SetBlocking(isBlockingNow);
	}

	behavior_.ResolveBehaviorRequest();

	if (isDead_)
		return;

	behavior_.UpdateCurrentBehavior();
	currentMelee_->UpdateWeaponState();
	currentMelee_->SetMeleePosition(*this);

	UpdateMovementState();
	UpdateBodyParts();

	if (currentFirearm_) {
		currentFirearm_->SetOffset(firearmOffset_.scale, firearmOffset_.rotation, firearmOffset_.translation);
		UpdateAimAndFirearm();
	}

	if (!controlLocked_) {
		UseFirearm();
		UseMeleeWeapon();
	}
}
void Player::Draw() {
	Model::PreDraw();
	for (auto& part : parts_) {
		if (!isDead_ && part.model)
			part.model->Draw(part.transform, *camera_, part.textureHandle);
	}
	if (behavior_.IsAttacking() && behavior_.GetAttackAnimState().phase == PlayerBehavior::AttackPhase::kAttack) {
		modelAttack_->Draw(worldTransformAttack_, *camera_, modelAttackTex_);
	}
	Model::PostDraw();

	if (!isDead_ && !behavior_.IsDeathBehavior()) {
		currentMelee_->Draw();
		if (currentFirearm_)
			currentFirearm_->Draw();
	}
}

const WorldTransform& Player::GetWorldTransform() const { return worldTransform_; }
void Player::SetWorldTransform(const KamataEngine::WorldTransform& worldTransform) {
	worldTransform_.translation_ = worldTransform.translation_;
	worldTransform_.rotation_ = worldTransform.rotation_;
	worldTransform_.scale_ = worldTransform.scale_;
}
Vector3 Player::GetWorldPosition() const { return {worldTransform_.translation_.x, worldTransform_.translation_.y, worldTransform_.translation_.z}; }

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};
	return aabb;
}
void Player::GroundMovement() {
	if (!onGround_) {
		velocity_ += Vector3(0, -kGravityAcceleration, 0);
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
	float speedFactor = 1.0f;
	if (IsOnSlowTerrain())
		speedFactor = MapChipField::kSlowSpeedFactor;
	// [NEW] timed/latched Slow debuff stacks by taking the stronger (smaller) factor
	if (slowActive_)
		speedFactor = std::min(speedFactor, slowFactor_);
	if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_A)) {
		Vector3 acceleration = {};
		if (Input::GetInstance()->PushKey(DIK_D)) {
			if (velocity_.x < 0.0f)
				velocity_.x *= (1.0f - kAttenuation);
			acceleration.x += kAcceleration * speedFactor;
			if (!behavior_.IsAttacking() && lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
			}
		} else if (Input::GetInstance()->PushKey(DIK_A)) {
			if (velocity_.x > 0.0f)
				velocity_.x *= (1.0f - kAttenuation);
			acceleration.x -= kAcceleration * speedFactor;
			if (!behavior_.IsAttacking() && lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
			}
		}
		velocity_ += acceleration;
		velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed * speedFactor, kLimitRunSpeed * speedFactor);
	} else {
		velocity_.x *= (1.0f - kAttenuation);
	}
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		worldTransform_.rotation_.y = EaseInOut(turnFirstRotationY_, destinationRotationY, 1.0f - (turnTimer_ / kTimeTurn));
	}
}
void Player::JumpMovement() {
	if (onGround_ && Input::GetInstance()->PushKey(DIK_SPACE))
		velocity_ += Vector3(0, kJumpAcceleration, 0);
}

void Player::UpdateBodyParts() {
	SRTOffset anim;
	if (behavior_.IsAttacking()) {
		anim = GetAttackPoseOffset();
	} else {
		anim = animManager_.GetMovementAnimOffset();
		if (currentFirearm_) {
			SRTOffset aimLean = BaseFirearm::ComputeAimLeanOffset(ComputeMouseAimDir(), lrDirection_ == LRDirection::kRight);
			anim = {
			    {anim.scale.x * aimLean.scale.x, anim.scale.y * aimLean.scale.y, anim.scale.z * aimLean.scale.z},
                anim.rotation + aimLean.rotation, anim.translation + aimLean.translation
            };
		}
	}
	if (currentFirearm_) {
		SRTOffset recoil = currentFirearm_->GetRecoilOffset();
		anim.translation += recoil.translation;
	}

	Matrix4x4 worldRotMatrix = Multiply(MakeRotateXMatrix(worldTransform_.rotation_.x), Multiply(MakeRotateYMatrix(worldTransform_.rotation_.y), MakeRotateZMatrix(worldTransform_.rotation_.z)));
	Vector3 rotatedBodyOffset = TransformNormal(bodyOffset_.translation, worldRotMatrix);

	parts_[body].transform.translation_ = worldTransform_.translation_ + rotatedBodyOffset + anim.translation;
	parts_[body].transform.rotation_ = worldTransform_.rotation_ + bodyOffset_.rotation + anim.rotation;
	parts_[body].transform.scale_ = {
	    worldTransform_.scale_.x * bodyOffset_.scale.x * anim.scale.x, worldTransform_.scale_.y * bodyOffset_.scale.y * anim.scale.y, worldTransform_.scale_.z * bodyOffset_.scale.z * anim.scale.z};
	WorldTransformUpdate(parts_[body].transform);

	Matrix4x4 rotMatrix =
	    Multiply(MakeRotateXMatrix(parts_[body].transform.rotation_.x), Multiply(MakeRotateYMatrix(parts_[body].transform.rotation_.y), MakeRotateZMatrix(parts_[body].transform.rotation_.z)));
	Vector3 rotatedHeadOffset = TransformNormal(headOffset_.translation, rotMatrix);
	parts_[head].transform.translation_ = parts_[body].transform.translation_ + rotatedHeadOffset;
	parts_[head].transform.rotation_ = parts_[body].transform.rotation_ + headOffset_.rotation;
	parts_[head].transform.scale_ = {
	    parts_[body].transform.scale_.x * headOffset_.scale.x, parts_[body].transform.scale_.y * headOffset_.scale.y, parts_[body].transform.scale_.z * headOffset_.scale.z};
	WorldTransformUpdate(parts_[head].transform);

	Matrix4x4 headRotMatrix =
	    Multiply(MakeRotateXMatrix(parts_[head].transform.rotation_.x), Multiply(MakeRotateYMatrix(parts_[head].transform.rotation_.y), MakeRotateZMatrix(parts_[head].transform.rotation_.z)));
	Vector3 rotatedHatOffset = TransformNormal(hatOffset_.translation, headRotMatrix);
	parts_[hat].transform.translation_ = parts_[head].transform.translation_ + rotatedHatOffset;
	parts_[hat].transform.rotation_ = parts_[head].transform.rotation_ + hatOffset_.rotation;
	parts_[hat].transform.scale_ = {parts_[head].transform.scale_.x * hatOffset_.scale.x, parts_[head].transform.scale_.y * hatOffset_.scale.y, parts_[head].transform.scale_.z * hatOffset_.scale.z};
	WorldTransformUpdate(parts_[hat].transform);
}

void Player::OnCollision(const BaseEnemy* enemy) {
	if (behavior_.IsKnockback())
		return;
	if (isAttack()) {
		if (enemy->WasBlocked()) {
			float pushDirection = (lrDirection_ == LRDirection::kRight) ? -1.0f : 1.0f;
			behavior_.RequestKnockback(pushDirection);
		}
		return;
	}
	if (enemy->IsCollisionDisabled())
		return;
}

void Player::TakeDamage(int damage, const Vector3& hitDirection, float impactPower, BaseEnemy* attacker) {
	if (isDead_ || behavior_.IsDeathBehavior())
		return;

	if (behavior_.IsBlocking()) {
		Vector3 effectPos = GetWorldPosition();
		if (gameScene_)
			gameScene_->CreateBlockSparkEffect(effectPos);
		behavior_.RequestHitKnockback(hitDirection, impactPower * 0.3f);
		if (attacker) {
			Vector3 pushDir = attacker->GetWorldPosition() - GetWorldPosition();
			float len = std::sqrtf(pushDir.x * pushDir.x + pushDir.y * pushDir.y);
			if (len > 0.0001f) {
				pushDir.x /= len;
				pushDir.y /= len;
			} else {
				pushDir = {isRight() ? 1.0f : -1.0f, 0.0f, 0.0f};
			}
			pushDir.y += 0.3f;
			attacker->RequestKnockback(pushDir, impactPower * kBlockAttackerKnockbackScale);
			PlayerBehavior::PlayBlockImpactSound();
		}
		return;
	}

	Vector3 effectPos = GetWorldPosition();
	if (gameScene_)
		gameScene_->CreateHitEffect(effectPos);

	hp_ = std::max(0, hp_ - damage);
	if (hp_ <= 0)
		behavior_.RequestDeath(hitDirection, impactPower);
	else
		behavior_.RequestHitKnockback(hitDirection, impactPower);
	PlayerBehavior::PlayHurtSound();
}

void Player::OnCollision() {
	if (behavior_.IsKnockback() || behavior_.IsDeathBehavior())
		return;
	TakeDamage(hp_, {0.0f, 1.0f, 0.0f}, 1.0f);
}

void Player::PositionUpdate(const CollisionInfo& collisionInfo) { worldTransform_.translation_ += collisionInfo.move; }
void Player::CeillingCollision(const CollisionInfo& collisionInfo) {
	if (collisionInfo.isHitTop)
		velocity_.y *= (1.0f - kAttenuationCeilling);
}
void Player::WallCollision(const CollisionInfo& collisionInfo) {
	if (collisionInfo.isHitWall)
		velocity_.x *= (1.0f - kAttenuationWall);
}

void Player::MapChipCollisionUpdate() {
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

	if (isBeingPushed_ && horizontalInfo.isHitWall) {
		++squishTimer_;
		if (squishTimer_ >= kSquishThreshold)
			isDead_ = true;
	} else {
		squishTimer_ = 0;
	}
}
void Player::MapChipCollisionUpdate(CollisionInfo& collisionInfo) {
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