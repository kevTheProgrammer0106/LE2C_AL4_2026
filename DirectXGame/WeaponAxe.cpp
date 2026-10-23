#define NOMINMAX
#include "WeaponAxe.h"
#include "BaseEnemy.h"
#include "GlobalVariables.h"
#include "PoseGV.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "WeaponAxe";
}

void WeaponAxe::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "HolsterOffsetScale", holsterOffset_.scale);
	gv->AddItem(kGroupName, "HolsterOffsetTranslation", holsterOffset_.translation);
	gv->AddItem(kGroupName, "HolsterOffsetRotation", holsterOffset_.rotation);
	gv->AddItem(kGroupName, "ComboDamage0", kComboDamageAxe[0]);
	gv->AddItem(kGroupName, "ComboDamage1", kComboDamageAxe[1]);
	gv->AddItem(kGroupName, "ComboDamage2", kComboDamageAxe[2]);
	gv->AddItem(kGroupName, "BaseImpact", kBaseImpactAxe);
	gv->AddItem(kGroupName, "ImpactStep", kImpactStepAxe);
	gv->AddItem(kGroupName, "SmashRaiseTranslation", kSmashRaise.translation);
	gv->AddItem(kGroupName, "SmashRaiseRotation", kSmashRaise.rotation);
	gv->AddItem(kGroupName, "SmashSlamTranslation", kSmashSlam.translation);
	gv->AddItem(kGroupName, "SmashSlamRotation", kSmashSlam.rotation);
	gv->AddItem(kGroupName, "SmashHalfWidth", kSmashHalfWidth);
	gv->AddItem(kGroupName, "SmashBelow", kSmashBelow);
	gv->AddItem(kGroupName, "SmashAbove", kSmashAbove);
	gv->AddItem(kGroupName, "SmashDamage", kSmashDamage);
	gv->AddItem(kGroupName, "SmashImpact", kSmashImpactPower);
	gv->AddItem(kGroupName, "PlayerComboDamage0", kComboDamageAxePlayer[0]);
	gv->AddItem(kGroupName, "PlayerComboDamage1", kComboDamageAxePlayer[1]);
	gv->AddItem(kGroupName, "PlayerComboDamage2", kComboDamageAxePlayer[2]);
	gv->AddItem(kGroupName, "PlayerBaseImpact", kBaseImpactAxePlayer);
	gv->AddItem(kGroupName, "PlayerImpactStep", kImpactStepAxePlayer);

	// ---- ポーズ(構え / リアクション / 下攻撃) ----
	PoseGV::Register(kGroupName, "Ready", readyStanceOffset_);
	PoseGV::Register(kGroupName, "Knockback", kKnockbackOffset);
	PoseGV::Register(kGroupName, "Stun", kStunOffset);
	PoseGV::Register(kGroupName, "Block", kBlockOffset);
	PoseGV::Register(kGroupName, "DownAttack", kDownAttackOffset);
	// ---- コンボ(3段 x 4キーフレーム) ----
	PoseGV::RegisterCombo(kGroupName, "Combo", comboPatterns_);
	// ---- コンボの射程 / 高さ(3段分を1行で) ----
	gv->AddItem(kGroupName, "ComboReach", PoseGV::ToVec3(kComboReach));
	gv->AddItem(kGroupName, "ComboSwingHeight", PoseGV::ToVec3(kComboSwingHeight));
}

void WeaponAxe::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	holsterOffset_.scale = gv->GetVector3Value(kGroupName, "HolsterOffsetScale");
	holsterOffset_.translation = gv->GetVector3Value(kGroupName, "HolsterOffsetTranslation");
	holsterOffset_.rotation = gv->GetVector3Value(kGroupName, "HolsterOffsetRotation");
	kComboDamageAxe[0] = gv->GetIntValue(kGroupName, "ComboDamage0");
	kComboDamageAxe[1] = gv->GetIntValue(kGroupName, "ComboDamage1");
	kComboDamageAxe[2] = gv->GetIntValue(kGroupName, "ComboDamage2");
	kBaseImpactAxe = gv->GetFloatValue(kGroupName, "BaseImpact");
	kImpactStepAxe = gv->GetFloatValue(kGroupName, "ImpactStep");
	kSmashRaise.translation = gv->GetVector3Value(kGroupName, "SmashRaiseTranslation");
	kSmashRaise.rotation = gv->GetVector3Value(kGroupName, "SmashRaiseRotation");
	kSmashSlam.translation = gv->GetVector3Value(kGroupName, "SmashSlamTranslation");
	kSmashSlam.rotation = gv->GetVector3Value(kGroupName, "SmashSlamRotation");
	kSmashHalfWidth = gv->GetFloatValue(kGroupName, "SmashHalfWidth");
	kSmashBelow = gv->GetFloatValue(kGroupName, "SmashBelow");
	kSmashAbove = gv->GetFloatValue(kGroupName, "SmashAbove");
	kSmashDamage = gv->GetIntValue(kGroupName, "SmashDamage");
	kSmashImpactPower = gv->GetFloatValue(kGroupName, "SmashImpact");
	kComboDamageAxePlayer[0] = gv->GetIntValue(kGroupName, "PlayerComboDamage0");
	kComboDamageAxePlayer[1] = gv->GetIntValue(kGroupName, "PlayerComboDamage1");
	kComboDamageAxePlayer[2] = gv->GetIntValue(kGroupName, "PlayerComboDamage2");
	kBaseImpactAxePlayer = gv->GetFloatValue(kGroupName, "PlayerBaseImpact");
	kImpactStepAxePlayer = gv->GetFloatValue(kGroupName, "PlayerImpactStep");

	PoseGV::Apply(kGroupName, "Ready", readyStanceOffset_);
	PoseGV::Apply(kGroupName, "Knockback", kKnockbackOffset);
	PoseGV::Apply(kGroupName, "Stun", kStunOffset);
	PoseGV::Apply(kGroupName, "Block", kBlockOffset);
	PoseGV::Apply(kGroupName, "DownAttack", kDownAttackOffset);
	PoseGV::ApplyCombo(kGroupName, "Combo", comboPatterns_);
	PoseGV::FromVec3(kComboReach, gv->GetVector3Value(kGroupName, "ComboReach"));
	PoseGV::FromVec3(kComboSwingHeight, gv->GetVector3Value(kGroupName, "ComboSwingHeight"));
}

void WeaponAxe::SetSmashPositionGeneric(const Vector3& ownerPosition, const Vector3& ownerRotation, bool facingRight, AttackPhase smashPhase, float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	SRTOffset pose;
	switch (smashPhase) {
	case AttackPhase::kAnticipate:
		pose = LerpSRT(readyStanceOffset_, kSmashRaise, t);
		break;
	case AttackPhase::kAttack:
		pose = LerpSRT(kSmashRaise, kSmashSlam, t);
		break;
	case AttackPhase::kRecovery:
	default:
		pose = LerpSRT(kSmashSlam, readyStanceOffset_, t);
		break;
	}
	ApplyPoseToWorld(pose, ownerPosition, ownerRotation, facingRight, true);
}

AABB WeaponAxe::GetSmashAABBGeneric(const Vector3& ownerPosition) const {
	const float halfBodyHeight = BaseEnemy::GetBodyHeight() * 0.5f;
	const float halfDepth = BaseEnemy::GetBodyWidth() * 0.5f;

	AABB aabb;
	aabb.min = {ownerPosition.x - kSmashHalfWidth, ownerPosition.y - halfBodyHeight - kSmashBelow, ownerPosition.z - halfDepth};
	aabb.max = {ownerPosition.x + kSmashHalfWidth, ownerPosition.y + kSmashAbove, ownerPosition.z + halfDepth};
	return aabb;
}