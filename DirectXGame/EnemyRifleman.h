#pragma once
#include "BaseEnemy.h"
#include "RiflemanBehavior.h"

#include "FirearmMusketBayonet.h"
#include "ProjectileBall.h"

class EnemyRifleman : public BaseEnemy {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update() override;
	void Draw() override;

	FirearmMusketBayonet* GetCurrentFirearm() const { return currentFirearm_.get(); }
	void SetMusketModel(KamataEngine::Model* model, uint32_t textureHandle);

	KamataEngine::Vector2 ComputeTargetAimPosition();
	void ShootFirearm();
	void UpdateFirearmHybrid();

	bool IsBayonetActive() const { return isBayonetActive_; }
	void SetBayonetActive(bool active) {
		if (active && !isBayonetActive_) {
			PlayBayonetLungeSound();
		}
		isBayonetActive_ = active;
	}
	bool HasHitPlayerThisSwing() const { return hasHitPlayerThisSwing_; }
	void MarkHitPlayerThisSwing() { hasHitPlayerThisSwing_ = true; }
	void ResetBayonetHit() { hasHitPlayerThisSwing_ = false; }
	AABB GetBayonetLungeAABB() const;

	enum class GunPose { kNeutral, kAiming, kFiring, kReloading };
	void SetGunPose(GunPose pose) {
		gunPose_ = pose;
		gunPoseTimer_ = 0.0f;
	}

	// --- SFX ---
	// bayonetがfalse->trueに切り替わった瞬間(踏み込み開始)に鳴らす。未設定なら無音。
	static void SetBayonetLungeSoundHandle(uint32_t handle) { sBayonetLungeSoundHandle_ = handle; }
	static void SetSfxVolume(float volume) { sSfxVolume_ = volume; }

	SRTOffset GetActionAnimOffset() const override;

	virtual float GetReloadTimeScale() const { return 1.0f; }
	virtual float GetAttackTimeScale() const { return 1.0f; }
	virtual float GetAimTimeScale() const { return 1.0f; }

	bool AttackObstacle(const KamataEngine::Vector3& cellPos) override { return FireAtObstacle(currentFirearm_.get(), cellPos); }
private:
	friend class DebugMenu;

	std::unique_ptr<FirearmMusketBayonet> currentFirearm_;
	bool isBayonetActive_ = false;
	bool hasHitPlayerThisSwing_ = false;

	static inline SRTOffset firearmOffset_ = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

	GunPose gunPose_ = GunPose::kNeutral;
	float gunPoseTimer_ = 0.0f;
	static inline SRTOffset kAimPose = {
	    {1.0f,   1.0f,   1.0f},
        {-0.15f, 0.0f,   0.0f},
        {0.0f,   -0.05f, 0.0f}
    };
	static inline SRTOffset kFirePose = {
	    {1.05f,  0.97f, 1.0f},
        {-0.2f,  0.0f,  0.0f},
        {-0.05f, 0.0f,  0.0f}
    };
	static inline SRTOffset kReloadPose = {
	    {1.0f,  0.95f, 1.0f},
        {0.25f, 0.0f,  0.0f},
        {0.0f,  -0.1f, 0.0f}
    };
	static inline float kFirePoseDuration = 0.12f;

	// --- SFX ---
	static inline uint32_t sBayonetLungeSoundHandle_ = 0;
	static inline float sSfxVolume_ = 1.0f;
	static void PlayBayonetLungeSound() {
		if (sBayonetLungeSoundHandle_ != 0) {
			AudioSettings::PlaySfx(sBayonetLungeSoundHandle_, false, sSfxVolume_);
		}
	}

	// Crouch back, then drive the shoulder into the bayonet thrust.
	static inline BodyLungePoseSet kBodyLunge = {
	    MakeBodyPose(-0.2f, -0.3f, 0.0f, {1.0f, 0.97f, 1.0f}, {0.0f, -0.05f, 0.0f}),
	    MakeBodyPose(0.35f, 0.25f, 0.0f, {1.02f, 0.98f, 1.0f}),
	};

protected:

	void RefreshWeaponVisual() override {
		SetBayonetActive(false);
		ResetBayonetHit();
		UpdateFirearmHybrid();
	}
	void SnapWeaponEquipState(bool equipped) override {
		if (currentFirearm_)
			currentFirearm_->SnapEquipped(equipped);
	}

	std::unique_ptr<BaseEnemyBehavior> CreateBehaviorModule() override { return std::make_unique<RiflemanBehavior>(*this); }
	bool GetDropInfo(DropItemCategory& outCategory, DropWeaponKind& outKind) const override {
		outCategory = DropItemCategory::kFirearm;
		outKind = DropWeaponKind::kMusketBayonet;
		return true;
	}
};