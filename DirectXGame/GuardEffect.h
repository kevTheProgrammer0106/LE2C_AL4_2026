#pragma once

#define NOMINMAX

#include "BaseEffect.h"

/// <summary>
/// ガード(シールド)演出エフェクト
/// </summary>
class GuardEffect : public BaseEffect {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	~GuardEffect() override;

	void Initialize(uint32_t textureHandle, KamataEngine::Vector3& spawnLocation);
	void Draw() override;

	static void SetModel(KamataEngine::Model* model) { model_ = model; }
	static void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

	static GuardEffect* Create(uint32_t textureHandle, KamataEngine::Vector3& spawnLocation);

protected:
	void OnExpandUpdate(float t) override;
	void OnCommonUpdate() override;

private:
	static KamataEngine::Camera* camera_;
	static KamataEngine::Model* model_;

	KamataEngine::WorldTransform ringWorldTransform_;

	static inline float kExpandDuration_ = 0.15f;
	static inline float kFadeDuration_ = 0.25f;

	static inline KamataEngine::Vector3 kRingScaleStart_{0.0f, 0.0f, 0.0f};
	static inline KamataEngine::Vector3 kRingScaleEnd_{1.2f, 1.2f, 1.0f};

	static inline float kRotationSpeed_ = 0.1f; // 既存(非const化してGVで調整可能に)
};