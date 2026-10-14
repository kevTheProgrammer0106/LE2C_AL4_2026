#pragma once
#include "BaseMelee.h"
#include "Global.h"

/// 斧。共通の近接ロジックはBaseMelee側にあり、このクラスはデータと「地面叩きつけ」(敵専用)だけを持つ。
/// WeaponHeavySabreのポーズ/当たり判定を土台にしている。
class WeaponAxe final : public BaseMelee {
public:
	WeaponAxe() { equipDuration_ = 0.3f; }

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	// --- 地面叩きつけ(敵専用) ---
	void SetSmashPositionGeneric(const KamataEngine::Vector3& ownerPosition, const KamataEngine::Vector3& ownerRotation, bool facingRight, AttackPhase smashPhase, float t);
	AABB GetSmashAABBGeneric(const KamataEngine::Vector3& ownerPosition) const;
	int GetSmashDamage() const { return kSmashDamage; }
	float GetSmashImpactPower() const { return kSmashImpactPower; }

	// 敵用 / プレイヤー用それぞれ斧専用の値を使う
	using BaseMelee::GetDamage;
	int GetDamage(int comboIndex, Owner owner) const override {
		const int i = std::clamp(comboIndex, 0, 2);
		return (owner == Owner::kPlayer) ? kComboDamageAxePlayer[i] : kComboDamageAxe[i];
	}
	using BaseMelee::GetImpactPower;
	float GetImpactPower(int comboIndex, Owner owner) const override {
		const float step = static_cast<float>(std::clamp(comboIndex, 0, 2));
		return (owner == Owner::kPlayer) ? kBaseImpactAxePlayer + step * kImpactStepAxePlayer : kBaseImpactAxe + step * kImpactStepAxe;
	}

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
	// WeaponHeavySabreと同じ構え/コンボ(斧のモデルに合わせて後から調整する想定)
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

	static inline SRTOffset kKnockbackOffset{
	    {1.0f,  1.0f, 1.0f },
        {-1.0f, 0.0f, 0.4f },
        {0.3f,  0.2f, 0.45f}
    };
	static inline SRTOffset kStunOffset{
	    {1.0f,  1.0f,   1.0f },
        {0.9f,  0.0f,   0.2f },
        {0.45f, -0.35f, 0.45f}
    };
	static inline SRTOffset kBlockOffset{
	    {1.0f,  1.0f,  1.0f },
        {-0.6f, 0.0f,  1.2f },
        {0.55f, 0.15f, 0.49f}
    };
	static inline SRTOffset kDownAttackOffset{
	    {1.0f, 1.0f,  1.0f},
        {1.2f, 0.0f,  0.0f},
        {0.0f, -1.0f, 0.0f}
    };

	static inline std::array<float, 3> kComboReach = {2.0f, 2.0f, 2.4f};
	static inline std::array<float, 3> kComboSwingHeight = {1.8f, 1.8f, 2.0f};
	static inline std::array<float, 3> kUpDownReach = {1.2f, 1.4f, 1.8f};
	static inline std::array<float, 3> kUpDownWidth = {1.2f, 1.3f, 1.5f};

	// ---- 敵用コンボのダメージ/衝撃力 ----
	static inline std::array<int, 3> kComboDamageAxe = {20, 20, 35};
	static inline float kBaseImpactAxe = 1.4f;
	static inline float kImpactStepAxe = 0.4f;
	// ---- プレイヤー用(ヘビーより重い一撃) ----
	static inline std::array<int, 3> kComboDamageAxePlayer = {60, 60, 100};
	static inline float kBaseImpactAxePlayer = 1.6f;
	static inline float kImpactStepAxePlayer = 0.5f;

	// ---- 地面叩きつけ ----
	// 振り上げ(頭上)→叩きつけ(前下)。値は仮置き。GlobalVariablesのWeaponAxeグループでリアルタイム調整できる
	static inline SRTOffset kSmashRaise{
	    {1.0f,  1.0f, 1.0f},
        {-0.4f, 0.0f, 2.6f},
        {0.2f,  0.9f, 0.3f}
    };
	static inline SRTOffset kSmashSlam{
	    {1.0f, 1.0f,  1.0f },
        {0.6f, 0.0f,  -0.2f},
        {0.5f, -0.6f, 0.2f }
    };
	static inline float kSmashHalfWidth = 2.0f; // 叩きつけ判定の左右の半幅(自分を中心に)
	static inline float kSmashBelow = 0.4f;     // 足元からさらに下へ
	static inline float kSmashAbove = 0.8f;     // 体の中心から上へ
	static inline int kSmashDamage = 35;
	static inline float kSmashImpactPower = 2.5f;
};