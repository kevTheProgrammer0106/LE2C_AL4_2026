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
	gv->AddItem(kGroupName, "CameraOffset", kCameraOffset);
	gv->AddItem(kGroupName, "WaveWaitDuration", kWaveWaitDuration);
}

void ArenaManager::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kWaveWaitDuration = gv->GetFloatValue(kGroupName, "WaveWaitDuration");
	kCameraOffset = GlobalVariables::GetInstance()->GetVector3Value(kGroupName, "CameraOffset");
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
	// [FIX] Sort ascending by wave slot (the 'S' tile's subID). This is what
	// makes "if S00 isn't there, S10 goes first" work for free -- we never
	// hard-code slot 0 as the start, we just take whatever's registered and
	// sort it, so the lowest subID actually present becomes wave index 0.
	std::sort(waveOrder_.begin(), waveOrder_.end(), [](EnemySpawner* a, EnemySpawner* b) {
		uint8_t slotA = a ? a->GetWaveSlot() : 0xFF;
		uint8_t slotB = b ? b->GetWaveSlot() : 0xFF;
		return slotA < slotB;
	});
	waveOrderBuilt_ = true;
}

void ArenaManager::ActivateCurrentWave() {
	// [FIX] This is the actual fix for "customization wave doesn't work":
	// only the spawner at currentWaveIndex_ turns on. Previously
	// ArenaManager::Activate() called spawner->SetActive(true) on every
	// registered spawner in the same loop, so every wave (S00, S10, S20...)
	// started spawning at the same time instead of one after another.
	for (size_t i = 0; i < waveOrder_.size(); ++i) {
		if (waveOrder_[i]) {
			waveOrder_[i]->SetActive(i == currentWaveIndex_);
		}
	}
}

void ArenaManager::Activate() {
	if (state_ != State::kDormant) {
		return;
	}
	state_ = State::kActive;

	for (SlidingGate* gate : gates_) {
		if (gate)
			gate->Close();
	}

	BuildWaveOrderIfNeeded();
	currentWaveIndex_ = 0;
	waitingForNextWave_ = false;
	waveWaitTimer_ = 0.0f;
	ActivateCurrentWave();
}

Vector3 ArenaManager::ComputeSpawnerCentroid() const {
	if (spawners_.empty()) {
		return {};
	}
	Vector3 sum{};
	int count = 0;
	for (EnemySpawner* spawner : spawners_) {
		if (spawner) {
			sum += spawner->GetPosition();
			++count;
		}
	}
	return count > 0 ? Vector3{sum.x / count, sum.y / count, sum.z / count} : Vector3{};
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

	// --- Advance the wave sequence ---
	// A wave counts as "finished spawning" the moment its spawner exhausts
	// its own spawn count and deactivates itself (see EnemySpawner::Update,
	// the kLimited branch). kEndless spawners never do this on their own,
	// so a wave configured as kEndless is effectively the last wave: the
	// sequence just never advances past it, which is the correct behavior
	// for an "endless" wave.
	if (currentWaveIndex_ < waveOrder_.size()) {
		EnemySpawner* current = waveOrder_[currentWaveIndex_];
		bool currentFinishedSpawning = current && !current->IsActive();

		if (currentFinishedSpawning) {
			if (!waitingForNextWave_) {
				waitingForNextWave_ = true;
				waveWaitTimer_ = 0.0f;
			}
			waveWaitTimer_ += 1.0f / 60.0f;
			if (waveWaitTimer_ >= kWaveWaitDuration) {
				waitingForNextWave_ = false;
				waveWaitTimer_ = 0.0f;
				++currentWaveIndex_;
				if (currentWaveIndex_ < waveOrder_.size()) {
					ActivateCurrentWave(); // turns on S10 (or whichever is next), turns nothing else on
				}
			}
		} else {
			// Current wave is still actively spawning -- not waiting.
			waitingForNextWave_ = false;
			waveWaitTimer_ = 0.0f;
		}
	}

	bool enemiesAlive = AnyEnemiesAliveFromThisArena(gameScene);
	// [FIX] "All spawners depleted" used to mean "none of the registered
	// spawners are currently active" -- which was TRUE while later waves
	// were simply waiting their turn (inactive, not yet started), causing
	// the arena to clear early. It now means "we've stepped past the last
	// wave in the sequence".
	bool allWavesFinished = currentWaveIndex_ >= waveOrder_.size();

	if (!enemiesAlive && allWavesFinished) {
		state_ = State::kCleared;
		waitingForNextWave_ = false;
		for (SlidingGate* gate : gates_) {
			if (gate)
				gate->Open();
		}
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

	// Only count the current wave and waves still queued up -- waves
	// already finished contribute 0 (GetRemainingSpawnCount() is 0 once
	// exhausted anyway, but skipping them explicitly is clearer intent).
	for (size_t i = currentWaveIndex_; i < waveOrder_.size(); ++i) {
		if (waveOrder_[i]) {
			remaining += waveOrder_[i]->GetRemainingSpawnCount();
		}
	}

	return remaining;
}