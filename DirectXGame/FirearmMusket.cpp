#define NOMINMAX
#include "FirearmMusket.h"
#include "GlobalVariables.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "FirearmMusket";
}

void FirearmMusket::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "OffsetScale", musketOffset_.scale);
	gv->AddItem(kGroupName, "OffsetRotate", musketOffset_.rotation);
	gv->AddItem(kGroupName, "OffsetTranslate", musketOffset_.translation);
	gv->AddItem(kGroupName, "OffsetHolsterScale", musketHolsterOffset_.scale);
	gv->AddItem(kGroupName, "OffsetHolsterRotate", musketHolsterOffset_.rotation);
	gv->AddItem(kGroupName, "OffsetHolsterTranslate", musketHolsterOffset_.translation);
	gv->AddItem(kGroupName, "OffsetReloadScale", musketReloadOffset_.scale);
	gv->AddItem(kGroupName, "OffsetReloadRotate", musketReloadOffset_.rotation);
	gv->AddItem(kGroupName, "OffsetReloadTranslate", musketReloadOffset_.translation);

	gv->AddItem(kGroupName, "ReloadDurationPlayer", kReloadDurationPlayer);
	gv->AddItem(kGroupName, "ReloadHitBonusPlayer", kReloadHitBonusPlayer);
	gv->AddItem(kGroupName, "ReloadDurationMinPlayer", kReloadDurationMinPlayer);
	gv->AddItem(kGroupName, "ReloadDurationEnemy", kReloadDurationEnemy);
	gv->AddItem(kGroupName, "ReloadHitBonusEnemy", kReloadHitBonusEnemy);
	gv->AddItem(kGroupName, "ReloadDurationMinEnemy", kReloadDurationMinEnemy);

	gv->AddItem(kGroupName, "DamageToEnemy", kDamageToEnemy);
	gv->AddItem(kGroupName, "DamageToPlayer", kDamageToPlayer);
}

void FirearmMusket::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	musketOffset_.scale = gv->GetVector3Value(kGroupName, "OffsetScale");
	musketOffset_.rotation = gv->GetVector3Value(kGroupName, "OffsetRotate");
	musketOffset_.translation = gv->GetVector3Value(kGroupName, "OffsetTranslate");
	musketHolsterOffset_.scale = gv->GetVector3Value(kGroupName, "OffsetHolsterScale");
	musketHolsterOffset_.rotation = gv->GetVector3Value(kGroupName, "OffsetHolsterRotate");
	musketHolsterOffset_.translation = gv->GetVector3Value(kGroupName, "OffsetHolsterTranslate");
	musketReloadOffset_.scale = gv->GetVector3Value(kGroupName, "OffsetReloadScale");
	musketReloadOffset_.rotation = gv->GetVector3Value(kGroupName, "OffsetReloadRotate");
	musketReloadOffset_.translation = gv->GetVector3Value(kGroupName, "OffsetReloadTranslate");

	kReloadDurationPlayer = gv->GetFloatValue(kGroupName, "ReloadDurationPlayer");
	kReloadHitBonusPlayer = gv->GetFloatValue(kGroupName, "ReloadHitBonusPlayer");
	kReloadDurationMinPlayer = gv->GetFloatValue(kGroupName, "ReloadDurationMinPlayer");
	kReloadDurationEnemy = gv->GetFloatValue(kGroupName, "ReloadDurationEnemy");
	kReloadHitBonusEnemy = gv->GetFloatValue(kGroupName, "ReloadHitBonusEnemy");
	kReloadDurationMinEnemy = gv->GetFloatValue(kGroupName, "ReloadDurationMinEnemy");

	kDamageToEnemy = gv->GetIntValue(kGroupName, "DamageToEnemy");
	kDamageToPlayer = gv->GetIntValue(kGroupName, "DamageToPlayer");
}

// Ammo / reload / drop-state logic now lives in BaseFirearm.
