#pragma once
#include "KamataEngine.h"
#include <array>
#include <memory>
#include <set>
#include <vector>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "BaseEffect.h"
#include "BaseEnemy.h"
#include "BaseFirearm.h"
#include "BaseMelee.h"

#include "StatusIndicator.h"
#include "Goal.h"
#include "HealthBlock.h"
#include "InputManager.h"
#include "SlidingGate.h"
#include "SwitchBlock.h"

#include "Player.h"

#include "EnemyRifleman.h"
#include "EnemyCuirassier.h"
#include "EnemyFighter.h"
#include "EnemyCarabinier.h"
#include "EnemyShotgunner.h"
#include "EnemyGrenadier.h"
#include "EnemyOfficer.h"
#include "EnemySapper.h"


#include "CameraController.h"
#include "Fade.h"
#include "Global.h"
#include "GuardEffect.h"
#include "HitEffect.h"
#include "MapChipField.h"
#include "ProjectileBall.h"
#include "Skydome.h"

#include "WeaponHeavySabre.h"
#include "WeaponSabre.h"

#include "FirearmMusket.h"
#include "Grenade.h"

#include "DroppableItem.h"
#include "UserInterfaceGameplayHUD.h"
#include "AimGuide.h"

#include "ArenaManager.h"
#include "DebugMenu.h"
#include "EnemySpawner.h"
#include "GameAssets.h"
#include "MusicManager.h"
#include "UIMenu.h"
#include"SettingsPanel.h"
#include"GameSettings.h"

#include "EffectMuzzleFlash.h"
#include "ScreenFlash.h"
#include "EffectParticleBurst.h"
#include "SpriteTutorial.h"

#include "BannerUI.h"
#include "BackgroundModel.h"

class StageManager;

struct BlockVisual {
	KamataEngine::Model* model = nullptr;
	uint32_t textureHandle = 0;
};

struct BlockInstance {
	KamataEngine::WorldTransform* transform = nullptr;
	KamataEngine::Model* model = nullptr;
	uint32_t textureHandle = 0;
};

class GameScene final {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	~GameScene();

	enum class State { Title, StageSelect, GamePlay, GameClear, GameOver };

	void CheckAllCollisions();

	enum class Phase { kFadeIn, kPlay, kPause, kDeath, kRetry, kFadeOut, kGoalSequence, kVictory };

	void ChangePhase();

	bool isReturnToMenuRequested() const { return returnToMenuRequested_; }

	void CreateHitEffect(KamataEngine::Vector3& spawnLocation, float rotationZ = 0.0f);
	void CreateGuardEffect(KamataEngine::Vector3& spawnLocation);
	void CreateBlockSparkEffect(KamataEngine::Vector3& spawnLocation);
	// エンティティ(敵/プレイヤー)の死亡: 控えめな紙吹雪
	void CreateEnemyDeathParticles(const KamataEngine::Vector3& position);
	// public(CreateObjectConfetti を差し替え)
	void CreateObjectSparkler(const KamataEngine::Vector3& position);
	// ゴール到達の紙吹雪(カラフル)
	void CreateGoalConfetti(const KamataEngine::Vector3& position, float loopDuration);
	void CreateMuzzleFlash(const KamataEngine::Vector3& muzzlePos, const KamataEngine::Vector2& aimDir, bool followPlayer);
	// 爆発(煙 + 火の玉 + 火花)を position に出す。smokeLoopDuration >= 0 なら煙のループ継続秒数を上書きする(負=プリセットのまま)
	void CreateExplosion(const KamataEngine::Vector3& position, float smokeLoopDuration = -1.0f);
	

	void CreateFirearmProjectile(
	    Owner owner, const KamataEngine::Vector3& position, const KamataEngine::Vector2& aimDirection, int damage = -1, int32_t lifeFrames = 0, const KamataEngine::Vector3* sweepFrom = nullptr);
	void CreateGrenade(const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity, Owner owner);



	DroppableItem* CreateDroppableItem(const KamataEngine::Vector3& position, DropItemCategory category, DropWeaponKind kind, DropItemState state = DropItemState::kReady);
	void ApplyItemPickup(const DroppableItem& item);


	void Initialize(StageManager* stageDataManager);
	void Update();
	void Draw();

	bool isFinished() const { return finished_; }
	bool isReloadRequested() const { return reloadRequested_; }   // Retry (checkpoint-aware)
	bool isRestartRequested() const { return restartRequested_; } // Restart (from scratch)
	bool isCreditsRequested() const { return creditsRequested_; } // last stage cleared -> credits

	void debugMenu();

	Player* GetPlayer() const { return player_.get(); }
	const std::vector<std::unique_ptr<BaseEnemy>>& GetEnemies() const { return enemies_; }

	void AlertNearbyEnemies(BaseEnemy* sourceEnemy, const KamataEngine::Vector3& alertPosition, float radius);

	void GenerateChunkTileMap(uint32_t chunkIndex);
	void GenerateChunkTileMapRow(uint32_t chunkIndex, uint32_t chunkRow); 
	void UpdateFieldStreaming();
	void UpdateCameraWorldBounds(); // camera limits = the chunks/rows currently loaded
	void UnloadChunkVisuals(uint32_t chunkIndex);
	void TriggerLevelClear();
	bool isLevelCleared() const { return levelCleared_; }

	// Returns the enemy that was created (nullptr if subID has no enemy type). Facing is snapped to the right.
	BaseEnemy* SpawnEnemyAt(const KamataEngine::Vector3& position, uint8_t subID);

	// Placement state painted on an 'E' map chip's link ID (3rd character): E<type><state>, e.g. "E21".
	enum class EnemyPlacementState : uint8_t {
		kWander = 0,     // original behavior (idle/roam)
		kHoldLeft = 1,   // stands still, faces left
		kHoldRight = 2,  // stands still, faces right
		kOmniscient = 4, // always knows where the player is (same awareness as arena-spawned enemies)
	};

	// --- Arena ---
	ArenaManager* GetOrCreateArena(uint8_t arenaID);
	void AddEnemySpawner(const KamataEngine::Vector3& position, uint8_t enemySubID, float interval, EnemySpawner::SpawnMode mode, int32_t maxSpawnCount, bool omniscient);

	// --- Checkpoint ---
	void SetCheckpoint(const KamataEngine::Vector3& position);

	// ============================================================
	// Hard-coded per-stage arena configuration.
	// No external files: every field's arena spawn tables and camera
	// offsets are declared directly in code and looked up by
	// (stageIndex, arenaID/linkID) at load time.
	// ============================================================
	struct ArenaSpawnEntry {
		uint8_t subIDSlot;
		uint8_t enemyType;
		float interval = 5.0f;
		EnemySpawner::SpawnMode mode = EnemySpawner::SpawnMode::kLimited;
		int32_t maxSpawnCount = 3;
	};
	// Arena table: waves only. Cameras live in CameraShotConfig below.
	struct ArenaFieldConfig {
		int32_t stageIndex;
		uint8_t arenaID;
		std::vector<ArenaSpawnEntry> spawnTable;
	};
	// Resolves the full wave configuration (enemy type + interval + spawn mode/count) for
	// (stageIndex, arenaID, subID). subID 1-3 default to the vanilla mapChip mapping
	// (1->Rifleman, 2->Cuirassier, 3->Fighter) with a standard wave unless a spawnTable
	// entry overrides them; subID 0 is a literal legacy fallback. subID >= 4 is fully
	// custom and MUST be declared in spawnTable, otherwise it resolves to an inert wave.
	static ArenaSpawnEntry ResolveArenaSpawnEntry(int32_t stageIndex, uint8_t arenaID, uint8_t subID);
	// Returns true and fills outOffset if a per-arena camera offset override is configured.

	// Camera shot table: keyed by (stage, cameraID). cameraID = linkID of 'F' (focus) and 'G' (trigger) tiles.
	// Camera zone table: keyed by (stage, cameraID). cameraID = linkID of 'F' (zoom point) and 'G' (trigger area) tiles.
	struct CameraShotConfig {
		int32_t stageIndex = 0;
		uint8_t cameraID = 0;
		KamataEngine::Vector3 offset{0.0f, 0.0f, -30.0f}; // added to the 'F' tile position (z = zoom distance)
		float enterDuration = 0.8f;
		float returnDuration = 0.8f;
		CameraController::Ease enterEase = CameraController::Ease::kInOut;
		CameraController::Ease returnEase = CameraController::Ease::kInOut;
		float reentryCooldown = 1.0f; // seconds the player must stay inside after re-entering before the zoom starts again
	};
	void BindAssets();

	void TriggerScreenFlash(ScreenFlash::Type type) { screenFlash_.Trigger(type); }
	void CreateBurstEffect(BurstType type, const KamataEngine::Vector3& position, const KamataEngine::Vector2& dir, float loopDuration = -1.0f);

	bool IsOfficerBuffActive() const { return buffOfficer_ != nullptr; }
	void ApplyOfficerBuff(BaseEnemy* officer);

	// True if pos is within any live grenade's blast radius + margin. Outputs the nearest grenade's position.
	bool FindGrenadeThreat(const KamataEngine::Vector3& pos, float margin, KamataEngine::Vector3& outGrenadePos) const;

	bool HitObstaclesInAABB(const AABB& box, BaseMelee* weapon);
	// One entry point for every camera reaction. intensity scales the preset (e.g. explosion distance falloff, damage size).
	enum class CameraFx { kDamage, kDeath, kExplosion, kBlock, kSwing, kSpecial, kHitEnemy, kShoot, kShootHeavy };
	void TriggerCameraFx(CameraFx fx, float intensity = 1.0f, const KamataEngine::Vector3* focus = nullptr);


private:
	friend class DebugMenu;
	GameAssets assets_;
	DebugMenu debugMenu_;

	static constexpr float kBlockScale = 1.0f;
	static constexpr float kChunkWorldWidth = MapChipField::kChunkBlockSize * kBlockScale;
	static constexpr float kChunkWorldHeight = MapChipField::kChunkBlockSize * kBlockScale;
	uint32_t maxFieldChunks_ = 1;
	uint32_t loadedChunkCount_ = 0;
	uint32_t oldestLoadedChunk_ = 0;
	bool levelCleared_ = false;
	static inline float kChunkLoadTriggerMargin = 24.0f;

	std::vector<std::unique_ptr<SwitchBlock>> switches_;
	std::unique_ptr<KamataEngine::Model> modelSwitch_ = nullptr;

	Phase phase_;
	bool finished_ = false;

	std::unique_ptr<Fade> fade_ = nullptr;

	std::unique_ptr<Fade> loadingCover_ = nullptr;
	bool loadingCoverActive_ = false;
	static inline float kLoadingCoverFadeOutDuration = 0.2f;

	bool isDebugCameraActive_ = false;
	KamataEngine::Camera camera_{};
	std::unique_ptr<CameraController> cameraController_ = nullptr;
	std::unique_ptr<KamataEngine::DebugCamera> debugCamera_ = nullptr;
	KamataEngine::WorldTransform originPoint{};

	std::unique_ptr<Skydome> skyDome_ = nullptr;
	std::unique_ptr<BackgroundModel> background_; // ステージごとの背景モデル

	std::vector<std::unique_ptr<BaseEnemy>> enemies_;

	std::unique_ptr<Player> player_ = nullptr;

	std::vector<std::unique_ptr<BaseEffect>> visualEffects_;

	float deathSceneTimer_ = 0.0f;
	static inline float kDeathSceneDuration = 2.0f; // 旧DeathParticlesのkDuration_と同じ

	std::unique_ptr<MapChipField> mapChipField_ = nullptr;
	std::vector<std::vector<BlockInstance>> worldTransformBlocks_;

	uint32_t ResolveBlockTexture(const std::unordered_map<uint8_t, uint32_t>& table, uint8_t subID, uint32_t defaultTex) {
		auto it = table.find(subID);
		return it != table.end() ? it->second : defaultTex;
	}

	bool reloadRequested_ = false;
	bool restartRequested_ = false;
	StageManager* stageManager_ = nullptr;

	bool returnToMenuRequested_ = false;
	bool pendingReturnToMenu_ = false;

	Phase phaseBeforePause_ = Phase::kPlay;
	UIMenu pauseMenuInput_;
	UIMenu::ButtonID pauseResumeButton_ = UIMenu::kInvalidButton;
	UIMenu::ButtonID pauseRetryButton_ = UIMenu::kInvalidButton;
	UIMenu::ButtonID pauseMainMenuButton_ = UIMenu::kInvalidButton;

	UIMenu gameplayHUDInput_;
	UIMenu::ButtonID inGamePauseButton_ = UIMenu::kInvalidButton;
	void InitializeInGamePauseButton();
	void UpdateInGamePauseButton();
	void DrawInGamePauseButton();

	UIMenu retryMenuInput_;
	UIMenu::ButtonID retryButton_ = UIMenu::kInvalidButton;
	UIMenu::ButtonID restartButton_ = UIMenu::kInvalidButton;
	UIMenu::ButtonID retryMainMenuButton_ = UIMenu::kInvalidButton;

	bool creditsRequested_ = false;
	bool HasNextStage() const;
	void BeginFadeToMenu(); // pendingReturnToMenu_ + fade out (shared by pause / retry / victory)

	UIMenu victoryMenuInput_;
	UIMenu::ButtonID victoryNextButton_ = UIMenu::kInvalidButton;
	UIMenu::ButtonID victoryRestartButton_ = UIMenu::kInvalidButton;
	UIMenu::ButtonID victoryMainMenuButton_ = UIMenu::kInvalidButton;
	std::unique_ptr<KamataEngine::Sprite> victoryOverlay_;

	// ---- メニュー共通の暗転オーバーレイ ----
	std::unique_ptr<KamataEngine::Sprite> menuOverlay_;
	static inline float kMenuOverlayAlpha = 0.6f; // 暗さの最大値(GlobalVariablesで調整可)
	void InitializeMenuOverlay();
	void DrawMenuOverlay(float fadeAlpha); // fadeAlpha: 各メニューのフェード値(0~1)

	SettingsPanel settingsPanel_; // 難易度の行を出さない(タイトルのみ)
	UIMenu settingsMenuInput_; // Backボタン専用
	UIMenu::ButtonID settingsBackButton_ = UIMenu::kInvalidButton;
	UIMenu::ButtonID pauseSettingsButton_ = UIMenu::kInvalidButton;
	uint32_t settingsButtonTex_ = 0;
	bool settingsOpen_ = false; // true の間はポーズメニューの代わりに設定パネルを出す

	// ---- 死亡演出の暗転 ----
	float deathOverlayFade_ = 0.0f;
	static inline float kDeathOverlayFadeDuration = 0.8f;
	void UpdateDeathOverlay();

	void InitializeSettingsPanel();
	void OpenSettingsPanel();
	void CloseSettingsPanel();
	void UpdateSettingsPanel();
	void DrawSettingsPanel();

	static inline float kButtonFadeDuration = 0.25f;
	void InitializeVictoryMenu();
	void UpdateVictoryMenu();
	void DrawVictoryMenu();

	void InitializePauseMenu();
	void InitializeRetryMenu();
	void UpdatePauseMenu();
	void UpdateRetryMenu();
	void DrawPauseMenu();
	void DrawRetryMenu();

	std::vector<std::unique_ptr<ProjectileBall>> playerBullets_;
	std::vector<std::unique_ptr<ProjectileBall>> enemyBullets_;

	std::vector<std::unique_ptr<BaseProjectile>> firearmProjectiles_;

	UIMenu cursor_;

	StatusIndicator statusIndicator_;

	std::unique_ptr<Goal> goal_ = nullptr;

	std::vector<std::unique_ptr<SlidingGate>> gates_;

	std::vector<std::unique_ptr<DroppableItem>> droppableItems_;
	std::vector<DroppableItem*> pendingPickups_;

	struct EnemySpawnPoint {
		KamataEngine::Vector3 position;
		uint8_t subID = 0;
		bool spawned = false;
		uint8_t placementState = 0; // EnemyPlacementState, from the chip's link ID
		uint64_t cellKey = 0;
	};
	void ApplyEnemyPlacementState(BaseEnemy* enemy, uint8_t placementState);
	std::vector<EnemySpawnPoint> enemySpawnPoints_;
	void UpdateEnemySpawning();

	bool IsWithinActiveRange(const KamataEngine::Vector3& position, float range) const;
	static inline float kEnemySpawnActivateDistance = 25.0f;
	static inline float kEntityCullDistance = 100.0f;

	std::unique_ptr<MapChipField> backgroundMapChipField_ = nullptr;
	std::vector<std::vector<BlockInstance>> worldTransformBackgroundBlocks_;
	static inline float kBackgroundDepthOffset = 2.0f;
	void GenerateBackgroundChunkTileMap(uint32_t chunkX, uint32_t chunkY);
	uint32_t loadedTopChunkRow_ = MapChipField::kMainChunkRow;
	uint32_t loadedBottomChunkRow_ = MapChipField::kMainChunkRow;
	static inline float kVerticalChunkLoadTriggerMargin = 24.0f;
	void LoadVerticalRow(uint32_t row);
	bool usesVerticalRows_ = false;

	std::set<std::pair<uint32_t, uint32_t>> generatedChunkRows_;

	InteractionPrompt interactionPrompt_;
	static inline float kInteractionRange = 1.5f;
	void UpdateInteractionPrompt();

	DroppableItem* nearestInteractableItem_ = nullptr;
	SwitchBlock* nearestInteractableSwitch_ = nullptr;
	void TryInteract();

	// --- Arena system ---
	std::vector<std::unique_ptr<ArenaManager>> arenas_;
	struct ArenaTriggerPoint {
		KamataEngine::Vector3 position;
		uint8_t arenaID;
		bool consumed = false;
	};
	std::vector<ArenaTriggerPoint> arenaTriggers_;
	std::vector<std::unique_ptr<EnemySpawner>> enemySpawners_;

	void UpdateArenaTriggers();
	void UpdateArenas();
	void UpdatePeriodicEnemySpawners();

	// --- Checkpoint ---
	struct CheckpointTriggerPoint {
		KamataEngine::Vector3 position;
		bool consumed = false;
	};
	std::vector<CheckpointTriggerPoint> checkpointTriggers_;
	void UpdateCheckpointTriggers();

	PlayerHealthUI playerHealthUI_;

	WeaponStatusUI weaponStatusUI_;

	std::vector<std::unique_ptr<HealthBlock>> healthBlocks_;
	void UpdateHealthBlocks();

	static const std::vector<ArenaFieldConfig>& GetArenaFieldConfigs();

	MusicManager musicManager_;

	void DrawCollisionDebug();
	bool showCollisionDebug_ = true;

	PlayerBlockUI playerBlockUI_;

	ArenaManager* FindActiveArena() const;

	ScreenFlash screenFlash_;

	enum class HitSource { kMelee, kFirearm };
	static uint64_t CellKey(uint32_t x, uint32_t y) { return (static_cast<uint64_t>(x) << 32) | y; }
	void ForEachMapCellInAABB(const AABB& box, const std::function<void(uint32_t, uint32_t)>& fn);

	// ---- glass / breakable ----
	struct BreakableState {
		int32_t hp = 3;
		int32_t maxHp = 3;
	};
	std::unordered_map<uint64_t, BreakableState> breakables_;
	std::set<uint64_t> breakableHitThisSwing_; // cells already hit by the current swing/lunge
	int breakableHitKey_ = -1;                 // identifies the current swing/lunge; a change clears the hit set
	static int BreakableStage(int32_t hp, int32_t maxHp) {
		float ratio = static_cast<float>(hp) / static_cast<float>(maxHp);
		return (ratio > 0.66f) ? 0 : (ratio > 0.33f) ? 1 : 2;
	}
	void RemoveBlockCell(uint32_t xIndex, uint32_t yIndex); // visual + chip + records it as broken
	void BreakGlassConnected(uint32_t xIndex, uint32_t yIndex);
	bool BreakGlassInAABB(const AABB& box);
	void DamageBreakableCell(uint32_t xIndex, uint32_t yIndex, int32_t amount = 1);
	bool DamageBreakablesInAABB(const AABB& box, HitSource source);
	bool DamageExplosivesInAABB(const AABB& box, HitSource source);
	bool HitDestructiblesInAABB(const AABB& box, HitSource source); // glass + breakable + explosive

	// ---- explosive ----
	std::vector<std::unique_ptr<Grenade>> grenades_;
	void UpdateGrenades();

	// One blast routine for barrels and grenades.
	struct ExplosionParams {
		float fatalRadius;
		float normalRadius;
		int32_t normalDamage;
		float impact;
		bool harmEnemies;       // enemies take damage from this blast
		bool killAllInRadius;   // true: enemies die anywhere in normalRadius (barrel). false: die in fatalRadius, take normalDamage outside
	};
	static ExplosionParams BarrelExplosionParams();
	static ExplosionParams GrenadeExplosionParams();
	void ExplodeAt(const KamataEngine::Vector3& center, const ExplosionParams& params);
	std::unordered_map<uint64_t, int32_t> explosives_; // cell -> remaining melee hits
	struct PendingExplosion {
		uint32_t x, y;
		float timer;
		bool flashed = false;
	};
	std::vector<PendingExplosion> pendingExplosions_;
	void QueueExplosion(uint32_t xIndex, uint32_t yIndex, float delay);
	void Explode(uint32_t xIndex, uint32_t yIndex);
	void UpdateExplosions();
	bool IsBlastExposed(const KamataEngine::Vector3& from, const KamataEngine::Vector3& to, int64_t skipX = -1, int64_t skipY = -1);
	static inline float kExplosionFatalRadius = 3.0f;
	static inline float kExplosionNormalRadius = 10.0f;
	static inline int32_t kExplosionNormalDamage = 15;
	static inline float kExplosionChainDelay = 0.15f;
	static inline float kExplosionImpact = 2.5f;
	// debug only: recent blasts (the block is gone, so the radii need their own record)
	struct DebugBlast {
		KamataEngine::Vector3 center;
		float fatalRadius;
		float normalRadius;
		float timer;
	};
	std::vector<DebugBlast> debugBlasts_;
	static inline float kDebugBlastShowTime = 1.5f;

	// ---- spike ----
	struct SpikeTracker {
		bool initialized = false;
		float prevFootY = 0.0f;
		std::unordered_set<uint64_t> immune;
	};
	SpikeTracker playerSpike_;
	std::unordered_map<const BaseEnemy*, SpikeTracker> enemySpike_;
	bool CheckSpikeHit(SpikeTracker& tracker, const AABB& box, bool onGround, float limitFallSpeed);
	void UpdateSpikes();
	static inline int32_t kSpikeDamage = 20;
	static inline float kSpikeLaunchPower = 6.5f;
	static inline float kSpikeMinFallRatio = 0.5f;

	// ---- fragile ----
	std::unordered_map<uint64_t, float> fragileTimers_;
	void TriggerFragileUnder(const AABB& body, bool onGround);
	void UpdateFragileBlocks();
	static inline float kFragileBreakDelay = 3.0f;
	static inline float kFragileShakeAmplitude = 0.08f;

	// -- Renerative Block calls -- //
	struct RegenCell {
		uint32_t x = 0, y = 0;
		MapChipType type = MapChipType::kBlank;
		uint8_t subID = 0;
		uint8_t linkID = 0;
		float timer = 0.0f; // seconds until it comes back
		KamataEngine::Model* model = nullptr;
		uint32_t tex = 0;
		std::unique_ptr<KamataEngine::WorldTransform> transform; // ghost
		std::unique_ptr<KamataEngine::ObjectColor> color;
	};
	std::vector<RegenCell> regenCells_;
	static bool IsRegenerative(MapChipType type, uint8_t subID) { return (type == MapChipType::kExplosive || type == MapChipType::kFragile) && subID == 1; }
	void QueueRegen(uint32_t x, uint32_t y, MapChipType type, uint8_t subID, uint8_t linkID);
	void UpdateRegenCells();
	void DrawRegenGhosts();
	static inline int32_t kExplosiveHits = 3;
	static inline float kExplosiveRegenDelay = 15.0f;
	static inline float kFragileRegenDelay = 10.0f;
	static inline float kRegenBlinkWarning = 3.0f;
	static inline float kRegenBlinkFrequency = 6.0f;

	// ---- weapon spawners ----
	struct WeaponSpawnerPoint {
		KamataEngine::Vector3 position = {};
		DropItemCategory category = DropItemCategory::kMelee;
		DropWeaponKind kind = DropWeaponKind::kSabreLight;
		int32_t id = 0;
		float timer = 0.0f;
	};
	std::vector<WeaponSpawnerPoint> weaponSpawners_;
	int32_t nextWeaponSpawnerID_ = 0;
	static inline float kWeaponRespawnDelay = 3.0f;
	void UpdateWeaponSpawners();
	bool IsWeaponSpawnerItemAlive(int32_t id) const;
	static bool ResolveWeaponSpawnerKind(MapChipType type, uint8_t subID, DropItemCategory& outCategory, DropWeaponKind& outKind);

	// ---- checkpoint-persistent world state ----
	std::set<uint64_t> brokenCells_;
	std::set<uint64_t> killedEnemyCells_;
	std::set<uint64_t> usedHealthCells_;
	std::set<uint8_t> clearedArenas_;
	std::set<uint8_t> pressedSwitchIDs_;
	std::unordered_map<const BaseEnemy*, uint64_t> enemyOrigin_;

	float goalSequenceTimer_ = 0.0f;
	static inline float kGoalSequenceDuration = 1.8f; // time before the victory menu appears
	static inline float kGoalZoomDelta = 10.0f;       // camera moves 10 closer (z offset -10 nearer). Flip the sign to zoom out.
	static inline float kGoalZoomDuration = 0.8f;

	// kGoalZoomDelta / kGoalZoomDuration を削除して、以下に差し替え
	static inline KamataEngine::Vector3 kGoalCameraOffset = {0.0f, 0.8f, -9.0f}; // ゴール位置からのカメラ位置(zがズーム距離)
	static inline float kGoalCameraEnter = 0.6f;
	static inline float kGoalConfettiDuration = 3.0f; // 勝利画面でも降り続ける
	bool goalConfettiSpawned_ = false;

	BaseEnemy* buffOfficer_ = nullptr; // the officer whose life the buff depends on
	void UpdateOfficerBuff();

	static inline float kGuardRecoilScale = 0.3f;

	// ---- 弾と地形/ゲートの判定 ----
	bool IsBulletBlockedAt(const KamataEngine::Vector3& position);
	bool BulletHitsSolid(const ProjectileBall& bullet);
	void CreateBulletImpact(const ProjectileBall& bullet, BurstType type);

	AimGuide aimGuide_;
	void UpdateAimGuide();

	bool AnyAlongBulletPath(const ProjectileBall& bullet, const std::function<bool(const KamataEngine::Vector3&)>& test);
	bool BulletHitsDestructible(const ProjectileBall& bullet);

	void UpdateParticles(); // visual effects + enemy death particles, shared by every phase

	static inline float kShakeDamageAmp = 0.35f;
	static inline float kShakeDamageDuration = 0.3f;
	static inline float kShakeDeathAmp = 1.2f;
	static inline float kShakeDeathDuration = 1.0f;
	static inline float kShakeExplosionAmp = 1.0f;
	static inline float kShakeExplosionDuration = 0.7f;
	static inline float kShakeExplosionRange = 25.0f; // beyond this the player barely feels it
	static inline float kBlockZoomIn = 0.08f;
	static inline float kBlockZoomOut = 0.45f;
	static inline float kShakeSwingAmp = 0.08f;
	static inline float kShakeSwingDuration = 0.12f;
	static inline float kShakeSpecialAmp = 0.18f; // lunge / bayonet / heavy lunge
	static inline float kShakeSpecialDuration = 0.18f;
	static inline float kShakeHitAmp = 0.14f; // player's attack connects
	static inline float kShakeHitDuration = 0.14f;
	static inline float kShakeShootAmp = 0.10f;
	static inline float kShakeShootDuration = 0.10f;
	static inline float kShakeHeavyShootAmp = 0.60f; // blunderbuss
	static inline float kShakeHeavyShootDuration = 0.40f;

	static inline float kBlockZoomAmount = 0.22f; // fraction of the camera distance moved toward the blocker
	static inline float kBlockZoomHold = 0.12f;
	static inline float kBlockZoomMaxDistance = 18.0f; // blocks this far from the player (snipers etc.) don't zoom

	std::vector<std::unique_ptr<SpriteTutorial>> tutorials_;

	BannerUI banner_;
	Phase bannerPrevPhase_ = Phase::kFadeIn; // バナー切り替え用に前フレームのフェーズを覚える
	float resultTimer_ = 0.0f;               // 死亡/ゴール到達からの経過秒数
	bool resultButtonsStarted_ = false;
	static inline float kResultButtonDelay = 3.0f; // 敗北/勝利バナーを見せてからボタンが出るまでの秒数
	static inline float kStartBannerHold = 2.0f;
	static inline float kArenaBannerHold = 2.0f;
	bool officerBannerShown_ = false; // 将校バフのバナーを出したか(初回のみ)
	static inline float kOfficerBannerHold = 2.0f;
	void UpdateBanners();
	void UpdateResultButtons();

	void UpdateBulletTrails();
	void UpdateMeleeTrails();
	std::unordered_map<const BaseMelee*, KamataEngine::Vector3> meleeTrailPrev_;
	static inline float kFxDepthOffset = -0.2f;     // 新しいエフェクトをカメラ側へ寄せる量
	static inline float kBlastSmokeDuration = 2.5f; // 爆風を受けた体が煙を出す最大秒数

	// includes: already has UserInterfaceGameplayHUD.h
	// members (next to playerHealthUI_):
	BossHealthBar bossHealthBar_;
	SwitchPointer switchPointer_;
	float bossBarMaxHP_ = 0.0f;
	std::array<int32_t, 16> enemyTypeMaxHP_{}; // learned from the first spawn of each type
	// methods:
	void UpdateBossHealthBar();

	uint32_t BurstTexture(BurstType type) const;
	// Dust/fragments for a destructible block. broke=true also spawns the shard particles.
	void CreateBlockDebris(const KamataEngine::Vector3& cellPos, bool broke);
	template<typename T> T* SpawnEnemyOfType(const KamataEngine::Vector3& position, const std::array<PartResources, 3>& parts, uint32_t hudIcon);

	// time between a barrel being triggered and exploding; the flash plays during it
	static inline float kExplosionFuseTime = 0.3f;

	// private に追加
	// ---- スポーン演出(レベル開始 / チェックポイント復帰) ----
	void BeginSpawnSequence();
	void UpdateSpawnSequence();
	bool spawnSequenceActive_ = false;
	float spawnSequenceTimer_ = 0.0f;
	static inline KamataEngine::Vector3 kSpawnCameraOffset = {0.0f, 0.8f, -9.0f}; // プレイヤー位置からのカメラ位置(zがズーム距離)
	static inline float kSpawnCameraEnter = 0.25f;                                // 寄る時間
	static inline float kSpawnCameraHold = 0.35f;                                 // 寄ったまま止まる時間
	static inline float kSpawnCameraReturn = 0.40f;                               // 元のカメラへ戻る時間(合計1秒)

	// private に追加(kSpawnCameraReturn の下)
	static inline float kSpawnInvulnerability = 5.0f; // スポーン直後の無敵時間(秒)

	// 操作説明スプライト
	void InitializeControlSprites();
	void DrawControlSprites();

	std::array<std::unique_ptr<KamataEngine::Sprite>, 8> controlSprites_{};
};