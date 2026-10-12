#pragma once

#include "Global.h"
#include <KamataEngine.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

/// <summary>
/// Firearm base.
/// - ANIMATION offsets (combat / holster / reload / knockback / stun / block) are virtual getters each firearm overrides.
/// - AMMO / RELOAD logic is implemented here once (it was copy-pasted in FirearmMusket and FirearmMusketBayonet).
///   Derived classes only supply the tuning numbers through the pure-virtual stat getters.
/// - offset_ is an EXTRA offset layered on top of the combat pose (Player / EnemyRifleman call SetOffset every frame).
/// </summary>
class BaseFirearm {
public:
	virtual ~BaseFirearm() = default;

	virtual void Initialize(KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera);
	virtual void UpdateFollow(const KamataEngine::WorldTransform& parentTransform, float localAimAngle, bool facingRight);
	virtual void Draw();

	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }
	const SRTOffset& GetOffset() const { return offset_; }

	// Extra offset on top of the combat pose (identity = no change).
	void SetOffset(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rotation, const KamataEngine::Vector3& translation) {
		offset_.scale = scale;
		offset_.rotation = rotation;
		offset_.translation = translation;
	}

	// 持ち主(プレイヤー/敵)。リロード時間などの値の切り替えに使う。Initializeより前に設定すること
	void SetOwner(Owner owner) { owner_ = owner; }
	Owner GetOwner() const { return owner_; }

	// ============================================================
	// Animation offset polymorphism. Override per firearm.
	// ============================================================
	virtual const SRTOffset& GetCombatOffset() const { return kIdentityOffset; }
	virtual const SRTOffset& GetHolsterOffset() const { return holsterOffset_; }
	virtual const SRTOffset& GetReloadOffset() const { return reloadOffset_; }
	virtual const SRTOffset& GetKnockbackOffset() const { return kDefaultKnockbackOffset; }
	virtual const SRTOffset& GetStunOffset() const { return kDefaultStunOffset; }
	virtual const SRTOffset& GetBlockOffset() const { return kDefaultBlockOffset; }
	// Extra yaw the model needs so that "rotation.y == 0" faces forward (musket models need +pi/2).
	virtual float GetModelYawOffset() const { return 0.0f; }

	static inline SRTOffset kIdentityOffset = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };
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
	// guard pose: gun held across the body (authored for a right-facing entity)
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
	// 持ち主に応じた1発のダメージ(プレイヤーが撃つ=対敵、敵が撃つ=対プレイヤー)
	int GetBulletDamage() const { return IsPlayerOwned() ? GetDamageToEnemy() : GetDamageToPlayer(); }

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	// --- SFX ---
	static void SetFireSoundHandle(uint32_t handle) { sFireSoundHandle_ = handle; }
	// One shared "reload finished" sound for every musket (the bayonet musket's own handle was never set before).
	static void SetLoadedSoundHandle(uint32_t handle) { sLoadedSoundHandle_ = handle; }
	static void SetSfxVolume(float volume) { sSfxVolume_ = volume; }
	static void PlayFireSound() { PlaySfxHandle(sFireSoundHandle_); }

	void SnapEquipped(bool equipped) {
		equipTarget_ = equipped ? 1.0f : 0.0f;
		equipFactor_ = equipTarget_;
	}

	// ============================================================
	// Shared ammo / reload (single implementation)
	// ============================================================
	virtual bool TryFire();
	bool CanFire() const { return currentAmmo_ > 0; }
	virtual bool IsReloading() const { return isReloading_; }
	virtual float GetReloadProgress01() const { return (isReloading_ && reloadDurationCurrent_ > 0.0f) ? std::clamp(reloadTimer_ / reloadDurationCurrent_, 0.0f, 1.0f) : 1.0f; }
	virtual int GetCurrentAmmo() const { return currentAmmo_; }
	// Hitting enemies shortens the NEXT reload.
	virtual void NotifyDealtDamage();

	// Drop state: kReady = loaded, kOnCooldown = unloaded.
	virtual DropItemState GetDropState() const { return (currentAmmo_ < GetMaxAmmo()) ? DropItemState::kOnCooldown : DropItemState::kReady; }
	// Called right after a picked-up gun is constructed. kOnCooldown restarts the reload from zero.
	virtual void RestoreDropState(DropItemState state);

		// ---- 発射パターン(ショットガン等のために仮想化。既定は単発) ----
	virtual int GetPelletCount() const { return 1; }                   // 1回の発射で出る弾数
	virtual float GetPelletAngle(int /*index*/) const { return 0.0f; } // 各弾の角度ずれ(rad)
	virtual int32_t GetProjectileLifeFrames() const { return 0; }      // 0=弾の既定寿命

		// ---- 銃口(銃モデルのローカル座標。バレルの向きはローカル+Z) ----
	virtual const KamataEngine::Vector3& GetMuzzleLocalOffset() const { return kMuzzleOffset; }
	// 銃口のワールド座標(UpdateFollow 後の最新の姿勢から求める)
	KamataEngine::Vector3 GetMuzzleWorldPosition() const { return TransformCoordinate(GetMuzzleLocalOffset(), worldTransform_.matWorld_); }
	static inline KamataEngine::Vector3 kMuzzleOffset = {0.0f, 0.0f, 1.2f};

protected:
	static inline float kImpactPower = 1.2f;
	static inline float kEquipLerpSpeed = 0.15f;

	// ---- tuning supplied by each firearm ----
	virtual int GetMaxAmmo() const = 0;
	virtual float GetBaseReloadDuration() const = 0;
	virtual float GetReloadHitBonus() const = 0;
	virtual float GetReloadDurationMin() const = 0;

	// Advances reload (called at the top of UpdateFollow).
	virtual void TickReload();

	int currentAmmo_ = 1;
	bool isReloading_ = false;
	float reloadTimer_ = 0.0f;
	float reloadDurationCurrent_ = 1.0f;

	// ---- 銃ごとのダメージ(各銃で必ず定義する) ----
	virtual int GetDamageToEnemy() const = 0;  // プレイヤーが撃ったときの敵へのダメージ
	virtual int GetDamageToPlayer() const = 0; // 敵が撃ったときのプレイヤーへのダメージ

	// Blends the combat pose toward knockback/stun/block. Ticks the blend once per call (once per UpdateFollow).
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
		return LerpSRT(base, *debuffPtr, reactionBlend_);
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

	// Extra offset layered on the combat pose (identity by default).
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

	bool IsPlayerOwned() const { return owner_ == Owner::kPlayer; }
	Owner owner_ = Owner::kPlayer;

	// --- SFX (class-wide default sounds) ---
	static inline uint32_t sFireSoundHandle_ = 0;
	static inline uint32_t sLoadedSoundHandle_ = 0;
	static inline float sSfxVolume_ = 1.0f;
	static void PlaySfxHandle(uint32_t handle) {
		if (handle != 0) {
			AudioSettings::PlaySfx(handle, false, sSfxVolume_);
		}
	}
};
