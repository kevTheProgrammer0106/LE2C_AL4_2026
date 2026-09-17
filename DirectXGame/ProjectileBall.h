#pragma once

#include "BaseProjectile.h"

class ProjectileBall final : public BaseProjectile {
public:
	ProjectileBall() {
		kWidth = 0.4f;
		kHeight = 0.4f;
		kLifeTime = 90;
		deathTimer_ = kLifeTime;
	}

	void InitializeAimed(
	    KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, const KamataEngine::Vector2& aimDirection, Owner owner);

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

private:
	static inline float kSpeed = 0.8f; // ← must be static inline; Register/Apply are static
};