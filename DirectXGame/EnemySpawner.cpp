#include "EnemySpawner.h"
#include "BaseEnemy.h"
#include "GameScene.h"
#include "GlobalVariables.h"

#include <map>
#include <vector>

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "EnemySpawner";
} // namespace

void EnemySpawner::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "SpawnActivateDistance", kSpawnActivateDistance);
}
void EnemySpawner::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kSpawnActivateDistance = gv->GetFloatValue(kGroupName, "SpawnActivateDistance");
}

void EnemySpawner::Initialize(const Vector3& position, uint8_t enemySubID, float interval, SpawnMode mode, int32_t maxSpawnCount) {
	position_ = position;
	enemySubID_ = enemySubID;
	interval_ = interval;
	mode_ = mode;
	maxSpawnCount_ = maxSpawnCount;
	spawnedCount_ = 0;
	timer_ = 0.0f;
	isActive_ = true;
	resolvedEnemyType_ = enemySubID;
}

void EnemySpawner::Update(GameScene* gameScene, const Vector3& playerPosition) {
	if (!isActive_ || !gameScene) {
		return;
	}

	if (mode_ == SpawnMode::kLimited && spawnedCount_ >= maxSpawnCount_) {
		isActive_ = false;
		return;
	}

	float dx = playerPosition.x - position_.x;
	float dy = playerPosition.y - position_.y;
	float distSq = dx * dx + dy * dy;
	if (distSq > kSpawnActivateDistance * kSpawnActivateDistance) {
		return;
	}

	timer_ += 1.0f / 60.0f;
	if (timer_ < interval_) {
		return;
	}
	timer_ = 0.0f;

	// -- SpawnEnemyAt's switch has no case for anything above 2, so any
	// spawner using a slot value outside 0-2 silently spawned nothing.
	gameScene->SpawnEnemyAt(position_, ResolveEnemyType());
	++spawnedCount_;

	if (!gameScene->GetEnemies().empty()) {
		BaseEnemy* justSpawned = gameScene->GetEnemies().back().get();
		justSpawned->SetArenaID(arenaID_);
		if (spawnedEnemiesOmniscient_) {
			justSpawned->ReceiveAlert(playerPosition);
			justSpawned->SetPersistentAwareness(true);
		}
	}

	if (mode_ == SpawnMode::kLimited && spawnedCount_ >= maxSpawnCount_) {
		isActive_ = false;
	}
}