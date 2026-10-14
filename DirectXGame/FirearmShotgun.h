#pragma once

#include "BaseFirearm.h"

/// <summary>
/// ショットガン。1回の発射で5発のペレットを固定の拡散角で撃つ。
/// 1発のダメージは通常弾の1/4、リロードはマスケットよりかなり遅い。
/// </summary>
class FirearmShotgun final : public BaseFirearm {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	// 見た目のオフセット(マスケットと同じ値で開始。独立して調整できる)
	const SRTOffset& GetCombatOffset() const override { return shotgunOffset_; }
	const SRTOffset& GetHolsterOffset() const override { return shotgunHolsterOffset_; }
	const SRTOffset& GetReloadOffset() const override { return shotgunReloadOffset_; }
	const SRTOffset& GetKnockbackOffset() const override { return shotgunKnockbackOffset_; }
	const SRTOffset& GetStunOffset() const override { return shotgunStunOffset_; }
	const SRTOffset& GetBlockOffset() const override { return shotgunBlockOffset_; }
	float GetModelYawOffset() const override { return std::numbers::pi_v<float> / 2.0f; }

	// ---- 発射パターン: 5発を固定角で等間隔に広げる ----
	int GetPelletCount() const override { return kPelletCount; }
	float GetPelletAngle(int index) const override {
		// 中央の弾が0、両側へ kSpreadStep ずつ(例: -2,-1,0,+1,+2 段)
		const float center = static_cast<float>(kPelletCount - 1) * 0.5f;
		return (static_cast<float>(index) - center) * kSpreadStep;
	}
	int32_t GetProjectileLifeFrames() const override { return kPelletLifeFrames; }

	float GetFireKnockbackPower() const override { return kFireKnockbackPower; }

protected:
	int GetMaxAmmo() const override { return kMaxAmmo; }
	// 持ち主ごとに値を切り替える
	float GetBaseReloadDuration() const override { return IsPlayerOwned() ? kReloadDurationPlayer : kReloadDurationEnemy; }
	float GetReloadHitBonus() const override { return IsPlayerOwned() ? kReloadHitBonusPlayer : kReloadHitBonusEnemy; }
	float GetReloadDurationMin() const override { return IsPlayerOwned() ? kReloadDurationMinPlayer : kReloadDurationMinEnemy; }
	int GetDamageToEnemy() const override { return kDamageToEnemy; }
	int GetDamageToPlayer() const override { return kDamageToPlayer; }

private:
	friend class DebugMenu;

	static inline int kMaxAmmo = 1;

	// ---- 発射パターン ----
	static inline int kPelletCount = 5;             // 1回で出るペレット数
	static inline float kSpreadStep = 0.09f;        // ペレット間の角度(rad)。約5.2度
	static inline int32_t kPelletLifeFrames = 40;   // 射程(フレーム)。通常弾より短い
	static inline int kDamageToEnemy = 40;          // 1発あたり(マスケット160の1/4)
	static inline int kDamageToPlayer = 8;          // 1発あたり(マスケット30の約1/4)

	// ---- リロード: プレイヤー用(マスケットの3.0秒よりかなり遅い) ----
	static inline float kReloadDurationPlayer = 6.0f;
	static inline float kReloadHitBonusPlayer = 0.05f;   // 命中1回あたりの短縮(ペレット数が多いので小さめ)
	static inline float kReloadDurationMinPlayer = 2.0f; // 下限
	// ---- リロード: 敵用 ----
	static inline float kReloadDurationEnemy = 6.0f;
	static inline float kReloadHitBonusEnemy = 0.05f;
	static inline float kReloadDurationMinEnemy = 2.0f;

	static inline SRTOffset shotgunOffset_ = {
	    {1.0f, 1.0f,        1.0f },
        {0.0f, -1.5707963f, 0.0f },
        {0.6f, 0.2f,        -0.1f}
    };
	static inline SRTOffset shotgunHolsterOffset_ = {
	    {1.0f,    1.0f,  1.0f },
        {-1.371f, 0.0f,  0.0f },
        {-0.4f,   -0.3f, -0.3f}
    };
	static inline SRTOffset shotgunReloadOffset_ = {
	    {1.0f,   1.0f,  1.0f },
        {-0.9f,  0.0f,  0.0f },
        {-0.25f, -0.1f, -0.2f}
    };
	static inline SRTOffset shotgunKnockbackOffset_ = {
	    {1.0f, 1.0f,        1.0f },
        {0.3f, -1.5707963f, 0.9f },
        {0.4f, 0.35f,       -0.1f}
    };
	static inline SRTOffset shotgunStunOffset_ = {
	    {1.0f,  1.0f,        1.0f },
        {0.0f,  -1.5707963f, -0.9f},
        {0.55f, -0.1f,       -0.1f}
    };
	static inline SRTOffset shotgunBlockOffset_ = {
	    {1.0f,  1.0f,        1.0f },
        {0.0f,  -1.5707963f, 0.7f },
        {0.45f, 0.25f,       -0.1f}
    };
	static inline float kFireKnockbackPower = 3.0f; // recoil on the shooter (player speed = power * 0.1)
};