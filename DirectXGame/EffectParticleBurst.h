#pragma once

#define NOMINMAX

#include "BaseEffect.h"
#include "RandomGenerator.h"
#include <array>

// バーストの種類
enum class BurstType { kHit = 0, kJump, kLand, kExplosion, kSmoke, kCount };

/// <summary>
/// 一発のパーティクルバースト(被弾の火花 / ジャンプ・着地の土煙)。使い捨て。
/// プレイヤーと敵の両方で使う。
/// </summary>
class EffectParticleBurst : public BaseEffect {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	static void SetModel(KamataEngine::Model* model) { model_ = model; }
	static void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

	// position から dir 方向を中心に弾ける。被弾は被弾方向、ジャンプ/着地は上向き(0,1)を渡す
	static EffectParticleBurst* Create(uint32_t textureHandle, BurstType type, const KamataEngine::Vector3& position, const KamataEngine::Vector2& dir);

	void Initialize(uint32_t textureHandle, BurstType type, const KamataEngine::Vector3& position, const KamataEngine::Vector2& dir);
	void Draw() override;

protected:
	void OnCommonUpdate() override;

private:
	// 種類ごとの調整値
	struct Preset {
		int count;
		KamataEngine::Vector3 colorStart;
		KamataEngine::Vector3 colorEnd;
		float alphaStart;
		float lifeMin;
		float lifeMax;
		float speedMin;
		float speedMax;
		float spread;  // 中心方向からの角度ばらつき(rad)。3.14で全方位
		float gravity; // 下向きの加速度(秒^2あたり)
		float drag;    // 毎フレームの速度の残り割合
		float sizeStart;
		float sizeEnd;
		float spawnRadius; // 発生位置のばらつき
		int additive;      // 1=加算合成 / 0=通常合成
	};

	struct Particle {
		KamataEngine::WorldTransform worldTransform;
		KamataEngine::ObjectColor objectColor;
		KamataEngine::Vector3 velocity = {};
		float lifetime = 1.0f;
		float age = 0.0f;
		bool active = false;
	};

	static constexpr size_t kMaxParticles = 32;

	std::array<Particle, kMaxParticles> particles_;
	size_t count_ = 0;
	Preset preset_{}; // 生成時の値をコピーして保持(飛んでいる間に調整値が変わっても影響しない)
	RandomGenerator random_;

	static KamataEngine::Camera* camera_;
	static KamataEngine::Model* model_;

	static inline std::array<Preset, static_cast<size_t>(BurstType::kCount)> kPresets = {
	    {
         // kHit: 赤〜オレンジの火花が全方位に弾ける(加算)
	        {14, {1.0f, 0.9f, 0.6f}, {1.0f, 0.2f, 0.05f}, 1.0f, 0.20f, 0.40f, 5.0f, 11.0f, 3.14f, 14.0f, 0.90f, 0.28f, 0.0f, 0.10f, 1},
         // kJump: 足元から左右上方へ土煙(通常)
	        {8, {0.75f, 0.70f, 0.62f}, {0.50f, 0.47f, 0.42f}, 0.6f, 0.30f, 0.55f, 1.5f, 4.0f, 1.30f, 0.0f, 0.92f, 0.20f, 0.50f, 0.15f, 0},
         // kLand: 着地は少し多く、広く
	        {12, {0.75f, 0.70f, 0.62f}, {0.50f, 0.47f, 0.42f}, 0.7f, 0.35f, 0.65f, 2.5f, 6.0f, 1.45f, 0.0f, 0.90f, 0.25f, 0.60f, 0.20f, 0},
         // kExplosion: 大きな火の玉。オレンジ→暗い赤へ変わりながら膨らみ、少し上昇する(加算)
	        {24, {1.0f, 0.85f, 0.4f}, {0.7f, 0.1f, 0.0f}, 1.0f, 0.30f, 0.60f, 3.0f, 9.0f, 3.14f, -2.0f, 0.88f, 0.50f, 1.40f, 0.30f, 1},
         // kSmoke: 黒っぽい煙。ゆっくり広がりながら上昇し、長く残る(通常)
	        {16, {0.35f, 0.33f, 0.31f}, {0.15f, 0.15f, 0.15f}, 0.7f, 1.00f, 1.80f, 0.8f, 2.5f, 3.14f, -1.5f, 0.95f, 0.50f, 1.60f, 0.40f, 0},
	     }
    };
};