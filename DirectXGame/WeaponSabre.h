#pragma once

#include "BaseMelee.h"

/// Light sabre. Equip state, pose blending and swing hitboxes live in BaseMelee; this class only holds its data.
class WeaponSabre final : public BaseMelee {
public:
	AABB GetAABB() const override;

	// 調整項目を登録
	static void RegisterGlobalVariables();
	// 調整項目を適用
	static void ApplyGlobalVariables();

	const SRTOffset& GetHolsterOffset() const override { return holsterOffset_; }
	const SRTOffset& GetReadyStanceOffset() const override { return readyStanceOffset_; }
	const SRTOffset& GetKnockbackOffset() const override { return kKnockbackOffset; }
	const SRTOffset& GetStunOffset() const override { return kStunOffset; }
	const SRTOffset& GetBlockOffset() const override { return kBlockOffset; }

protected:
	const std::array<ComboAttackPattern, 3>& GetComboPatterns() const override { return comboPatterns_; }
	const std::array<float, 3>& GetComboReach() const override { return kComboReach; }
	const std::array<float, 3>& GetComboSwingHeight() const override { return kComboSwingHeight; }
	const std::array<float, 3>& GetUpDownReach() const override { return kUpDownReach; }
	const std::array<float, 3>& GetUpDownWidth() const override { return kUpDownWidth; }
	const SRTOffset& GetDownAttackOffset() const override { return kDownAttackOffset; }

private:
	friend class DebugMenu;

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
	    {1.0f,   1.0f,   1.0f  },
        {0.0f,   3.140f, 0.5f  },
        {0.250f, -0.1f,  0.490f}
    };
	static inline SRTOffset readyStanceOffset_{
	    {1.0f,  1.0f, 1.0f  },
        {-0.6f, 0.0f, 0.0f  },
        {0.5f,  0.0f, 0.490f}
    };
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

	static inline std::array<float, 3> kUpDownReach = {1.0f, 1.2f, 1.5f};
	static inline std::array<float, 3> kUpDownWidth = {1.0f, 1.1f, 1.3f};

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
