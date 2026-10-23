#define NOMINMAX
#include "FirearmShotgun.h"
#include "GlobalVariables.h"
#include "PoseGV.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "FirearmShotgun";
}

void FirearmShotgun::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);

	gv->AddItem(kGroupName, "OffsetScale", shotgunOffset_.scale);
	gv->AddItem(kGroupName, "OffsetRotate", shotgunOffset_.rotation);
	gv->AddItem(kGroupName, "OffsetTranslate", shotgunOffset_.translation);
	gv->AddItem(kGroupName, "OffsetHolsterScale", shotgunHolsterOffset_.scale);
	gv->AddItem(kGroupName, "OffsetHolsterRotate", shotgunHolsterOffset_.rotation);
	gv->AddItem(kGroupName, "OffsetHolsterTranslate", shotgunHolsterOffset_.translation);
	gv->AddItem(kGroupName, "OffsetReloadScale", shotgunReloadOffset_.scale);
	gv->AddItem(kGroupName, "OffsetReloadRotate", shotgunReloadOffset_.rotation);
	gv->AddItem(kGroupName, "OffsetReloadTranslate", shotgunReloadOffset_.translation);

	// ---- リアクションの構え(ノックバック / スタン / ブロック) ----
	PoseGV::Register(kGroupName, "Knockback", shotgunKnockbackOffset_);
	PoseGV::Register(kGroupName, "Stun", shotgunStunOffset_);
	PoseGV::Register(kGroupName, "Block", shotgunBlockOffset_);

	gv->AddItem(kGroupName, "PelletCount", kPelletCount);
	gv->AddItem(kGroupName, "SpreadStep", kSpreadStep);
	gv->AddItem(kGroupName, "DamageToEnemy", kDamageToEnemy);
	gv->AddItem(kGroupName, "DamageToPlayer", kDamageToPlayer);
	gv->AddItem(kGroupName, "PelletLifeFrames", static_cast<int32_t>(kPelletLifeFrames));

	gv->AddItem(kGroupName, "ReloadDurationPlayer", kReloadDurationPlayer);
	gv->AddItem(kGroupName, "ReloadHitBonusPlayer", kReloadHitBonusPlayer);
	gv->AddItem(kGroupName, "ReloadDurationMinPlayer", kReloadDurationMinPlayer);
	gv->AddItem(kGroupName, "ReloadDurationEnemy", kReloadDurationEnemy);
	gv->AddItem(kGroupName, "ReloadHitBonusEnemy", kReloadHitBonusEnemy);
	gv->AddItem(kGroupName, "ReloadDurationMinEnemy", kReloadDurationMinEnemy);
	gv->AddItem(kGroupName, "FireKnockbackPower", kFireKnockbackPower);
}

void FirearmShotgun::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();

	shotgunOffset_.scale = gv->GetVector3Value(kGroupName, "OffsetScale");
	shotgunOffset_.rotation = gv->GetVector3Value(kGroupName, "OffsetRotate");
	shotgunOffset_.translation = gv->GetVector3Value(kGroupName, "OffsetTranslate");
	shotgunHolsterOffset_.scale = gv->GetVector3Value(kGroupName, "OffsetHolsterScale");
	shotgunHolsterOffset_.rotation = gv->GetVector3Value(kGroupName, "OffsetHolsterRotate");
	shotgunHolsterOffset_.translation = gv->GetVector3Value(kGroupName, "OffsetHolsterTranslate");
	shotgunReloadOffset_.scale = gv->GetVector3Value(kGroupName, "OffsetReloadScale");
	shotgunReloadOffset_.rotation = gv->GetVector3Value(kGroupName, "OffsetReloadRotate");
	shotgunReloadOffset_.translation = gv->GetVector3Value(kGroupName, "OffsetReloadTranslate");

	PoseGV::Apply(kGroupName, "Knockback", shotgunKnockbackOffset_);
	PoseGV::Apply(kGroupName, "Stun", shotgunStunOffset_);
	PoseGV::Apply(kGroupName, "Block", shotgunBlockOffset_);

	kPelletCount = std::max(1, gv->GetIntValue(kGroupName, "PelletCount"));
	kSpreadStep = gv->GetFloatValue(kGroupName, "SpreadStep");
	kDamageToEnemy = gv->GetIntValue(kGroupName, "DamageToEnemy");
	kDamageToPlayer = gv->GetIntValue(kGroupName, "DamageToPlayer");
	kPelletLifeFrames = static_cast<int32_t>(gv->GetIntValue(kGroupName, "PelletLifeFrames"));

	kReloadDurationPlayer = gv->GetFloatValue(kGroupName, "ReloadDurationPlayer");
	kReloadHitBonusPlayer = gv->GetFloatValue(kGroupName, "ReloadHitBonusPlayer");
	kReloadDurationMinPlayer = gv->GetFloatValue(kGroupName, "ReloadDurationMinPlayer");
	kReloadDurationEnemy = gv->GetFloatValue(kGroupName, "ReloadDurationEnemy");
	kReloadHitBonusEnemy = gv->GetFloatValue(kGroupName, "ReloadHitBonusEnemy");
	kReloadDurationMinEnemy = gv->GetFloatValue(kGroupName, "ReloadDurationMinEnemy");
	kFireKnockbackPower = gv->GetFloatValue(kGroupName, "FireKnockbackPower");
}