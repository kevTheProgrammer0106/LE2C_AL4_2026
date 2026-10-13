#pragma once

#define NOMINMAX

#include "Global.h"
#include <KamataEngine.h>

class MapChipField;

/// Lobbed bomb. Flies on a fixed arc, bounces on terrain, and detonates when the fuse runs out.
/// Last kFlashStartTime seconds: flashes red, faster over time. Last kEnlargeDuration seconds: eases up in size.
/// GameScene owns it and calls ExplodeAt() when IsReadyToExplode() turns true.
class Grenade {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(
	    KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera, MapChipField* field, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity,
	    Owner owner);
	void Update();
	void Draw();

	bool IsReadyToExplode() const { return readyToExplode_ && !isDead_; }
	bool IsDead() const { return isDead_; }
	void MarkDead() { isDead_ = true; }
	Owner GetOwner() const { return owner_; }
	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

	// Fixed lob: same speed whatever the distance (the "fixed vx / vy + gravity" algorithm)
	static KamataEngine::Vector3 MakeThrowVelocity(bool facingRight);

	// Blast values used by GameScene::ExplodeAt
	static float GetFatalRadius() { return kFatalRadius; }
	static float GetNormalRadius() { return kNormalRadius; }
	static int32_t GetNormalDamage() { return kNormalDamage; }
	static float GetImpact() { return kImpact; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::ObjectColor objectColor_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	MapChipField* mapChipField_ = nullptr;
	uint32_t textureHandle_ = 0;

	KamataEngine::Vector3 velocity_{};
	Owner owner_ = Owner::kEnemy;

	float fuseTimer_ = 3.0f;
	float flashPhase_ = 0.0f;
	bool readyToExplode_ = false;
	bool isDead_ = false;

	// ---- fuse / visuals ----
	static inline float kFuseTime = 3.0f;
	static inline float kFlashStartTime = 1.5f;  // flashing starts when this much fuse is left
	static inline float kFlashFreqStart = 4.0f;  // blinks per second at the start of flashing
	static inline float kFlashFreqEnd = 14.0f;   // ...and just before the end
	static inline float kEnlargeDuration = 0.5f; // final seconds spent growing
	static inline float kEnlargeScale = 1.8f;    // size multiplier at detonation
	static inline float kBaseScale = 1.5f;

	// ---- flight (world units per frame) ----
	static inline float kThrowSpeedX = 0.25f;
	static inline float kThrowSpeedY = 0.4f; // with gravity 0.025 -> 32 frames in the air, lands about 8 units away
	static inline float kGravity = 0.025f;
	static inline float kMaxFallSpeed = 0.5f;
	static inline float kRestitution = 0.4f;
	static inline float kFriction = 0.7f;
	static inline float kSettleSpeed = 0.05f;
	static inline float kBodySize = 0.5f;
	static inline float kKillY = -5.0f;

	// ---- blast ----
	static inline float kFatalRadius = 1.5f;  // instant kill (like the barrel's fatal radius)
	static inline float kNormalRadius = 5.0f; // damage (like the barrel's normal radius)
	static inline int32_t kNormalDamage = 20;
	static inline float kImpact = 2.0f;
};