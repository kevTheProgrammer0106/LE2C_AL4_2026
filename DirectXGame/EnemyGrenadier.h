#pragma once

#include "EnemyFighter.h"
#include "GrenadierBehavior.h"

class EnemyGrenadier final : public EnemyFighter {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update() override;

	// Spawns a grenade toward the player with the fixed lob arc and plays the throw pose.
	void ThrowGrenade();

	SRTOffset GetActionAnimOffset() const override;

protected:
	std::unique_ptr<BaseEnemyBehavior> CreateBehaviorModule() override { return std::make_unique<GrenadierBehavior>(*this); }

private:
	float throwPoseTimer_ = 999.0f;

	static inline int32_t kGrenadierHP = 100;
	static inline float kThrowPoseDuration = 0.3f;
	static inline SRTOffset kThrowPose = MakeBodyPose(0.35f, 0.25f, 0.0f, {1.02f, 0.98f, 1.0f}, {0.0f, -0.03f, 0.0f});
};