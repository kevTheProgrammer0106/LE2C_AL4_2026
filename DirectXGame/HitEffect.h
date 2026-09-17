#pragma once

#define NOMINMAX

#include "BaseEffect.h"
#include <array>

class RandomGenerator;

class HitEffect : public BaseEffect {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	~HitEffect() override;

	void Initialize(uint32_t textureHandle, KamataEngine::Vector3& spawnLocation, float rotationZ = 0.0f);
	void Draw() override;

	static void SetModel(KamataEngine::Model* model) { model_ = model; }
	static void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

	static HitEffect* Create(uint32_t textureHandle, KamataEngine::Vector3& spawnLocation, float rotationZ = 0.0f);

protected:
	void OnExpandUpdate(float t) override;
	void OnCommonUpdate() override;

private:
	RandomGenerator* randomizer = nullptr;

	static KamataEngine::Camera* camera_;
	static KamataEngine::Model* model_;

	static inline const uint32_t kNumEllipses = 3;
	KamataEngine::WorldTransform circleWorldTransform_;
	std::array<KamataEngine::WorldTransform, kNumEllipses> ellipseWorldTransforms_;

	static inline float kExpandDuration_ = 0.15f;
	static inline float kFadeDuration_ = 0.25f;

	static inline KamataEngine::Vector3 kCircleScaleStart_{0.0f, 0.0f, 0.0f};
	static inline KamataEngine::Vector3 kCircleScaleEnd_{2.0f, 2.0f, 1.0f};
	static inline KamataEngine::Vector3 kEllipseScaleStart_{0.0f, 0.0f, 0.0f};
	static inline KamataEngine::Vector3 kEllipseScaleEnd_{1.5f, 0.045f, 1.0f};
};