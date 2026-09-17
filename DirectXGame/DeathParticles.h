#pragma once

#define NOMINMAX

#include "BaseEffect.h"
#include <array>
#include <numbers>

/// <summary>
/// デス演出用パーティクル
/// </summary>
class DeathParticles : public BaseEffect {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update() override;
	void Draw() override;

private:
	static inline const uint32_t kNumParticles_ = 8;
	std::array<KamataEngine::WorldTransform, kNumParticles_> worldTransforms_;

	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::Model* model_ = nullptr;

	static inline float kDuration_ = 2.0f;
	static inline float kSpeed_ = 0.05f;
	static inline float kAngleUnit_ = 2.0f * std::numbers::pi_v<float> / kNumParticles_;
};