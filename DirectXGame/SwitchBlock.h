#pragma once

#include "Global.h"
#include <KamataEngine.h>

/// <summary>
/// ゲート開放用レバー。未使用時は直立、使用すると左へバウンドしながら倒れる。
/// supportModel(土台)は動かず、leverModel(棒)だけが倒れる。supportModelはnullptr可。
/// モデル: Blenderで頭が+Z上、棒の根元が下端(原点)。ゲームでは頭が+Y。
/// </summary>
class SwitchBlock {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(
	    KamataEngine::Model* leverModel, uint32_t leverTex, KamataEngine::Model* supportModel, uint32_t supportTex, KamataEngine::Camera* camera, const KamataEngine::Vector3& position,
	    uint8_t buttonID);
	void Update();
	void Draw();

	AABB GetAABB() const;
	uint8_t GetButtonID() const { return buttonID_; }

	bool IsPressed() const { return isPressed_; }
	void SetPressed(bool pressed);
	void SnapPressed();

private:
	KamataEngine::WorldTransform worldTransform_;   // lever
	KamataEngine::WorldTransform supportTransform_; // support (static)
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* supportModel_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0;
	uint32_t supportTex_ = 0;

	KamataEngine::Vector3 position_{};
	uint8_t buttonID_ = 0;
	bool isPressed_ = false;
	float pressTimer_ = 0.0f;

	static inline float kHalfWidth = 0.4f;
	static inline float kHalfHeight = 0.4f;
	static inline float kHalfDepth = 0.4f;

	static inline float kPivotOffsetY = -0.5f;
	static inline float kTiltAngle = 1.0f;
	static inline float kBounceFrequency = 14.0f;
	static inline float kBounceDamping = 6.0f;
};