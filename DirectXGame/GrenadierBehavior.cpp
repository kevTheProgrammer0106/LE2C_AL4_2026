#include "GrenadierBehavior.h"
#include "EnemyGrenadier.h"
#include "GameScene.h"
#include "GlobalVariables.h"
#include "Player.h"

#include <cmath>

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "GrenadierBehavior";
}

void GrenadierBehavior::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "ThrowInterval", kThrowInterval);
	gv->AddItem(kGroupName, "ThrowRangeMin", kThrowRangeMin);
	gv->AddItem(kGroupName, "ThrowRangeMax", kThrowRangeMax);
	gv->AddItem(kGroupName, "ThrowHeightTolerance", kThrowHeightTolerance);
	gv->AddItem(kGroupName, "RetreatSpeed", kRetreatSpeed);
}
void GrenadierBehavior::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kThrowInterval = gv->GetFloatValue(kGroupName, "ThrowInterval");
	kThrowRangeMin = gv->GetFloatValue(kGroupName, "ThrowRangeMin");
	kThrowRangeMax = gv->GetFloatValue(kGroupName, "ThrowRangeMax");
	kThrowHeightTolerance = gv->GetFloatValue(kGroupName, "ThrowHeightTolerance");
	kRetreatSpeed = gv->GetFloatValue(kGroupName, "RetreatSpeed");
}

GrenadierBehavior::GrenadierBehavior(EnemyGrenadier& owner) : FighterBehavior(owner), grenadierOwner_(owner) {}

void GrenadierBehavior::BehaviorRootUpdate() {
	constexpr float kDt = 1.0f / 60.0f;

	// same bookkeeping FighterBehavior does in its root update
	if (cooldownTimer_ > 0.0f) {
		cooldownTimer_ -= kDt;
	}
	noAttackTimer_ += kDt;
	if (noAttackTimer_ >= kComboResetDuration) {
		comboIndex_ = 0;
	}
	if (throwTimer_ > 0.0f) {
		throwTimer_ -= kDt;
	}

	GameScene* scene = OwnerGameScene();
	Player* player = scene ? scene->GetPlayer() : nullptr;

	// arena / omniscient enemies keep fighting even when the vision cone has lost the player
	const bool aware = IsPlayerSpotted() || HasPersistentAwareness();
	if (!player || !aware) {
		BaseEnemyBehavior::BehaviorRootUpdate(); // walk to the last known position
		return;
	}

	const Vector3 playerPos = player->GetWorldPosition();
	const Vector3 myPos = owner_.GetWorldPosition();
	const float dx = playerPos.x - myPos.x;
	const float dy = playerPos.y - myPos.y;
	const float distance = std::sqrt(dx * dx + dy * dy);

	OwnerFaceDirection(dx < 0.0f ? 1 : 0);

	if (!CanMakeDecision()) {
		ContinueCommittedMove(); // mid-hop
		return;
	}

	// 1) close range: Fighter melee
	if (distance <= kMeleeEngageRange && IsPlayerOnSameFloor() && cooldownTimer_ <= 0.0f) {
		RequestBehavior(Behavior::kMelee);
		return;
	}

	// 2) too close (or melee on cooldown): back off to the throwing band
	if (distance < kThrowRangeMin) {
		if (RetreatAwayFrom(playerPos, ScaleRetreatSpeed(distance, kThrowRangeMin, kMeleeEngageRange, kRetreatSpeed))) {
			return;
		}
		// cornered: fall through and throw at point-blank
	}
	// 3) too far / wrong floor: close in with the normal chase
	else if (distance > kThrowRangeMax || std::abs(dy) > kThrowHeightTolerance) {
		BaseEnemyBehavior::BehaviorRootUpdate();
		return;
	}

	// 4) throwing band (or cornered): hold position and lob
	OwnerVelocity().x = 0.0f;
	if (throwTimer_ <= 0.0f) {
		grenadierOwner_.ThrowGrenade();
		throwTimer_ = kThrowInterval;
	}
}