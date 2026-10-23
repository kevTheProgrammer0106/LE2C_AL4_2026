#include "WeaponSpear.h"
#include "GlobalVariables.h"
#include "PoseGV.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "WeaponSpear";
}

void WeaponSpear::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "Reach", kSpearReach);
	gv->AddItem(kGroupName, "HolsterOffsetScale", holsterOffset_.scale);
	gv->AddItem(kGroupName, "HolsterOffsetTranslation", holsterOffset_.translation);
	gv->AddItem(kGroupName, "HolsterOffsetRotation", holsterOffset_.rotation);

	// ---- ポーズ(構え / リアクション / 下攻撃) ----
	PoseGV::Register(kGroupName, "Ready", readyStanceOffset_);
	PoseGV::Register(kGroupName, "Knockback", kKnockbackOffset);
	PoseGV::Register(kGroupName, "Stun", kStunOffset);
	PoseGV::Register(kGroupName, "Block", kBlockOffset);
	PoseGV::Register(kGroupName, "DownAttack", kDownAttackOffset);
	// ---- コンボ(3段 x 4キーフレーム) ----
	PoseGV::RegisterCombo(kGroupName, "Combo", comboPatterns_);
	// ---- 高さ(射程は上のReachで3段共通) ----
	gv->AddItem(kGroupName, "ComboSwingHeight", PoseGV::ToVec3(kComboSwingHeight));
}

void WeaponSpear::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kSpearReach = gv->GetFloatValue(kGroupName, "Reach");
	kComboReach.fill(kSpearReach); // 3段とも同じ射程
	holsterOffset_.scale = gv->GetVector3Value(kGroupName, "HolsterOffsetScale");
	holsterOffset_.translation = gv->GetVector3Value(kGroupName, "HolsterOffsetTranslation");
	holsterOffset_.rotation = gv->GetVector3Value(kGroupName, "HolsterOffsetRotation");

	PoseGV::Apply(kGroupName, "Ready", readyStanceOffset_);
	PoseGV::Apply(kGroupName, "Knockback", kKnockbackOffset);
	PoseGV::Apply(kGroupName, "Stun", kStunOffset);
	PoseGV::Apply(kGroupName, "Block", kBlockOffset);
	PoseGV::Apply(kGroupName, "DownAttack", kDownAttackOffset);
	PoseGV::ApplyCombo(kGroupName, "Combo", comboPatterns_);
	PoseGV::FromVec3(kComboSwingHeight, gv->GetVector3Value(kGroupName, "ComboSwingHeight"));
}