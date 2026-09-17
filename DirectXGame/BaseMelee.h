#pragma once

#include "Global.h"
#include <KamataEngine.h>

enum class WeaponState { kHolstered, kEquipped };

class BaseMelee {
public:
	enum class MeleeBehavior { kIdle, kAttacking };
	MeleeBehavior behavior_ = MeleeBehavior::kIdle;

	void ResetSwingHits() { hitTargetsThisSwing_.clear(); }
	bool HasHit(const void* target) const { return std::find(hitTargetsThisSwing_.begin(), hitTargetsThisSwing_.end(), target) != hitTargetsThisSwing_.end(); }
	void MarkHit(const void* target) { hitTargetsThisSwing_.push_back(target); }

	virtual void BehaviorIdleInitialize() {}
	virtual void BehaviorIdleUpdate() {}
	virtual void BehaviorAttackingInitialize() {}
	virtual void BehaviorAttackingUpdate() {}

	void SetBehavior(MeleeBehavior behavior) {
		if (behavior_ == behavior)
			return;
		behavior_ = behavior;
		switch (behavior_) {
		case MeleeBehavior::kIdle:
			BehaviorIdleInitialize();
			break;
		case MeleeBehavior::kAttacking:
			BehaviorAttackingInitialize();
			PlayDefaultSwingSound();
			break;
		}
	}

	bool IsAttacking() const { return behavior_ == MeleeBehavior::kAttacking; }

	virtual ~BaseMelee() = default;

	virtual void Initialize(KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	virtual void Update();
	virtual void Draw();

	bool IsDead() const { return isDead_; }
	virtual AABB GetAABB() const;

	virtual void OnCollision() { isDead_ = true; }

	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }
	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }
	virtual void SetWorldPosition(KamataEngine::Vector3& TargetPosition) { worldTransform_.translation_ = TargetPosition; }

	float GetHalfWidthMin() { return kHalfWidthMin; }
	float GetHalfHeightMin() { return kHalfHeightMin; }
	float GetHalfDepthMin() { return kHalfDepthMin; }
	float GetHalfWidthMax() { return kHalfWidthMax; }
	float GetHalfHeightMax() { return kHalfHeightMax; }
	float GetHalfDepthMax() { return kHalfDepthMax; }

	void SetHalfWidthMin(float& v) { kHalfWidthMin = v; }
	void SetHalfHeightMin(float& v) { kHalfHeightMin = v; }
	void SetHalfDepthMin(float& v) { kHalfDepthMin = v; }
	void SetHalfWidthMax(float& v) { kHalfWidthMax = v; }
	void SetHalfHeightMax(float& v) { kHalfHeightMax = v; }
	void SetHalfDepthMax(float& v) { kHalfDepthMax = v; }

	int GetMaxCombo() const { return kMaxCombo; }
	float GetComboAttackSpeed(int comboIndex) const { return kComboAttackSpeed[comboIndex]; }
	float GetComboAnticipateDuration(int comboIndex) const { return kComboAnticipateDuration[comboIndex]; }
	float GetComboAttackDuration(int comboIndex) const { return kComboAttackDuration[comboIndex]; }
	float GetComboRecoveryDuration(int comboIndex) const { return kComboRecoveryDuration[comboIndex]; }

	enum class AttackPhase { kAnticipate, kAttack, kRecovery };
	struct GenericAttackAnimState {
		AttackPhase phase = AttackPhase::kAnticipate;
		float t = 0.0f;
		int comboIndex = 0;
	};
	virtual float GetImpactPower(int comboIndex) const { return kBaseImpactPower + static_cast<float>(comboIndex) * kImpactPowerStep; }

	virtual int GetDamage(int comboIndex) const { return kComboDamageEnemy[comboIndex]; }
	virtual int GetDamage(int comboIndex, Owner owner) const { return (owner == Owner::kPlayer) ? kComboDamagePlayer[comboIndex] : kComboDamageEnemy[comboIndex]; }

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	// --- SFX ---
	// デフォルトの振りSFX(WeaponSabre系全般で共有)。GameSceneがロード後にセットする。
	static void SetDefaultSwingSoundHandle(uint32_t handle) { sDefaultSwingSoundHandle_ = handle; }
	static void SetSwingSoundHandle(int comboIndex, uint32_t handle) {
		// コンボ番号が範囲外なら何もしない
		if (comboIndex < 0 || comboIndex >= kMaxCombo) {
			return;
		}

		sComboSwingHandle[comboIndex] = handle;
	}
	static void SetSfxVolume(float volume) { sSfxVolume_ = volume; }
	// virtualにしておき、WeaponHeavySabre等が独自の音を鳴らしたい場合はoverride可能
	virtual void PlayDefaultSwingSound() {
		if (sDefaultSwingSoundHandle_ != 0) {
			KamataEngine::Audio::GetInstance()->PlayWave(sDefaultSwingSoundHandle_, false, sSfxVolume_);
		}
	}
	// virtualにしておき、WeaponHeavySabre等が独自の音を鳴らしたい場合はoverride可能
	virtual void PlaySwingSound(int comboIndex) {
		// コンボ番号が範囲外なら何もしない
		if (comboIndex < 0 || comboIndex >= kMaxCombo) {
			return;
		}

		uint32_t handle = sComboSwingHandle[comboIndex];

		// 個別のコンボSFXが設定されていればそれを使い、
		// 未設定ならデフォルトSFXを使う
		if (handle == 0) {
			handle = sDefaultSwingSoundHandle_;
		}

		if (handle != 0) {
			KamataEngine::Audio::GetInstance()->PlayWave(handle, false, sSfxVolume_);
		}
	}

protected:
	static inline float kBaseImpactPower = 1.0f;
	static inline float kImpactPowerStep = 0.4f;

	static inline std::array<int, 3> kComboDamagePlayer = {45, 45, 75};
	static inline std::array<int, 3> kComboDamageEnemy = {10, 10, 20};

	std::vector<const void*> hitTargetsThisSwing_;

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0;

	KamataEngine::Vector3 velocity_{};

	static inline int32_t kLifeTime = 60;
	int32_t deathTimer_ = kLifeTime;

	bool isDead_ = false;

	float kHalfWidthMin = 0.2f;
	float kHalfHeightMin = 0.2f;
	float kHalfWidthMax = 0.2f;
	float kHalfHeightMax = 0.2f;
	float kHalfDepthMin = 0.2f;
	float kHalfDepthMax = 0.2f;

	static inline int kMaxCombo = 3;
	static inline std::array<float, 3> kComboAttackSpeed = {0.25f, 0.25f, 0.7f};
	static inline std::array<float, 3> kComboAnticipateDuration = {10.0f, 10.0f, 15.0f};
	static inline std::array<float, 3> kComboAttackDuration = {8.0f, 8.0f, 15.0f};
	static inline std::array<float, 3> kComboRecoveryDuration = {15.0f, 15.0f, 30.0f};

	// --- SFX ---
	static inline uint32_t sDefaultSwingSoundHandle_ = 0;
	static inline std::array<uint32_t, 3> sComboSwingHandle = {0, 0, 0};
	static inline float sSfxVolume_ = 1.0f;
};