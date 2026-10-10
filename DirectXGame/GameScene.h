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
#include "HealthBlock.h"
#include "InputManager.h"
#include "SlidingGate.h"
#include "SwitchBlock.h"

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
#include "DebugMenu.h"
#include "EnemySpawner.h"
#include "GameAssets.h"
#include "MusicManager.h"
#include "UIMenu.h"

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
	void CreateMuzzleFlash(KamataEngine::Vector3& spawnLocation);
	void CreateBlockSparkEffect(KamataEngine::Vector3& spawnLocation);
	void CreateEnemyDeathParticles(const KamataEngine::Vector3& position);

	void CreateFirearmProjectile(Owner owner, const KamataEngine::Vector3& position, const KamataEngine::Vector2& aimDirection, int damage = -1, int32_t lifeFrames = 0);
	DroppableItem* CreateDroppableItem(const KamataEngine::Vector3& position, DropItemCategory category, DropWeaponKind kind, DropItemState state = DropItemState::kReady);
	void ApplyItemPickup(const DroppableItem& item);


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

	void GenerateChunkTileMap(uint32_t chunkIndex);
	void GenerateChunkTileMapRow(uint32_t chunkIndex, uint32_t chunkRow); // [NEW] row-scoped, idempotent
	void UpdateFieldStreaming();
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

	std::vector<std::unique_ptr<BaseEnemy>> enemies_;

	std::unique_ptr<Player> player_ = nullptr;

	std::vector<std::unique_ptr<BaseEffect>> visualEffects_;
	std::unique_ptr<DeathParticles> deathParticles_ = nullptr;
	std::vector<std::unique_ptr<DeathParticles>> enemyDeathParticles_;

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

	UIMenu victoryMenuInput_;
	UIMenu::ButtonID victoryRestartButton_ = UIMenu::kInvalidButton;
	UIMenu::ButtonID victoryMainMenuButton_ = UIMenu::kInvalidButton;
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

	std::vector<std::unique_ptr<BaseProjectile>> firearmProjectiles_;

	UIMenu cursor_;

	EnemyHUD enemyHUD_;

	std::unique_ptr<Goal> goal_ = nullptr;

	std::vector<std::unique_ptr<SlidingGate>> gates_;

	std::vector<std::unique_ptr<DroppableItem>> droppableItems_;
	std::vector<DroppableItem*> pendingPickups_;

	struct EnemySpawnPoint {
		KamataEngine::Vector3 position;
		uint8_t subID = 0;
		bool spawned = false;
		uint8_t placementState = 0; // EnemyPlacementState, from the chip's link ID
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
	uint32_t loadedTopChunkRow_ = MapChipField::kGroundChunkRow;
	uint32_t loadedBottomChunkRow_ = MapChipField::kGroundChunkRow;
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
};