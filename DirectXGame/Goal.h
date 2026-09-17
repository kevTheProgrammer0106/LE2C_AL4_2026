#pragma once

#include "Global.h"
#include <KamataEngine.h>

/// <summary>
/// ステージクリア地点（ゴール）
/// GameScene::CheckAllCollisions() から毎フレーム CheckCollision() を呼んでもらう想定。
/// プレイヤーのAABBと重なったら isCleared_ を立てる（以後維持）。
/// </summary>
class Goal {
public:
	void Initialize(KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	// プレイヤーのAABBと衝突しているか判定する（GameScene::CheckAllCollisionsから呼ぶ）
	bool CheckCollision(const AABB& playerAABB);

	AABB GetAABB() const;

	bool IsCleared() const { return isCleared_; }

	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

private:
	void UpdateIdleAnimation();
	void UpdateClearedAnimation();

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr; // GameSceneが所有、Goalは借用のみ（WeaponSabre等と同じ方針）
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0;

	bool isCleared_ = false;
	float animTimer_ = 0.0f;

	KamataEngine::Vector3 basePosition_ = {0.0f, 0.0f, 0.0f};
	KamataEngine::Vector3 baseScale_ = {1.0f, 1.0f, 1.0f};

	// GlobalVariablesで調整する値（static: WeaponSabreの kHalfWidthMin 等と同じ扱い）
	static inline float kHalfWidth = 1.0f;
	static inline float kHalfHeight = 1.5f;
	static inline float kHalfDepth = 1.0f;

	static inline float kBobAmplitude = 0.1f;
	static inline float kBobSpeed = 2.0f;

	static inline float kClearPulseScale = 1.3f;
	static inline float kClearPulseDuration = 0.3f;
};