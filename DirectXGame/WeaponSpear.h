#pragma once

#include "BaseMelee.h"

/// <summary>
/// スポントン(槍)。コンボのポーズ/タイミング/高さはWeaponSabreと同じ3段コンボ。
/// 射程だけCuirassierのランジ射程(WeaponHeavySabre::kLungeReach = 4.5)にしてある。
/// 共通の近接ロジックはBaseMeleeにあり、このクラスはデータだけを持つ。
/// </summary>
class WeaponSpear final : public BaseMelee {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	const SRTOffset& GetHolsterOffset() const override { return holsterOffset_; }
	const SRTOffset& GetReadyStanceOffset() const override { return readyStanceOffset_; }
	const SRTOffset& GetKnockbackOffset() const override { return kKnockbackOffset; }
	const SRTOffset& GetStunOffset() const override { return kStunOffset; }
	const SRTOffset& GetBlockOffset() const override { return kBlockOffset; }

	KamataEngine::Vector3 GetTrailTipLocalOffset() const override { return {0.0f, 0.0f, 3.0f}; }

	// thrust, thrust, slashAlt
	AttackEffectType GetComboEffectType(int comboIndex) const override { return (comboIndex >= 2) ? AttackEffectType::kSlashAlt : AttackEffectType::kThrust; }

protected:
	const std::array<ComboAttackPattern, 3>& GetComboPatterns() const override { return comboPatterns_; }
	const std::array<float, 3>& GetComboReach() const override { return kComboReach; }
	const std::array<float, 3>& GetComboSwingHeight() const override { return kComboSwingHeight; }
	const std::array<float, 3>& GetUpDownReach() const override { return kUpDownReach; }
	const std::array<float, 3>& GetUpDownWidth() const override { return kUpDownWidth; }
	const SRTOffset& GetDownAttackOffset() const override { return kDownAttackOffset; }

private:
	// WeaponSabreと同じコンボ(槍のモデルに合わせて後から調整する想定)
	static inline std::array<ComboAttackPattern, 3> comboPatterns_ = {
	    ComboAttackPattern{
	                       {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.5f}, {0.2f, 0.15f, -0.6f}},
	                       {{1.0f, 1.0f, 1.0f}, {0.23f, 0.0f, 0.0f}, {-1.485f, 0.3f, 0.6f}},
	                       {{1.0f, 1.0f, 1.0f}, {0.04f, 0.0f, 0.0f}, {1.4f, -0.25f, -0.6f}},
	                       {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {-1.250f, -0.2f, -0.6f}},
	                       },
	    ComboAttackPattern{
	                       {{1.0f, 1.0f, 1.0f}, {-0.04f, 0.0f, 0.0f}, {-1.25f, -0.2f, -0.6f}},
	                       {{1.0f, 1.0f, 1.0f}, {0.23f, 0.0f, 0.0f}, {-1.485f, 0.3f, -0.6f}},
	                       {{1.0f, 1.0f, 1.0f}, {0.04f, 0.0f, 0.0f}, {1.4f, 0.130f, -0.6f}},
	                       {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.2f, -0.3f, -0.6f}},
	                       },
	    ComboAttackPattern{
	                       {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.2f, -0.3f, -0.6f}},
	                       {{1.0f, 1.0f, 1.0f}, {0.16f, 0.0f, 0.0f}, {-0.91f, 0.8f, 0.7f}},
	                       {{1.0f, 1.0f, 1.0f}, {0.16f, 0.0f, 0.0f}, {0.9f, -0.2f, 0.0f}},
	                       {{1.0f, 1.0f, 1.0f}, {-0.260f, 0.0f, 0.0f}, {-0.250f, -0.120f, 0.630f}},
	                       },
	};

	static inline SRTOffset holsterOffset_{
	    {1.0f,   1.0f,   1.0f  },
        {-2.08f, 3.140f, 0.5f   },
        {-0.430f, -0.580f, 0.250f}
    };
	static inline SRTOffset readyStanceOffset_{
	    {1.0f,  1.0f, 1.0f  },
        {-0.260f, 0.0f, 0.0f  },
        {-0.250f,  -0.120f, 0.630f}
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
        {-0.2f, 1.57f,  0.0f },
        {0.48f, 0.23f, 1.45f}
    };
	static inline SRTOffset kDownAttackOffset{
	    {1.0f, 1.0f,  1.0f},
        {1.2f, 0.0f,  0.0f},
        {0.0f, -0.9f, 0.0f}
    };

	// 射程はCuirassierのランジ射程と同じ(3段とも共通)。高さ/上下攻撃はサーベルのまま
	static inline float kSpearReach = 4.5f;
	static inline std::array<float, 3> kComboReach = {4.5f, 4.5f, 4.5f};
	static inline std::array<float, 3> kComboSwingHeight = {1.6f, 1.6f, 1.8f};
	static inline std::array<float, 3> kUpDownReach = {1.0f, 1.2f, 1.5f};
	static inline std::array<float, 3> kUpDownWidth = {1.0f, 1.1f, 1.3f};
};