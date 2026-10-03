#pragma once

#include "Global.h"
#include <KamataEngine.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

/// <summary>
/// Firearm base. ANIMATION offsets (combat / holster / reload / knockback / stun) live here as virtual getters;
/// each concrete firearm overrides them with its own tuned values. STATS (ammo, reload time, damage, bayonet
/// reach, ...) stay entirely in the derived classes -- nothing here touches them.
/// UpdateFollow is shared and built only from those getters, so any firearm can be driven the same way
/// (player or enemy). Derived classes with per-frame state (reloading) hook in via TickReload().
/// </summary>
class BaseFirearm {
public:
	virtual ~BaseFirearm() = default;

	virtual void Initialize(KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera);
	virtual void UpdateFollow(const KamataEngine::WorldTransform& parentTransform, float localAimAngle, bool facingRight);
	virtual void Draw();

	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }
	const SRTOffset& GetOffset() const { return offset_; }

	void SetOffset(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rotation, const KamataEngine::Vector3& translation) {
		offset_.scale = scale;
		offset_.rotation = rotation;
		offset_.translation = translation;
	}

	// ============================================================
	// [NEW] Animation offset polymorphism. Override per firearm.
	// ============================================================
	virtual const SRTOffset& GetCombatOffset() const { return offset_; }
	virtual const SRTOffset& GetHolsterOffset() const { return holsterOffset_; }
	virtual const SRTOffset& GetReloadOffset() const { return reloadOffset_; }
	virtual const SRTOffset& GetKnockbackOffset() const { return kDefaultKnockbackOffset; }
	virtual const SRTOffset& GetStunOffset() const { return kDefaultStunOffset; }
	virtual const SRTOffset& GetBlockOffset() const { return kDefaultBlockOffset; }
	// Extra yaw the model needs so that "rotation.y == 0" faces forward (musket models need +pi/2).
	virtual float GetModelYawOffset() const { return 0.0f; }

	// Fallbacks for firearms that don't override knockback/stun.
	static inline SRTOffset kDefaultKnockbackOffset = {
	    {1.0f, 1.0f,  1.0f },
        {0.3f, 0.0f,  0.9f },
        {0.4f, 0.35f, -0.1f}
    };
	static inline SRTOffset kDefaultStunOffset = {
	    {1.0f,  1.0f,  1.0f },
        {0.0f,  0.0f,  -0.9f},
        {0.55f, -0.1f, -0.1f}
    };
	// [NEW] guard pose: gun held across the body (authored for a right-facing entity)
	static inline SRTOffset kDefaultBlockOffset = {
	    {1.0f,  1.0f,  1.0f },
        {0.0f,  0.0f,  0.7f },
        {0.45f, 0.25f, -0.1f}
    };
	// lerp factor per frame toward/away from the knockback/stun/block pose
	static inline float kDebuffBlendSpeed = 0.2f;

	// Call every frame before UpdateFollow. Pose blends in/out smoothly.
	void SetReactionPose(ReactionPose pose) {
		debuffPose_ = pose;
		if (pose != ReactionPose::kNone)
			lastReactionPose_ = pose;
	}

	void SetEquipped(bool equipped) { equipTarget_ = equipped ? 1.0f : 0.0f; }
	void SetReloading(bool reloading) { isShowingReloadPose_ = reloading; }
	bool IsShowingReloadPose() const { return isShowingReloadPose_; }
	float GetEquipFactor() const { return equipFactor_; }

	static inline float kAimLeanAmount = 0.12f;
	static inline uint32_t kRecoilDuration = 6;
	static inline float kRecoilKick = 0.08f;

	void TriggerRecoil(bool facingRight) {
		recoilTimer_ = kRecoilDuration;
		recoilFacingRight_ = facingRight;
	}
	void TickRecoil() {
		if (recoilTimer_ > 0)
			--recoilTimer_;
	}
	SRTOffset GetRecoilOffset() const {
		if (recoilTimer_ == 0)
			return SRTOffset{};
		float t = static_cast<float>(recoilTimer_) / static_cast<float>(kRecoilDuration);
		float kickX = (recoilFacingRight_ ? -1.0f : 1.0f) * kRecoilKick * t;
		return {
		    {1.0f,  1.0f, 1.0f},
            {0.0f,  0.0f, 0.0f},
            {kickX, 0.0f, 0.0f}
        };
	}
	static SRTOffset ComputeAimLeanOffset(const KamataEngine::Vector2& aimDir, bool facingRight) {
		float angle = std::atan2(aimDir.y, aimDir.x);
		float relative = facingRight ? angle : (std::numbers::pi_v<float> - angle);
		while (relative > std::numbers::pi_v<float>)
			relative -= 2.0f * std::numbers::pi_v<float>;
		while (relative <= -std::numbers::pi_v<float>)
			relative += 2.0f * std::numbers::pi_v<float>;
		float lean = std::clamp(relative / (std::numbers::pi_v<float> / 2.0f), -1.0f, 1.0f) * kAimLeanAmount;
		return {
		    {1.0f, 1.0f, 1.0f},
            {lean, 0.0f, 0.0f},
            {0.0f, 0.0f, 0.0f}
        };
	}

	virtual float GetImpactPower() const { return kImpactPower; }

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	// --- SFX ---
	static void SetFireSoundHandle(uint32_t handle) { sFireSoundHandle_ = handle; }
	static void SetReloadSoundHandle(uint32_t handle) { sReloadSoundHandle_ = handle; }
	static void SetSfxVolume(float volume) { sSfxVolume_ = volume; }
	static void PlayFireSound() { PlaySfxHandle(sFireSoundHandle_); }
	static void PlayReloadSound() { PlaySfxHandle(sReloadSoundHandle_); }

	void SnapEquipped(bool equipped) {
		equipTarget_ = equipped ? 1.0f : 0.0f;
		equipFactor_ = equipTarget_;
	}

protected:
	static inline float kImpactPower = 1.2f;

	static inline float kEquipLerpSpeed = 0.15f;

	// Derived classes advance their own reload/ammo state here (called at the top of UpdateFollow).
	virtual void TickReload() {}

	// Blends the combat pose toward knockback/stun. Ticks the blend once per call (once per UpdateFollow).
	SRTOffset ComputeCombatOffset(const SRTOffset& base) {
		float target = (debuffPose_ != ReactionPose::kNone) ? 1.0f : 0.0f;
		reactionBlend_ = Lerp(reactionBlend_, target, kDebuffBlendSpeed);
		if (reactionBlend_ < 0.001f) {
			reactionBlend_ = 0.0f;
			return base;
		}
		const SRTOffset* debuffPtr = &GetKnockbackOffset();
		if (lastReactionPose_ == ReactionPose::kStun)
			debuffPtr = &GetStunOffset();
		else if (lastReactionPose_ == ReactionPose::kBlock)
			debuffPtr = &GetBlockOffset();
		const SRTOffset& debuff = *debuffPtr;
		return LerpSRT(base, debuff, reactionBlend_);
	}

	float equipFactor_ = 1.0f;
	float equipTarget_ = 1.0f;
	bool isShowingReloadPose_ = false;

	ReactionPose debuffPose_ = ReactionPose::kNone;
	ReactionPose lastReactionPose_ = ReactionPose::kKnockback;
	float reactionBlend_ = 0.0f;

	uint32_t recoilTimer_ = 0;
	bool recoilFacingRight_ = true;

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0;

	// Generic defaults (used by GetCombatOffset/GetHolsterOffset/GetReloadOffset unless overridden)
	SRTOffset offset_ = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };
	SRTOffset holsterOffset_ = {
	    {1.0f,   1.0f,  1.0f},
        {0.0f,   0.0f,  1.2f},
        {-0.15f, 0.25f, 0.0f}
    };
	SRTOffset reloadOffset_ = {
	    {1.0f,  1.0f,  1.0f },
        {0.0f,  0.0f,  0.35f},
        {-0.1f, 0.35f, 0.0f }
    };

	virtual bool TryFire() { return true; }

	// --- SFX (shared, static since these are class-wide default sounds) ---
	static inline uint32_t sFireSoundHandle_ = 0;
	static inline uint32_t sReloadSoundHandle_ = 0;
	static inline float sSfxVolume_ = 1.0f;
	static void PlaySfxHandle(uint32_t handle) {
		if (handle != 0) {
			KamataEngine::Audio::GetInstance()->PlayWave(handle, false, sSfxVolume_);
		}
	}
};