#pragma once

#include "BaseFirearm.h"

class FirearmMusket final : public BaseFirearm {
public:
	// 調整項目を登録
	static void RegisterGlobalVariables();
	// 調整項目を適用
	static void ApplyGlobalVariables();
	FirearmMusket() { offset_ = musketOffset_; }

	// --- [NEW] animation offsets (own values; stats below are NOT shared with FirearmMusketBayonet) ---
	const SRTOffset& GetCombatOffset() const override { return musketOffset_; }
	const SRTOffset& GetHolsterOffset() const override { return musketHolsterOffset_; }
	const SRTOffset& GetReloadOffset() const override { return musketReloadOffset_; }
	const SRTOffset& GetKnockbackOffset() const override { return musketKnockbackOffset_; }
	const SRTOffset& GetStunOffset() const override { return musketStunOffset_; }
	const SRTOffset& GetBlockOffset() const override { return musketBlockOffset_; }
	float GetModelYawOffset() const override { return std::numbers::pi_v<float> / 2.0f; }

	bool CanFire() const { return currentAmmo_ > 0; }

	bool TryFire() override;  // 発砲を試みる。成功したらtrueを返し弾薬を消費する
	void NotifyDealtDamage(); // この銃の弾が敵に命中した時に呼ばれる（次回リロードが少し遅くなる）
	bool IsReloading() const { return isReloading_; }
	int GetCurrentAmmo() const { return currentAmmo_; }
	float GetReloadProgress01() const { return isReloading_ ? std::clamp(reloadTimer_ / reloadDurationCurrent_, 0.0f, 1.0f) : 1.0f; }

	static void SetMusketLoadedSoundHandle(uint32_t handle) { sMusketLoadedSoundHandle_ = handle; }

protected:
	void TickReload() override;

private:
	friend class DebugMenu;
	// --- 弾薬・リロード関連 (stats) ---
	int currentAmmo_ = 1;
	static inline int kMaxAmmo = 1;

	bool isReloading_ = false;
	float reloadTimer_ = 0.0f;

	static inline float kReloadDuration = 3.0f;     // 基本リロード時間
	static inline float kReloadHitBonus = 0.15f;    // 命中1回につき次回リロードが短縮される秒数
	static inline float kReloadDurationMin = 0.3f;  // リロード時間の下限（0や負値を防ぐ）
	float reloadDurationCurrent_ = kReloadDuration; // 命中ボーナス込みの実効リロード時間

	// --- animation offsets ---
	static inline SRTOffset musketOffset_ = {
	    {1.0f, 1.0f,        1.0f },
        {0.0f, -1.5707963f, 0.0f },
        {0.6f, 0.2f,        -0.1f}
    };
	// 収納時のポーズ（背中に担いだ想定。値は要調整）
	static inline SRTOffset musketHolsterOffset_ = {
	    {1.0f,    1.0f,  1.0f },
        {-1.371f, 0.0f,  0.0f },
        {-0.4f,   -0.3f, -0.3f}
    };
	// [NEW] reload pose (same starting values as the bayonet musket; tune independently)
	static inline SRTOffset musketReloadOffset_ = {
	    {1.0f,   1.0f,  1.0f },
        {-0.9f,  0.0f,  0.0f },
        {-0.25f, -0.1f, -0.2f}
    };
	// [NEW] knockback / stun poses (rotation.y matches musketOffset_ so the model keeps facing forward)
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

	// [NEW] guard pose (rotation.y matches musketOffset_)
	static inline SRTOffset musketBlockOffset_ = {
	    {1.0f,  1.0f,        1.0f },
        {0.0f,  -1.5707963f, 0.7f },
        {0.45f, 0.25f,       -0.1f}
    };

	static inline uint32_t sMusketLoadedSoundHandle_ = 0;
};