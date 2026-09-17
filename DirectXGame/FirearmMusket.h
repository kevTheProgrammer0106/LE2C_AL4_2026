#pragma once

#include "BaseFirearm.h"

class FirearmMusket final : public BaseFirearm {
public:
	// 調整項目を登録
	static void RegisterGlobalVariables();
	// 調整項目を適用
	static void ApplyGlobalVariables();
	FirearmMusket()
	{
		offset_ = musketOffset_;
	}
	void UpdateFollow(const KamataEngine::WorldTransform& parentTransform, float localAimAngle, bool facingRight) override;

	bool CanFire() const { return currentAmmo_ > 0 && !isReloading_ == false ? currentAmmo_ > 0 : false; } // see note below, simplify to:
	// bool CanFire() const { return currentAmmo_ > 0; }

	bool TryFire();           // 発砲を試みる。成功したらtrueを返し弾薬を消費する
	void NotifyDealtDamage(); // この銃の弾が敵に命中した時に呼ばれる（次回リロードが少し遅くなる）
	bool IsReloading() const { return isReloading_; }
	int GetCurrentAmmo() const { return currentAmmo_; }
	float GetReloadProgress01() const { return isReloading_ ? std::clamp(reloadTimer_ / reloadDurationCurrent_, 0.0f, 1.0f) : 1.0f; }

	static void SetMusketLoadedSoundHandle(uint32_t handle) { sMusketLoadedSoundHandle_ = handle; }

private:
	// --- 弾薬・リロード関連 ---
	int currentAmmo_ = 1;
	static inline int kMaxAmmo = 1;

	bool isReloading_ = false;
	float reloadTimer_ = 0.0f;

	static inline float kReloadDuration = 3.0f;     // 基本リロード時間
	static inline float kReloadHitBonus = 0.15f;    // 命中1回につき次回リロードが短縮される秒数
	static inline float kReloadDurationMin = 0.3f;  // リロード時間の下限（0や負値を防ぐ）
	float reloadDurationCurrent_ = kReloadDuration; // 命中ボーナス込みの実効リロード時間

	static inline SRTOffset musketOffset_ = {
	    {1.0f,  1.0f, 1.0f},
        {0.0f,  -1.5707963f, 0.0f},
		{0.6f, 0.2f, -0.1f}
    };
	// 収納時のポーズ（背中に担いだ想定。値は要調整）
	static inline SRTOffset musketHolsterOffset_ = {
	    {1.0f,   1.0f,  1.0f},
        {-1.371f, 0.0f, 0.0f},
        {-0.4f,   -0.3f, -0.3f}
    };

		static inline uint32_t sMusketLoadedSoundHandle_ = 0;
};