#pragma once

#define NOMINMAX

#include "Global.h"
#include <KamataEngine.h>
#include <cstdint>

class GameScene;

class EnemySpawner {
public:
	enum class SpawnMode { kLimited, kEndless };

	void Initialize(const KamataEngine::Vector3& position, uint8_t enemySubID, float interval, SpawnMode mode, int32_t maxSpawnCount = -1);

	void Update(GameScene* gameScene, const KamataEngine::Vector3& playerPosition);

	bool IsActive() const { return isActive_; }
	void SetActive(bool active) { isActive_ = active; }

	int32_t GetSpawnedCount() const { return spawnedCount_; }
	const KamataEngine::Vector3& GetPosition() const { return position_; }

	void SetArenaID(uint8_t arenaID) { arenaID_ = arenaID; }
	uint8_t GetArenaID() const { return arenaID_; }

	uint8_t GetWaveSlot() const { return enemySubID_; }

	void SetSpawnedEnemiesOmniscient(bool omniscient) { spawnedEnemiesOmniscient_ = omniscient; }

	void ForceReset() {
		spawnedCount_ = 0;
		timer_ = 0.0f;
	}

	void SetResolvedEnemyType(uint8_t enemyType) { resolvedEnemyType_ = enemyType; }
	uint8_t GetResolvedEnemyType() const { return resolvedEnemyType_; }
	uint8_t ResolveEnemyType() const { return resolvedEnemyType_; }

	int32_t GetRemainingSpawnCount() const {
		if (mode_ == SpawnMode::kEndless) {
			return 0;
		}
		return (maxSpawnCount_ > spawnedCount_) ? (maxSpawnCount_ - spawnedCount_) : 0;
	}

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

private:
	friend class DebugMenu;
	uint8_t resolvedEnemyType_ = 0xFF; 

	KamataEngine::Vector3 position_{};
	uint8_t enemySubID_ = 0;
	uint8_t arenaID_ = 0;
	float interval_ = 5.0f;
	float timer_ = 0.0f;
	SpawnMode mode_ = SpawnMode::kLimited;
	int32_t maxSpawnCount_ = 1;
	int32_t spawnedCount_ = 0;
	bool isActive_ = true;
	bool spawnedEnemiesOmniscient_ = false;

	static inline float kSpawnActivateDistance = 1000.0f;
};