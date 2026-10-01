#pragma once

#include "AnimationManager.h"
#include "BlockDetector.h"
#include "Global.h"
#include "PlayerBehavior.h"

#include "FirearmMusket.h"
#include "ProjectileBall.h"
#include "WeaponSabre.h"

class GameScene;
class BaseEnemy;
class MapChipField;

class Player {
public:
	~Player();

	void KillInstant();
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(
	    const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, KamataEngine::Model* modelAttack, uint32_t textureHandleAttack, KamataEngine::Camera* camera,
	    const KamataEngine::Vector3& position);
	void Update();
	void Draw();

	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	enum class LRDirection { kRight, kLeft };
	bool isLeft() const { return lrDirection_ == LRDirection::kLeft; }
	bool isRight() const { return lrDirection_ == LRDirection::kRight; }

	const KamataEngine::WorldTransform& GetWorldTransform() const;
	LRDirection GetLRDirection() const { return lrDirection_; }
	void SetWorldTransform(const KamataEngine::WorldTransform& worldTransform);
	KamataEngine::Vector3 GetWorldPosition() const;
	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }
	void ResetVelocity() { velocity_ = {}; }

	AABB GetAABB();

	void MapChipCollisionUpdate();
	void MapChipCollisionUpdate(CollisionInfo& collisionInfo);

	void GroundMovement();
	void JumpMovement();
	void ApplyScrollPush();

	void PositionUpdate(const CollisionInfo& collisionInfo);
	void CeillingCollision(const CollisionInfo& collisionInfo);
	void WallCollision(const CollisionInfo& collisionInfo);

	void SetScrollBoundary(float leftBoundary, bool active) {
		scrollBoundaryX_ = leftBoundary;
		scrollPushActive_ = active;
	}
	bool isBeingPushed() const { return isBeingPushed_; }

	using Behavior = PlayerBehavior::Behavior;
	using AttackPhase = PlayerBehavior::AttackPhase;
	using AttackDirection = PlayerBehavior::AttackDirection;
	using AttackAnimState = PlayerBehavior::AttackAnimState;

	void OnCollision(const BaseEnemy* enemy);
	void OnCollision();

	bool isDead() const { return isDead_; }
	bool isAttack() const { return behavior_.IsAttacking(); }

	int GetComboIndex() const { return behavior_.GetComboIndex(); }
	AttackAnimState GetAttackAnimState() const { return behavior_.GetAttackAnimState(); }
	AttackDirection GetAttackDirection() const { return behavior_.GetAttackDirection(); }

	BaseMelee* GetCurrentMelee() const { return currentMelee_.get(); }
	FirearmMusket* GetCurrentFirearm() const { return currentFirearm_.get(); }
	void SetMusketModel(KamataEngine::Model* model, uint32_t textureHandle);
	void SetMeleeModel(KamataEngine::Model* model, uint32_t textureHandle);
	DropWeaponKind GetCurrentMeleeKind() const { return currentMeleeKind_; }
	void SetCurrentMeleeKind(DropWeaponKind kind) { currentMeleeKind_ = kind; }
	DropWeaponKind GetCurrentFirearmKind() const { return currentFirearmKind_; }
	void SetCurrentFirearmKind(DropWeaponKind kind) { currentFirearmKind_ = kind; }

	void UpdateMovementState();
	SRTOffset GetMovementAnimOffset() const { return animManager_.GetMovementAnimOffset(); }
	void UpdateBodyParts();

	enum class WeaponMode { kMelee, kFirearm };
	void UseFirearm();
	void UseMeleeWeapon();
	void UpdateAimAndFirearm();
	WeaponMode GetWeaponMode() const { return weaponMode_; }
	void SetWeaponMode(WeaponMode mode);

	bool IsMovingHorizontally() const { return KamataEngine::Input::GetInstance()->PushKey(DIK_D) || KamataEngine::Input::GetInstance()->PushKey(DIK_A); }
	KamataEngine::Vector2 GetEffectiveAimDir() const;

	friend class PlayerBehavior;

	int GetHP() const { return hp_; }
	int GetMaxHP() const { return kMaxHP; }
	void SetHP(int hp) { hp_ = std::clamp(hp, 0, kMaxHP); }
	void SetMaxHP(int maxHp) { kMaxHP = maxHp; }

		// --- SFX ---
	// 近接/銃火器の切り替え(DIK_Q)時に鳴らす。GameSceneがロード後にセットする。
	static void SetWeaponSwitchSoundHandle(uint32_t handle) { sWeaponSwitchSoundHandle_ = handle; }
	static void SetSfxVolume(float volume) { sSfxVolume_ = volume; }

	static void SetFootstepSoundHandle(uint32_t handle) { sFootstepSoundHandle_ = handle; }
	static void SetJumpSoundHandle(uint32_t handle) { sJumpSoundHandle_ = handle; }
	static void SetLandSoundHandle(uint32_t handle) { sLandSoundHandle_ = handle; }


	void TakeDamage(int damage, const KamataEngine::Vector3& hitDirection, float impactPower, BaseEnemy* attacker = nullptr);

	void SetStunned(float duration) { behavior_.RequestStun(duration); }
	void SetKnockback(const KamataEngine::Vector3& direction, float power) { behavior_.RequestHitKnockback(direction, power); }

	void SetBlocking(bool blocking) { behavior_.SetBlocking(blocking); }
	bool IsBlocking() const { return behavior_.IsBlocking(); }
	bool IsStunnedState() const { return behavior_.IsStunned(); }

	void SetControlLocked(bool locked) { controlLocked_ = locked; }
	bool IsControlLocked() const { return controlLocked_; }

	// ============================================================
	//  Debuff status / control (used by GameScene::debugMenu, usable from gameplay too)
	// ============================================================
	struct DebuffStatus {
		bool stunned = false;
		float stunRemaining = 0.0f;

		bool knockedBack = false;

		bool slowedByTerrain = false; // standing on a kSlowBlock
		bool slowedByEffect = false;  // timed/latched Slow applied via ApplySlow()
		float slowFactor = 1.0f;      // effect factor (1.0 = none)
		float slowRemaining = 0.0f;   // seconds left (ignored if slowUntilCleared)
		bool slowUntilCleared = false;

		bool controlLocked = false;

		bool beingPushed = false; // forced-scroll push (display only)
		float pushVelocityX = 0.0f;
		uint32_t squishTimer = 0;
	};
	DebuffStatus GetDebuffStatus() const;
	bool HasAnyDebuff() const;

	// duration <= 0 -> stays until ClearSlow()/ClearAllDebuffs()
	void ApplySlow(float factor, float duration);
	void ClearSlow();
	void ClearStun() { behavior_.ClearStun(); }
	void ClearKnockback() { behavior_.ClearKnockback(); }
	// Instantly removes stun, knockback, slow effect, control lock and squish progress.
	void ClearAllDebuffs();

private:
	bool controlLocked_ = false; 

	// Slow effect state
	bool slowActive_ = false;
	bool slowUntilCleared_ = false;
	float slowFactor_ = 1.0f;
	float slowTimer_ = 0.0f;
	void TickDebuffs();
	bool IsOnSlowTerrain() const;

	DropWeaponKind currentMeleeKind_ = DropWeaponKind::kSabreLight;
	DropWeaponKind currentFirearmKind_ = DropWeaponKind::kMusketLong;

	int hp_ = kMaxHP;
	static inline int kMaxHP = 100;

	BlockDetector blockDetector_;
	WeaponMode weaponMode_ = WeaponMode::kMelee;

	KamataEngine::Vector2 ComputeMouseAimDir() const;

	GameScene* gameScene_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Camera* camera_ = nullptr;

	static inline SRTOffset bodyOffset_ = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };
	static inline SRTOffset headOffset_ = {
	    {1.0f,    1.0f,   1.0f},
        {0.15f,   0.0f,   0.0f},
        {-0.050f, 0.720f, 0.0f}
    };
	static inline SRTOffset hatOffset_ = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };
	static inline SRTOffset firearmOffset_ = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

	static constexpr size_t kPartCount = static_cast<size_t>(PartSlot::kCount);
	static constexpr size_t body = static_cast<size_t>(PartSlot::kBody);
	static constexpr size_t head = static_cast<size_t>(PartSlot::kHead);
	static constexpr size_t hat = static_cast<size_t>(PartSlot::kHat);

	std::array<BodyPart, static_cast<size_t>(PartSlot::kCount)> parts_;

	KamataEngine::Model* modelAttack_ = nullptr;
	KamataEngine::WorldTransform worldTransformAttack_;
	uint32_t modelAttackTex_ = 0;

	std::unique_ptr<WeaponSabre> currentMelee_;
	std::unique_ptr<FirearmMusket> currentFirearm_;

	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;
	static inline float kTimeTurn = 0.3f;

	LRDirection lrDirection_ = LRDirection::kRight;
	KamataEngine::Vector3 velocity_ = {0, 0, 0};

	static inline float kAcceleration = 0.05f;
	static inline float kAttenuation = 0.1f;
	static inline float kLimitRunSpeed = 0.25f;

	bool onGround_ = true;

	static inline float kGravityAcceleration = 0.05f;
	static inline float kLimitFallSpeed = 0.5f;
	static inline float kJumpAcceleration = 0.75f;

	static inline float kPushAcceleration = 0.015f;
	static inline float kMaxPushSpeed = 0.15f;
	uint32_t squishTimer_ = 0;
	static inline uint32_t kSquishThreshold = 15;

	float scrollBoundaryX_ = 0.0f;
	bool scrollPushActive_ = false;
	bool isBeingPushed_ = false;
	float pushVelocityX_ = 0.0f;

	static inline float kBulletSpeed = 1.0f;
	static inline float kBlockAttackerKnockbackScale = 2.5f; // how much harder the attacker gets pushed vs. the player's own block pushback
	PlayerBehavior behavior_{*this};

	MapChipField* mapChipField_ = nullptr;
	static inline float kWidth = 0.8f;
	static inline float kHeight = 1.6f;
	static inline float kBlank = 0.01f;
	static inline float kAttenuationLanding = 0.2f;
	static inline float kAttenuationWall = 0.2f;
	static inline float kAttenuationCeilling = 0.2f;

	bool isDead_ = false;

		// --- SFX ---
	static inline uint32_t sWeaponSwitchSoundHandle_ = 0;
	static inline float sSfxVolume_ = 1.0f;
	static void PlaySfxHandle(uint32_t handle) {
		if (handle != 0) {
			KamataEngine::Audio::GetInstance()->PlayWave(handle, false, sSfxVolume_);
		}
	}
	static inline uint32_t sFootstepSoundHandle_ = 0;
	static inline uint32_t sJumpSoundHandle_ = 0;
	static inline uint32_t sLandSoundHandle_ = 0;
	static inline float kMovementSfxVolume = 0.4f;

	static void PlayMovementSfxHandle(uint32_t handle) {
		if (handle != 0) {
			KamataEngine::Audio::GetInstance()->PlayWave(handle, false, kMovementSfxVolume);
		}
	}
	SRTOffset GetAttackPoseOffset() const { return behavior_.GetAttackPoseOffset(); }

	AnimationManager animManager_;
};