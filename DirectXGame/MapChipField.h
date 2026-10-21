#pragma once
#include "KamataEngine.h"
#include <vector>

enum class MapChipType {
	kBlank,
	kPlayer,
	kEnemy,
	kGoal,

	kBlock,
	kGlass,          // 'Y'
	kBreakable,      // 'V'
	kExplosive,      // 'Z' passable
	kSpike,          // 'I' passable
	kFragile,        // '9'
	kMeleeSpawner,   // 'Q'
	kFirearmSpawner, // 'O'
	kPlatform,
	kSlowBlock,
	kBarrier,
	kKillBlock,

	kPathNode,
	kJumpNode,

	kGateUp,
	kGateDown,
	kGateLeft,
	kGateRight,
	kSwitch,

	kArenaGateUp,
	kArenaGateDown,
	kArenaGateLeft,
	kArenaGateRight,
	kArenaTrigger, // 'R' — subID unused, linkID = arena ID
	kEnemySpawner, // 'S' — subID = enemy type (0/1/2), linkID = arena ID
	kCheckpoint,   // 'C' — touch trigger, saves progress
	kJumpLandTarget, // 'L' — landing target for a J2 (targeted) jump node; paired via linkID


	kArenaCamera, // 'F' — linkID = arenaID, marks exact camera focus point for that arena
	kHealthBlock, // 'H' — subID: 0 = limited (one-time), non-zero = timer (respawns every N sec)
	kCameraTrigger, // 'G' — walkable (non-solid) trigger, linkID = camera ID to fire
	kTutorialBlock, // 'U' — subID = TutorialTextureType, non-solid
};

enum MapChipCharIndex { kChipType = 0, kChipSubID = 1, kChipLinkID = 2 };

struct MapChipDataUnit {
	MapChipType type;
	uint8_t subID;
	uint8_t linkID = 0;
};

struct TileInfo {
	float width = 0.0f;
	float height = 0.0f;
	KamataEngine::Vector3 offset = {0.0f, 0.0f, 0.0f};
	bool isSolid = false;
	bool isOneWay = false;
	bool isClimbable = false;
	bool isSlow = false;
};

struct MapChipData {
	std::vector<std::vector<MapChipDataUnit>> data;
};

class MapChipField {
public:
	static inline const uint32_t kChunkBlockSize = 100;
	static inline const uint32_t kMaxFieldChunks = 5;
	static inline const uint32_t kMaxVerticalChunks = 3;
	static inline const uint32_t kGroundChunkRow = 1;

	static inline float kSlowSpeedFactor = 0.45f;

	void Initialize(uint32_t chunkCountX, uint32_t chunkCountY = kMaxVerticalChunks);

	void LoadMapChipCsv(const std::string& filePath);
	void LoadChunkCsv(uint32_t chunkIndex, const std::string& filePath);
	void LoadChunkCsv(uint32_t chunkX, uint32_t chunkY, const std::string& filePath);
	void UnloadChunk(uint32_t chunkIndex);

	void ResetMapChipData();

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	uint8_t GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t yIndex);
	uint8_t GetMapChipLinkIDByIndex(uint32_t xIndex, uint32_t yIndex);
	void SetMapChipByIndex(uint32_t xIndex, uint32_t yIndex, MapChipType type, uint8_t subID, uint8_t linkID);

	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	uint32_t GetNumBlockVertical() const;
	uint32_t GetNumBlockHorizontal() const;

	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};
	void ClearMapChipByIndex(uint32_t xIndex, uint32_t yIndex);
	std::vector<IndexSet> CollectConnectedByType(uint32_t xIndex, uint32_t yIndex, MapChipType type);
	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);
	struct Rect {
		float left;
		float right;
		float bottom;
		float top;
	};
	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

	TileInfo GetTileInfo(uint32_t xIndex, uint32_t yIndex);

	bool IsOneWayAtPosition(const KamataEngine::Vector3& position);

	float GetWorldWidth() const { return static_cast<float>(GetNumBlockHorizontal()) * kBlockWidth; }
	float GetWorldHeight() const { return static_cast<float>(GetNumBlockVertical()) * kBlockHeight; }

static bool IsGroundType(MapChipType type) {
		return type == MapChipType::kBlock || type == MapChipType::kPlatform || type == MapChipType::kSlowBlock || type == MapChipType::kGlass || type == MapChipType::kBreakable ||
		       type == MapChipType::kFragile;
	}

	bool IsKillAtPosition(const KamataEngine::Vector3& position);
	bool IsSolidAtPosition(const KamataEngine::Vector3& position, bool ignoreBarrier = false);
	bool IsSlowAtPosition(const KamataEngine::Vector3& position);

private:
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	uint32_t totalBlockVertical_ = kChunkBlockSize;
	uint32_t totalBlockHorizontal_ = kChunkBlockSize;

	MapChipData mapChipData_;

	void LoadCsvIntoRegion(const std::string& filePath, uint32_t xOffset, uint32_t width, uint32_t yOffset = 0, uint32_t height = kChunkBlockSize);
};