#pragma once

#include "Global.h"
#include <KamataEngine.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

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
	// GameSceneがロード後にセットする(未設定=0ならAudio呼び出しをスキップする)
	static void SetFireSoundHandle(uint32_t handle) { sFireSoundHandle_ = handle; }
	static void SetReloadSoundHandle(uint32_t handle) { sReloadSoundHandle_ = handle; }
	static void SetSfxVolume(float volume) { sSfxVolume_ = volume; }
	static void PlayFireSound() { PlaySfxHandle(sFireSoundHandle_); }
	static void PlayReloadSound() { PlaySfxHandle(sReloadSoundHandle_); }

protected:
	static inline float kImpactPower = 1.2f;

	static inline float kEquipLerpSpeed = 0.15f;

	float equipFactor_ = 1.0f;
	float equipTarget_ = 1.0f;
	bool isShowingReloadPose_ = false;

	uint32_t recoilTimer_ = 0;
	bool recoilFacingRight_ = true;

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0;

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