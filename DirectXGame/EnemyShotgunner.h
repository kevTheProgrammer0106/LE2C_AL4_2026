#pragma once

#include "BaseEnemy.h"
#include "FirearmShotgun.h"
#include "ShotgunnerBehavior.h"

/// <summary>
/// ショットガンを持つ敵。EnemyRifleman(近接なし版)と同じ構造。
/// 1回の発砲でFirearmShotgunのペレット数ぶんの弾を固定角で扇状に撃つ。
/// </summary>
class EnemyShotgunner final : public BaseEnemy {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update() override;
	void Draw() override;

	FirearmShotgun* GetCurrentFirearm() const { return currentFirearm_.get(); }
	void SetShotgunModel(KamataEngine::Model* model, uint32_t textureHandle);

	KamataEngine::Vector2 ComputeTargetAimPosition();
	void ShootFirearm();
	void UpdateFirearmHybrid();

	enum class GunPose { kNeutral, kAiming, kFiring, kReloading };
	void SetGunPose(GunPose pose) {
		gunPose_ = pose;
		gunPoseTimer_ = 0.0f;
	}

	SRTOffset GetActionAnimOffset() const override;

	bool AttackObstacle(const KamataEngine::Vector3& cellPos) override { return FireAtObstacle(currentFirearm_.get(), cellPos); }

private:
	friend class DebugMenu;

	std::unique_ptr<FirearmShotgun> currentFirearm_;

	static inline SRTOffset firearmOffset_ = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

	GunPose gunPose_ = GunPose::kNeutral;
	float gunPoseTimer_ = 0.0f;

	// ショットガンは反動が大きいので、発砲ポーズはライフルより強め
	static inline SRTOffset kAimPose = {
	    {1.0f,   1.0f,   1.0f},
        {-0.15f, 0.0f,   0.0f},
        {0.0f,   -0.05f, 0.0f}
    };
	static inline SRTOffset kFirePose = {
	    {1.08f, 0.95f, 1.0f},
        {-0.3f, 0.0f,  0.0f},
        {-0.1f, 0.0f,  0.0f}
    };
	static inline SRTOffset kReloadPose = {
	    {1.0f,  0.95f, 1.0f},
        {0.25f, 0.0f,  0.0f},
        {0.0f,  -0.1f, 0.0f}
    };
	static inline float kFirePoseDuration = 0.2f;

protected:
	void RefreshWeaponVisual() override { UpdateFirearmHybrid(); }
	void SnapWeaponEquipState(bool equipped) override {
		if (currentFirearm_)
			currentFirearm_->SnapEquipped(equipped);
	}

	std::unique_ptr<BaseEnemyBehavior> CreateBehaviorModule() override { return std::make_unique<ShotgunnerBehavior>(*this); }
	bool GetDropInfo(DropItemCategory& outCategory, DropWeaponKind& outKind) const override {
		outCategory = DropItemCategory::kFirearm;
		outKind = DropWeaponKind::kShotgun;
		return true;
	}
};