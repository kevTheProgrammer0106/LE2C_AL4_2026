#pragma once
#include "EnemyRifleman.h"

/// Rifleman behavior with Cuirassier-level HP, slower reload and slower bayonet/aim.
class EnemyCarabinier final : public EnemyRifleman {
public:
	void Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	float GetAimTimeScale() const override { return kAimTimeScale; }

protected:
	float GetReloadTimeScale() const override { return kReloadTimeScale; }
	float GetAttackTimeScale() const override { return kAttackTimeScale; }

private:
	static inline int kCarabinerHP = 350;         // same as Cuirassier
	static inline float kReloadTimeScale = 1.75f; // 1.5s -> ~2.6s
	static inline float kAttackTimeScale = 1.5f;  // bayonet phases 50% longer
	static inline float kAimTimeScale = 1.25f;
};