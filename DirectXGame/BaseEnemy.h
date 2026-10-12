#pragma once

#define NOMINMAX

#include "AnimationManager.h"
#include "BaseEnemyBehavior.h"
#include "BlockDetector.h"
#include "Global.h"
#include "PathFindingAI.h"

class Player;
class GameScene;
class BaseProjectile;

class BaseEnemy {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	enum class LRDirection { kRight, kLeft };
	bool isLeft() const { return lrDirection_ == LRDirection::kLeft; }
	bool isRight() const { return lrDirection_ == LRDirection::kRight; }
	LRDirection GetLRDirection() const { return lrDirection_; }

	using Behavior = BaseEnemyBehavior::Behavior;
	using FlankPreference = BaseEnemyBehavior::FlankPreference;

	enum class FirearmMode { kHolster, kArmed, kReloading, kMelee };
	FirearmMode GetFirearmMode() const { return firearmMode_; }
	enum class MeleeMode { kHolster, kReady, kBlock };
	MeleeMode GetMeleeMode() const { return meleeMode_; }
	enum class ThrowableMode { kHolster, kReady, kReloading };
	ThrowableMode GetThrowableMode() const { return throwableMode_; }

	friend class PathFindingAI;
	friend class BaseEnemyBehavior;

	virtual ~BaseEnemy() = default;

	virtual void Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	virtual void Update();
	virtual void Draw();

	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }
	void SetMapChipField(MapChipField* mapChipField) {
		mapChipField_ = mapChipField;
		if (behavior_)
			behavior_->SetMapChipField(mapChipField);
		if (pathfindingModule_)
			pathfindingModule_->SetMapChipField(mapChipField);
	}

	const KamataEngine::WorldTransform& GetWorldTransform() const;
	void SetWorldTransform(const KamataEngine::WorldTransform& worldTransform);
	KamataEngine::Vector3 GetWorldPosition() const;

	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

	AABB GetAABB();
	AABB GetAABB() const;

	bool IsDead() const { return isDead_; }
	bool IsCollisionDisabled() const { return isCollisionDisabled_; }
	bool IsDebug() const { return isDebug_; }
	void SetDebug(bool debug) { isDebug_ = debug; }

	virtual void CheckPlayerDirection();

	virtual void OnCollision(const Player* player);
	virtual void OnCollision(const BaseProjectile* bullet);

	virtual bool WasBlocked() const { return wasBlocked_; }

	bool IsPlayerSpotted() const { return behavior_->IsPlayerSpotted(); }
	const VisionSensor& GetVisionSensor() const { return behavior_->GetVisionSensor(); }

	void SetFlankPreference(FlankPreference preference) { behavior_->SetFlankPreference(preference); }
	FlankPreference GetFlankPreference() const { return behavior_->GetFlankPreference(); }

	PathFindingAlgorithm::PathCostProfile BuildPathCostProfile() const { return behavior_->BuildPathCostProfile(); }

	void ReceiveAlert(const KamataEngine::Vector3& alertedPosition) { behavior_->ReceiveAlert(alertedPosition); }
	void SetPersistentAwareness(bool value) { behavior_->SetPersistentAwareness(value); }

	virtual void UpdateMovementState();
	virtual SRTOffset GetMovementAnimOffset() const;

	static void SetEndlessChaseEnabled(bool enabled) { BaseEnemyBehavior::SetEndlessChaseEnabled(enabled); }
	static bool IsEndlessChaseEnabled() { return BaseEnemyBehavior::IsEndlessChaseEnabled(); }

	BaseEnemyBehavior& GetBehaviorModule() { return *behavior_; }
	const BaseEnemyBehavior& GetBehaviorModule() const { return *behavior_; }
	AnimationManager& GetAnimationManager() { return *animManager_; }
	const AnimationManager& GetAnimationManager() const { return *animManager_; }

	float GetAABBMarkerSize() const { return aabbMarkerSize_; }
	void CacheAABBMarkerSize();

	void SetHUDIconTexture(uint32_t textureHandle) { hudIconTexture_ = textureHandle; }
	uint32_t GetHUDIconTexture() const { return hudIconTexture_; }

	virtual bool GetDropInfo(DropItemCategory& outCategory, DropWeaponKind& outKind) const {
		outCategory = DropItemCategory::kMelee;
		outKind = DropWeaponKind::kSabreLight;
		return false;
	}
	void SpawnDropItem();
	virtual void KillInstant();

	void TakeDamage(int damage, const KamataEngine::Vector3& hitDirection, float impactPower);
	int GetHP() const { return hp_; }
	int GetMaxHP() const { return maxHP_; }
	void SetHP(int hp) { hp_ = std::clamp(hp, 0, maxHP_); }
	void SetMaxHP(int maxHp) { maxHP_ = maxHp; }

	void RequestKnockback(const KamataEngine::Vector3& direction, float power) { behavior_->RequestKnockback(direction, power); }
	void RequestStun(float duration) { behavior_->RequestStun(duration); }
	void ClearDebuffs() { behavior_->ClearDebuffs(); }

	void SetArenaID(uint8_t id) { arenaID_ = id; }
	uint8_t GetArenaID() const { return arenaID_; }

	static float GetBodyWidth() { return kWidth; }
	static float GetBodyHeight() { return kHeight; }

	// 近接攻撃(スイング)中に出す斬撃エフェクトのモデル/テクスチャ(プレイヤーと同じものを渡す)
	void SetAttackEffect(KamataEngine::Model* model, uint32_t textureHandle) {
		modelAttack_ = model;
		modelAttackTex_ = textureHandle;
		worldTransformAttack_.Initialize();
	}

	// --- SFX ---
	// デフォルトの被弾/死亡SFX(全敵共通のフォールバック)。GameSceneがロード後にセットする。
	static void SetDefaultHurtSoundHandle(uint32_t handle) { sDefaultHurtSoundHandle_ = handle; }
	static void SetDefaultDeathSoundHandle(uint32_t handle) { SetDefaultDeathSoundHandle(0, handle); }
	static void SetDefaultDeathSoundHandle(int index, uint32_t handle) {
		if (index >= 0 && index < static_cast<int>(sDefaultDeathSoundHandles_.size())) {
			sDefaultDeathSoundHandles_[index] = handle;
		}
	}
	static void SetSfxVolume(float volume) { sSfxVolume_ = volume; }
	static void SetDefaultFootstepSoundHandle(uint32_t handle) { sDefaultFootstepSoundHandle_ = handle; }
	static void SetDefaultJumpSoundHandle(uint32_t handle) { sDefaultJumpSoundHandle_ = handle; }
	static void SetDefaultLandSoundHandle(uint32_t handle) { sDefaultLandSoundHandle_ = handle; }

	// Light = equips while still moving. Heavy = enemy stops while equipping/unequipping.
	enum class EquipStyle { kLight, kHeavy };
	virtual EquipStyle GetEquipStyle() const { return EquipStyle::kLight; }

	bool IsWeaponEquipped() const { return weaponEquipped_; }
	// True while a heavy equip/unequip is in progress (movement is held).
	bool IsEquipLocked() const { return equipLockTimer_ > 0.0f; }
	// Arena-spawned enemies: no equip states, weapon is always ready and never locks movement.
	void SetIgnoreEquipStates(bool ignore);

	// モデル層(見た目専用)。ボディパーツと武器はルートではなくこちらに追従する
	const KamataEngine::WorldTransform& GetModelWorldTransform() const { return modelWorldTransform_; }
	KamataEngine::Vector3 GetModelWorldPosition() const { return modelWorldTransform_.translation_; }

	// 一方通行足場の下り(ドロップスルー)
	void RequestDropThrough();
	bool IsStandingOnPlatform() const;
	bool IsDroppingThrough() const { return dropThroughTimer_ > 0.0f; }

	bool TryBlockAttack(const KamataEngine::Vector3& attackerPos);
	virtual SRTOffset GetActionAnimOffset() const { return attackBodyPose_; }

	bool IsOnGround() const { return onGround_; }
	static float GetLimitFallSpeed() { return kLimitFallSpeed; }

protected:
	friend class DebugMenu;

	ReactionPose GetReactionPose() const {
		if (!behavior_)
			return ReactionPose::kNone;
		if (behavior_->IsKnockedBack())
			return ReactionPose::kKnockback;
		if (behavior_->IsStunned())
			return ReactionPose::kStun;
		if (behavior_->GetBehavior() == Behavior::kBlock)
			return ReactionPose::kBlock;
		return ReactionPose::kNone;
	}

	int hp_ = 100;
	int maxHP_ = 100;
	KamataEngine::Vector3 pendingHitDirection_ = {1.0f, 0.3f, 0.0f};
	float pendingImpactPower_ = 1.0f;

	uint8_t arenaID_ = 0; 

	void MapChipCollisionUpdate();
	void MapChipCollisionUpdate(CollisionInfo& collisionInfo);
	bool PerformJump();

	bool hitWallThisFrame_ = false;

	BlockDetector blockDetector_;

	float aabbMarkerSize_ = 0.1f;
	bool wasBlocked_ = false;
	uint32_t hudIconTexture_ = 0;

	void PositionUpdate(const CollisionInfo& collisionInfo);
	void CeillingCollision(const CollisionInfo& collisionInfo);
	void WallCollision(const CollisionInfo& collisionInfo);
	void GroundSwitch(const CollisionInfo& collisionInfo);

	virtual void UpdateBodyParts();
	void PhysicsAndCollisionUpdate();
	virtual void JumpMovement(const CollisionInfo& collisionInfo);

	virtual std::unique_ptr<BaseEnemyBehavior> CreateBehaviorModule() { return std::make_unique<BaseEnemyBehavior>(*this); }
	virtual std::unique_ptr<AnimationManager> CreateAnimationManager() { return std::make_unique<AnimationManager>(); }

	bool isDebug_ = false;
	bool isDead_ = false;
	bool isCollisionDisabled_ = false;

	GameScene* gameScene_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Camera* camera_ = nullptr;

	static constexpr size_t kPartCount = static_cast<size_t>(PartSlot::kCount);
	static constexpr size_t body = static_cast<size_t>(PartSlot::kBody);
	static constexpr size_t head = static_cast<size_t>(PartSlot::kHead);
	static constexpr size_t hat = static_cast<size_t>(PartSlot::kHat);

	std::array<BodyPart, kPartCount> parts_;

	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;
	static inline float kTimeTurn = 0.3f;

	LRDirection lrDirection_ = LRDirection::kRight;

	KamataEngine::Vector3 velocity_ = {0, 0, 0};
	static inline float kAcceleration = 0.05f;
	static inline float kAttenuation = 0.1f;
	static inline float kLimitRunSpeed = 0.025f;

	static inline float kWalkSpeed = 0.02f;
	static inline float kChaseSpeed = 0.1f;

	static inline float kWalkMotionAngleStart = 0.0f;
	static inline float kWalkMotionAngleEnd = 1.2f;
	static inline float kWalkMotionTime = 0.6f;

	float walkTimer_ = 0.0f;
	bool onGround_ = true;

	static inline float kGravityAcceleration = 0.05f;
	static inline float kLimitFallSpeed = 0.5f;
	static inline float kJumpAcceleration = 0.65f;

	MapChipField* mapChipField_ = nullptr;
	static inline float kWidth = 0.8f;
	static inline float kHeight = 1.6f;
	static inline float kBlank = 0.01f;

		// --- モデル層 ---
	void UpdateModelCollision();
	BlockDetector detectorProbe_; // 詳細タイルを読むディテクター(ルートは blockDetector_ = 簡易タイル)
	KamataEngine::WorldTransform modelWorldTransform_;
	float modelStandOffsetY_ = 0.0f;
	static inline float kModelStandLerp = 0.2f;   // 毎フレームの補間率
	static inline float kModelMaxOffset = 0.5f;   // モデルがルートから離れすぎないための上限
	static inline float kModelProbeDepth = 1.0f;  // 足元から下をどこまで調べるか
	static inline float kModelProbeStepUp = 0.3f; // 足元より上の面をどこまで許容するか

	// --- ドロップスルー ---
	float dropThroughTimer_ = 0.0f;
	static inline float kDropThroughDuration = 0.15f;  // 足場を無視する時間
	static inline float kDropThroughVelocity = -0.12f; // 落下開始の初速

	static inline float kAttenuationLanding = 0.2f;
	static inline float kAttenuationWall = 0.2f;
	static inline float kAttenuationCeilling = 0.2f;

	FirearmMode firearmMode_ = FirearmMode::kHolster;
	MeleeMode meleeMode_ = MeleeMode::kHolster;
	ThrowableMode throwableMode_ = ThrowableMode::kHolster;

	void FaceDirection(LRDirection direction);

	std::unique_ptr<PathFindingAI> pathfindingModule_ = nullptr;

	std::unique_ptr<AnimationManager> animManager_;
	std::unique_ptr<BaseEnemyBehavior> behavior_;

	// --- SFX ---
	static inline uint32_t sDefaultHurtSoundHandle_ = 0;
	//  up to 3 death SFX variations; unset slots stay 0 and are skipped
	static inline std::array<uint32_t, 3> sDefaultDeathSoundHandles_ = {};
	static inline float sSfxVolume_ = 1.0f;

	static inline uint32_t sDefaultFootstepSoundHandle_ = 0;
	static inline uint32_t sDefaultJumpSoundHandle_ = 0;
	static inline uint32_t sDefaultLandSoundHandle_ = 0;
	static inline float kMovementSfxVolume = 0.4f; 

	// virtual: 派生クラス(EnemyCuirassier)が独自のSFXを鳴らしたければoverrideする
	virtual void PlayHurtSound() {
		if (sDefaultHurtSoundHandle_ != 0) {
			AudioSettings::PlaySfx(sDefaultHurtSoundHandle_, false, sSfxVolume_);
		}
	}
	//  picks one of up to 3 registered death SFX variations at random,
	// so repeated enemy deaths don't all sound identical. Only non-zero
	// (i.e. actually-set) handles are considered; if none are set, this is
	// silently a no-op just like before.
	virtual void PlayDeathSound() {
		std::array<uint32_t, 3> candidates{};
		size_t count = 0;
		for (uint32_t handle : sDefaultDeathSoundHandles_) {
			if (handle != 0) {
				candidates[count++] = handle;
			}
		}
		if (count == 0) {
			return;
		}
		uint32_t chosen = candidates[static_cast<size_t>(std::rand()) % count];
		AudioSettings::PlaySfx(chosen, false, sSfxVolume_);
	}
	// 移動アニメ(足音等)のSFXフック。デフォルトは何もしない。
	// Cuirassierだけこれをoverrideして重量感のある足音を鳴らす。他の敵は現状無音のまま。

		virtual void PlayFootstepSound() {
		if (sDefaultFootstepSoundHandle_ != 0) {
		AudioSettings::PlaySfx(sDefaultFootstepSoundHandle_, false, kMovementSfxVolume);
		}
	}
	virtual void PlayJumpSound() {
		if (sDefaultJumpSoundHandle_ != 0) {
			AudioSettings::PlaySfx(sDefaultJumpSoundHandle_, false, kMovementSfxVolume);
		}
	}
	virtual void PlayMovementSound() {
		if (sDefaultLandSoundHandle_ != 0) {
			AudioSettings::PlaySfx(sDefaultLandSoundHandle_, false, kMovementSfxVolume);
		}
	}

	// Called every frame while stunned/knocked back: drops stale attack hitboxes and keeps the held weapon attached.
	virtual void RefreshWeaponVisual() {}
	void UpdateWeaponEquipState();
	bool ShouldWeaponBeEquipped() const;
	virtual void ApplyWeaponEquipState(bool /*equipped*/) {}
	virtual void SnapWeaponEquipState(bool /*equipped*/) {}

	bool weaponEquipped_ = false;
	bool ignoreEquipStates_ = false;
	float equipLockTimer_ = 0.0f;
	static inline float kHeavyEquipLockDuration = 1.5f;

	AnimationManager::AnimationContext BuildAnimationContext() const;

	// Written each frame by melee enemies' UpdateMeleeVisual / UpdateLungeVisual from their own pose set.
	SRTOffset attackBodyPose_{};

	// 斬撃エフェクトを描画する。active=スイングの攻撃フェーズ中。向きと位置は描画時の最新状態から決める
	void DrawAttackEffect(bool active, const AABB& swingBox);

	KamataEngine::Model* modelAttack_ = nullptr;
	KamataEngine::WorldTransform worldTransformAttack_;
	uint32_t modelAttackTex_ = 0;
	static inline float kAttackEffectScale = 1.0f;  // 斬撃モデルの大きさ(判定の中心に置いたまま拡縮)
};