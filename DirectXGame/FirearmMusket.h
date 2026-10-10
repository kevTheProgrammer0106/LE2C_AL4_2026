#pragma once

#include "BaseFirearm.h"

class FirearmMusket final : public BaseFirearm {
public:
	// 調整項目を登録
	static void RegisterGlobalVariables();
	// 調整項目を適用
	static void ApplyGlobalVariables();

	// --- animation offsets (own values; stats below are NOT shared with FirearmMusketBayonet) ---
	const SRTOffset& GetCombatOffset() const override { return musketOffset_; }
	const SRTOffset& GetHolsterOffset() const override { return musketHolsterOffset_; }
	const SRTOffset& GetReloadOffset() const override { return musketReloadOffset_; }
	const SRTOffset& GetKnockbackOffset() const override { return musketKnockbackOffset_; }
	const SRTOffset& GetStunOffset() const override { return musketStunOffset_; }
	const SRTOffset& GetBlockOffset() const override { return musketBlockOffset_; }
	float GetModelYawOffset() const override { return std::numbers::pi_v<float> / 2.0f; }

	// Kept for GameScene; forwards to the shared handle.
	static void SetMusketLoadedSoundHandle(uint32_t handle) { SetLoadedSoundHandle(handle); }

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
	// ---- プレイヤー用 ----
	static inline float kReloadDurationPlayer = 3.0f;    // 基本リロード時間
	static inline float kReloadHitBonusPlayer = 0.15f;   // 命中1回につき次回リロードが短縮される秒数
	static inline float kReloadDurationMinPlayer = 0.3f; // リロード時間の下限
	// ---- 敵用 ----
	static inline float kReloadDurationEnemy = 3.0f;
	static inline float kReloadHitBonusEnemy = 0.15f;
	static inline float kReloadDurationMinEnemy = 0.3f;
	static inline int kDamageToEnemy = 160; // プレイヤーが撃ったとき敵に与える
	static inline int kDamageToPlayer = 30; // 敵が撃ったときプレイヤーに与える

	static inline SRTOffset musketOffset_ = {
	    {1.0f, 1.0f,        1.0f },
        {0.0f, -1.5707963f, 0.0f },
        {0.6f, 0.2f,        -0.1f}
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
};
