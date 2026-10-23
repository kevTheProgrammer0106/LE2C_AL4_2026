#pragma once

#define NOMINMAX

#include "BaseEffect.h"
#include "RandomGenerator.h"
#include <array>

// バーストの種類
enum class BurstType {
	kHit = 0,
	kJump,
	kLand,
	kExplosion,
	kSmoke,
	kAura,
	kBulletBlock,
	kBulletEntity,
	kSpawn,
	kSmash,
	kGrenadeFuse,
	kBlockDamage,
	kBulletTrail,
	kSwingTrail,
	kSpike,
	kBlastSmoke,
	kCount
};

/// <summary>
/// 一発のパーティクルバースト(被弾の火花 / ジャンプ・着地の土煙 / 爆発の煙)。使い捨て。
/// プレイヤーと敵の両方で使う。
/// loopDuration > 0 のプリセットは、その間 寿命が尽きた粒子を発生位置で再生成し続ける(煙が残る表現)。
/// </summary>
class EffectParticleBurst : public BaseEffect {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	static void SetModel(KamataEngine::Model* model) { model_ = model; }
	static void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

	// position から dir 方向を中心に弾ける。被弾は被弾方向、ジャンプ/着地は上向き(0,1)を渡す
	// loopDurationOverride >= 0 ならプリセットのループ継続秒数をこの値で上書きする(負ならプリセットのまま)
	static EffectParticleBurst* Create(uint32_t textureHandle, BurstType type, const KamataEngine::Vector3& position, const KamataEngine::Vector2& dir, float loopDurationOverride = -1.0f);

	void Initialize(uint32_t textureHandle, BurstType type, const KamataEngine::Vector3& position, const KamataEngine::Vector2& dir, float loopDurationOverride = -1.0f);
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
		float spawnRadius;  // 発生位置のばらつき
		int additive;       // 1=加算合成 / 0=通常合成
		float loopDuration; // 粒子を再生成し続ける秒数(0=ループなし)
	};

	struct Particle {
		KamataEngine::WorldTransform worldTransform;
		KamataEngine::ObjectColor objectColor;
		KamataEngine::Vector3 velocity = {};
		float lifetime = 1.0f;
		float age = 0.0f;        // 負の間は再生成待ち(動かさず描画もしない)
		float alphaScale = 1.0f; // 再生成された粒子ほど薄くする係数
		bool active = false;
	};

	// 粒子1個を(再)生成する。WorldTransform/ObjectColorのInitializeは呼ばない(再利用のため)
	void SpawnParticle(Particle& particle, float alphaScale, float initialAge);

	static constexpr size_t kMaxParticles = 32;

	std::array<Particle, kMaxParticles> particles_;
	size_t count_ = 0;
	Preset preset_{}; // 生成時の値をコピーして保持(飛んでいる間に調整値が変わっても影響しない)
	RandomGenerator random_;

	// ループ再生成用
	KamataEngine::Vector3 origin_{};
	float baseAngle_ = 0.0f;
	float emitTimer_ = 0.0f; // 生成からの経過秒数
	float loopDuration_ = 0.0f;

	static KamataEngine::Camera* camera_;
	static KamataEngine::Model* model_;

	static inline std::array<Preset, static_cast<size_t>(BurstType::kCount)> kPresets = {
	    {
         // kHit
	        {14, {1.0f, 0.9f, 0.6f}, {1.0f, 0.2f, 0.05f}, 1.0f, 0.20f, 0.40f, 5.0f, 11.0f, 3.14f, 14.0f, 0.90f, 0.28f, 0.0f, 0.10f, 1, 0.0f},
         // kJump
	        {8, {0.75f, 0.70f, 0.62f}, {0.50f, 0.47f, 0.42f}, 0.6f, 0.30f, 0.55f, 1.5f, 4.0f, 1.30f, 0.0f, 0.92f, 0.20f, 0.50f, 0.15f, 0, 0.0f},
         // kLand
	        {12, {0.75f, 0.70f, 0.62f}, {0.50f, 0.47f, 0.42f}, 0.7f, 0.35f, 0.65f, 2.5f, 6.0f, 1.45f, 0.0f, 0.90f, 0.25f, 0.60f, 0.20f, 0, 0.0f},
         // kExplosion
	        {24, {1.0f, 0.85f, 0.4f}, {0.7f, 0.1f, 0.0f}, 1.0f, 0.30f, 0.60f, 3.0f, 9.0f, 3.14f, -2.0f, 0.88f, 0.50f, 1.40f, 0.30f, 1, 0.0f},
         // kSmoke
	        {16, {0.35f, 0.33f, 0.31f}, {0.15f, 0.15f, 0.15f}, 0.7f, 1.00f, 1.80f, 0.8f, 2.5f, 3.14f, -1.5f, 0.95f, 0.50f, 1.60f, 0.40f, 0, 4.0f},
         // kAura: 見えやすいよう個数/サイズを増やした
	        {6, {1.0f, 0.85f, 0.3f}, {1.0f, 0.45f, 0.1f}, 0.9f, 0.50f, 0.90f, 0.4f, 1.2f, 0.7f, -1.5f, 0.97f, 0.32f, 0.0f, 0.45f, 1, 0.0f},
         // kBulletBlock
	        {8, {1.0f, 0.95f, 0.7f}, {0.55f, 0.4f, 0.25f}, 1.0f, 0.15f, 0.30f, 3.0f, 8.0f, 1.1f, 12.0f, 0.90f, 0.20f, 0.0f, 0.05f, 1, 0.0f},
         // kBulletEntity
	        {10, {1.0f, 0.25f, 0.2f}, {0.45f, 0.0f, 0.0f}, 1.0f, 0.20f, 0.40f, 3.0f, 9.0f, 0.9f, 8.0f, 0.90f, 0.24f, 0.0f, 0.08f, 0, 0.0f},
         // kSpawn
	        {14, {0.8f, 1.0f, 1.0f}, {0.3f, 0.7f, 1.0f}, 0.9f, 0.30f, 0.60f, 1.5f, 4.5f, 3.14f, -1.0f, 0.92f, 0.25f, 0.0f, 0.45f, 1, 0.0f},
         // kSmash: 工兵の叩きつけ。地面を這う土煙(左右に出す)
	        {14, {0.80f, 0.74f, 0.62f}, {0.50f, 0.46f, 0.40f}, 0.8f, 0.40f, 0.80f, 4.0f, 9.0f, 0.5f, 0.0f, 0.92f, 0.35f, 0.90f, 0.50f, 0, 0.0f},
         // kGrenadeFuse: 導火線の火花(加算)
	        {3, {1.0f, 0.7f, 0.25f}, {0.6f, 0.2f, 0.05f}, 1.0f, 0.20f, 0.40f, 1.0f, 3.0f, 3.14f, -1.5f, 0.90f, 0.20f, 0.0f, 0.10f, 1, 0.0f},
         // kBlockDamage: 壊せるブロックの破片(通常合成)
	        {8, {0.75f, 0.70f, 0.62f}, {0.40f, 0.37f, 0.33f}, 0.9f, 0.30f, 0.60f, 2.0f, 6.0f, 3.14f, 14.0f, 0.92f, 0.22f, 0.05f, 0.35f, 0, 0.0f},
         // kBulletTrail: 弾の尾(加算)
	        {2, {1.0f, 0.9f, 0.6f}, {1.0f, 0.35f, 0.1f}, 0.8f, 0.10f, 0.20f, 0.5f, 1.5f, 0.6f, 0.0f, 0.90f, 0.16f, 0.0f, 0.04f, 1, 0.0f},
         // kSwingTrail: 剣の軌跡(加算)
	        {3, {1.0f, 1.0f, 1.0f}, {0.6f, 0.8f, 1.0f}, 0.7f, 0.12f, 0.25f, 0.5f, 2.0f, 0.8f, 0.0f, 0.90f, 0.25f, 0.0f, 0.08f, 1, 0.0f},
         // kSpike: 針の被ダメ(赤い火花、上向き)
	        {12, {1.0f, 0.4f, 0.3f}, {0.6f, 0.05f, 0.05f}, 1.0f, 0.25f, 0.50f, 4.0f, 10.0f, 1.2f, 14.0f, 0.90f, 0.26f, 0.0f, 0.25f, 1, 0.0f},
         // kBlastSmoke: 爆風で吹き飛んだ体から出る煙(軽め)
	        {3, {0.35f, 0.33f, 0.31f}, {0.15f, 0.15f, 0.15f}, 0.6f, 0.50f, 1.00f, 0.3f, 1.2f, 3.14f, -1.2f, 0.95f, 0.30f, 0.90f, 0.25f, 0, 0.0f},
	     }
    };
};