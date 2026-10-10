#pragma once
#include "BaseEnemy.h"
#include "CuirassierBehavior.h"
#include "WeaponHeavySabre.h"

class EnemyCuirassier final : public BaseEnemy {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update() override;
	void Draw() override;

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
	
	EquipStyle GetEquipStyle() const override { return EquipStyle::kHeavy; }

protected:
	void RefreshWeaponVisual() override { UpdateMeleeVisual(nullptr); }

	std::unique_ptr<BaseEnemyBehavior> CreateBehaviorModule() override { return std::make_unique<CuirassierBehavior>(*this); }
	bool GetDropInfo(DropItemCategory& outCategory, DropWeaponKind& outKind) const override {
		outCategory = DropItemCategory::kMelee;
		outKind = DropWeaponKind::kSabreHeavy;
		return true;
	}

	void ApplyWeaponEquipState(bool equipped) override {
		if (!currentMelee_)
			return;
		currentMelee_->SetEquipDuration(kHeavyEquipLockDuration);
		currentMelee_->SetWeaponState(equipped ? WeaponState::kEquipped : WeaponState::kHolstered);
	}
	void SnapWeaponEquipState(bool equipped) override {
		if (!currentMelee_)
			return;
		currentMelee_->SetEquipDuration(kHeavyEquipLockDuration);
		currentMelee_->SnapWeaponState(equipped ? WeaponState::kEquipped : WeaponState::kHolstered);
	}

	// BaseEnemyのvirtualフックをoverride。専用ハンドルがあればそちらを、なければ
	// BaseEnemyのデフォルトにそのままフォールバックする(hurt/deathのみ；movementはデフォルト無音のため
	// 専用ハンドルが無ければ何も鳴らさない)。
	void PlayHurtSound() override {
		if (sHurtSoundHandle_ != 0) {
			AudioSettings::PlaySfx(sHurtSoundHandle_, false, sSfxVolume_);
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
		AudioSettings::PlaySfx(chosen, false, sSfxVolume_);
	}
	void PlayMovementSound() override {
		if (sMovementSoundHandle_ != 0) {
			AudioSettings::PlaySfx(sMovementSoundHandle_, false, sSfxVolume_);
		}
	}

private:
	std::unique_ptr<WeaponHeavySabre> currentMelee_;
	bool isTurning_ = false;

	bool isSwingActive_ = false;
	bool isLungeActive_ = false;
	int currentComboIndex_ = 0;

	// Heavy: deep wind-ups, big forward lean, body sinks into each swing.
	static inline BodyComboPoseSet kBodyCombo = {
	    {MakeBodyPose(-0.25f, -1.0f, 0.0f, {1.0f, 1.03f, 1.0f},  {0.0f, 0.03f, 0.0f}),  MakeBodyPose(-0.25f, 1.3f, 0.0f, {1.0f, 1.03f, 1.0f},  {0.0f, 0.03f, 0.0f}),
	     MakeBodyPose(-0.6f, -1.0f, 0.0f, {1.0f, 1.05f, 1.0f},  {0.0f, 0.05f, 0.0f}) },
	    {MakeBodyPose(0.4f,   0.35f, 0.0f, {1.03f, 0.97f, 1.0f}, {0.0f, -0.04f, 0.0f}), MakeBodyPose(0.45f,  0.0f, 0.0f, {1.15f, 0.97f, 1.0f}, {0.0f, -0.04f, 0.0f}),
	     MakeBodyPose(0.7f,  -0.2f, 0.0f, {1.05f, 0.95f, 1.0f}, {0.0f, -0.08f, 0.0f})},
	    {MakeBodyPose(0.4f,   1.0f,  0.0f, {1.0f, 0.98f, 1.0f},  {0.0f, -0.03f, 0.0f}), MakeBodyPose(0.5f,   0.0f, 0.0f, {1.0f, 0.98f, 1.0f},  {0.0f, -0.03f, 0.0f}),
	     MakeBodyPose(0.65f, 0.0f,  0.0f, {1.0f, 0.97f, 1.0f},  {0.0f, -0.05f, 0.0f})},
	};
	static inline BodyLungePoseSet kBodyLunge = {
	    MakeBodyPose(-0.3f, -0.5f, 0.0f, {0.97f, 1.03f, 1.0f}),
	    MakeBodyPose(0.55f, 0.35f, 0.0f, {1.05f, 0.95f, 1.0f}, {0.0f, -0.05f, 0.0f}),
	};

	// --- SFX ---
	static inline uint32_t sHurtSoundHandle_ = 0;
	static inline std::array<uint32_t, 3> sDeathSoundHandles_ = {};
	static inline uint32_t sDeathSoundHandle_ = 0;
	static inline uint32_t sMovementSoundHandle_ = 0;
	static inline uint32_t sBlockSoundHandle_ = 0;
	static inline float sSfxVolume_ = 1.0f;
};