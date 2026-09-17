#pragma once

#include "BaseFirearm.h"

class FirearmMusketBayonet final : public BaseFirearm {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
	FirearmMusketBayonet() { offset_ = musketOffset_; }
	void UpdateFollow(const KamataEngine::WorldTransform& parentTransform, float localAimAngle, bool facingRight) override;

	bool CanFire() const { return currentAmmo_ > 0 && !isReloading_ == false ? currentAmmo_ > 0 : false; }

	bool TryFire();
	void NotifyDealtDamage();
	bool IsReloading() const { return isReloading_; }
	int GetCurrentAmmo() const { return currentAmmo_; }
	float GetReloadProgress01() const { return isReloading_ ? std::clamp(reloadTimer_ / reloadDurationCurrent_, 0.0f, 1.0f) : 1.0f; }

	int GetLungeDamage() const { return kLungeDamage; }
	float GetLungeImpactPower() const { return kLungeImpactPower; }

	static void SetMusketLoadedSoundHandle(uint32_t handle) { sMusketLoadedSoundHandle_ = handle; }

private:
	int currentAmmo_ = 1;
	static inline int kMaxAmmo = 1;

	bool isReloading_ = false;
	float reloadTimer_ = 0.0f;

	static inline float kReloadDuration = 1.5f;
	static inline float kReloadHitBonus = 0.15f;
	static inline float kReloadDurationMin = 0.3f;
	float reloadDurationCurrent_ = kReloadDuration;

	static inline int kLungeDamage = 10;
	static inline float kLungeImpactPower = 1.8f;

	static inline SRTOffset musketOffset_ = {
	    {1.0f, 1.0f,        1.0f },
        {0.0f, -1.5707963f, 0.0f },
        {0.6f, 0.2f,        -0.1f}
    };
	static inline SRTOffset musketHolsterOffset_ = {
	    {1.0f,    1.0f,  1.0f },
        {-1.371f, 0.0f,  0.0f },
        {-0.4f,   -0.3f, -0.3f}
    };
	static inline SRTOffset musketReloadOffset_ = {
	    {1.0f,   1.0f,  1.0f },
        {-0.9f,  0.0f,  0.0f },
        {-0.25f, -0.1f, -0.2f}
    };

	static inline uint32_t sMusketLoadedSoundHandle_ = 0;
};