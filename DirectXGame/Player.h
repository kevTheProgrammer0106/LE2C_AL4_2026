#pragma once

#include "AnimationManager.h"
#include "BlockDetector.h"
#include "Global.h"
#include "PlayerBehavior.h"

#include "FirearmMusket.h"
#include "FirearmMusketBayonet.h"
#include "FirearmShotgun.h"
#include "FirearmPistols.h"

#include "ProjectileBall.h"

#include "WeaponSabre.h"
#include "WeaponHeavySabre.h"
#include "WeaponAxe.h"
#include "WeaponSpear.h"


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
	void MapChipCollisionUpdate(CollisionInfo& collisionInfo, bool ledgeGuard = false);

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
	BaseFirearm* GetCurrentFirearm() const { return currentFirearm_.get(); }

	void SetMusketModel(KamataEngine::Model* model, uint32_t textureHandle, DropWeaponKind kind = DropWeaponKind::kMusketLong);
	void SetMeleeModel(KamataEngine::Model* model, uint32_t textureHandle, DropWeaponKind kind = DropWeaponKind::kSabreLight);

	DropItemState GetCurrentMeleeState() const;
	DropItemState GetCurrentFirearmState() const;

	// ---------------- Lunge (special attack) ----------------
	bool IsLunge() const { return behavior_.IsLunge(); }
	// Combo swing in progress (a bayonet lunge or any lunge does NOT count). Use for sabre hit detection / poses.
	bool IsMeleeSwingActive() const { return behavior_.IsAttacking() && !behavior_.IsLunge(); }
	bool IsLungeHitActive() const { return behavior_.IsLunge() && behavior_.GetAttackAnimState().phase == AttackPhase::kAttack; }
	LungeSource GetCurrentLungeSource() const { return behavior_.GetLungeSource(); }
	LungeSpec GetLungeSpec(LungeSource source) const;
	AABB GetLungeAABB();
	void BeginLungeCooldown(LungeSource source);
	void PlayLungeSound(LungeSource source);

	bool IsMeleeLungeCooling() const { return meleeLungeCooldown_ > 0.0f; }
	float GetMeleeLungeProgress01() const { return meleeLungeCooldownMax_ > 0.0f ? std::clamp(1.0f - meleeLungeCooldown_ / meleeLungeCooldownMax_, 0.0f, 1.0f) : 1.0f; }
	bool IsBayonetLungeCooling() const { return bayonetLungeCooldown_ > 0.0f; }
	float GetBayonetLungeProgress01() const { return bayonetLungeCooldownMax_ > 0.0f ? std::clamp(1.0f - bayonetLungeCooldown_ / bayonetLungeCooldownMax_, 0.0f, 1.0f) : 1.0f; }

	// Targets hit by the current lunge (one hit per target per lunge)
	void ResetLungeHits() { lungeHits_.clear(); }
	bool HasLungeHit(const void* target) const { return std::find(lungeHits_.begin(), lungeHits_.end(), target) != lungeHits_.end(); }
	void MarkLungeHit(const void* target) { lungeHits_.push_back(target); }
	bool HasLungeHits() const { return !lungeHits_.empty(); }
	void RestoreMeleeState(DropItemState state);
	void RestoreFirearmState(DropItemState state);

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

	// 無敵状態: 被弾・ブロック・ジャンプ/着地で付与される。ノックバック中は常に無敵。
	// 無敵中はノックバック/スタンなどの「リアクション」を受け付けない(ダメージ自体は通る)
	bool IsInvulnerable() const { return invulnTimer_ > 0.0f || behavior_.IsKnockback(); }
	float GetInvulnerabilityRemaining() const { return invulnTimer_; }

	void SetStunned(float duration) {
		if (IsInvulnerable())
			return;
		behavior_.RequestStun(duration);
	}
	void SetKnockback(const KamataEngine::Vector3& direction, float power) {
		if (IsInvulnerable())
			return;
		behavior_.RequestHitKnockback(direction, power);
	}
	void SetBlocking(bool blocking) { behavior_.SetBlocking(blocking); }
	bool IsBlocking() const { return behavior_.IsBlocking(); }

	// --- Block hitbox ---
	// The block box is the player's AABB stretched forward (facing side) by kBlockExtraWidth.
	// It is only live while the block button is held and the player isn't attacking/dead/stunned.
	// Attacks must overlap it AND come from the facing side to be blocked; everything else hurts.
	bool IsBlockActive() const;
	AABB GetBlockAABB();
	bool IsBlockingAttack(const AABB& attackBox, const KamataEngine::Vector3& attackerPos);
	// Call instead of TakeDamage() when IsBlockingAttack() returned true. Latches the block direction until the button is released.
	void OnBlocked(const KamataEngine::Vector3& hitDirection, float impactPower, BaseEnemy* attacker = nullptr);
	bool IsBlockLatched() const { return blockLatched_; }

		// --- Block gauge ---
	float GetBlockGauge() const { return blockGauge_; }
	float GetMaxBlockGauge() const { return kMaxBlockGauge; }
	bool IsBlockBroken() const { return blockBroken_; }
	float GetBlockRecoverThreshold() const { return kBlockRecoverThreshold; }

	bool IsStunnedState() const { return behavior_.IsStunned(); }
	void SetControlLocked(bool locked) { controlLocked_ = locked; }
	bool IsControlLocked() const { return controlLocked_; }

	void RequestDropThrough();
	bool IsStandingOnPlatform() const;
	bool IsDroppingThrough() const { return dropThroughTimer_ > 0.0f; }

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
	// Instantly removes stun, knockback, slow effect, control lock and squish progress. yes
	void ClearAllDebuffs();

	// Model layer (cosmetic). Body parts and weapons follow this, not the root.
	const KamataEngine::WorldTransform& GetModelWorldTransform() const { return modelWorldTransform_; }
	KamataEngine::Vector3 GetModelWorldPosition() const { return modelWorldTransform_.translation_; }

	bool IsDefeated() const { return isDead_ || behavior_.IsDeathBehavior(); }

	bool IsOnGround() const { return onGround_; }
	static float GetLimitFallSpeed() { return kLimitFallSpeed; }
	int32_t GetMeleeSourceSpawner() const { return meleeSourceSpawnerID_; }
	void SetMeleeSourceSpawner(int32_t id) { meleeSourceSpawnerID_ = id; }
	int32_t GetFirearmSourceSpawner() const { return firearmSourceSpawnerID_; }
	void SetFirearmSourceSpawner(int32_t id) { firearmSourceSpawnerID_ = id; }
	void ResetToPosition(const KamataEngine::Vector3& position);

	void BeginGoalSequence(const KamataEngine::Vector3& goalPosition);
	bool IsGoalSequenceActive() const { return goalSeqActive_; }

private:
	bool controlLocked_ = false;

	void UpdateTurnRotation();

	// Slow effect state
	bool slowActive_ = false;
	bool slowUntilCleared_ = false;
	float slowFactor_ = 1.0f;
	float slowTimer_ = 0.0f;
	void TickDebuffs();
	bool IsOnSlowTerrain() const;

	// --- 無敵 ---
	void GrantInvulnerability(float seconds); // 残り時間より長い場合のみ上書き(重ね掛けしない)
	void TickInvulnerability();
	float invulnTimer_ = 0.0f;
	static inline float kInvulnJumpLand = 1.0f; // ジャンプ・着地時
	static inline float kInvulnDamage = 3.0f;   // 被弾時
	static inline float kInvulnBlock = 1.0f;    // ブロック成功時

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

	std::unique_ptr<BaseMelee> currentMelee_;
	std::unique_ptr<BaseFirearm> currentFirearm_;

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
	static inline float kJumpAcceleration = 0.65f;

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
	static inline float kBlockExtraWidth = 1.2f;             // how far the block box reaches past the body on the facing side
	bool blockLatched_ = false;                              // set when an attack is blocked: facing is locked until block is released
	PlayerBehavior behavior_{*this};

	MapChipField* mapChipField_ = nullptr;

	static inline float kWidth = 0.8f;
	static inline float kHeight = 1.6f;
	static inline float kBlank = 0.01f;

		// --- Model layer ---
	void UpdateModelCollision();
	BlockDetector detectorProbe_;
	KamataEngine::Vector3 detectorPosition_{};
	KamataEngine::WorldTransform modelWorldTransform_;
	float modelStandOffsetY_ = 0.0f;
	static inline float kModelStandLerp = 0.2f;   // per-frame lerp toward the target offset
	static inline float kModelMaxOffset = 0.5f;   // clamp so the model never detaches from the root
	static inline float kModelProbeDepth = 1.0f;  // how far below the feet to look
	static inline float kModelProbeStepUp = 0.3f; // how far above the feet a surface may be

	static inline float kAttenuationLanding = 0.2f;
	static inline float kAttenuationWall = 0.2f;
	static inline float kAttenuationCeilling = 0.2f;

	bool isDead_ = false;

	// --- SFX ---
	static inline uint32_t sWeaponSwitchSoundHandle_ = 0;
	static inline float sSfxVolume_ = 1.0f;
	static void PlaySfxHandle(uint32_t handle) {
		if (handle != 0) {
			AudioSettings::PlaySfx(handle, false, sSfxVolume_);
		}
	}
	static inline uint32_t sFootstepSoundHandle_ = 0;
	static inline uint32_t sJumpSoundHandle_ = 0;
	static inline uint32_t sLandSoundHandle_ = 0;
	static inline float kMovementSfxVolume = 0.4f;

	static void PlayMovementSfxHandle(uint32_t handle) {
		if (handle != 0) {
			AudioSettings::PlaySfx(handle, false, kMovementSfxVolume);
		}
	}
	SRTOffset GetAttackPoseOffset() const { return behavior_.GetAttackPoseOffset(); }

	AnimationManager animManager_;
	SRTOffset smoothedBodyAnim_ = {};

	float dropThroughTimer_ = 0.0f;

	// プラットフォームを通り抜けるために無視する時間
	static inline float kDropThroughDuration = 0.15f;

	// 落下開始用の小さな下向き速度
	static inline float kDropThroughVelocity = -0.12f;

		// --- Block gauge ---
	void TickBlockGauge();
	float blockGauge_ = kMaxBlockGauge;
	bool blockBroken_ = false;          // true after the gauge hits 0, until it refills to kBlockRecoverThreshold
	float blockRegenDelayTimer_ = 0.0f; // regen pauses briefly after each blocked hit
	static inline float kMaxBlockGauge = 100.0f;
	static inline float kBlockCostPerHit = 20.0f;       // 5 hits empties the bar
	static inline float kBlockRecoverThreshold = 60.0f; // block is usable again at this value (~3 hits' worth)
	static inline float kBlockRegenRate = 15.0f;        // per second
	static inline float kBlockRegenDelay = 1.0f;        // seconds after a blocked hit before regen starts

		void UseBayonetLungeIfReloading();
	void TickLungeCooldowns();
	void UpdateMeleeVisual();

	float meleeLungeCooldown_ = 0.0f;
	float meleeLungeCooldownMax_ = 0.0f;
	float bayonetLungeCooldown_ = 0.0f;
	float bayonetLungeCooldownMax_ = 0.0f;

	// Heavy sabre: tap = combo (fires on release), hold >= kLungeHoldTime = lunge
	bool meleePressPending_ = false;
	float meleeHoldTimer_ = 0.0f;
	static inline float kLungeHoldTime = 0.3f;

	std::vector<const void*> lungeHits_;

	int32_t meleeSourceSpawnerID_ = -1;
	int32_t firearmSourceSpawnerID_ = -1;

	// Fires the current firearm once (projectiles, muzzle flash, sound, recoil knockback). TryFire() must have succeeded.
	void FireCurrentFirearm();

	// Dual pistols: hold the button after the first shot (both loaded) to fire the second pistol too
	bool dualHoldArmed_ = false;
	float dualHoldTimer_ = 0.0f;
	static inline float kDualHoldTime = 0.25f;

	void UpdateGoalSequence();
	bool goalSeqActive_ = false;
	float goalSeqTimer_ = 0.0f;
	KamataEngine::Vector3 goalSeqStart_{};
	KamataEngine::Vector3 goalSeqTarget_{};
	static inline float kGoalMoveDuration = 0.8f;  // seconds to ease into the goal
	static inline float kGoalJumpHeight = 1.8f;    // arc height while travelling
	static inline float kGoalSpinSpeed = 0.35f;    // rad/frame around Y
	static inline float kGoalLoopHopHeight = 0.8f; // hop height after arriving
	static inline float kGoalLoopHopPeriod = 0.6f; // seconds per hop
};