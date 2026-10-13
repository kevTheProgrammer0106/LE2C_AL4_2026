#pragma once

#include "EnemyFighter.h"
#include "OfficerBehavior.h"

/// Buffs every enemy when alerted by the player; the buff ends when this officer dies.
/// Uses the sabre until WeaponSpear exists.
class EnemyOfficer final : public EnemyFighter {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update() override;

	bool IsEscortLeader() const override { return true; }

	void BeginBuffPose() { buffPoseTimer_ = 0.0f; }
	float GetBuffAnimDuration() const { return kBuffAnimDuration; }
	float GetBuffApplyTime() const { return kBuffApplyTime; }

	SRTOffset GetActionAnimOffset() const override;

protected:
	std::unique_ptr<BaseEnemyBehavior> CreateBehaviorModule() override { return std::make_unique<OfficerBehavior>(*this); }

	// When WeaponSpear exists: override SetMeleeModel (see EnemyCuirassier::SetMeleeModel) to create it instead of WeaponSabre.

private:
	float buffPoseTimer_ = 999.0f;

	static inline int32_t kOfficerHP = 200;
	static inline float kBuffAnimDuration = 1.4f;
	static inline float kBuffApplyTime = 0.6f;
	static inline SRTOffset kBuffPose = MakeBodyPose(-0.35f, 0.0f, 0.0f, {1.0f, 1.05f, 1.0f}, {0.0f, 0.05f, 0.0f});
};