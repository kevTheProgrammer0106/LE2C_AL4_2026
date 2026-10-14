#pragma once

#include "BaseFirearm.h"
#include <vector>

/// <summary>
/// Dual pistols (player weapon). Two pistols, each with its own loaded state.
/// - Max ammo 2 (one per pistol). Firing empties one pistol and queues it for reload.
/// - Reloads are SEQUENTIAL: the whole set takes 2 x the reload time. After the first pistol
///   is reloaded you can fire once, then it cycles (fire / reload / fire ...).
/// - Damage = musket - 10, reload = musket - 1s (see the k... values below, tunable in GlobalVariables).
/// - Both pistols are drawn (second one is the first one shifted by kSecondDelta). Unloaded pistols are darkened.
/// - "Hold to fire both" is handled by Player::UseFirearm (needs IsFullyLoaded()).
/// </summary>
class FirearmPistols final : public BaseFirearm {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera) override;
	void UpdateFollow(const KamataEngine::WorldTransform& parentTransform, float localAimAngle, bool facingRight) override;
	void Draw() override;

	bool TryFire() override;
	void RestoreDropState(DropItemState state) override;
	// Overall progress of the whole set (both pistols), 0..1
	float GetReloadProgress01() const override;

	// Both pistols loaded (the "hold to fire both" condition)
	bool IsFullyLoaded() const { return currentAmmo_ >= GetMaxAmmo(); }

	// Muzzle of the pistol that fired last (call right after TryFire)
	KamataEngine::Vector3 GetFiredMuzzleWorldPosition() const override;

	// --- animation offsets (own values, applied to the first pistol; the second follows via kSecondDelta) ---
	const SRTOffset& GetCombatOffset() const override { return pistolOffset_; }
	const SRTOffset& GetHolsterOffset() const override { return pistolHolsterOffset_; }
	const SRTOffset& GetReloadOffset() const override { return pistolReloadOffset_; }
	const SRTOffset& GetKnockbackOffset() const override { return pistolKnockbackOffset_; }
	const SRTOffset& GetStunOffset() const override { return pistolStunOffset_; }
	const SRTOffset& GetBlockOffset() const override { return pistolBlockOffset_; }
	float GetModelYawOffset() const override { return std::numbers::pi_v<float> / 2.0f; }

protected:
	int GetMaxAmmo() const override { return kMaxAmmo; }
	float GetBaseReloadDuration() const override { return IsPlayerOwned() ? kReloadDurationPlayer : kReloadDurationEnemy; }
	float GetReloadHitBonus() const override { return IsPlayerOwned() ? kReloadHitBonusPlayer : kReloadHitBonusEnemy; }
	float GetReloadDurationMin() const override { return IsPlayerOwned() ? kReloadDurationMinPlayer : kReloadDurationMinEnemy; }
	int GetDamageToEnemy() const override { return kDamageToEnemy; }
	int GetDamageToPlayer() const override { return kDamageToPlayer; }

	// Sequential reload: one pistol per reload duration
	void TickReload() override;

private:
	friend class DebugMenu;

	void UpdateColors();

	// per-pistol state (index 0 / 1)
	bool loaded_[2] = {true, true};
	std::vector<int> reloadQueue_; // fired order = reload order
	int nextSlot_ = 0;             // pistol preferred for the next shot (alternates)
	int lastFiredSlot_ = 0;

	KamataEngine::WorldTransform worldTransformSecond_;
	KamataEngine::ObjectColor colorSlot_[2];

	static inline int kMaxAmmo = 2;
	// Musket is 3.0s / 160 dmg / 30 dmg-to-player: this is 1s faster per pistol and 10 dmg lower.
	static inline float kReloadDurationPlayer = 2.0f; // per pistol -> 4.0s for both
	static inline float kReloadHitBonusPlayer = 0.15f;
	static inline float kReloadDurationMinPlayer = 0.3f;
	static inline float kReloadDurationEnemy = 2.0f;
	static inline float kReloadHitBonusEnemy = 0.15f;
	static inline float kReloadDurationMinEnemy = 0.3f;
	static inline int kDamageToEnemy = 150;
	static inline int kDamageToPlayer = 20;

	// second pistol = first pistol + this delta (parent-local space, scaled by equip factor so stowed pistols overlap)
	static inline KamataEngine::Vector3 kSecondDelta = {-1.1f, 0.0f, 0.0f};
	static inline KamataEngine::Vector4 kUnloadedColor = {0.35f, 0.35f, 0.35f, 1.0f};

	static inline SRTOffset pistolOffset_ = {
	    {0.7f, 0.7f,        0.7f },
        {0.0f, -1.5707963f, 0.0f },
        {0.6f, 0.2f,        -0.1f}
    };
	static inline SRTOffset pistolHolsterOffset_ = {
	    {0.7f,    0.7f,  0.7f },
        {-1.371f, 0.0f,  0.0f },
        {-0.4f,   -0.3f, -0.3f}
    };
	static inline SRTOffset pistolReloadOffset_ = {
	    {0.7f,   0.7f,  0.7f },
        {-0.9f,  0.0f,  0.0f },
        {-0.25f, -0.1f, -0.2f}
    };
	static inline SRTOffset pistolKnockbackOffset_ = {
	    {0.7f, 0.7f,        0.7f },
        {0.3f, -1.5707963f, 0.9f },
        {0.4f, 0.35f,       -0.1f}
    };
	static inline SRTOffset pistolStunOffset_ = {
	    {0.7f,  0.7f,        0.7f },
        {0.0f,  -1.5707963f, -0.9f},
        {0.55f, -0.1f,       -0.1f}
    };
	static inline SRTOffset pistolBlockOffset_ = {
	    {0.7f,  0.7f,        0.7f },
        {0.0f,  -1.5707963f, 0.7f },
        {0.45f, 0.25f,       -0.1f}
    };
};