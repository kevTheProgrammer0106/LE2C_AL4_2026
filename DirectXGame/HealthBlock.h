#pragma once

#include "Global.h"
#include <KamataEngine.h>

/// <summary>
/// フィールド上に置かれ、触れるとプレイヤーのHPを全回復する固定オブジェクト。
/// kLimited: 一度使うと消えたまま。kTimer: 使用後、一定時間(kRespawnInterval)で再出現する。
/// 見た目のアニメーション(浮遊+回転)はDroppableItemと同じ挙動。
/// </summary>
class HealthBlock {
public:
	enum class Mode { kLimited, kTimer };

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, Mode mode);
	void Update();
	void Draw();

	AABB GetAABB() const;

	bool IsActive() const { return isActive_; }
	Mode GetMode() const { return mode_; }

	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

	// GameSceneがプレイヤーとの接触を検知した時に呼ぶ。HP全回復後、
	// kLimitedなら恒久的に非アクティブ化、kTimerなら再出現タイマーを開始する。
	void Consume();
	static void SetConsumeSoundHandle(uint32_t handle) { sConsumeSoundHandle_ = handle; }
	static void SetSfxVolume(float volume) { sSfxVolume_ = volume; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0;

	Mode mode_ = Mode::kLimited;
	bool isActive_ = true;

	KamataEngine::Vector3 basePosition_{};
	float bobTimer_ = 0.0f;

	float respawnTimer_ = 0.0f;
	static inline float kRespawnInterval = 15.0f; // [NEW] kTimerモードの再出現までの秒数

	static inline float kHalfWidth = 0.4f;
	static inline float kHalfHeight = 0.4f;
	static inline float kHalfDepth = 0.4f;
	static inline float kBobAmplitude = 0.1f;
	static inline float kBobSpeed = 2.0f;
	static inline float kSpinSpeed = 1.5f;

	// [NEW] 再出現直前の点滅警告(DroppableItemの消滅前点滅と同じ演出)
	static inline float kBlinkWarningDuration = 3.0f;
	static inline float kBlinkFrequency = 6.0f;

		static inline uint32_t sConsumeSoundHandle_ = 0;
	static inline float sSfxVolume_ = 1.0f;
};