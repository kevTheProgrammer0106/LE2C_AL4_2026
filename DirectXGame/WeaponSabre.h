#pragma once

#include "BaseMelee.h"

class Player;

class WeaponSabre final : public BaseMelee {
public:
	void SetWorldPosition(KamataEngine::Vector3& TargetPosition) override;
	void SetMeleePosition(const Player& player);

	// Player以外(敵)がこの武器を装備している場合の位置更新。
	// ownerPosition/ownerRotation/facingRightは呼び出し側(EnemyFighter/EnemyCuirassier)が
	// 自分のworldTransform_から渡す。attackState==nullptrなら非攻撃時のホルスター/構えのみ。
	void SetMeleePositionGeneric(const KamataEngine::Vector3& ownerPosition, const KamataEngine::Vector3& ownerRotation, bool facingRight, const GenericAttackAnimState* attackState);

	// 調整項目を登録
	static void RegisterGlobalVariables();
	// 調整項目を適用
	static void ApplyGlobalVariables();
	AABB GetAABB() const override;
	AABB GetSwingAABB(const Player& player, int comboIndex) const;

	// [NEW] 敵の攻撃判定用。中心座標・向き・コンボ段を明示的に渡す汎用版。
	AABB GetSwingAABBGeneric(const KamataEngine::Vector3& ownerPosition, bool facingRight, int comboIndex) const;

	void SetWeaponState(WeaponState state);
	void UpdateWeaponState();

	void SnapWeaponState(WeaponState state) {
		weaponState_ = state;
		equipTimer_ = 1.0f;
		isTransitioning_ = false;
	}

	const SRTOffset& GetHolsterOffset() const override { return holsterOffset_; }
	const SRTOffset& GetReadyStanceOffset() const override { return readyStanceOffset_; }
	const SRTOffset& GetKnockbackOffset() const override { return kKnockbackOffset; }
	const SRTOffset& GetStunOffset() const override { return kStunOffset; }
	const SRTOffset& GetBlockOffset() const override { return kBlockOffset; }

private:
	friend class DebugMenu;
	WeaponState weaponState_ = WeaponState::kEquipped;
	float equipTimer_ = 1.0f;
	float equipDuration_ = 0.25f;
	bool isTransitioning_ = false;

	static inline std::array<ComboAttackPattern, 3> comboPatterns_ = {
	    ComboAttackPattern{
	                       /*anticipateStart*/ {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.5f}, {0.650f, -0.1f, 0.90f}},
	                       /*anticipateEnd*/ {{1.0f, 1.0f, 1.0f}, {-0.2f, -1.0f, 1.9f}, {0.350f, 0.3f, 0.6f}},
	                       /*attackEnd*/ {{1.0f, 1.0f, 1.0f}, {2.4f, -1.1f, 1.4f}, {0.350f, -0.2f, -0.6f}},
	                       /*recoveryEnd*/ {{1.0f, 1.0f, 1.0f}, {2.4f, -1.1f, 1.4f}, {0.350f, -0.2f, -0.6f}},
	                       },
	    ComboAttackPattern{
	                       {{1.0f, 1.0f, 1.0f}, {2.4f, -1.1f, 1.4f}, {0.350f, -0.2f, -0.6f}},
	                       {{1.0f, 1.0f, 1.0f}, {1.3f, -2.1f, 4.5f}, {0.2f, 0.4f, -0.35f}},
	                       {{1.0f, 1.0f, 1.0f}, {5.6f, -1.1f, 4.6f}, {0.2f, -0.3f, 0.6f}},
	                       {{1.0f, 1.0f, 1.0f}, {5.6f, -1.1f, 4.6f}, {0.2f, -0.3f, 0.6f}},
	                       },
	    ComboAttackPattern{
	                       {{1.0f, 1.0f, 1.0f}, {5.6f, -1.1f, 4.6f}, {0.2f, -0.3f, 0.6f}},
	                       {{1.0f, 1.0f, 1.0f}, {4.8f, -1.5f, 0.8f}, {-0.6f, 0.6f, 0.7f}},
	                       {{1.0f, 1.0f, 1.0f}, {6.2f, -0.1f, -0.5f}, {0.9f, -0.2f, 0.0f}},
	                       {{1.0f, 1.0f, 1.0f}, {6.2f, -0.1f, -0.5f}, {0.9f, -0.2f, 0.0f}},
	                       },
	};

	static inline SRTOffset holsterOffset_{
	    {1.0f, 1.0f, 1.0f},
	    {
         0.0f, 3.140f,
         0.5f, },
	    {
         0.250f, -0.1f,
         0.490f, }
    };
	static inline SRTOffset readyStanceOffset_{
	    {1.0f,  1.0f, 1.0f  },
        {-0.6f, 0.0f, 0.0f  },
        {0.5f,  0.0f, 0.490f}
    };
	// private
	static inline SRTOffset kKnockbackOffset{
	    {1.0f,  1.0f, 1.0f },
        {-1.0f, 0.0f, 0.4f },
        {0.3f,  0.2f, 0.49f}
    };
	static inline SRTOffset kStunOffset{
	    {1.0f,  1.0f,   1.0f },
        {0.9f,  0.0f,   0.2f },
        {0.45f, -0.35f, 0.49f}
    };
	static inline SRTOffset kBlockOffset{
	    {1.0f,  1.0f,  1.0f },
        {-0.6f, 0.0f,  1.2f },
        {0.55f, 0.15f, 0.49f}
    };

	static inline std::array<float, 3> kComboReach = {1.7f, 1.7f, 2.0f};
	static inline std::array<float, 3> kComboSwingHeight = {1.6f, 1.6f, 1.8f};

	static inline float kReach = 1.2f;
	static inline float kSwingHeight = 1.6f;

	static inline std::array<float, 3> kUpDownReach = {1.0f, 1.2f, 1.5f};
	static inline std::array<float, 3> kUpDownWidth = {1.0f, 1.1f, 1.3f};

	static inline SRTOffset kUpAttackOffset{
	    {1.0f,  1.0f, 1.0f},
        {-1.2f, 0.0f, 0.0f},
        {0.0f,  0.9f, 0.0f}
    };
	static inline SRTOffset kDownAttackOffset{
	    {1.0f, 1.0f,  1.0f},
        {1.2f, 0.0f,  0.0f},
        {0.0f, -0.9f, 0.0f}
    };

	static inline float kHalfWidthMin = 0.8f;
	static inline float kHalfHeightMin = 0.8f;

	static inline float kHalfWidthMax = 0.8f;
	static inline float kHalfHeightMax = 0.8f;

	static inline float kHalfDepthMin = 0.8f;
	static inline float kHalfDepthMax = 0.8f;
};