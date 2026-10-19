#include "OfficerBehavior.h"
#include "EnemyOfficer.h"
#include "GameScene.h"
#include "GlobalVariables.h"
#include "Player.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "OfficerBehavior";
}

void OfficerBehavior::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "SpearEngageRange", kSpearEngageRange);
	gv->AddItem(kGroupName, "CommandRange", kCommandRange);
	gv->AddItem(kGroupName, "OverseeRange", kOverseeRange);
	gv->AddItem(kGroupName, "EscortCountRadius", kEscortCountRadius);
	gv->AddItem(kGroupName, "OverseeBehindGap", kOverseeBehindGap);
	gv->AddItem(kGroupName, "OverseeRetreatSpeed", kOverseeRetreatSpeed);
	gv->AddItem(kGroupName, "GrenadeRetreatSpeed", kGrenadeRetreatSpeed);
	gv->AddItem(kGroupName, "GrenadeDodgeCooldown", kGrenadeDodgeCooldown);
}
void OfficerBehavior::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kSpearEngageRange = gv->GetFloatValue(kGroupName, "SpearEngageRange");
	kCommandRange = gv->GetFloatValue(kGroupName, "CommandRange");
	kOverseeRange = gv->GetFloatValue(kGroupName, "OverseeRange");
	kEscortCountRadius = gv->GetFloatValue(kGroupName, "EscortCountRadius");
	kOverseeBehindGap = gv->GetFloatValue(kGroupName, "OverseeBehindGap");
	kOverseeRetreatSpeed = gv->GetFloatValue(kGroupName, "OverseeRetreatSpeed");
	kGrenadeRetreatSpeed = gv->GetFloatValue(kGroupName, "GrenadeRetreatSpeed");
	kGrenadeDodgeCooldown = gv->GetFloatValue(kGroupName, "GrenadeDodgeCooldown");
}

OfficerBehavior::OfficerBehavior(EnemyOfficer& owner) : FighterBehavior(owner), officerOwner_(owner) {}

void OfficerBehavior::InitializeBehaviorState(Behavior behavior) {
	if (behavior == Behavior::kCommand) {
		if (BaseMelee* melee = officerOwner_.GetCurrentMelee())
			melee->SetBehavior(BaseMelee::MeleeBehavior::kIdle);
		BehaviorCommandInitialize();
		return;
	}
	FighterBehavior::InitializeBehaviorState(behavior);
}

void OfficerBehavior::UpdateBehaviorState() {
	grenadeDodgeCooldown_ = std::max(0.0f, grenadeDodgeCooldown_ - 1.0f / 60.0f); // ticks in every state

	if (GetBehavior() == Behavior::kCommand) {
		BehaviorCommandUpdate();
		RunPhysicsAndCollisionUpdate();
		return;
	}
	FighterBehavior::UpdateBehaviorState();
}

bool OfficerBehavior::FindFrontEscort(const Vector3& playerPos, Vector3& outPos) const {
	GameScene* scene = OwnerGameScene();
	if (!scene)
		return false;
	const Vector3 me = owner_.GetWorldPosition();
	float bestToPlayer = 1e9f;
	bool found = false;
	for (const std::unique_ptr<BaseEnemy>& e : scene->GetEnemies()) {
		if (!e || e.get() == &owner_ || e->IsDead() || e->IsEscortLeader() || e->GetBehaviorModule().IsDeathBehavior())
			continue;
		const Vector3 p = e->GetWorldPosition();
		if (std::hypot(p.x - me.x, p.y - me.y) > kEscortCountRadius)
			continue;
		const float toPlayer = std::abs(p.x - playerPos.x);
		if (toPlayer < bestToPlayer) {
			bestToPlayer = toPlayer;
			outPos = p;
			found = true;
		}
	}
	return found;
}

void OfficerBehavior::OverseeFight(const Vector3& playerPos, const Vector3& frontEscortPos) {
	constexpr float kDt = 1.0f / 60.0f;
	if (cooldownTimer_ > 0.0f)
		cooldownTimer_ -= kDt; // FighterBehavior's root update isn't running, keep its timer honest

	const Vector3 myPos = owner_.GetWorldPosition();
	OwnerFaceDirection(playerPos.x < myPos.x ? 1 : 0);
	if (!CanMakeDecision()) {
		ContinueCommittedMove();
		return;
	}

	// must be farther from the player than the closest escort, plus a gap
	if (std::abs(myPos.x - playerPos.x) < std::abs(frontEscortPos.x - playerPos.x) + kOverseeBehindGap) {
		if (RetreatAwayFrom(playerPos, kOverseeRetreatSpeed))
			return;
		// cornered with the player on top of it: fight back rather than stand still
		const float dist = std::hypot(playerPos.x - myPos.x, playerPos.y - myPos.y);
		if (dist <= GetMeleeEngageRange()) {
			FighterBehavior::BehaviorRootUpdate();
			return;
		}
	}
	OwnerVelocity().x = 0.0f;
}

void OfficerBehavior::BehaviorRootUpdate() {
	GameScene* scene = OwnerGameScene();
	Player* player = scene ? scene->GetPlayer() : nullptr;

	// ---- 1) grenade dodge (10s cooldown after each dodge) ----
	Vector3 grenadePos{};
	const bool inDanger = IsInGrenadeDanger(grenadePos);
	if (inDanger && (dodgingGrenade_ || grenadeDodgeCooldown_ <= 0.0f)) {
		dodgingGrenade_ = true;
		if (RetreatFromGrenade(grenadePos, kGrenadeRetreatSpeed))
			return;
	} else if (dodgingGrenade_) {
		dodgingGrenade_ = false;
		grenadeDodgeCooldown_ = kGrenadeDodgeCooldown;
	}

	// ---- 2) buff: only when it actually sees the player, not from spawn awareness ----
	float playerDistance = 1e9f;
	Vector3 playerPos{};
	if (player) {
		playerPos = player->GetWorldPosition();
		const Vector3 me = owner_.GetWorldPosition();
		playerDistance = std::hypot(playerPos.x - me.x, playerPos.y - me.y);
	}

	if (!IsChasing()) {
		alertHandled_ = false; // lost the player: the next sighting may buff again
	} else if (!alertHandled_ && player && IsPlayerSpotted() && playerDistance <= kCommandRange) {
		if (scene && scene->IsOfficerBuffActive()) {
			alertHandled_ = true; // someone else already buffed: only one buff exists
		} else {
			RequestBehavior(Behavior::kCommand);
			return;
		}
	}

	// ---- 3) player close: oversee from behind the escorts, or fight if none are left ----
	if (player && (IsPlayerSpotted() || HasPersistentAwareness()) && playerDistance <= kOverseeRange) {
		Vector3 frontEscort{};
		if (FindFrontEscort(playerPos, frontEscort)) {
			OverseeFight(playerPos, frontEscort);
			return;
		}
	}

	// ---- 4) otherwise: chase (leads the escorts) and, with no escorts, melee when in range ----
	FighterBehavior::BehaviorRootUpdate();
}

void OfficerBehavior::BehaviorCommandInitialize() {
	commandTimer_ = 0.0f;
	buffApplied_ = false;
	OwnerVelocity().x = 0.0f;
	officerOwner_.BeginBuffPose();

	if (GameScene* scene = OwnerGameScene()) {
		Vector3 pos = owner_.GetModelWorldPosition();
		scene->CreateGuardEffect(pos); // rising ring as the gesture starts
	}
}

void OfficerBehavior::BehaviorCommandUpdate() {
	commandTimer_ += 1.0f / 60.0f;
	OwnerVelocity().x = 0.0f;
	FacePlayer();

	if (!buffApplied_ && commandTimer_ >= officerOwner_.GetBuffApplyTime()) {
		buffApplied_ = true;
		alertHandled_ = true;
		if (GameScene* scene = OwnerGameScene())
			scene->ApplyOfficerBuff(&owner_);
	}
	if (commandTimer_ >= officerOwner_.GetBuffAnimDuration())
		RequestBehavior(Behavior::kRoot);
}