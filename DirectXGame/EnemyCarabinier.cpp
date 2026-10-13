#include "EnemyCarabinier.h"

void EnemyCarabinier::Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	EnemyRifleman::Initialize(partResources, camera, position);
	SetMaxHP(kCarabinerHP);
	SetHP(kCarabinerHP);
}