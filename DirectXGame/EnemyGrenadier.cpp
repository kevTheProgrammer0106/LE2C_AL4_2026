#include "EnemyGrenadier.h"
#include "GameScene.h"
#include "GlobalVariables.h"
#include "Grenade.h"
#include "Player.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "EnemyGrenadier";
}

void EnemyGrenadier::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "MaxHP", kGrenadierHP);
	gv->AddItem(kGroupName, "ThrowPoseDuration", kThrowPoseDuration);
	GrenadierBehavior::RegisterGlobalVariables();
}
void EnemyGrenadier::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kGrenadierHP = gv->GetIntValue(kGroupName, "MaxHP");
	kThrowPoseDuration = gv->GetFloatValue(kGroupName, "ThrowPoseDuration");
	GrenadierBehavior::ApplyGlobalVariables();
}

void EnemyGrenadier::Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, Camera* camera, const Vector3& position) {
	EnemyFighter::Initialize(partResources, camera, position);
	SetMaxHP(kGrenadierHP);
	SetHP(kGrenadierHP);
}

void EnemyGrenadier::Update() {
	EnemyFighter::Update();
	throwPoseTimer_ = std::min(throwPoseTimer_ + 1.0f / 60.0f, 999.0f);
}

void EnemyGrenadier::ThrowGrenade() {
	if (!gameScene_ || !gameScene_->GetPlayer()) {
		return;
	}
	const Vector3 playerPos = gameScene_->GetPlayer()->GetWorldPosition();
	const bool right = playerPos.x > GetWorldPosition().x; // direction from the sign of dx, as in the original algorithm

	const Vector3 origin = GetModelWorldPosition() + Vector3(right ? 0.4f : -0.4f, 0.3f, 0.0f);
	gameScene_->CreateGrenade(origin, Grenade::MakeThrowVelocity(right), Owner::kEnemy);
	throwPoseTimer_ = 0.0f;
}

SRTOffset EnemyGrenadier::GetActionAnimOffset() const {
	if (throwPoseTimer_ < kThrowPoseDuration) {
		const float t = std::clamp(throwPoseTimer_ / kThrowPoseDuration, 0.0f, 1.0f);
		return LerpSRT(kThrowPose, SRTOffset{}, EaseOut(0.0f, 1.0f, t));
	}
	return BaseEnemy::GetActionAnimOffset(); // fighter melee pose
}