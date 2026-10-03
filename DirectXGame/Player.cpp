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
	gv->AddItem(kGroupName, "BlockExtraWidth", kBlockExtraWidth);
	gv->AddItem(kGroupName, "ModelStandLerp", kModelStandLerp);
	gv->AddItem(kGroupName, "ModelMaxOffset", kModelMaxOffset);
	gv->AddItem(kGroupName, "ModelProbeDepth", kModelProbeDepth);
	gv->AddItem(kGroupName, "ModelProbeStepUp", kModelProbeStepUp);
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
	kBlank = gv->GetFloatValue(kGroupName, "Blank");
	kWidth = gv->GetFloatValue(kGroupName, "Width");
	kHeight = gv->GetFloatValue(kGroupName, "Height");
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
	kBlockExtraWidth = gv->GetFloatValue(kGroupName, "BlockExtraWidth");

	kModelStandLerp = gv->GetFloatValue(kGroupName, "ModelStandLerp");
	kModelMaxOffset = gv->GetFloatValue(kGroupName, "ModelMaxOffset");
	kModelProbeDepth = gv->GetFloatValue(kGroupName, "ModelProbeDepth");
	kModelProbeStepUp = gv->GetFloatValue(kGroupName, "ModelProbeStepUp");
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
	modelWorldTransform_.Initialize();
	modelWorldTransform_.rotation_ = worldTransform_.rotation_;
	blockDetector_.SetUseDetailedTiles(false); // root: plain 1.0 cells
	detectorProbe_.SetUseDetailedTiles(true);  // detector: real tile shapes/offsets

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
	Vector3 origin = GetModelWorldPosition();
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
	currentFirearm_->UpdateFollow(modelWorldTransform_, aim.localAngle, aim.facingRight);
}
void Player::UpdateMovementState() {
	AnimationManager::AnimationContext context;
	context.velocity = velocity_;
	context.onGround = onGround_;
	context.isDead = isDead_ || behavior_.IsDeathBehavior();
	context.isKnockedBack = behavior_.IsKnockback();
	context.isStunned = behavior_.IsStunned();
	context.isBlocking = behavior_.GetBehavior() == Behavior::kBlock;
	context.facingRight = (lrDirection_ == LRDirection::kRight);

	// アニメーションが返すイベント(ジャンプ/着地/足音)でSFXを鳴らす
	AnimationManager::MovementEvents events = animManager_.UpdateMovementState(context);
	if (!isDead_ && !behavior_.IsDeathBehavior()) {
		if (events.justJumped) {
			PlayMovementSfxHandle(sJumpSoundHandle_);
		}
		if (events.justLanded) {
			PlayMovementSfxHandle(sLandSoundHandle_);
		}
		if (events.footstepTick) {
			PlayMovementSfxHandle(sFootstepSoundHandle_);
		}
	}
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

	Vector3 origin = GetModelWorldPosition();
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
		if (Input::GetInstance()->PushKey(DIK_S))
			direction = PlayerBehavior::AttackDirection::kDown;
		behavior_.RequestAttack(direction);
	}
}
void Player::Update() {
	ApplyGlobalVariables();
	TickDebuffs();

	if (dropThroughTimer_ > 0.0f) {
		dropThroughTimer_ -= 1.0f / 60.0f;
		if (dropThroughTimer_ < 0.0f) {
			dropThroughTimer_ = 0.0f;
		}
	}

	if (currentFirearm_)
		currentFirearm_->TickRecoil();

	if (behavior_.IsStunned())
		behavior_.SetBlocking(false);
	const bool canAct = !controlLocked_ && !behavior_.IsStunned();
	if (canAct) {
		if (Input::GetInstance()->TriggerKey(DIK_Q)) {
			SetWeaponMode(weaponMode_ == WeaponMode::kMelee ? WeaponMode::kFirearm : WeaponMode::kMelee);
			PlaySfxHandle(sWeaponSwitchSoundHandle_);
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
	if (canAct && Input::GetInstance()->PushKey(DIK_S)) {
		RequestDropThrough();
	}

	behavior_.ResolveBehaviorRequest();

	if (isDead_)
		return;

	behavior_.UpdateCurrentBehavior();
	UpdateModelCollision();     
	currentMelee_->UpdateWeaponState();
	currentMelee_->SetMeleePosition(*this);

	ReactionPose reactionPose = ReactionPose::kNone;
	if (behavior_.IsKnockback())
		reactionPose = ReactionPose::kKnockback;
	else if (behavior_.IsStunned())
		reactionPose = ReactionPose::kStun;
	else if (behavior_.GetBehavior() == Behavior::kBlock)
		reactionPose = ReactionPose::kBlock;
	currentMelee_->SetReactionPose(reactionPose);
	if (currentFirearm_)
		currentFirearm_->SetReactionPose(reactionPose);

	UpdateMovementState();
	UpdateBodyParts();

	if (currentFirearm_) {
		currentFirearm_->SetOffset(firearmOffset_.scale, firearmOffset_.rotation, firearmOffset_.translation);
		UpdateAimAndFirearm();
	}

	if (canAct) {
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
	const bool debuffed = behavior_.IsKnockback() || behavior_.IsStunned();
	if (behavior_.IsDeathBehavior()) {
		anim = SRTOffset{};
	} else if (behavior_.IsAttacking()) {
		anim = GetAttackPoseOffset();
	} else {
		anim = animManager_.GetMovementAnimOffset(); // includes the knockback / stunned poses
		if (currentFirearm_ && !debuffed) {
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
	smoothedBodyAnim_ = AnimationManager::SmoothSRT(smoothedBodyAnim_, anim);
	anim = smoothedBodyAnim_;

	Matrix4x4 worldRotMatrix = Multiply(MakeRotateXMatrix(worldTransform_.rotation_.x), Multiply(MakeRotateYMatrix(worldTransform_.rotation_.y), MakeRotateZMatrix(worldTransform_.rotation_.z)));
	Vector3 rotatedBodyOffset = TransformNormal(bodyOffset_.translation, worldRotMatrix);

	parts_[body].transform.translation_ = modelWorldTransform_.translation_ + rotatedBodyOffset + anim.translation;
	parts_[body].transform.rotation_ = modelWorldTransform_.rotation_ + bodyOffset_.rotation + anim.rotation;
	parts_[body].transform.scale_ = {
	    modelWorldTransform_.scale_.x * bodyOffset_.scale.x * anim.scale.x, modelWorldTransform_.scale_.y * bodyOffset_.scale.y * anim.scale.y,
	    modelWorldTransform_.scale_.z * bodyOffset_.scale.z * anim.scale.z};
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

void Player::TakeDamage(int damage, const Vector3& hitDirection, float impactPower, BaseEnemy* attacker) {
	attacker;
	if (isDead_ || behavior_.IsDeathBehavior())
		return;

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

#pragma region Block hitbox
bool Player::IsBlockActive() const { return behavior_.IsBlocking() && !isDead_ && !behavior_.IsDeathBehavior() && !behavior_.IsAttacking() && !behavior_.IsStunned(); }

AABB Player::GetBlockAABB() {
	AABB box = GetAABB();
	if (lrDirection_ == LRDirection::kRight)
		box.max.x += kBlockExtraWidth;
	else
		box.min.x -= kBlockExtraWidth;
	return box;
}

bool Player::IsBlockingAttack(const AABB& attackBox, const Vector3& attackerPos) {
	if (!IsBlockActive())
		return false;
	// Only the facing side is covered: an attacker behind the player is never blocked.
	float dx = attackerPos.x - worldTransform_.translation_.x;
	bool attackerInFront = isRight() ? (dx >= 0.0f) : (dx <= 0.0f);
	if (!attackerInFront)
		return false;
	return IsCollisionAABB(GetBlockAABB(), attackBox);
}

void Player::OnBlocked(const Vector3& hitDirection, float impactPower, BaseEnemy* attacker) {
	if (isDead_ || behavior_.IsDeathBehavior())
		return;

	blockLatched_ = true; // block direction stays fixed until the button is released

	// [CHANGED] spawn the spark from the visible body (model), not the root
	Vector3 effectPos = GetModelWorldPosition();
	effectPos.x += (isRight() ? 1.0f : -1.0f) * (kWidth * 0.5f);
	if (gameScene_)
		gameScene_->CreateBlockSparkEffect(effectPos);

	behavior_.RequestHitKnockback(hitDirection, impactPower * 0.3f);

	if (attacker) {
		// gameplay direction: root to root
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
	}
	PlayerBehavior::PlayBlockImpactSound();
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
	blockDetector_.SetSize(kWidth, kHeight);

	blockDetector_.SetIgnoreOneWay(IsDroppingThrough());

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

		if (squishTimer_ >= kSquishThreshold) {
			isDead_ = true;
		}
	} else {
		squishTimer_ = 0;
	}
}

void Player::MapChipCollisionUpdate(CollisionInfo& collisionInfo) {
	blockDetector_.SetWorldTransform(worldTransform_);
	blockDetector_.SetVelocity(velocity_);
	blockDetector_.SetOnGround(onGround_);
	blockDetector_.SetMapChipField(mapChipField_);
	blockDetector_.SetSize(kWidth, kHeight);

	// Sでドロップスルー中はワンウェイ足場への着地だけ無視する。
	blockDetector_.SetIgnoreOneWay(IsDroppingThrough());

	blockDetector_.MapChipCollision(collisionInfo);

	PositionUpdate(collisionInfo);
	WallCollision(collisionInfo);
	CeillingCollision(collisionInfo);

	blockDetector_.GroundSwitch(collisionInfo);

	onGround_ = blockDetector_.IsOnGround();
	velocity_ = blockDetector_.GetVelocity();
}
bool Player::IsStandingOnPlatform() const {
	if (!mapChipField_ || !onGround_) {
		return false;
	}

	const float checkOffset = 0.05f;

	Vector3 leftFoot = worldTransform_.translation_ + Vector3(-kWidth * 0.5f + kBlank, -kHeight * 0.5f - checkOffset, 0.0f);
	Vector3 rightFoot = worldTransform_.translation_ + Vector3(kWidth * 0.5f - kBlank, -kHeight * 0.5f - checkOffset, 0.0f);

	// 左右どちらかの足元がワンウェイ足場なら、現在その足場に立っている。
	return mapChipField_->IsOneWayAtPosition(leftFoot) || mapChipField_->IsOneWayAtPosition(rightFoot);
}

void Player::RequestDropThrough() {
	if (!IsStandingOnPlatform()) {
		return;
	}

	dropThroughTimer_ = kDropThroughDuration;

	// 今いる足場を一度抜けるため、接地状態を解除して下へ進ませる。
	onGround_ = false;
	velocity_.y = std::min(velocity_.y, kDropThroughVelocity);
}
void Player::UpdateModelCollision() {
	// Root and detector share the same position and 0.8 x 1.6 size.
	const Vector3 rootPos = worldTransform_.translation_;

	float targetOffsetY = 0.0f;
	if (onGround_ && !isDead_ && !behavior_.IsDeathBehavior() && mapChipField_) {
		blockDetector_.SetMapChipField(mapChipField_);
		blockDetector_.SetSize(kWidth, kHeight);
		detectorProbe_.SetMapChipField(mapChipField_);
		detectorProbe_.SetSize(kWidth, kHeight);

		float rootTopY = 0.0f;     // surface the root thinks it stands on (full cell)
		float detectorTopY = 0.0f; // surface the detector finds (real tile shape)
		if (blockDetector_.ProbeGroundTop(rootPos, kModelProbeStepUp, kModelProbeDepth, rootTopY) && detectorProbe_.ProbeGroundTop(rootPos, kModelProbeStepUp, kModelProbeDepth, detectorTopY)) {
			targetOffsetY = std::clamp(detectorTopY - rootTopY, -kModelMaxOffset, kModelMaxOffset);
		}
	}
	// Airborne or dead: target is 0, so the model eases back to the root.
	modelStandOffsetY_ = Lerp(modelStandOffsetY_, targetOffsetY, kModelStandLerp);

	modelWorldTransform_.translation_ = rootPos + Vector3(0.0f, modelStandOffsetY_, 0.0f);
	modelWorldTransform_.rotation_ = worldTransform_.rotation_;
	modelWorldTransform_.scale_ = worldTransform_.scale_;
	WorldTransformUpdate(modelWorldTransform_);
}