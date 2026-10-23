#include "EnemyOfficer.h"
#include "GlobalVariables.h"
#include "OfficerBuff.h"
#include "PoseGV.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "EnemyOfficer";
}

void EnemyOfficer::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "MaxHP", kOfficerHP);
	gv->AddItem(kGroupName, "BuffAnimDuration", kBuffAnimDuration);
	gv->AddItem(kGroupName, "BuffApplyTime", kBuffApplyTime);
	gv->AddItem(kGroupName, "BuffDamageReduction", OfficerBuff::kDamageReduction);
	gv->AddItem(kGroupName, "BuffMoveBonus", OfficerBuff::kMoveBonus);
	gv->AddItem(kGroupName, "BuffChaseTimeBonus", OfficerBuff::kChaseTimeBonus);
	PoseGV::Register(kGroupName, "BuffPose", kBuffPose); // バフ時の体のポーズ
	WeaponSpear::RegisterGlobalVariables();
	OfficerBehavior::RegisterGlobalVariables();
}
void EnemyOfficer::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kOfficerHP = gv->GetIntValue(kGroupName, "MaxHP");
	kBuffAnimDuration = gv->GetFloatValue(kGroupName, "BuffAnimDuration");
	kBuffApplyTime = gv->GetFloatValue(kGroupName, "BuffApplyTime");
	OfficerBuff::kDamageReduction = gv->GetFloatValue(kGroupName, "BuffDamageReduction");
	OfficerBuff::kMoveBonus = gv->GetFloatValue(kGroupName, "BuffMoveBonus");
	OfficerBuff::kChaseTimeBonus = gv->GetFloatValue(kGroupName, "BuffChaseTimeBonus");
	PoseGV::Apply(kGroupName, "BuffPose", kBuffPose);
	WeaponSpear::ApplyGlobalVariables();
	OfficerBehavior::ApplyGlobalVariables();
}

void EnemyOfficer::Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, Camera* camera, const Vector3& position) {
	EnemyFighter::Initialize(partResources, camera, position);
	SetMaxHP(kOfficerHP);
	SetHP(kOfficerHP);
}

void EnemyOfficer::Update() {
	EnemyFighter::Update();
	if (GetBehaviorModule().GetBehavior() == Behavior::kCommand) {
		buffPoseTimer_ = std::min(buffPoseTimer_ + 1.0f / 60.0f, 999.0f);
	} else {
		buffPoseTimer_ = 999.0f; // interrupted or finished: drop the pose
	}
}

SRTOffset EnemyOfficer::GetActionAnimOffset() const {
	const float d = std::max(kBuffAnimDuration, 0.01f);
	if (buffPoseTimer_ < d) {
		const float t = buffPoseTimer_ / d;
		const float w = std::clamp(std::min(t, 1.0f - t) * 5.0f, 0.0f, 1.0f); // quick raise, hold, quick release
		return LerpSRT(SRTOffset{}, kBuffPose, w);
	}
	return BaseEnemy::GetActionAnimOffset(); // fighter melee pose
}