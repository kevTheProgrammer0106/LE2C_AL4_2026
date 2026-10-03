#define NOMINMAX
#include "FirearmMusketBayonet.h"
#include "GlobalVariables.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "FirearmMusketBayonet";
}

void FirearmMusketBayonet::RegisterGlobalVariables() {
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

	gv->AddItem(kGroupName, "ReloadDuration", kReloadDuration);
	gv->AddItem(kGroupName, "ReloadHitBonus", kReloadHitBonus);
	gv->AddItem(kGroupName, "ReloadDurationMin", kReloadDurationMin);

	gv->AddItem(kGroupName, "LungeDamage", kLungeDamage);
	gv->AddItem(kGroupName, "LungeImpactPower", kLungeImpactPower);

	gv->AddItem(kGroupName, "BayonetAnticipateDuration", kBayonetAnticipateDuration);
	gv->AddItem(kGroupName, "BayonetAttackDuration", kBayonetAttackDuration);
	gv->AddItem(kGroupName, "BayonetRecoveryDuration", kBayonetRecoveryDuration);
	gv->AddItem(kGroupName, "BayonetAttackSpeed", kBayonetAttackSpeed);
	gv->AddItem(kGroupName, "BayonetReach", kBayonetReach);
	gv->AddItem(kGroupName, "BayonetSwingHeight", kBayonetSwingHeight);
}

void FirearmMusketBayonet::ApplyGlobalVariables() {
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

	kReloadDuration = gv->GetFloatValue(kGroupName, "ReloadDuration");
	kReloadHitBonus = gv->GetFloatValue(kGroupName, "ReloadHitBonus");
	kReloadDurationMin = gv->GetFloatValue(kGroupName, "ReloadDurationMin");

	kLungeDamage = gv->GetIntValue(kGroupName, "LungeDamage");
	kLungeImpactPower = gv->GetFloatValue(kGroupName, "LungeImpactPower");

	kBayonetAnticipateDuration = gv->GetFloatValue(kGroupName, "BayonetAnticipateDuration");
	kBayonetAttackDuration = gv->GetFloatValue(kGroupName, "BayonetAttackDuration");
	kBayonetRecoveryDuration = gv->GetFloatValue(kGroupName, "BayonetRecoveryDuration");
	kBayonetAttackSpeed = gv->GetFloatValue(kGroupName, "BayonetAttackSpeed");
	kBayonetReach = gv->GetFloatValue(kGroupName, "BayonetReach");
	kBayonetSwingHeight = gv->GetFloatValue(kGroupName, "BayonetSwingHeight");
}

// Called by BaseFirearm::UpdateFollow. Stats only; pose math (incl. reload pose) lives in the base class.
void FirearmMusketBayonet::TickReload() {
	if (currentAmmo_ < kMaxAmmo) {
		reloadTimer_ += 1.0f / 60.0f;
		if (reloadTimer_ >= reloadDurationCurrent_) {
			currentAmmo_ = kMaxAmmo;
			reloadTimer_ = 0.0f;
			reloadDurationCurrent_ = kReloadDuration;
			if (sMusketLoadedSoundHandle_ != 0) {
				Audio::GetInstance()->PlayWave(sMusketLoadedSoundHandle_, false, sSfxVolume_);
			}
		}
	}
	isReloading_ = (currentAmmo_ < kMaxAmmo);
}

bool FirearmMusketBayonet::TryFire() {
	if (currentAmmo_ <= 0) {
		return false;
	}
	--currentAmmo_;
	return true;
}

void FirearmMusketBayonet::NotifyDealtDamage() { reloadDurationCurrent_ = std::max(reloadDurationCurrent_ - kReloadHitBonus, kReloadDurationMin); }

// Same phase structure as WeaponSabre::SetMeleePosition's combo blend: one linear LerpSRT per phase.
// t = frames elapsed in the phase / phase duration (0..1), supplied by RiflemanBehavior.
void FirearmMusketBayonet::SetLungePose(LungePhase phase, float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	switch (phase) {
	case LungePhase::kAnticipate:
		lungeBlendedOffset_ = LerpSRT(musketOffset_, kLungePoses.windUp, t);
		break;
	case LungePhase::kAttack:
		lungeBlendedOffset_ = LerpSRT(kLungePoses.windUp, kLungePoses.thrust, t);
		break;
	case LungePhase::kRecovery:
	default:
		lungeBlendedOffset_ = LerpSRT(kLungePoses.thrust, musketOffset_, t);
		break;
	}
	lungePoseActive_ = true;
}