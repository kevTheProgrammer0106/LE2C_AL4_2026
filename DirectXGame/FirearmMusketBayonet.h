#pragma once

#include "BaseFirearm.h"

class FirearmMusketBayonet final : public BaseFirearm {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	// ------------------------------------------------------------------
	// Bayonet lunge animation. Same structure as the sabre combo poses:
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
	// FIX: was missing, so blocking with this musket used the default pose (rotation.y = 0) and the gun turned sideways.
	const SRTOffset& GetBlockOffset() const override { return musketBlockOffset_; }
	float GetModelYawOffset() const override { return std::numbers::pi_v<float> / 2.0f; }

	int GetLungeDamage() const { return kLungeDamage; }
	float GetLungeImpactPower() const { return kLungeImpactPower; }

	// Bayonet lunge timings (frames @60fps) and advance speed.
	float GetBayonetAnticipateDuration() const { return kBayonetAnticipateDuration; }
	float GetBayonetAttackDuration() const { return kBayonetAttackDuration; }
	float GetBayonetRecoveryDuration() const { return kBayonetRecoveryDuration; }
	float GetBayonetAttackSpeed() const { return kBayonetAttackSpeed; }
	float GetBayonetReach() const { return kBayonetReach; }
	float GetBayonetSwingHeight() const { return kBayonetSwingHeight; }

	// Kept for compatibility; forwards to the shared handle (GameScene only ever set FirearmMusket's, so this one stayed silent).
	static void SetMusketLoadedSoundHandle(uint32_t handle) { SetLoadedSoundHandle(handle); }

	LungePhase GetLungePhase() const { return lungePhase_; }
	float GetLungeT() const { return lungeT_; }

		// ---------------- Player lunge (click while reloading). Separate values from the rifleman's ----------------
	LungeSpec GetPlayerLungeSpec() const {
		return {kPlayerBayonetAnticipate, kPlayerBayonetAttack, kPlayerBayonetRecovery, kPlayerBayonetSpeed, kPlayerBayonetDamage, kPlayerBayonetImpact, kPlayerBayonetCooldown};
	}
	void SetPlayerLungePose(LungePhase phase, float t);
	AABB GetPlayerLungeAABB(const AABB& playerAABB, bool facingRight) const;

	static void SetLungeSoundHandle(uint32_t handle) { sLungeSoundHandle_ = handle; }
	static void PlayLungeSound() { PlaySfxHandle(sLungeSoundHandle_); }

protected:
	int GetMaxAmmo() const override { return kMaxAmmo; }
	float GetBaseReloadDuration() const override { return IsPlayerOwned() ? kReloadDurationPlayer : kReloadDurationEnemy; }
	float GetReloadHitBonus() const override { return IsPlayerOwned() ? kReloadHitBonusPlayer : kReloadHitBonusEnemy; }
	float GetReloadDurationMin() const override { return IsPlayerOwned() ? kReloadDurationMinPlayer : kReloadDurationMinEnemy; }
	int GetDamageToEnemy() const override { return kDamageToEnemy; }
	int GetDamageToPlayer() const override { return kDamageToPlayer; }
private:
	friend class DebugMenu;

	static inline int kMaxAmmo = 1;
	static inline float kReloadDurationPlayer = 1.5f;
	static inline float kReloadHitBonusPlayer = 0.15f;
	static inline float kReloadDurationMinPlayer = 0.3f;
	static inline float kReloadDurationEnemy = 1.5f;
	static inline float kReloadHitBonusEnemy = 0.15f;
	static inline float kReloadDurationMinEnemy = 0.3f;
	static inline int kDamageToEnemy = 160; // プレイヤーが撃ったとき敵に与える
	static inline int kDamageToPlayer = 30; // 敵が撃ったときプレイヤーに与える

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
	static inline SRTOffset musketBlockOffset_ = {
	    {1.0f,  1.0f,        1.0f },
        {0.0f,  -1.5707963f, 0.7f },
        {0.45f, 0.25f,       -0.1f}
    };

	LungePhase lungePhase_ = LungePhase::kAnticipate;
	float lungeT_ = 0.0f;

	// Lunge timings: same defaults as BaseMelee combo 0 (anticipate 10 / attack 8 / recovery 15 frames, speed 0.25).
	static inline float kBayonetAnticipateDuration = 10.0f;
	static inline float kBayonetAttackDuration = 8.0f;
	static inline float kBayonetRecoveryDuration = 15.0f;
	static inline float kBayonetAttackSpeed = 0.25f;

	static inline float kBayonetReach = 2.0f;
	static inline float kBayonetSwingHeight = 1.8f;

	private:
		// プレイヤを使用
	    static inline BayonetLungePoses kPlayerLungePoses = {
	        {{1.0f, 1.0f, 1.0f}, {-0.3f, -1.5707963f, 0.0f}, {0.45f, 0.05f, -0.5f}},
	        {{1.0f, 1.0f, 1.0f}, {0.0f, -1.5707963f, 0.0f},  {0.5f, 0.15f, 1.0f}  },
	    };
	    static inline float kPlayerBayonetAnticipate = 10.0f; // frames
	    static inline float kPlayerBayonetAttack = 8.0f;
	    static inline float kPlayerBayonetRecovery = 15.0f;
	    static inline float kPlayerBayonetSpeed = 0.3f;
	    static inline int kPlayerBayonetDamage = 70;
	    static inline float kPlayerBayonetImpact = 1.8f;
	    static inline float kPlayerBayonetCooldown = 3.0f;
	    static inline float kPlayerBayonetReach = 2.2f;
	    static inline float kPlayerBayonetHeight = 1.8f;
	    static inline uint32_t sLungeSoundHandle_ = 0;
};
