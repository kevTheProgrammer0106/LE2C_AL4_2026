#include "WeaponSpear.h"
#include "GlobalVariables.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "WeaponSpear";
}

void WeaponSpear::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "Reach", kSpearReach);
	gv->AddItem(kGroupName, "HolsterOffsetTranslation", holsterOffset_.translation);
	gv->AddItem(kGroupName, "HolsterOffsetRotation", holsterOffset_.rotation);
}

void WeaponSpear::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kSpearReach = gv->GetFloatValue(kGroupName, "Reach");
	kComboReach.fill(kSpearReach); // 3段とも同じ射程
	holsterOffset_.translation = gv->GetVector3Value(kGroupName, "HolsterOffsetTranslation");
	holsterOffset_.rotation = gv->GetVector3Value(kGroupName, "HolsterOffsetRotation");
}