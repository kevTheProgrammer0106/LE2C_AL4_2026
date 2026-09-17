#pragma once

#define NOMINMAX

#include "Global.h"
#include <KamataEngine.h>
#include <cstdint>
#include <vector>

class GameScene;
class SlidingGate;
class EnemySpawner;

class ArenaManager {
public:
	void SetCameraFocusOverride(const KamataEngine::Vector3& pos) {
		cameraFocusOverride_ = pos;
		hasCameraFocusOverride_ = true;
	}
	KamataEngine::Vector3 GetCameraFocusPoint() const { return hasCameraFocusOverride_ ? cameraFocusOverride_ : ComputeSpawnerCentroid(); }
	void SetCameraOffsetOverride(const KamataEngine::Vector3& offset) {
		cameraOffsetOverride_ = offset;
		hasCameraOffsetOverride_ = true;
	}
	KamataEngine::Vector3 GetEffectiveCameraOffset() const { return hasCameraOffsetOverride_ ? cameraOffsetOverride_ : kCameraOffset; }

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
	enum class State { kDormant, kActive, kCleared };

	void Initialize(uint8_t arenaID);
	void Activate();
	void Update(GameScene* gameScene);

	uint8_t GetArenaID() const { return arenaID_; }
	State GetState() const { return state_; }
	static KamataEngine::Vector3 GetCameraOffset() { return kCameraOffset; }

	void RegisterGate(SlidingGate* gate) { gates_.push_back(gate); }
	// [FIX] Registering a spawner after the wave order was already built
	// (e.g. streamed in from a later-loaded chunk) must invalidate the
	// cached order so the new spawner gets folded into the sequence.
	void RegisterSpawner(EnemySpawner* spawner) {
		spawners_.push_back(spawner);
		waveOrderBuilt_ = false;
	}

	KamataEngine::Vector3 ComputeSpawnerCentroid() const;

	// True while the current wave has finished spawning and we're pausing
	// (kWaveWaitDuration) before the next wave's spawner turns on.
	bool IsWaitingForNextWave() const { return waitingForNextWave_; }
	// Enemies still alive from this arena + everything the current and
	// not-yet-started waves still owe (kEndless spawners don't count,
	// their total is unbounded).
	int32_t GetRemainingEnemyCount(GameScene* gameScene) const;

private:
	KamataEngine::Vector3 cameraFocusOverride_{};
	bool hasCameraFocusOverride_ = false;
	static inline KamataEngine::Vector3 kCameraOffset = {0.0f, 0.0f, -30.0f};

	KamataEngine::Vector3 cameraOffsetOverride_{};
	bool hasCameraOffsetOverride_ = false;

	bool AnyEnemiesAliveFromThisArena(GameScene* gameScene) const;

	// ============================================================
	// Sequential wave system.
	// Spawners are placed as 'S' tiles whose subID is the wave *order*
	// within the arena (S00 -> wave 0, S10 -> wave 1, S20 -> wave 2, all
	// sharing the same linkID == arenaID). Only the spawner whose turn it
	// is is ever active. When it finishes spawning (its spawn count is
	// exhausted -> it deactivates itself), we wait kWaveWaitDuration
	// seconds and activate the next spawner in ascending subID order.
	// Whichever subID is actually present with the lowest value goes
	// first -- if S00 isn't placed in the field, S10 starts the sequence
	// instead, with no gap or dead wave in between.
	// ============================================================
	void BuildWaveOrderIfNeeded();
	void ActivateCurrentWave();

	std::vector<EnemySpawner*> waveOrder_; // spawners_ sorted ascending by GetWaveSlot()
	bool waveOrderBuilt_ = false;
	size_t currentWaveIndex_ = 0;

	uint8_t arenaID_ = 0;
	State state_ = State::kDormant;

	std::vector<SlidingGate*> gates_;
	std::vector<EnemySpawner*> spawners_;

	float waveWaitTimer_ = 0.0f;
	static inline float kWaveWaitDuration = 3.0f;
	bool waitingForNextWave_ = false;
};