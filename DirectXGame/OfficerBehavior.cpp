#include "OfficerBehavior.h"
#include "EnemyOfficer.h"
#include "GameScene.h"
#include "GlobalVariables.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "OfficerBehavior";
}

void OfficerBehavior::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "SpearEngageRange", kSpearEngageRange);
}
void OfficerBehavior::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kSpearEngageRange = gv->GetFloatValue(kGroupName, "SpearEngageRange");
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
	if (GetBehavior() == Behavior::kCommand) {
		BehaviorCommandUpdate();
		RunPhysicsAndCollisionUpdate();
		return;
	}
	FighterBehavior::UpdateBehaviorState();
}

void OfficerBehavior::BehaviorRootUpdate() {
	if (!IsChasing()) {
		alertHandled_ = false; // lost the player: the next alert may buff again
	} else if (!alertHandled_) {
		GameScene* scene = OwnerGameScene();
		if (scene && scene->IsOfficerBuffActive()) {
			alertHandled_ = true; // someone else already buffed: only one buff exists
		} else {
			RequestBehavior(Behavior::kCommand);
			return;
		}
	}
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