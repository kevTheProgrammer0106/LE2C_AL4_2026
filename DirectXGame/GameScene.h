#pragma once
#include "KamataEngine.h"
#include <array>
#include <memory>
#include <set>
#include <vector>

#include "BaseEffect.h"
#include "BaseEnemy.h"
#include "BaseFirearm.h"
#include "BaseMelee.h"

#include "EnemyHUD.h"
#include "Goal.h"
#include "InputManager.h"
#include "SlidingGate.h"
#include "SwitchBlock.h"
#include "HealthBlock.h"

#include "Player.h"

#include "EnemyCuirassier.h"
#include "EnemyFighter.h"
#include "EnemyRifleman.h"

#include "CameraController.h"
#include "DeathParticles.h"
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

#include "DroppableItem.h"
#include "UserInterfaceGameplayHUD.h"

#include "ArenaManager.h"
#include "EnemySpawner.h"
#include "MusicManager.h"

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

	enum class Phase { kFadeIn, kPlay, kPause, kDeath, kRetry, kFadeOut, kVictory };

	void ChangePhase();

	bool isReturnToMenuRequested() const { return returnToMenuRequested_; }

	void CreateHitEffect(KamataEngine::Vector3& spawnLocation, float rotationZ = 0.0f);
	void CreateGuardEffect(KamataEngine::Vector3& spawnLocation);
	void CreateFirearmProjectile(Owner owner, const KamataEngine::Vector3& position, const KamataEngine::Vector2& aimDirection);
	void CreateMuzzleFlash(KamataEngine::Vector3& spawnLocation);
	void CreateBlockSparkEffect(KamataEngine::Vector3& spawnLocation);
	void CreateEnemyDeathParticles(const KamataEngine::Vector3& position);

	void Initialize(StageManager* stageDataManager);
	void Update();
	void Draw();

	bool isFinished() const { return finished_; }
	bool isReloadRequested() const { return reloadRequested_; }   // Retry (checkpoint-aware)
	bool isRestartRequested() const { return restartRequested_; } // Restart (from scratch)

	void debugMenu();

	Player* GetPlayer() const { return player_.get(); }
	const std::vector<std::unique_ptr<BaseEnemy>>& GetEnemies() const { return enemies_; }

	void AlertNearbyEnemies(BaseEnemy* sourceEnemy, const KamataEngine::Vector3& alertPosition, float radius);

	void CreateDroppableItem(const KamataEngine::Vector3& position, DropItemCategory category, DropWeaponKind kind);
	void ApplyItemPickup(const DroppableItem& item);

	void GenerateChunkTileMap(uint32_t chunkIndex);
	void GenerateChunkTileMapRow(uint32_t chunkIndex, uint32_t chunkRow); // [NEW] row-scoped, idempotent
	void UpdateFieldStreaming();
	void UnloadChunkVisuals(uint32_t chunkIndex);
	void TriggerLevelClear();
	bool isLevelCleared() const { return levelCleared_; }

	void SpawnEnemyAt(const KamataEngine::Vector3& position, uint8_t subID);

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
		uint8_t subIDSlot;   // matches the map-chip subID painted on the 'S' tile == which "wave" this is
		uint8_t enemyType;   // 0 = Rifleman, 1 = Cuirassier, 2 = Fighter (0xFF = undeclared/inert custom slot)
		float interval = 5.0f;                                     // seconds between spawns for this wave
		EnemySpawner::SpawnMode mode = EnemySpawner::SpawnMode::kLimited;
		int32_t maxSpawnCount = 3;                                 // ignored when mode == kEndless
	};
	struct ArenaFieldConfig {
		int32_t stageIndex; // which stage this entry applies to
		uint8_t arenaID;  // linkID of the arena (matches 'R'/'S' linkID)
		std::vector<ArenaSpawnEntry> spawnTable; // subID -> wave config, specific to this arena
		bool hasCameraOffsetOverride = false;
		KamataEngine::Vector3 cameraOffsetOverride{}; // overrides ArenaManager::kCameraOffset for this arena only
	};

	// Resolves the full wave configuration (enemy type + interval + spawn mode/count) for
	// (stageIndex, arenaID, subID). subID 1-3 default to the vanilla mapChip mapping
	// (1->Rifleman, 2->Cuirassier, 3->Fighter) with a standard wave unless a spawnTable
	// entry overrides them; subID 0 is a literal legacy fallback. subID >= 4 is fully
	// custom and MUST be declared in spawnTable, otherwise it resolves to an inert wave.
	static ArenaSpawnEntry ResolveArenaSpawnEntry(int32_t stageIndex, uint8_t arenaID, uint8_t subID);
	// Returns true and fills outOffset if a per-arena camera offset override is configured.
	static bool GetArenaCameraOffsetOverride(int32_t stageIndex, uint8_t arenaID, KamataEngine::Vector3& outOffset);

private:
	uint32_t maxFieldChunks_ = 1;
	uint32_t loadedChunkCount_ = 0;
	uint32_t oldestLoadedChunk_ = 0;
	bool levelCleared_ = false;
	static inline float kChunkWorldWidth = MapChipField::kChunkBlockSize * 2.0f;
	static inline float kChunkLoadTriggerMargin = 24.0f;

	std::vector<std::unique_ptr<SwitchBlock>> switches_;
	std::unique_ptr<KamataEngine::Model> modelSwitch_ = nullptr;
	uint32_t switchTex_ = 0;
	uint32_t switchPressedTex_ = 0;

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
	uint32_t skyDomeTextureHandle_ = 0;
	std::unique_ptr<KamataEngine::Model> modelSkyDome_ = nullptr;

	std::vector<std::unique_ptr<BaseEnemy>> enemies_;

	//std::unique_ptr<BaseEnemy> enemyTest;
	std::unique_ptr<KamataEngine::Model> modelEnemyBody_ = nullptr;
	std::unique_ptr<KamataEngine::Model> modelEnemyHead_ = nullptr;
	std::unique_ptr<KamataEngine::Model> modelEnemyHat_ = nullptr;
	uint32_t enemyBodyTex_ = 0;
	uint32_t enemyHeadTex_ = 0;
	uint32_t enemyHatTex_ = 0;

	std::unique_ptr<Player> player_ = nullptr;

	std::unique_ptr<KamataEngine::Model> modelPlayerHead_ = nullptr;
	std::unique_ptr<KamataEngine::Model> modelPlayerHat_ = nullptr;
	std::unique_ptr<KamataEngine::Model> modelPlayerBody_ = nullptr;
	uint32_t playerBodyTex_, playerHeadTex_, playerHatTex_;

	std::unique_ptr<KamataEngine::Model> modelSabreLight_ = nullptr;
	uint32_t texSabreLight_;
	std::unique_ptr<KamataEngine::Model> modelSabreHeavy_ = nullptr;
	uint32_t texSabreHeavy_;

	uint32_t playerTextureHandle_ = 0;
	std::unique_ptr<KamataEngine::Model> modelPlayer_ = nullptr;

	uint32_t slashEffectTex_ = 0;
	std::unique_ptr<KamataEngine::Model> modelSlashEffect_ = nullptr;

	std::vector<std::unique_ptr<BaseEffect>> visualEffects_;
	std::unique_ptr<DeathParticles> deathParticles_ = nullptr;
	std::vector<std::unique_ptr<DeathParticles>> enemyDeathParticles_; // [NEW]
	uint32_t hitEffectTex_ = 0;
	KamataEngine::Model* hitEffectModel_ = nullptr;
	uint32_t guardEffectTex_ = 0;
	std::unique_ptr<KamataEngine::Model> guardEffectModel_ = nullptr;
	uint32_t deathParticleTextureHandle_ = 0;
	std::unique_ptr<KamataEngine::Model> deathParticleModel = nullptr;

	uint32_t blockTextureHandle = 0;
	std::unique_ptr<KamataEngine::Model> blockModel = nullptr;
	uint32_t blockPlatformTextureHandle = 0;
	std::unique_ptr<KamataEngine::Model> blockPlatformModel = nullptr;
	//uint32_t blockHalfUpHandle = 0;
	//std::unique_ptr<KamataEngine::Model> blockHalfUpModel = nullptr;
	//uint32_t blockHalfDownHandle = 0;
	//std::unique_ptr<KamataEngine::Model> blockHalfDownModel = nullptr;

	uint32_t cursorTex_[2] = {0, 0};

	std::unique_ptr<MapChipField> mapChipField_ = nullptr;
	std::vector<std::vector<BlockInstance>> worldTransformBlocks_;

	//uint32_t blockDebugTextureHandle = 0;
	//std::unique_ptr<KamataEngine::Model> blockDebugModel = nullptr;


	std::unordered_map<uint8_t, uint32_t> blockTexVariants_;
	//std::unordered_map<uint8_t, uint32_t> blockHalfUpTexVariants_;
	//std::unordered_map<uint8_t, uint32_t> blockHalfDownTexVariants_;
	std::unordered_map<uint8_t, uint32_t> platformTexVariants_;

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
	uint32_t pauseButtonTex_[3] = {0, 0, 0};
	uint32_t retryButtonTex_[3] = {0, 0, 0};
	uint32_t victoryButtonTex_[2] = {0, 0};

	InputManager pauseMenuInput_;
	InputManager::ButtonID pauseResumeButton_ = InputManager::kInvalidButton;
	InputManager::ButtonID pauseRetryButton_ = InputManager::kInvalidButton;
	InputManager::ButtonID pauseMainMenuButton_ = InputManager::kInvalidButton;

	InputManager gameplayHUDInput_;
	InputManager::ButtonID inGamePauseButton_ = InputManager::kInvalidButton;
	uint32_t inGamePauseButtonTex_ = 0;
	void InitializeInGamePauseButton();
	void UpdateInGamePauseButton();
	void DrawInGamePauseButton();

	InputManager retryMenuInput_;
	InputManager::ButtonID retryButton_ = InputManager::kInvalidButton;
	InputManager::ButtonID restartButton_ = InputManager::kInvalidButton; 
	InputManager::ButtonID retryMainMenuButton_ = InputManager::kInvalidButton;

	InputManager victoryMenuInput_;
	InputManager::ButtonID victoryRestartButton_ = InputManager::kInvalidButton;
	InputManager::ButtonID victoryMainMenuButton_ = InputManager::kInvalidButton;
	std::unique_ptr<KamataEngine::Sprite> victoryOverlay_;

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

	std::unique_ptr<KamataEngine::Model> modelMusketLong_ = nullptr;
	uint32_t musketLongTex_ = 0;
	std::unique_ptr<KamataEngine::Model> modelMusketBayonet_ = nullptr;
	uint32_t musketBayonetTex_ = 0;
	std::unique_ptr<KamataEngine::Model> modelProjectileBall_ = nullptr;
	uint32_t firearmBallTex_ = 0;

	std::vector<std::unique_ptr<BaseProjectile>> firearmProjectiles_;

	InputManager cursor_;

	EnemyHUD enemyHUD_;
	uint32_t enemyIconRiflemanTex_ = 0;
	uint32_t enemyIconCuirassierTex_ = 0;
	uint32_t enemyIconFighterTex_ = 0;
	uint32_t enemyIconArrowTex_ = 0;

	std::unique_ptr<Goal> goal_ = nullptr;
	std::unique_ptr<KamataEngine::Model> modelGoal_ = nullptr;
	uint32_t goalTex_ = 0;

	std::vector<std::unique_ptr<SlidingGate>> gates_;
	std::unique_ptr<KamataEngine::Model> modelGate_ = nullptr;
	uint32_t gateTex_ = 0;

	std::vector<std::unique_ptr<DroppableItem>> droppableItems_;
	std::vector<DroppableItem*> pendingPickups_; 

	struct EnemySpawnPoint {
		KamataEngine::Vector3 position;
		uint8_t subID = 0;
		bool spawned = false;
	};
	std::vector<EnemySpawnPoint> enemySpawnPoints_;
	void UpdateEnemySpawning();

	bool IsWithinActiveRange(const KamataEngine::Vector3& position, float range) const;
	static inline float kEnemySpawnActivateDistance = 25.0f;
	static inline float kEntityCullDistance = 100.0f;

	std::unique_ptr<MapChipField> backgroundMapChipField_ = nullptr;
	std::vector<std::vector<BlockInstance>> worldTransformBackgroundBlocks_;
	static inline float kBackgroundDepthOffset = 6.0f;
	void GenerateBackgroundChunkTileMap(uint32_t chunkX, uint32_t chunkY);

	uint32_t loadedTopChunkRow_ = MapChipField::kGroundChunkRow;
	uint32_t loadedBottomChunkRow_ = MapChipField::kGroundChunkRow;
	static inline float kChunkWorldHeight = MapChipField::kChunkBlockSize * 2.0f;
	static inline float kVerticalChunkLoadTriggerMargin = 24.0f;
	void LoadVerticalRow(uint32_t row);
	bool usesVerticalRows_ = false;

	std::set<std::pair<uint32_t, uint32_t>> generatedChunkRows_; // [NEW] guards against duplicate generation

	InteractionPrompt interactionPrompt_;
	uint32_t interactionPromptTex_ = 0;
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
	ArenaManager* activeArenaLockingCamera_ = nullptr;
	bool arenaReturningToFollow_ = false;
	void UpdateArenaTriggers();
	void UpdateArenas();
	void UpdatePeriodicEnemySpawners();
	struct ArenaCameraPoint {
		KamataEngine::Vector3 position;
		uint8_t arenaID;
	};
	std::vector<ArenaCameraPoint> arenaCameraPoints_;


	// --- Checkpoint ---
	struct CheckpointTriggerPoint {
		KamataEngine::Vector3 position;
		bool consumed = false;
	};
	std::vector<CheckpointTriggerPoint> checkpointTriggers_;
	void UpdateCheckpointTriggers();

	PlayerHealthUI playerHealthUI_;
	uint32_t healthBarBackgroundTex_ = 0;
	uint32_t healthBarFillTex_ = 0;

	WeaponStatusUI weaponStatusUI_;
	uint32_t meleeIconTex_ = 0;
	uint32_t firearmIconTex_ = 0;

	std::vector<std::unique_ptr<HealthBlock>> healthBlocks_;
	std::unique_ptr<KamataEngine::Model> modelHealthBlock_ = nullptr;
	uint32_t healthBlockTex_ = 0;
	void UpdateHealthBlocks();

	static const std::vector<ArenaFieldConfig>& GetArenaFieldConfigs();

	// --- Music ---
	MusicManager musicManager_;
	uint32_t ingameMusicSoundDataHandle_ = 0;
	uint32_t arenaMusicSoundDataHandle_ = 0;

	// --- Misc SFX ---
	uint32_t sfxSwitchPressHandle_ = 0;
	uint32_t sfxGateOpenHandle_ = 0;
	uint32_t sfxGoalReachedHandle_ = 0;
	uint32_t sfxItemPickupHandle_ = 0;
};