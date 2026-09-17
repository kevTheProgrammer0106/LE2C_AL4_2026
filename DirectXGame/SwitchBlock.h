#pragma once

#include "Global.h"
#include <KamataEngine.h>

/// <summary>
/// ゲート開放用スイッチ。プレイヤーが重なると押された扱いになり、
/// 同じbuttonID_を持つSlidingGateをすべて開く(GameScene側で判定)。
/// </summary>
class SwitchBlock {
public:
	void Initialize(KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, uint8_t buttonID);
	void Update();
	void Draw();

	AABB GetAABB() const;
	uint8_t GetButtonID() const { return buttonID_; }

	bool IsPressed() const { return isPressed_; }
	void SetPressed(bool pressed) { isPressed_ = pressed; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0;

	uint8_t buttonID_ = 0;
	bool isPressed_ = false;

	static inline float kHalfWidth = 0.8f;
	static inline float kHalfHeight = 0.3f;
	static inline float kHalfDepth = 0.8f;

	static inline float kPressedScaleY = 0.4f; // 押されると平べったくなる(見た目のフィードバック)

	// [NEW] 未使用時は赤、押されると緑になる(視覚的フィードバック)
	KamataEngine::ObjectColor objectColor_;
	static inline KamataEngine::Vector4 kUnpressedColor = {1.0f, 0.15f, 0.15f, 1.0f};
	static inline KamataEngine::Vector4 kPressedColor = {0.15f, 1.0f, 0.15f, 1.0f};
};