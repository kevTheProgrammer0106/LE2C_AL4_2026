#pragma once

#include "BaseFirearm.h"

class FirearmMusketBayonet final : public BaseFirearm {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	FirearmMusketBayonet() { offset_ = musketOffset_; }

	// ------------------------------------------------------------------
	// Bayonet lunge animation. Same structure as WeaponSabre's combo poses:
	//   anticipate : ready pose -> wind-up pose   (linear, t = frame / anticipateDuration)
	//   attack     : wind-up    -> thrust pose
	//   recovery   : thrust     -> ready pose
	// RiflemanBehavior calls SetLungePose() every frame of the lunge.
	// ------------------------------------------------------------------
	enum class LungePhase { kAnticipate, kAttack, kRecovery };
	void SetLungePose(LungePhase phase, float t);
	void ClearLungePose() { lungePoseActive_ = false; }
	bool IsLungePoseActive() const { return lungePoseActive_; }

	// --- animation offsets (own values; independent from FirearmMusket) ---
	const SRTOffset& GetCombatOffset() const override { return lungePoseActive_ ? lungeBlendedOffset_ : musketOffset_; }
	const SRTOffset& GetHolsterOffset() const override { return musketHolsterOffset_; }
	const SRTOffset& GetReloadOffset() const override { return musketReloadOffset_; }
	const SRTOffset& GetKnockbackOffset() const override { return musketKnockbackOffset_; }
	const SRTOffset& GetStunOffset() const override { return musketStunOffset_; }
	float GetModelYawOffset() const override { return std::numbers::pi_v<float> / 2.0f; }

	bool CanFire() const { return currentAmmo_ > 0 && !isReloading_; }

	bool TryFire() override;
	void NotifyDealtDamage();

	bool IsReloading() const { return isReloading_; }
	int GetCurrentAmmo() const { return currentAmmo_; }

	float GetReloadProgress01() const { return isReloading_ ? std::clamp(reloadTimer_ / reloadDurationCurrent_, 0.0f, 1.0f) : 1.0f; }

	int GetLungeDamage() const { return kLungeDamage; }
	float GetLungeImpactPower() const { return kLungeImpactPower; }

	// Bayonet lunge timings (frames @60fps) and advance speed.
	float GetBayonetAnticipateDuration() const { return kBayonetAnticipateDuration; }
	float GetBayonetAttackDuration() const { return kBayonetAttackDuration; }
	float GetBayonetRecoveryDuration() const { return kBayonetRecoveryDuration; }
	float GetBayonetAttackSpeed() const { return kBayonetAttackSpeed; }
	float GetBayonetReach() const { return kBayonetReach; }
	float GetBayonetSwingHeight() const { return kBayonetSwingHeight; }

	static void SetMusketLoadedSoundHandle(uint32_t handle) { sMusketLoadedSoundHandle_ = handle; }

protected:
	void TickReload() override;

private:
	friend class DebugMenu;
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

	// --- lunge pose state ---
	bool lungePoseActive_ = false;
	SRTOffset lungeBlendedOffset_{};

	// --- animation offsets ---
	static inline SRTOffset musketOffset_ = {
	    {1.0f, 1.0f,        1.0f },
        {0.0f, -1.5707963f, 0.0f },
        {0.6f, 0.2f,        -0.1f}
    };

	// Lunge poses. Axes of the firearm offset space (right-facing owner):
	//   translation.z = forward along the thrust, translation.x = toward the camera side, translation.y = up
	//   rotation.x    = barrel pitch (negative = barrel up). rotation.y MUST stay -pi/2 (cancels the model yaw offset).
	// The "ready" pose at both ends of the lunge is the live musketOffset_, so GlobalVariables tweaks never cause a pop.
	struct BayonetLungePoses {
		SRTOffset windUp;
		SRTOffset thrust;
	};
	static inline BayonetLungePoses kLungePoses = {
	    // windUp: musket drawn back to the hip, barrel slightly raised
	    {{1.0f, 1.0f, 1.0f}, {-0.3f, -1.5707963f, 0.0f}, {0.45f, 0.05f, -0.5f}},
	    // thrust: musket driven forward and level
	    {{1.0f, 1.0f, 1.0f}, {0.0f, -1.5707963f, 0.0f},  {0.5f, 0.15f, 1.0f}  },
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
	static inline SRTOffset musketKnockbackOffset_ = {
	    {1.0f, 1.0f,        1.0f },
        {0.3f, -1.5707963f, 0.9f },
        {0.4f, 0.35f,       -0.1f}
    };
	static inline SRTOffset musketStunOffset_ = {
	    {1.0f,  1.0f,        1.0f },
        {0.0f,  -1.5707963f, -0.9f},
        {0.55f, -0.1f,       -0.1f}
    };

	static inline uint32_t sMusketLoadedSoundHandle_ = 0;

	// Lunge timings: same defaults as BaseMelee combo 0 (anticipate 10 / attack 8 / recovery 15 frames, speed 0.25).
	static inline float kBayonetAnticipateDuration = 10.0f;
	static inline float kBayonetAttackDuration = 8.0f;
	static inline float kBayonetRecoveryDuration = 15.0f;
	static inline float kBayonetAttackSpeed = 0.25f;

	static inline float kBayonetReach = 2.0f;
	static inline float kBayonetSwingHeight = 1.8f;
};