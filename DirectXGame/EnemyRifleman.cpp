#include "EnemyRifleman.h"
#include "GameScene.h"
#include "Global.h"
#include "GlobalVariables.h"
#include "Player.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "EnemyRifleman";
}

void EnemyRifleman::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "AimPoseTranslation", kAimPose.translation);
	gv->AddItem(kGroupName, "AimPoseRotation", kAimPose.rotation);
	gv->AddItem(kGroupName, "FirePoseTranslation", kFirePose.translation);
	gv->AddItem(kGroupName, "FirePoseRotation", kFirePose.rotation);
	gv->AddItem(kGroupName, "ReloadPoseTranslation", kReloadPose.translation);
	gv->AddItem(kGroupName, "ReloadPoseRotation", kReloadPose.rotation);
	gv->AddItem(kGroupName, "FirePoseDuration", kFirePoseDuration);
}
void EnemyRifleman::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kAimPose.translation = gv->GetVector3Value(kGroupName, "AimPoseTranslation");
	kAimPose.rotation = gv->GetVector3Value(kGroupName, "AimPoseRotation");
	kFirePose.translation = gv->GetVector3Value(kGroupName, "FirePoseTranslation");
	kFirePose.rotation = gv->GetVector3Value(kGroupName, "FirePoseRotation");
	kReloadPose.translation = gv->GetVector3Value(kGroupName, "ReloadPoseTranslation");
	kReloadPose.rotation = gv->GetVector3Value(kGroupName, "ReloadPoseRotation");
	kFirePoseDuration = gv->GetFloatValue(kGroupName, "FirePoseDuration");
}

void EnemyRifleman::Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, Camera* camera, const Vector3& position) {
	BaseEnemy::Initialize(partResources, camera, position);
	SetMaxHP(125);
	SetHP(125);
	velocity_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	firearmMode_ = FirearmMode::kHolster;
}

void EnemyRifleman::Update() {
	BaseEnemy::Update();

	if (!isDead_ && currentFirearm_) {
		currentFirearm_->SetOffset(firearmOffset_.scale, firearmOffset_.rotation, firearmOffset_.translation);
		currentFirearm_->TickRecoil();
	}

	if (!isDead_) {
		gunPoseTimer_ += 1.0f / 60.0f;
		SRTOffset pose{};
		switch (gunPose_) {
		case GunPose::kAiming:
			pose = kAimPose;
			break;
		case GunPose::kFiring: {
			float t = std::clamp(gunPoseTimer_ / kFirePoseDuration, 0.0f, 1.0f);
			pose = LerpSRT(kFirePose, kAimPose, t);
			break;
		}
		case GunPose::kReloading:
			pose = kReloadPose;
			break;
		default:
			break;
		}

		parts_[body].transform.translation_ += pose.translation;
		parts_[body].transform.rotation_ += pose.rotation;
		parts_[body].transform.scale_ = {
		    parts_[body].transform.scale_.x * pose.scale.x,
		    parts_[body].transform.scale_.y * pose.scale.y,
		    parts_[body].transform.scale_.z * pose.scale.z,
		};
		WorldTransformUpdate(parts_[body].transform);
	}
}

void EnemyRifleman::Draw() {
	BaseEnemy::Draw();
	if (!isDead_ && !GetBehaviorModule().IsDeathBehavior() && currentFirearm_) {
		currentFirearm_->Draw();
	}
}

void EnemyRifleman::SetMusketModel(Model* model, uint32_t textureHandle) {
	currentFirearm_ = std::make_unique<FirearmMusketBayonet>();
	currentFirearm_->Initialize(model, textureHandle, camera_);
}

Vector2 EnemyRifleman::ComputeTargetAimPosition() {
	if (!gameScene_ || !gameScene_->GetPlayer()) {
		return isRight() ? Vector2{1.0f, 0.0f} : Vector2{-1.0f, 0.0f};
	}
	Vector3 self = GetWorldPosition();
	Vector3 target = gameScene_->GetPlayer()->GetWorldPosition();
	Vector2 dir{target.x - self.x, target.y - self.y};
	float len = std::sqrtf(dir.x * dir.x + dir.y * dir.y);
	if (len < 0.0001f) {
		return isRight() ? Vector2{1.0f, 0.0f} : Vector2{-1.0f, 0.0f};
	}
	return {dir.x / len, dir.y / len};
}

void EnemyRifleman::UpdateFirearmHybrid() {
	if (!currentFirearm_)
		return;

	Behavior currentBehavior = GetBehaviorModule().GetBehavior();

	if (currentBehavior == Behavior::kAiming && IsPlayerSpotted()) {
		Vector2 aimDir = ComputeTargetAimPosition();
		AimFacing aim = ComputeAimFacing(aimDir, isRight());
		if (aim.facingRight != isRight()) {
			FaceDirection(aim.facingRight ? LRDirection::kRight : LRDirection::kLeft);
		}
		currentFirearm_->SetEquipped(true);
		currentFirearm_->UpdateFollow(GetWorldTransform(), aim.localAngle, aim.facingRight);
	} else if (currentBehavior == Behavior::kMelee) {
		currentFirearm_->SetEquipped(true);
		currentFirearm_->UpdateFollow(GetWorldTransform(), 0.0f, isRight());
	} else {
		currentFirearm_->SetEquipped(false);
		currentFirearm_->UpdateFollow(GetWorldTransform(), 0.0f, 0.0f);
	}
}

void EnemyRifleman::ShootFirearm() {
	if (!currentFirearm_ || firearmMode_ != FirearmMode::kArmed)
		return;
	if (!currentFirearm_->TryFire())
		return;

	Vector3 origin = GetWorldPosition();
	Vector2 aimDir = ComputeTargetAimPosition();

	if (gameScene_) {
		gameScene_->CreateFirearmProjectile(Owner::kEnemy, origin, aimDir);
		Vector3 muzzlePos = origin;
		gameScene_->CreateMuzzleFlash(muzzlePos);
	}
	currentFirearm_->TriggerRecoil(isRight());
	BaseFirearm::PlayFireSound();
}

AABB EnemyRifleman::GetBayonetLungeAABB() const {
	constexpr float kOverlapMargin = 0.1f;
	constexpr float kOwnerHalfWidth = 0.4f;
	constexpr float kBayonetReach = 1.3f;
	constexpr float kBayonetHeight = 1.0f;

	Vector3 pos = GetWorldPosition();
	bool facingRight = isRight();

	AABB aabb;
	if (facingRight) {
		aabb.min.x = pos.x + kOwnerHalfWidth - kOverlapMargin;
		aabb.max.x = pos.x + kOwnerHalfWidth + kBayonetReach;
	} else {
		aabb.min.x = pos.x - kOwnerHalfWidth - kBayonetReach;
		aabb.max.x = pos.x - kOwnerHalfWidth + kOverlapMargin;
	}
	aabb.min.y = pos.y - kBayonetHeight * 0.5f;
	aabb.max.y = pos.y + kBayonetHeight * 0.5f;
	aabb.min.z = pos.z - kOwnerHalfWidth;
	aabb.max.z = pos.z + kOwnerHalfWidth;
	return aabb;
}