#pragma once
#include "BaseEnemy.h"
#include "CuirassierBehavior.h"
#include "WeaponHeavySabre.h"

class EnemyCuirassier final : public BaseEnemy {
public:
	void Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update() override;
	void Draw() override;

	void OnCollision(const Player* player) override;
	void CheckPlayerDirection() override;

	WeaponHeavySabre* GetCurrentMelee() const { return currentMelee_.get(); }
	void SetMeleeModel(KamataEngine::Model* model, uint32_t textureHandle);

	// 通常コンボの見た目を更新する(FighterBehaviorのUpdateMeleeVisualと同じ役割)
	void UpdateMeleeVisual(const BaseMelee::GenericAttackAnimState* attackState);
	// ランジ突撃専用の見た目を更新する
	void UpdateLungeVisual(WeaponHeavySabre::AttackPhase lungePhase, float t);

	bool IsSwingActive() const { return isSwingActive_; }
	bool IsLungeActive() const { return isLungeActive_; }
	AABB GetSwingAABB() const;
	AABB GetLungeAABB() const;

	int GetCurrentComboIndex() const { return currentComboIndex_; }
	
	// --- SFX ---
	// Cuirassier固有: 被弾/死亡/移動(足音)/ガード開始SFX。未設定ならBaseEnemyのデフォルトにフォールバック(hurt/death)。
	static void SetHurtSoundHandle(uint32_t handle) { sHurtSoundHandle_ = handle; }
	static void SetDeathSoundHandle(uint32_t handle) { SetDeathSoundHandle(0, handle); }
	static void SetDeathSoundHandle(int index, uint32_t handle) {
		if (index >= 0 && index < static_cast<int>(sDeathSoundHandles_.size())) {
			sDeathSoundHandles_[index] = handle;
		}
	}
	static void SetMovementSoundHandle(uint32_t handle) { sMovementSoundHandle_ = handle; }
	

protected:
	std::unique_ptr<BaseEnemyBehavior> CreateBehaviorModule() override { return std::make_unique<CuirassierBehavior>(*this); }
	bool GetDropInfo(DropItemCategory& outCategory, DropWeaponKind& outKind) const override {
		outCategory = DropItemCategory::kMelee;
		outKind = DropWeaponKind::kSabreHeavy;
		return true;
	}

	// BaseEnemyのvirtualフックをoverride。専用ハンドルがあればそちらを、なければ
	// BaseEnemyのデフォルトにそのままフォールバックする(hurt/deathのみ；movementはデフォルト無音のため
	// 専用ハンドルが無ければ何も鳴らさない)。
	void PlayHurtSound() override {
		if (sHurtSoundHandle_ != 0) {
			KamataEngine::Audio::GetInstance()->PlayWave(sHurtSoundHandle_, false, sSfxVolume_);
		} else {
			BaseEnemy::PlayHurtSound();
		}
	}
	void PlayDeathSound() override {
		std::array<uint32_t, 3> candidates{};
		size_t count = 0;
		for (uint32_t handle : sDeathSoundHandles_) {
			if (handle != 0) {
				candidates[count++] = handle;
			}
		}
		if (count == 0) {
			BaseEnemy::PlayDeathSound();
			return;
		}
		uint32_t chosen = candidates[static_cast<size_t>(std::rand()) % count];
		KamataEngine::Audio::GetInstance()->PlayWave(chosen, false, sSfxVolume_);
	}
	void PlayMovementSound() override {
		if (sMovementSoundHandle_ != 0) {
			KamataEngine::Audio::GetInstance()->PlayWave(sMovementSoundHandle_, false, sSfxVolume_);
		}
	}

private:
	std::unique_ptr<WeaponHeavySabre> currentMelee_;
	bool isTurning_ = false;

	bool isSwingActive_ = false;
	bool isLungeActive_ = false;
	int currentComboIndex_ = 0;

	// --- SFX ---
	static inline uint32_t sHurtSoundHandle_ = 0;
	static inline std::array<uint32_t, 3> sDeathSoundHandles_ = {};
	static inline uint32_t sDeathSoundHandle_ = 0;
	static inline uint32_t sMovementSoundHandle_ = 0;
	static inline uint32_t sBlockSoundHandle_ = 0;
	static inline float sSfxVolume_ = 1.0f;
};