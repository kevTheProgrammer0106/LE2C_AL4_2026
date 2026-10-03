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
	// [NEW] reload pose
	gv->AddItem(kGroupName, "OffsetReloadScale", musketReloadOffset_.scale);
	gv->AddItem(kGroupName, "OffsetReloadRotate", musketReloadOffset_.rotation);
	gv->AddItem(kGroupName, "OffsetReloadTranslate", musketReloadOffset_.translation);

	gv->AddItem(kGroupName, "ReloadDuration", kReloadDuration);
	gv->AddItem(kGroupName, "ReloadHitBonus", kReloadHitBonus);
	gv->AddItem(kGroupName, "ReloadDurationMin", kReloadDurationMin);
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

	kReloadDuration = gv->GetFloatValue(kGroupName, "ReloadDuration");
	kReloadHitBonus = gv->GetFloatValue(kGroupName, "ReloadHitBonus");
	kReloadDurationMin = gv->GetFloatValue(kGroupName, "ReloadDurationMin");
}

// Called by BaseFirearm::UpdateFollow. Stats only; pose math lives in the base class.
void FirearmMusket::TickReload() {
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

bool FirearmMusket::TryFire() {
	if (currentAmmo_ <= 0) {
		return false;
	}
	--currentAmmo_;
	return true;
}

void FirearmMusket::NotifyDealtDamage() {
	// 敵にダメージを与えるほど次のリロードが短くなる（積極的なプレイを促す）
	reloadDurationCurrent_ = std::max(reloadDurationCurrent_ - kReloadHitBonus, kReloadDurationMin);
}