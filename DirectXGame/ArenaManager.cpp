#include "ArenaManager.h"
#include "BaseEnemy.h"
#include "EnemySpawner.h"
#include "GameScene.h"
#include "GlobalVariables.h"
#include "SlidingGate.h"

#include <algorithm>

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "ArenaManager";
}

void ArenaManager::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "WaveWaitDuration", kWaveWaitDuration);
}

void ArenaManager::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kWaveWaitDuration = gv->GetFloatValue(kGroupName, "WaveWaitDuration");
}

void ArenaManager::PlaySfx(uint32_t handle) {
	if (handle != 0) {
		Audio::GetInstance()->PlayWave(handle, false, sSfxVolume_);
	}
}
void ArenaManager::RegisterGate(SlidingGate* gate) {
	gates_.push_back(gate);
	if (!gate)
		return;
	if (!gate->IsArenaGate()) {
		if (state_ == State::kCleared)
			gate->SnapOpen(); // arena cleared before: the gate it opens stays open
		return;
	}
	if (state_ == State::kDormant)
		gate->SnapOpen();
	else
		gate->Close();
}
void ArenaManager::Initialize(uint8_t arenaID) {
	arenaID_ = arenaID;
	state_ = State::kDormant;
	waveWaitTimer_ = 0.0f;
	waitingForNextWave_ = false;
	waveOrder_.clear();
	waveOrderBuilt_ = false;
	currentWaveIndex_ = 0;
}

void ArenaManager::BuildWaveOrderIfNeeded() {
	if (waveOrderBuilt_) {
		return;
	}
	waveOrder_ = spawners_;
	// Lowest subID actually present becomes wave 0 (S00 missing -> S10 goes first).
	std::sort(waveOrder_.begin(), waveOrder_.end(), [](EnemySpawner* a, EnemySpawner* b) {
		uint8_t slotA = a ? a->GetWaveSlot() : 0xFF;
		uint8_t slotB = b ? b->GetWaveSlot() : 0xFF;
		return slotA < slotB;
	});
	waveOrderBuilt_ = true;
}

void ArenaManager::ActivateCurrentWave() {
	for (size_t i = 0; i < waveOrder_.size(); ++i) {
		if (waveOrder_[i]) {
			waveOrder_[i]->SetActive(i == currentWaveIndex_);
		}
	}
	if (currentWaveIndex_ < waveOrder_.size()) {
		PlaySfx(sWaveStartSoundHandle_); // wave started
	}
}

void ArenaManager::Activate() {
	if (state_ != State::kDormant) {
		return;
	}
	state_ = State::kActive;

	for (SlidingGate* gate : gates_) {
		if (gate && gate->IsArenaGate())
			gate->Close(); 
	}

	BuildWaveOrderIfNeeded();
	currentWaveIndex_ = 0;
	waitingForNextWave_ = false;
	waveWaitTimer_ = 0.0f;

	PlaySfx(sArenaStartSoundHandle_); // arena triggered
	ActivateCurrentWave();
}

bool ArenaManager::AnyEnemiesAliveFromThisArena(GameScene* gameScene) const {
	for (const std::unique_ptr<BaseEnemy>& enemy : gameScene->GetEnemies()) {
		if (enemy && !enemy->IsDead() && enemy->GetArenaID() == arenaID_) {
			return true;
		}
	}
	return false;
}

void ArenaManager::Update(GameScene* gameScene) {
	if (state_ != State::kActive || !gameScene) {
		return;
	}

	// A wave is "finished" once its spawner exhausts its count and deactivates itself.
	// kEndless spawners never do, so an endless wave is effectively the last wave.
	if (currentWaveIndex_ < waveOrder_.size()) {
		EnemySpawner* current = waveOrder_[currentWaveIndex_];
		bool currentFinishedSpawning = current && !current->IsActive();

		if (currentFinishedSpawning) {
			if (!waitingForNextWave_) {
				waitingForNextWave_ = true;
				waveWaitTimer_ = 0.0f;
				PlaySfx(sWaveEndSoundHandle_); // wave ended (played once per wave)
			}
			waveWaitTimer_ += 1.0f / 60.0f;
			if (waveWaitTimer_ >= kWaveWaitDuration) {
				waitingForNextWave_ = false;
				waveWaitTimer_ = 0.0f;
				++currentWaveIndex_;
				if (currentWaveIndex_ < waveOrder_.size()) {
					ActivateCurrentWave(); // also plays wave-start SFX
				}
			}
		} else {
			waitingForNextWave_ = false;
			waveWaitTimer_ = 0.0f;
		}
	}

	bool enemiesAlive = AnyEnemiesAliveFromThisArena(gameScene);
	bool allWavesFinished = currentWaveIndex_ >= waveOrder_.size();

	if (!enemiesAlive && allWavesFinished) {
		state_ = State::kCleared;
		waitingForNextWave_ = false;
		for (SlidingGate* gate : gates_) {
			if (gate && !gate->IsArenaGate())
				gate->Open();
		}
		PlaySfx(sArenaClearSoundHandle_); // arena over
	}
}

int32_t ArenaManager::GetRemainingEnemyCount(GameScene* gameScene) const {
	int32_t remaining = 0;

	if (gameScene) {
		for (const std::unique_ptr<BaseEnemy>& enemy : gameScene->GetEnemies()) {
			if (enemy && !enemy->IsDead() && enemy->GetArenaID() == arenaID_) {
				++remaining;
			}
		}
	}
	for (size_t i = currentWaveIndex_; i < waveOrder_.size(); ++i) {
		if (waveOrder_[i]) {
			remaining += waveOrder_[i]->GetRemainingSpawnCount();
		}
	}
	return remaining;
}
std::vector<ArenaManager::PendingSpawn> ArenaManager::GetPendingSpawns() {
	BuildWaveOrderIfNeeded();
	std::vector<PendingSpawn> result;
	for (size_t i = currentWaveIndex_; i < waveOrder_.size(); ++i) {
		EnemySpawner* s = waveOrder_[i];
		if (s && s->GetRemainingSpawnCount() > 0)
			result.push_back({s->GetResolvedEnemyType(), s->GetRemainingSpawnCount()});
	}
	return result;
}