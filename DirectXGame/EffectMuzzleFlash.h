#pragma once

#define NOMINMAX

#include "BaseEffect.h"
#include "RandomGenerator.h"
#include <array>

/// <summary>
/// 発砲時のマズルエフェクト(1発ごとに生成する使い捨て)。
/// kExpand = 赤い火花が弾ける / kFadeOut = 遅れて出た煙が銃口に追従しながら消える。
/// </summary>
class EffectMuzzleFlash : public BaseEffect {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	static void SetModel(KamataEngine::Model* model) { model_ = model; }
	static void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

	// プレイヤーの銃口位置を毎フレーム渡す(煙の追従用)。Player::Update から呼ぶ
	static void SetPlayerAnchor(const KamataEngine::Vector3& pos);

	// muzzlePos から aimDir 方向へ発砲エフェクトを出す。followPlayer=true なら銃口の移動に追従する
	static EffectMuzzleFlash* Create(uint32_t fireTexture, uint32_t smokeTexture, const KamataEngine::Vector3& muzzlePos, const KamataEngine::Vector2& aimDir, bool followPlayer);
	void Initialize(uint32_t fireTexture, uint32_t smokeTexture, const KamataEngine::Vector3& muzzlePos, const KamataEngine::Vector2& aimDir, bool followPlayer);
	
	void Draw() override;

protected:
	void OnCommonUpdate() override;

private:
	struct Particle {
		KamataEngine::WorldTransform worldTransform;
		KamataEngine::ObjectColor objectColor;
		KamataEngine::Vector3 velocity = {};
		float lifetime = 1.0f;
		float age = 0.0f; // 負の間は待機中(描画しない)
		bool active = false;
	};

	void UpdateParticle(Particle& particle, bool isSmoke);
	void DrawGroup(const std::array<Particle, 32>& group, size_t count, KamataEngine::Model::BlendMode blend);

	static constexpr size_t kMaxFire = 32;
	static constexpr size_t kMaxSmoke = 32;

	std::array<Particle, kMaxFire> fire_;
	std::array<Particle, kMaxSmoke> smoke_;
	uint32_t smokeTextureHandle_ = 0;
	size_t fireCount_ = 0;
	size_t smokeCount_ = 0;

	RandomGenerator random_;
	bool follow_ = false;
	bool firstFrame_ = true; // 生成フレームは銃の移動が既に反映済みなので追従しない

	static KamataEngine::Camera* camera_;
	static KamataEngine::Model* model_;

	// 追従用の銃口位置(全インスタンス共有)
	static inline KamataEngine::Vector3 sAnchor_ = {};
	static inline KamataEngine::Vector3 sAnchorDelta_ = {}; // このフレームの銃口の移動量
	static inline bool sHasAnchor_ = false;

	// ---- 火花(赤い弾け) ----
	static inline int kFireCount = 12;
	static inline KamataEngine::Vector3 kFireColorStart = {1.0f, 0.45f, 0.1f};
	static inline KamataEngine::Vector3 kFireColorEnd = {0.8f, 0.0f, 0.0f};
	static inline float kFireAlphaStart = 1.0f;
	static inline float kFireLifeMin = 0.10f;
	static inline float kFireLifeMax = 0.22f;
	static inline float kFireSpeedMin = 6.0f; // 秒あたりの移動量
	static inline float kFireSpeedMax = 14.0f;
	static inline float kFireSpread = 0.45f; // 発射方向からの角度ばらつき(rad)
	static inline float kFireSizeStart = 0.35f;
	static inline float kFireSizeEnd = 0.0f;

	// ---- 煙 ----
	static inline int kSmokeCount = 7;
	static inline KamataEngine::Vector3 kSmokeColorStart = {0.75f, 0.75f, 0.75f};
	static inline KamataEngine::Vector3 kSmokeColorEnd = {0.4f, 0.4f, 0.4f};
	static inline float kSmokeAlphaStart = 0.55f;
	static inline float kSmokeLifeMin = 0.6f;
	static inline float kSmokeLifeMax = 1.1f;
	static inline float kSmokeDelayMax = 0.12f; // 火花より遅れて出る最大秒数
	static inline float kSmokeSpeedMin = 1.0f;
	static inline float kSmokeSpeedMax = 3.0f;
	static inline float kSmokeRiseMin = 0.8f; // 上昇速度
	static inline float kSmokeRiseMax = 1.6f;
	static inline float kSmokeSizeStart = 0.15f;
	static inline float kSmokeSizeEnd = 0.7f;

	// 銃口へ追従する秒数(だんだん弱まる)
	static inline float kFollowTime = 0.35f;
};