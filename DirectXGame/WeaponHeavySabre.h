#pragma once
#include "BaseMelee.h"

class WeaponHeavySabre final : public BaseMelee {
public:
	void SetWorldPosition(KamataEngine::Vector3& TargetPosition) override;

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	AABB GetAABB() const override;
	AABB GetSwingAABBGeneric(const KamataEngine::Vector3& ownerPosition, bool facingRight, int comboIndex) const;

	void SetWeaponState(WeaponState state);
	void UpdateWeaponState();

	void SetMeleePositionGeneric(const KamataEngine::Vector3& ownerPosition, const KamataEngine::Vector3& ownerRotation, bool facingRight, const GenericAttackAnimState* attackState);
	void SetLungePositionGeneric(const KamataEngine::Vector3& ownerPosition, const KamataEngine::Vector3& ownerRotation, bool facingRight, AttackPhase lungePhase, float t);
	AABB GetLungeAABBGeneric(const KamataEngine::Vector3& ownerPosition, bool facingRight) const;
	
	int GetLungeDamage() const { return kLungeDamage; }
	float GetLungeImpactPower() const { return kLungeImpactPower; }

	// --- SFX ---
	// Cuirassier固有の振りSFX(通常コンボ・ランジ共通)。未設定ならBaseMeleeのデフォルトを使う。
	static void SetSwingSoundHandle(uint32_t handle) { sSwingSoundHandle_ = handle; }
	// ランジ突撃専用の踏み込みSFX。呼び出し側(CuirassierBehavior)が明示的に鳴らす。
	static void SetLungeSoundHandle(uint32_t handle) { sLungeSoundHandle_ = handle; }
	static void PlayLungeSound() {
		if (sLungeSoundHandle_ != 0) {
			KamataEngine::Audio::GetInstance()->PlayWave(sLungeSoundHandle_, false, sSfxVolume_);
		}
	}

	void SnapWeaponState(WeaponState state) {
		weaponState_ = state;
		equipTimer_ = 1.0f;
		isTransitioning_ = false;
	}
	void SetEquipDuration(float seconds);
	float GetEquipDuration() const { return equipDuration_; }

private:
	friend class DebugMenu;
	WeaponState weaponState_ = WeaponState::kEquipped;
	float equipTimer_ = 1.0f;
	float equipDuration_ = 0.3f;
	bool isTransitioning_ = false;

	static inline SRTOffset holsterOffset_{
	    {1.0f, 1.0f,  1.0f },
        {0.0f, 3.0f,  0.6f },
        {0.2f, -0.1f, 0.45f}
    };
	static inline SRTOffset readyStanceOffset_{
	    {1.0f,  1.0f, 1.0f },
        {-0.5f, 0.0f, 0.0f },
        {0.45f, 0.0f, 0.45f}
    };

	static inline std::array<ComboAttackPattern, 3> comboPatterns_ = {
	    ComboAttackPattern{
	                       {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.4f}, {0.6f, -0.1f, 0.8f}},
	                       {{1.0f, 1.0f, 1.0f}, {-0.2f, -0.8f, 1.6f}, {0.4f, 0.2f, 0.5f}},
	                       {{1.0f, 1.0f, 1.0f}, {2.0f, -1.0f, 1.2f}, {0.4f, -0.2f, -0.5f}},
	                       {{1.0f, 1.0f, 1.0f}, {2.0f, -1.0f, 1.2f}, {0.4f, -0.2f, -0.5f}},
	                       },
	    ComboAttackPattern{
	                       {{1.0f, 1.0f, 1.0f}, {2.0f, -1.0f, 1.2f}, {0.4f, -0.2f, -0.5f}},
	                       {{1.0f, 1.0f, 1.0f}, {1.1f, -1.8f, 3.8f}, {0.25f, 0.35f, -0.3f}},
	                       {{1.0f, 1.0f, 1.0f}, {4.8f, -1.0f, 3.9f}, {0.25f, -0.25f, 0.5f}},
	                       {{1.0f, 1.0f, 1.0f}, {4.8f, -1.0f, 3.9f}, {0.25f, -0.25f, 0.5f}},
	                       },
	    ComboAttackPattern{
	                       {{1.0f, 1.0f, 1.0f}, {4.8f, -1.0f, 3.9f}, {0.25f, -0.25f, 0.5f}},
	                       {{1.0f, 1.0f, 1.0f}, {4.2f, -1.3f, 0.7f}, {-0.5f, 0.5f, 0.6f}},
	                       {{1.0f, 1.0f, 1.0f}, {5.4f, -0.1f, -0.4f}, {0.8f, -0.15f, 0.0f}},
	                       {{1.0f, 1.0f, 1.0f}, {5.4f, -0.1f, -0.4f}, {0.8f, -0.15f, 0.0f}},
	                       },
	};

	static inline SRTOffset kLungeAnticipate{
	    {1.0f, 1.0f,   1.0f},
        {0.0f, -0.3f,  0.6f},
        {0.3f, -0.05f, 0.5f}
    };
	static inline SRTOffset kLungeExtend{
	    {1.0f, 1.0f,  1.0f },
        {0.0f, -0.1f, 1.5f },
        {0.3f, -0.1f, -0.3f}
    };

	static inline std::array<float, 3> kComboReach = {2.0f, 2.0f, 2.4f};
	static inline std::array<float, 3> kComboSwingHeight = {1.8f, 1.8f, 2.0f};

	static inline float kLungeReach = 4.5f;

	// [NEW]
	static inline int kLungeDamage = 25;
	static inline float kLungeImpactPower = 2.2f;

	static inline float kHalfWidthMin = 0.9f;
	static inline float kHalfHeightMin = 0.9f;
	static inline float kHalfWidthMax = 0.9f;
	static inline float kHalfHeightMax = 0.9f;
	static inline float kHalfDepthMin = 0.9f;
	static inline float kHalfDepthMax = 0.9f;

	// --- SFX ---
	static inline uint32_t sSwingSoundHandle_ = 0;
	static inline uint32_t sLungeSoundHandle_ = 0;

	// BaseMelee::PlayDefaultSwingSound()をoverrideして、専用ハンドルがあればそちらを鳴らす
	void PlayDefaultSwingSound() override {
		uint32_t handle = (sSwingSoundHandle_ != 0) ? sSwingSoundHandle_ : sDefaultSwingSoundHandle_;
		if (handle != 0) {
			KamataEngine::Audio::GetInstance()->PlayWave(handle, false, sSfxVolume_);
		}
	}

public:
	static float GetLungeReach() { return kLungeReach; }
};