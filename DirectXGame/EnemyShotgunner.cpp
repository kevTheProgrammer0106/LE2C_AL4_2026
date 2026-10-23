#include "EnemyShotgunner.h"
#include "GameScene.h"
#include "Global.h"
#include "GlobalVariables.h"
#include "Player.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "EnemyShotgunner";
}

void EnemyShotgunner::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "AimPoseTranslation", kAimPose.translation);
	gv->AddItem(kGroupName, "AimPoseRotation", kAimPose.rotation);
	gv->AddItem(kGroupName, "FirePoseTranslation", kFirePose.translation);
	gv->AddItem(kGroupName, "FirePoseRotation", kFirePose.rotation);
	gv->AddItem(kGroupName, "ReloadPoseTranslation", kReloadPose.translation);
	gv->AddItem(kGroupName, "ReloadPoseRotation", kReloadPose.rotation);
	gv->AddItem(kGroupName, "FirePoseDuration", kFirePoseDuration);
	ShotgunnerBehavior::RegisterGlobalVariables();
}

void EnemyShotgunner::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kAimPose.translation = gv->GetVector3Value(kGroupName, "AimPoseTranslation");
	kAimPose.rotation = gv->GetVector3Value(kGroupName, "AimPoseRotation");
	kFirePose.translation = gv->GetVector3Value(kGroupName, "FirePoseTranslation");
	kFirePose.rotation = gv->GetVector3Value(kGroupName, "FirePoseRotation");
	kReloadPose.translation = gv->GetVector3Value(kGroupName, "ReloadPoseTranslation");
	kReloadPose.rotation = gv->GetVector3Value(kGroupName, "ReloadPoseRotation");
	kFirePoseDuration = gv->GetFloatValue(kGroupName, "FirePoseDuration");
	ShotgunnerBehavior::ApplyGlobalVariables();
}

void EnemyShotgunner::Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, Camera* camera, const Vector3& position) {
	BaseEnemy::Initialize(partResources, camera, position);
	SetMaxHP(125);
	SetHP(125);
	velocity_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	firearmMode_ = FirearmMode::kHolster;
}

void EnemyShotgunner::Update() {
	if (!isDead_) {
		gunPoseTimer_ += 1.0f / 60.0f;
	}

	BaseEnemy::Update();

	if (isDead_ || !currentFirearm_)
		return;

	currentFirearm_->SetOffset(firearmOffset_.scale, firearmOffset_.rotation, firearmOffset_.translation);
	currentFirearm_->TickRecoil();
	currentFirearm_->SetReactionPose(GetReactionPose());

	if (GetBehaviorModule().IsDeathBehavior())
		return;

	UpdateWeaponEquipState();

	// 照準中/リロード中はBehavior側がUpdateFirearmHybridを呼ぶ。それ以外はここで追従させる
	using B = BaseEnemyBehavior::Behavior;
	B current = GetBehaviorModule().GetBehavior();
	if (current != B::kAiming && current != B::kReloading) {
		UpdateFirearmHybrid();
	}
}

void EnemyShotgunner::Draw() {
	BaseEnemy::Draw();
	if (!isDead_ && !GetBehaviorModule().IsDeathBehavior() && currentFirearm_) {
		currentFirearm_->Draw();
	}
}

void EnemyShotgunner::SetShotgunModel(Model* model, uint32_t textureHandle) {
	// Initializeより前に持ち主を設定する(リロード時間などが持ち主で切り替わるため)
	currentFirearm_ = std::make_unique<FirearmShotgun>();
	currentFirearm_->SetOwner(Owner::kEnemy);
	currentFirearm_->Initialize(model, textureHandle, camera_);
	SnapWeaponEquipState(ignoreEquipStates_);
}

Vector2 EnemyShotgunner::ComputeTargetAimPosition() {
	const Vector2 fallback = isRight() ? Vector2{1.0f, 0.0f} : Vector2{-1.0f, 0.0f};

	const AimTracker& tracker = GetBehaviorModule().GetAimTracker();
	if (GetBehaviorModule().GetBehavior() == Behavior::kAiming && tracker.IsActive()) {
		return tracker.GetAimDir();
	}

	if (!gameScene_ || !gameScene_->GetPlayer())
		return fallback;
	Vector3 self = GetWorldPosition();
	Vector3 target = gameScene_->GetPlayer()->GetWorldPosition();
	Vector2 dir{target.x - self.x, target.y - self.y};
	float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	if (len < 0.0001f)
		return fallback;
	return {dir.x / len, dir.y / len};
}

void EnemyShotgunner::UpdateFirearmHybrid() {
	if (!currentFirearm_) {
		return;
	}

	if (GetBehaviorModule().GetBehavior() == Behavior::kAiming) {
		const Vector2 aimDir = ComputeTargetAimPosition();
		const AimFacing aim = ComputeAimFacing(aimDir, isRight());

		if (aim.facingRight != isRight()) {
			FaceDirection(aim.facingRight ? LRDirection::kRight : LRDirection::kLeft);
		}

		currentFirearm_->SetEquipped(true);
		currentFirearm_->UpdateFollow(GetModelWorldTransform(), aim.localAngle, aim.facingRight);
	} else {
		currentFirearm_->SetEquipped(false);
		currentFirearm_->UpdateFollow(GetModelWorldTransform(), 0.0f, false);
	}
}

void EnemyShotgunner::ShootFirearm() {
	if (!currentFirearm_ || firearmMode_ != FirearmMode::kArmed)
		return;
	if (!currentFirearm_->TryFire())
		return;

	const Vector3 bodyAlignment = GetWorldPosition();
	Vector3 muzzle = currentFirearm_->GetFiredMuzzleWorldPosition();
	muzzle.z = bodyAlignment.z;
	Vector2 aimDir = ComputeTargetAimPosition();

	if (gameScene_) {

		gameScene_->CreateMuzzleFlash(muzzle, aimDir, false);
	}
	if (gameScene_) {
		// 銃の発射パターンに従って、ペレットを固定角で扇状に撃つ(Player::UseFirearmと同じ回転計算)
		const int pelletCount = std::max(1, currentFirearm_->GetPelletCount());
		for (int i = 0; i < pelletCount; ++i) {
			const float angle = currentFirearm_->GetPelletAngle(i);
			const float c = std::cos(angle);
			const float s = std::sin(angle);
			Vector2 dir = {aimDir.x * c - aimDir.y * s, aimDir.x * s + aimDir.y * c};
			gameScene_->CreateFirearmProjectile(Owner::kEnemy, muzzle, dir, currentFirearm_->GetBulletDamage(), currentFirearm_->GetProjectileLifeFrames(), &bodyAlignment);
		}
		// 敵は銃口に追従させない
		gameScene_->CreateMuzzleFlash(muzzle, aimDir, false);
	}
	currentFirearm_->TriggerRecoil(isRight());
	BaseFirearm::PlayFireSound();
}

SRTOffset EnemyShotgunner::GetActionAnimOffset() const {
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
	case GunPose::kNeutral:
	default:
		break;
	}
	return pose;
}