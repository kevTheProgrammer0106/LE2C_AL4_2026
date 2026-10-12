#pragma once

#define NOMINMAX

#include "Global.h"
#include <KamataEngine.h>
#include <cstdint>
#include <vector>

class GameScene;
class SlidingGate;
class EnemySpawner;

/// Arena = gates + sequential waves + SFX. No camera logic (see CameraController::StartFocus).
class ArenaManager {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
	enum class State { kDormant, kActive, kCleared };

	void Initialize(uint8_t arenaID);
	void Activate();
	void Update(GameScene* gameScene);

	void RestoreCleared() { state_ = State::kCleared; }

	uint8_t GetArenaID() const { return arenaID_; }
	State GetState() const { return state_; }

	void RegisterGate(SlidingGate* gate);
	void RegisterSpawner(EnemySpawner* spawner) {
		spawners_.push_back(spawner);
		waveOrderBuilt_ = false;
	}

	bool IsWaitingForNextWave() const { return waitingForNextWave_; }
	size_t GetCurrentWaveIndex() const { return currentWaveIndex_; }
	size_t GetWaveCount() const { return waveOrder_.size(); }
	int32_t GetRemainingEnemyCount(GameScene* gameScene) const;

	// --- SFX (0 = silent). Set once from GameScene after loading. ---
	static void SetArenaStartSoundHandle(uint32_t h) { sArenaStartSoundHandle_ = h; }
	static void SetWaveStartSoundHandle(uint32_t h) { sWaveStartSoundHandle_ = h; }
	static void SetWaveEndSoundHandle(uint32_t h) { sWaveEndSoundHandle_ = h; }
	static void SetArenaClearSoundHandle(uint32_t h) { sArenaClearSoundHandle_ = h; }
	static void SetSfxVolume(float v) { sSfxVolume_ = v; }

private:
	bool AnyEnemiesAliveFromThisArena(GameScene* gameScene) const;
	void BuildWaveOrderIfNeeded();
	void ActivateCurrentWave();
	static void PlaySfx(uint32_t handle);

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

	static inline uint32_t sArenaStartSoundHandle_ = 0;
	static inline uint32_t sWaveStartSoundHandle_ = 0;
	static inline uint32_t sWaveEndSoundHandle_ = 0;
	static inline uint32_t sArenaClearSoundHandle_ = 0;
	static inline float sSfxVolume_ = 1.0f;
};