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
#include "EffectMuzzleFlash.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "Player";
// ガード時のノックバック調整値(GlobalVariablesで調整できる)
// 通常攻撃・銃弾をガードした時に、ガードした側(プレイヤー)が押される強さ。小さめ
float kBlockerKnockbackScale = 0.3f;
// 特殊攻撃をガードした時、ガードした側と攻撃した側の両方に同じ強さで掛ける倍率(衝撃力に掛ける)
float kBlockSpecialKnockbackScale = 1.5f;
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
	gv->AddItem(kGroupName, "BlockMax", kMaxBlockGauge);
	gv->AddItem(kGroupName, "BlockCostPerHit", kBlockCostPerHit);
	gv->AddItem(kGroupName, "BlockRecoverThreshold", kBlockRecoverThreshold);
	gv->AddItem(kGroupName, "BlockRegenRate", kBlockRegenRate);
	gv->AddItem(kGroupName, "BlockRegenDelay", kBlockRegenDelay);
	gv->AddItem(kGroupName, "LungeHoldTime", kLungeHoldTime);
	gv->AddItem(kGroupName, "InvulnJumpLand", kInvulnJumpLand);
	gv->AddItem(kGroupName, "InvulnDamage", kInvulnDamage);
	gv->AddItem(kGroupName, "InvulnBlock", kInvulnBlock);
	gv->AddItem(kGroupName, "AttackEnemyCollisionMode", kAttackEnemyCollisionMode);
	gv->AddItem(kGroupName, "AttackEnemyGap", kAttackEnemyGap);
	gv->AddItem(kGroupName, "CornerLookAhead", kCornerLookAhead);
	gv->AddItem(kGroupName, "BlockerKnockbackScale", kBlockerKnockbackScale);
	gv->AddItem(kGroupName, "BlockSpecialKnockbackScale", kBlockSpecialKnockbackScale);
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

		kMaxBlockGauge = gv->GetFloatValue(kGroupName, "BlockMax");
	kBlockCostPerHit = gv->GetFloatValue(kGroupName, "BlockCostPerHit");
	kBlockRecoverThreshold = gv->GetFloatValue(kGroupName, "BlockRecoverThreshold");
	kBlockRegenRate = gv->GetFloatValue(kGroupName, "BlockRegenRate");
	kBlockRegenDelay = gv->GetFloatValue(kGroupName, "BlockRegenDelay");

	kLungeHoldTime = gv->GetFloatValue(kGroupName, "LungeHoldTime");
	kInvulnJumpLand = gv->GetFloatValue(kGroupName, "InvulnJumpLand");
	kInvulnDamage = gv->GetFloatValue(kGroupName, "InvulnDamage");
	kInvulnBlock = gv->GetFloatValue(kGroupName, "InvulnBlock");
	kAttackEnemyCollisionMode = gv->GetIntValue(kGroupName, "AttackEnemyCollisionMode");
	kAttackEnemyGap = gv->GetFloatValue(kGroupName, "AttackEnemyGap");
	kCornerLookAhead = gv->GetFloatValue(kGroupName, "CornerLookAhead");

		kBlockerKnockbackScale = gv->GetFloatValue(kGroupName, "BlockerKnockbackScale");
	kBlockSpecialKnockbackScale = gv->GetFloatValue(kGroupName, "BlockSpecialKnockbackScale");
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
void Player::GrantInvulnerability(float seconds) { invulnTimer_ = std::max(invulnTimer_, seconds); }

void Player::TickInvulnerability() {
	if (invulnTimer_ > 0.0f)
		invulnTimer_ = std::max(0.0f, invulnTimer_ - 1.0f / 60.0f);
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
	const bool lunging = behavior_.IsLunge() && behavior_.GetLungeSource() == LungeSource::kBayonet;
	Vector2 aimDir = lunging ? ((lrDirection_ == LRDirection::kRight) ? Vector2{1.0f, 0.0f} : Vector2{-1.0f, 0.0f}) : GetEffectiveAimDir();
	AimFacing aim = ComputeAimFacing(aimDir, lrDirection_ == LRDirection::kRight);
	if (!behavior_.IsAttacking() && aim.facingRight != (lrDirection_ == LRDirection::kRight)) {
		lrDirection_ = aim.facingRight ? LRDirection::kRight : LRDirection::kLeft;
		turnFirstRotationY_ = worldTransform_.rotation_.y;
		turnTimer_ = kTimeTurn;
	}
	if (FirearmMusketBayonet* bayonet = dynamic_cast<FirearmMusketBayonet*>(currentFirearm_.get())) {
		if (lunging) {
			AttackAnimState anim = behavior_.GetAttackAnimState();
			bayonet->SetPlayerLungePose(static_cast<FirearmMusketBayonet::LungePhase>(static_cast<int>(anim.phase)), anim.t);
		} else {
			bayonet->ClearLungePose();
		}
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
		// 足元(ルート座標の下端)
		const Vector3 feet = worldTransform_.translation_ + Vector3(0.0f, -kHeight * 0.5f, 0.0f);
		if (events.justJumped) {
			GrantInvulnerability(kInvulnJumpLand);
			PlayMovementSfxHandle(sJumpSoundHandle_);
			if (gameScene_)
				gameScene_->CreateBurstEffect(BurstType::kJump, feet, {0.0f, 1.0f});
		}
		if (events.justLanded) {
			GrantInvulnerability(kInvulnJumpLand); 
			PlayMovementSfxHandle(sLandSoundHandle_);
			if (gameScene_)
				gameScene_->CreateBurstEffect(BurstType::kLand, feet, {0.0f, 1.0f});
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
	if (!currentFirearm_ || weaponMode_ != WeaponMode::kFirearm) {
		dualHoldArmed_ = false;
		dualHoldTimer_ = 0.0f;
		return;
	}
	Input* input = Input::GetInstance();

	if (input->IsTriggerMouse(0)) {
		dualHoldArmed_ = false;
		dualHoldTimer_ = 0.0f;
		auto* dual = dynamic_cast<FirearmPistols*>(currentFirearm_.get());
		const bool bothLoaded = dual && dual->IsFullyLoaded();

		if (!currentFirearm_->TryFire()) {
			// unloaded: with a bayonet musket the click becomes a lunge while it reloads
			UseBayonetLungeIfReloading();
			return;
		}
		FireCurrentFirearm();
		dualHoldArmed_ = bothLoaded; // first shot of a full set: holding now fires the other pistol too
		return;
	}

	// Dual pistols: still holding after the first shot -> fire the second pistol, then back to normal
	if (dualHoldArmed_) {
		if (input->IsPressMouse(0)) {
			dualHoldTimer_ += 1.0f / 60.0f;
			if (dualHoldTimer_ >= kDualHoldTime) {
				dualHoldArmed_ = false;
				dualHoldTimer_ = 0.0f;
				if (currentFirearm_->TryFire())
					FireCurrentFirearm();
			}
		} else {
			dualHoldArmed_ = false; // released early = plain single shot
			dualHoldTimer_ = 0.0f;
		}
	}
}

void Player::FireCurrentFirearm() {
	Vector3 origin = GetModelWorldPosition();
	Vector2 aimDir = GetEffectiveAimDir();

	if (gameScene_) {
		// pellet pattern per firearm (normal guns: 1 pellet, angle 0)
		const int pelletCount = std::max(1, currentFirearm_->GetPelletCount());
		for (int i = 0; i < pelletCount; ++i) {
			const float angle = currentFirearm_->GetPelletAngle(i);
			const float c = std::cos(angle);
			const float s = std::sin(angle);
			Vector2 dir = {aimDir.x * c - aimDir.y * s, aimDir.x * s + aimDir.y * c};
			gameScene_->CreateFirearmProjectile(Owner::kPlayer, origin, dir, currentFirearm_->GetBulletDamage(), currentFirearm_->GetProjectileLifeFrames());
		}
		// from the barrel that fired (dual pistols: the right one); follows the gun
		gameScene_->CreateMuzzleFlash(currentFirearm_->GetFiredMuzzleWorldPosition(), aimDir, true);
	}
	currentFirearm_->TriggerRecoil(lrDirection_ == LRDirection::kRight);
	BaseFirearm::PlayFireSound();
	if (gameScene_)
		gameScene_->TriggerCameraFx(currentFirearm_->GetFireKnockbackPower() > 0.0f ? GameScene::CameraFx::kShootHeavy : GameScene::CameraFx::kShoot);

	// Recoil: 狙った方向と逆へ、上下も含めて全方向に吹き飛ぶ(blunderbuss / shotgun)
	const float kbPower = currentFirearm_->GetFireKnockbackPower();
	if (kbPower > 0.0f && !isDead_ && !behavior_.IsDeathBehavior()) {
		float len = std::sqrt(aimDir.x * aimDir.x + aimDir.y * aimDir.y);
		if (len > 0.0001f) {
			aimDir.x /= len;
			aimDir.y /= len;
		} else {
			aimDir = {isRight() ? 1.0f : -1.0f, 0.0f};
		}
		behavior_.RequestHitKnockback({-aimDir.x, -aimDir.y, 0.0f}, kbPower, false);
	}
}

void Player::UseMeleeWeapon() {
	if (weaponMode_ != WeaponMode::kMelee) {
		meleePressPending_ = false;
		meleeHoldTimer_ = 0.0f;
		return;
	}
	Input* input = Input::GetInstance();

	auto normalAttack = [&]() {
		currentMelee_->SetWeaponState(WeaponState::kEquipped);
		PlayerBehavior::AttackDirection direction = PlayerBehavior::AttackDirection::kSide;
		if (input->PushKey(DIK_S))
			direction = PlayerBehavior::AttackDirection::kDown;
		behavior_.RequestAttack(direction);
	};

	// Light sabre: unchanged, attacks on press.
	if (!dynamic_cast<WeaponHeavySabre*>(currentMelee_.get())) {
		if (input->IsTriggerMouse(0))
			normalAttack();
		return;
	}

	// Heavy sabre: tap = combo (on release), hold = lunge.
	if (input->IsTriggerMouse(0)) {
		meleePressPending_ = true;
		meleeHoldTimer_ = 0.0f;
	}
	if (!meleePressPending_)
		return;

	if (input->IsPressMouse(0)) {
		meleeHoldTimer_ += 1.0f / 60.0f;
		if (meleeHoldTimer_ >= kLungeHoldTime) {
			meleePressPending_ = false;
			if (meleeLungeCooldown_ <= 0.0f && !behavior_.IsAttacking() && !input->PushKey(DIK_S)) {
				currentMelee_->SetWeaponState(WeaponState::kEquipped);
				behavior_.RequestLunge(LungeSource::kHeavySabre);
			} else {
				normalAttack(); // lunge on cooldown: the held click becomes a normal swing
			}
		}
	} else {
		meleePressPending_ = false; // released early = tap
		normalAttack();
	}
}
void Player::BeginGoalSequence(const Vector3& goalPosition) {
	goalSeqActive_ = true;
	goalSeqTimer_ = 0.0f;
	goalSeqStart_ = worldTransform_.translation_;
	goalSeqTarget_ = goalPosition;
	velocity_ = {};
	onGround_ = false;
	controlLocked_ = true;

	// drop any attack/block so no slash effect or guard pose lingers
	behavior_.SetBlocking(false);
	behavior_.RequestBehavior(Behavior::kRoot);
	behavior_.ResolveBehaviorRequest();
}

void Player::UpdateGoalSequence() {
	constexpr float kDt = 1.0f / 60.0f;
	goalSeqTimer_ += kDt;

	Vector3 pos;
	if (goalSeqTimer_ < kGoalMoveDuration) {
		float t = std::clamp(goalSeqTimer_ / kGoalMoveDuration, 0.0f, 1.0f);
		pos = Lerp(goalSeqStart_, goalSeqTarget_, EaseInOut(0.0f, 1.0f, t));
		pos.y += std::sin(std::numbers::pi_v<float> * t) * kGoalJumpHeight; // jump arc
	} else {
		float loop = goalSeqTimer_ - kGoalMoveDuration;
		pos = goalSeqTarget_;
		pos.y += std::fabs(std::sin(std::numbers::pi_v<float> * loop / kGoalLoopHopPeriod)) * kGoalLoopHopHeight; // keeps hopping
	}

	worldTransform_.translation_ = pos;
	worldTransform_.rotation_.y += kGoalSpinSpeed; // spin the whole time
	WorldTransformUpdate(worldTransform_);

	velocity_ = {};
	onGround_ = false;

	UpdateModelCollision();
	UpdateMeleeVisual();
	UpdateBodyParts();
}
void Player::UpdateTurnRotation() {
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		worldTransform_.rotation_.y = EaseInOut(turnFirstRotationY_, destinationRotationY, 1.0f - (turnTimer_ / kTimeTurn));
	}
}
void Player::Update() {
	ApplyGlobalVariables();
	if (goalSeqActive_) {
		UpdateGoalSequence();
		return;
	}
	TickDebuffs();
	TickInvulnerability();
	TickBlockGauge();
	TickLungeCooldowns();


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
		if (Input::GetInstance()->TriggerKey(DIK_Q) && !behavior_.IsAttacking()) {
			SetWeaponMode(weaponMode_ == WeaponMode::kMelee ? WeaponMode::kFirearm : WeaponMode::kMelee);
			PlaySfxHandle(sWeaponSwitchSoundHandle_);
		}
		bool isBlockingNow = Input::GetInstance()->IsPressMouse(1) && !blockBroken_;
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
	UpdateMeleeVisual();

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
		if (gameScene_)
			EffectMuzzleFlash::SetPlayerAnchor(currentFirearm_->GetMuzzleWorldPosition());
	}

	if (canAct) {
		if (!behavior_.IsKnockback() && !behavior_.IsDeathBehavior())
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
	// 斬撃: 通常コンボの攻撃フェーズ、または突進(ヘビーサーベル/銃剣)の当たり判定中に表示する
	const bool comboSlash = behavior_.IsAttacking() && !behavior_.IsLunge() && behavior_.GetAttackAnimState().phase == PlayerBehavior::AttackPhase::kAttack;
	const bool lungeSlash = IsLungeHitActive();
	if (lungeSlash) {
		// 突進の判定ボックスの中心に置く(向きは lrDirection_ から決める)
		const AABB box = GetLungeAABB();
		worldTransformAttack_.translation_ = {(box.min.x + box.max.x) * 0.5f, (box.min.y + box.max.y) * 0.5f, worldTransform_.translation_.z};
		worldTransformAttack_.rotation_ = {0.0f, (lrDirection_ == LRDirection::kRight) ? 1.570796f : 4.712389f, 0.0f};
		WorldTransformUpdate(worldTransformAttack_);
	}
	if (comboSlash || lungeSlash) {
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

	// 無敵の判定はタイマーを更新する前に行う(この一撃自身で無敵になって自分のノックバックを消さないため)
	const bool wasInvulnerable = IsInvulnerable();

	Vector3 effectPos = GetWorldPosition();
	if (gameScene_) {
		gameScene_->CreateHitEffect(effectPos);
		// 被弾: モデル中心から火花が弾ける(向きは攻撃を受けた方向)
		gameScene_->CreateBurstEffect(BurstType::kHit, GetModelWorldPosition(), {hitDirection.x, hitDirection.y});
		gameScene_->TriggerScreenFlash(ScreenFlash::Type::kDamage); // 被弾: 赤
		if (damage < hp_)
			gameScene_->TriggerCameraFx(GameScene::CameraFx::kDamage, std::clamp(0.5f + static_cast<float>(damage) / 60.0f, 0.5f, 1.5f));
	}
	hp_ = std::max(0, hp_ - damage);
	GrantInvulnerability(kInvulnDamage); // 被弾で3秒(残りがそれ以上ならそのまま)

	if (hp_ <= 0)
		behavior_.RequestDeath(hitDirection, impactPower);
	else if (!wasInvulnerable)
		behavior_.RequestHitKnockback(hitDirection, impactPower); // 無敵中はノックバックしない(スタンロック防止)
	PlayerBehavior::PlayHurtSound();
}

#pragma region Block hitbox
bool Player::IsBlockActive() const { return behavior_.IsBlocking() && !blockBroken_ && !isDead_ && !behavior_.IsDeathBehavior() && !behavior_.IsAttacking() && !behavior_.IsStunned(); }

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

// ガード成功時の処理。
//  - ガードした側(プレイヤー): 通常攻撃・銃弾は小さく、特殊攻撃は攻撃側と同じ強さで押される
//  - 攻撃した側(attackerがいる時): 通常は普通にノックバック+スタン、特殊攻撃はプレイヤーと同じ強さでノックバック
void Player::OnBlocked(const KamataEngine::Vector3& hitDirection, float impactPower, BaseEnemy* attacker, bool special) {
	if (isDead_ || behavior_.IsDeathBehavior())
		return;

	const bool wasInvulnerable = IsInvulnerable();

	blockLatched_ = true;
	blockGauge_ = std::max(0.0f, blockGauge_ - kBlockCostPerHit);
	blockRegenDelayTimer_ = kBlockRegenDelay;
	if (blockGauge_ <= 0.0f)
		blockBroken_ = true;
	Vector3 effectPos = GetModelWorldPosition();
	effectPos.x += (isRight() ? 1.0f : -1.0f) * (kWidth * 0.5f);
	if (gameScene_) {
		gameScene_->CreateBlockSparkEffect(effectPos);
		gameScene_->TriggerScreenFlash(ScreenFlash::Type::kBlock); // ブロック成功: 白
	}

	GrantInvulnerability(kInvulnBlock); // ブロック成功で1秒

	// ---- ガードした側(プレイヤー)のノックバック ----
	// 全てのガードで押される。通常攻撃・銃弾は小さく、特殊攻撃は攻撃側と同じ倍率。
	// 無敵中は押されない(散弾の複数ペレットで重ね掛けされないように)。ガードなので跳ね上がりはしない
	if (!wasInvulnerable) {
		const float blockerScale = special ? kBlockSpecialKnockbackScale : kBlockerKnockbackScale;
		behavior_.RequestHitKnockback(hitDirection, impactPower * blockerScale, false);
	}

	// ---- 攻撃した側のノックバック(銃弾はattackerがいないので対象外) ----
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
		// 通常攻撃: 普通のノックバック(続けてスタンも付く)。特殊攻撃: ガードした側と同じ強さ
		const float attackerScale = special ? kBlockSpecialKnockbackScale : kBlockAttackerKnockbackScale;
		attacker->RequestBlockKnockback(pushDir, impactPower * attackerScale);
	}
	PlayerBehavior::PlayBlockImpactSound();
}
void Player::OnCollision(const BaseEnemy* enemy) {
	if (behavior_.IsKnockback())
		return;
	if (enemy->WasBlocked() && !IsInvulnerable()) {
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
void Player::ResetToPosition(const Vector3& position) {
	const float baseRotationY = std::numbers::pi_v<float> / 2.0f; // same base as Initialize(): facing right

	worldTransform_.translation_ = position;
	worldTransform_.rotation_ = {0.0f, baseRotationY, 0.0f};
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	lrDirection_ = LRDirection::kRight;
	turnTimer_ = 0.0f;
	turnFirstRotationY_ = baseRotationY;

	velocity_ = {};
	onGround_ = false;

	worldTransformAttack_.rotation_ = worldTransform_.rotation_;

	modelStandOffsetY_ = 0.0f;
	modelWorldTransform_.translation_ = position;
	modelWorldTransform_.rotation_ = worldTransform_.rotation_;
	modelWorldTransform_.scale_ = worldTransform_.scale_;

	WorldTransformUpdate(worldTransform_);
	WorldTransformUpdate(modelWorldTransform_);
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

void Player::MapChipCollisionUpdate(CollisionInfo& collisionInfo, bool ledgeGuard) {
	blockDetector_.SetWorldTransform(worldTransform_);
	blockDetector_.SetVelocity(velocity_);
	blockDetector_.SetOnGround(onGround_);
	blockDetector_.SetMapChipField(mapChipField_);
	blockDetector_.SetSize(kWidth, kHeight);

	blockDetector_.SetIgnoreOneWay(IsDroppingThrough());

	blockDetector_.MapChipCollision(collisionInfo);
	if (ledgeGuard) {
		blockDetector_.LedgeGuard(collisionInfo, kLimitRunSpeed); // knockback slower than max run speed stops at the edge
	}

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
void Player::TickBlockGauge() {
	blockGauge_ = std::min(blockGauge_, kMaxBlockGauge);
	if (blockRegenDelayTimer_ > 0.0f) {
		blockRegenDelayTimer_ -= 1.0f / 60.0f;
		return;
	}
	if (blockGauge_ < kMaxBlockGauge) {
		blockGauge_ = std::min(kMaxBlockGauge, blockGauge_ + kBlockRegenRate * (1.0f / 60.0f));
	}
	if (blockBroken_ && blockGauge_ >= kBlockRecoverThreshold) {
		blockBroken_ = false;
	}
}

void Player::SetMusketModel(Model* model, uint32_t textureHandle, DropWeaponKind kind) {
	if (kind == DropWeaponKind::kMusketBayonet) {
		currentFirearm_ = std::make_unique<FirearmMusketBayonet>();
	} else if (kind == DropWeaponKind::kShotgun) {
		currentFirearm_ = std::make_unique<FirearmShotgun>();
	} else if (kind == DropWeaponKind::kDualPistols) {
		currentFirearm_ = std::make_unique<FirearmPistols>();
	} else {
		kind = DropWeaponKind::kMusketLong;
		currentFirearm_ = std::make_unique<FirearmMusket>();
	}
	currentFirearm_->SetOwner(Owner::kPlayer);
	currentFirearm_->Initialize(model, textureHandle, camera_);
	currentFirearm_->SnapEquipped(weaponMode_ == WeaponMode::kFirearm);
	currentFirearmKind_ = kind;
	bayonetLungeCooldown_ = 0.0f;
	bayonetLungeCooldownMax_ = 0.0f;
	dualHoldArmed_ = false;
	dualHoldTimer_ = 0.0f;
}

void Player::SetMeleeModel(Model* model, uint32_t textureHandle, DropWeaponKind kind) {
	if (kind == DropWeaponKind::kSabreHeavy) {
		currentMelee_ = std::make_unique<WeaponHeavySabre>();
	} else if (kind == DropWeaponKind::kLongAxe) {
		currentMelee_ = std::make_unique<WeaponAxe>();
	} else if (kind == DropWeaponKind::kSpear) {
		currentMelee_ = std::make_unique<WeaponSpear>();
	} else {
		kind = DropWeaponKind::kSabreLight;
		currentMelee_ = std::make_unique<WeaponSabre>();
	}
	currentMelee_->Initialize(model, textureHandle, camera_, worldTransform_.translation_);
	currentMelee_->SnapWeaponState(weaponMode_ == WeaponMode::kMelee ? WeaponState::kEquipped : WeaponState::kHolstered);
	currentMeleeKind_ = kind;
	meleeLungeCooldown_ = 0.0f;
	meleeLungeCooldownMax_ = 0.0f;
}

DropItemState Player::GetCurrentMeleeState() const {
	if (!currentMelee_)
		return DropItemState::kReady;
	return (currentMelee_->GetDropState() == DropItemState::kOnCooldown || meleeLungeCooldown_ > 0.0f) ? DropItemState::kOnCooldown : DropItemState::kReady;
}

DropItemState Player::GetCurrentFirearmState() const {
	if (!currentFirearm_)
		return DropItemState::kReady;
	return (currentFirearm_->GetDropState() == DropItemState::kOnCooldown || bayonetLungeCooldown_ > 0.0f) ? DropItemState::kOnCooldown : DropItemState::kReady;
}

void Player::RestoreFirearmState(DropItemState state) {
	if (!currentFirearm_)
		return;
	currentFirearm_->RestoreDropState(state);
	// premature re-pickup: the lunge cooldown restarts from full too
	if (state == DropItemState::kOnCooldown && dynamic_cast<FirearmMusketBayonet*>(currentFirearm_.get()))
		BeginLungeCooldown(LungeSource::kBayonet);
}

void Player::RestoreMeleeState(DropItemState state) {
	if (!currentMelee_ || state != DropItemState::kOnCooldown)
		return;
	currentMelee_->SnapWeaponState(WeaponState::kHolstered); // replays the draw animation
	if (dynamic_cast<WeaponHeavySabre*>(currentMelee_.get()))
		BeginLungeCooldown(LungeSource::kHeavySabre);
}

LungeSpec Player::GetLungeSpec(LungeSource source) const {
	if (source == LungeSource::kHeavySabre) {
		if (auto* heavy = dynamic_cast<WeaponHeavySabre*>(currentMelee_.get()))
			return heavy->GetPlayerLungeSpec();
	} else if (source == LungeSource::kBayonet) {
		if (auto* bayonet = dynamic_cast<FirearmMusketBayonet*>(currentFirearm_.get()))
			return bayonet->GetPlayerLungeSpec();
	}
	return LungeSpec{};
}

AABB Player::GetLungeAABB() {
	const AABB playerBody = GetAABB();
	if (behavior_.GetLungeSource() == LungeSource::kHeavySabre) {
		if (auto* heavy = dynamic_cast<WeaponHeavySabre*>(currentMelee_.get()))
			return heavy->GetPlayerLungeAABB(playerBody, isRight());
	} else if (behavior_.GetLungeSource() == LungeSource::kBayonet) {
		if (auto* bayonet = dynamic_cast<FirearmMusketBayonet*>(currentFirearm_.get()))
			return bayonet->GetPlayerLungeAABB(playerBody, isRight());
	}
	return playerBody;
}

void Player::BeginLungeCooldown(LungeSource source) {
	const float cooldown = GetLungeSpec(source).cooldown;
	if (source == LungeSource::kHeavySabre) {
		meleeLungeCooldown_ = meleeLungeCooldownMax_ = cooldown;
	} else if (source == LungeSource::kBayonet) {
		bayonetLungeCooldown_ = bayonetLungeCooldownMax_ = cooldown;
	}
}

void Player::PlayLungeSound(LungeSource source) {
	if (source == LungeSource::kHeavySabre)
		WeaponHeavySabre::PlayLungeSound();
	else if (source == LungeSource::kBayonet)
		FirearmMusketBayonet::PlayLungeSound();
}

void Player::TickLungeCooldowns() {
	constexpr float kDt = 1.0f / 60.0f;
	meleeLungeCooldown_ = std::max(0.0f, meleeLungeCooldown_ - kDt);
	bayonetLungeCooldown_ = std::max(0.0f, bayonetLungeCooldown_ - kDt);
}

void Player::UseBayonetLungeIfReloading() {
	auto* bayonet = dynamic_cast<FirearmMusketBayonet*>(currentFirearm_.get());
	if (!bayonet || !bayonet->IsReloading())
		return;
	if (bayonetLungeCooldown_ > 0.0f || behavior_.IsAttacking())
		return;
	behavior_.RequestLunge(LungeSource::kBayonet);
}

void Player::UpdateMeleeVisual() {
	currentMelee_->UpdateWeaponState();
	if (behavior_.IsLunge() && behavior_.GetLungeSource() == LungeSource::kHeavySabre) {
		if (auto* heavy = dynamic_cast<WeaponHeavySabre*>(currentMelee_.get())) {
			AttackAnimState anim = behavior_.GetAttackAnimState();
			heavy->SetPlayerLungePosition(*this, static_cast<BaseMelee::AttackPhase>(static_cast<int>(anim.phase)), anim.t);
			return;
		}
	}
	currentMelee_->SetMeleePosition(*this);
}
bool Player::HasCornerAhead(bool right) const {
	if (!mapChipField_)
		return false;
	const float sign = right ? 1.0f : -1.0f;
	const float feetY = worldTransform_.translation_.y - kHeight * 0.5f;
	// 足元の高さ(最下段のセル)に壁があれば、そこは自分より1ブロック以上高い角
	for (float d = 0.25f; d <= kCornerLookAhead; d += 0.5f) {
		const Vector3 p = {worldTransform_.translation_.x + sign * (kWidth * 0.5f + d), feetY + 0.5f, 0.0f};
		if (mapChipField_->IsSolidAtPosition(p, true) && !mapChipField_->IsOneWayAtPosition(p))
			return true;
	}
	return false;
}

float Player::LimitAttackMoveByEnemies(float moveX) {
	if (!gameScene_ || std::abs(moveX) < 0.0001f)
		return moveX;
	const bool right = moveX > 0.0f;
	if (kAttackEnemyCollisionMode == 1 && !HasCornerAhead(right))
		return moveX;

	const AABB me = GetAABB();
	float allowed = moveX;
	for (const std::unique_ptr<BaseEnemy>& enemy : gameScene_->GetEnemies()) {
		if (!enemy || enemy->IsDead() || enemy->IsCollisionDisabled())
			continue;
		const AABB e = enemy->GetAABB();
		if (me.max.y <= e.min.y || me.min.y >= e.max.y)
			continue; // 縦に重なっていない敵は無視
		if (right) {
			if (me.max.x > e.min.x)
				continue; // すでに重なっている/通り過ぎている: 抜けられるように止めない
			allowed = std::min(allowed, std::max(0.0f, (e.min.x - me.max.x) - kAttackEnemyGap));
		} else {
			if (me.min.x < e.max.x)
				continue;
			allowed = std::max(allowed, -std::max(0.0f, (me.min.x - e.max.x) - kAttackEnemyGap));
		}
	}
	return allowed;
}
bool Player::GetAimGuideInfo(Vector3& outOrigin, Vector2& outDir, float& outCursorDistance) const {
	if (!camera_)
		return false;
	outOrigin = GetModelWorldPosition(); // 弾の発射位置と同じ(FireCurrentFirearm)
	const Vector2 raw = GetEffectiveAimDir();
	const float len = std::sqrt(raw.x * raw.x + raw.y * raw.y);
	if (len < 0.0001f)
		return false;
	outDir = {raw.x / len, raw.y / len};

	// カーソルまでの距離: 画面上の距離を「1ワールド単位あたりのピクセル数」で割る
	auto mouse = Input::GetInstance()->GetMousePosition();
	Matrix4x4 matViewport = MakeViewportMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight), 0.0f, 1.0f);
	Matrix4x4 matVPV = Multiply(camera_->matView, Multiply(camera_->matProjection, matViewport));
	const Vector3 o = TransformCoordinate(outOrigin, matVPV);
	const Vector3 p = TransformCoordinate(outOrigin + Vector3(outDir.x, outDir.y, 0.0f), matVPV);
	const float pixelsPerUnit = std::sqrt((p.x - o.x) * (p.x - o.x) + (p.y - o.y) * (p.y - o.y));
	const float screenDistance = std::sqrt((static_cast<float>(mouse.x) - o.x) * (static_cast<float>(mouse.x) - o.x) + (static_cast<float>(mouse.y) - o.y) * (static_cast<float>(mouse.y) - o.y));
	outCursorDistance = (pixelsPerUnit > 0.0001f) ? screenDistance / pixelsPerUnit : 0.0f;
	return true;
}