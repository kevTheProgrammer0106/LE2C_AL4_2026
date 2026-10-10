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

	gv->AddItem(kGroupName, "ReloadDurationPlayer", kReloadDurationPlayer);
	gv->AddItem(kGroupName, "ReloadHitBonusPlayer", kReloadHitBonusPlayer);
	gv->AddItem(kGroupName, "ReloadDurationMinPlayer", kReloadDurationMinPlayer);
	gv->AddItem(kGroupName, "ReloadDurationEnemy", kReloadDurationEnemy);
	gv->AddItem(kGroupName, "ReloadHitBonusEnemy", kReloadHitBonusEnemy);
	gv->AddItem(kGroupName, "ReloadDurationMinEnemy", kReloadDurationMinEnemy);

	gv->AddItem(kGroupName, "LungeDamage", kLungeDamage);
	gv->AddItem(kGroupName, "LungeImpactPower", kLungeImpactPower);

	gv->AddItem(kGroupName, "BayonetAnticipateDuration", kBayonetAnticipateDuration);
	gv->AddItem(kGroupName, "BayonetAttackDuration", kBayonetAttackDuration);
	gv->AddItem(kGroupName, "BayonetRecoveryDuration", kBayonetRecoveryDuration);
	gv->AddItem(kGroupName, "BayonetAttackSpeed", kBayonetAttackSpeed);
	gv->AddItem(kGroupName, "BayonetReach", kBayonetReach);
	gv->AddItem(kGroupName, "BayonetSwingHeight", kBayonetSwingHeight);

		gv->AddItem(kGroupName, "PlayerBayonetAnticipate", kPlayerBayonetAnticipate);
	gv->AddItem(kGroupName, "PlayerBayonetAttack", kPlayerBayonetAttack);
	gv->AddItem(kGroupName, "PlayerBayonetRecovery", kPlayerBayonetRecovery);
	gv->AddItem(kGroupName, "PlayerBayonetSpeed", kPlayerBayonetSpeed);
	gv->AddItem(kGroupName, "PlayerBayonetDamage", kPlayerBayonetDamage);
	gv->AddItem(kGroupName, "PlayerBayonetImpact", kPlayerBayonetImpact);
	gv->AddItem(kGroupName, "PlayerBayonetCooldown", kPlayerBayonetCooldown);
	gv->AddItem(kGroupName, "PlayerBayonetReach", kPlayerBayonetReach);
	gv->AddItem(kGroupName, "PlayerBayonetHeight", kPlayerBayonetHeight);
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

	kReloadDurationPlayer = gv->GetFloatValue(kGroupName, "ReloadDurationPlayer");
	kReloadHitBonusPlayer = gv->GetFloatValue(kGroupName, "ReloadHitBonusPlayer");
	kReloadDurationMinPlayer = gv->GetFloatValue(kGroupName, "ReloadDurationMinPlayer");
	kReloadDurationEnemy = gv->GetFloatValue(kGroupName, "ReloadDurationEnemy");
	kReloadHitBonusEnemy = gv->GetFloatValue(kGroupName, "ReloadHitBonusEnemy");
	kReloadDurationMinEnemy = gv->GetFloatValue(kGroupName, "ReloadDurationMinEnemy");

	kLungeDamage = gv->GetIntValue(kGroupName, "LungeDamage");
	kLungeImpactPower = gv->GetFloatValue(kGroupName, "LungeImpactPower");

	kBayonetAnticipateDuration = gv->GetFloatValue(kGroupName, "BayonetAnticipateDuration");
	kBayonetAttackDuration = gv->GetFloatValue(kGroupName, "BayonetAttackDuration");
	kBayonetRecoveryDuration = gv->GetFloatValue(kGroupName, "BayonetRecoveryDuration");
	kBayonetAttackSpeed = gv->GetFloatValue(kGroupName, "BayonetAttackSpeed");
	kBayonetReach = gv->GetFloatValue(kGroupName, "BayonetReach");
	kBayonetSwingHeight = gv->GetFloatValue(kGroupName, "BayonetSwingHeight");
	kPlayerBayonetAnticipate = gv->GetFloatValue(kGroupName, "PlayerBayonetAnticipate");
	kPlayerBayonetAttack = gv->GetFloatValue(kGroupName, "PlayerBayonetAttack");
	kPlayerBayonetRecovery = gv->GetFloatValue(kGroupName, "PlayerBayonetRecovery");
	kPlayerBayonetSpeed = gv->GetFloatValue(kGroupName, "PlayerBayonetSpeed");
	kPlayerBayonetDamage = gv->GetIntValue(kGroupName, "PlayerBayonetDamage");
	kPlayerBayonetImpact = gv->GetFloatValue(kGroupName, "PlayerBayonetImpact");
	kPlayerBayonetCooldown = gv->GetFloatValue(kGroupName, "PlayerBayonetCooldown");
	kPlayerBayonetReach = gv->GetFloatValue(kGroupName, "PlayerBayonetReach");
	kPlayerBayonetHeight = gv->GetFloatValue(kGroupName, "PlayerBayonetHeight");
}

// Ammo / reload / drop-state logic now lives in BaseFirearm.

// Same phase structure as the sabre combo blend: one linear LerpSRT per phase.
// t = frames elapsed in the phase / phase duration (0..1), supplied by RiflemanBehavior.
void FirearmMusketBayonet::SetLungePose(LungePhase phase, float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	lungePhase_ = phase;
	lungeT_ = t;
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
void FirearmMusketBayonet::SetPlayerLungePose(LungePhase phase, float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	lungePhase_ = phase;
	lungeT_ = t;
	switch (phase) {
	case LungePhase::kAnticipate:
		lungeBlendedOffset_ = LerpSRT(musketOffset_, kPlayerLungePoses.windUp, t);
		break;
	case LungePhase::kAttack:
		lungeBlendedOffset_ = LerpSRT(kPlayerLungePoses.windUp, kPlayerLungePoses.thrust, t);
		break;
	case LungePhase::kRecovery:
	default:
		lungeBlendedOffset_ = LerpSRT(kPlayerLungePoses.thrust, musketOffset_, t);
		break;
	}
	lungePoseActive_ = true;
}

AABB FirearmMusketBayonet::GetPlayerLungeAABB(const AABB& body, bool facingRight) const {
	constexpr float kOverlapMargin = 0.1f;
	const float centerY = (body.min.y + body.max.y) * 0.5f;

	AABB aabb;
	if (facingRight) {
		aabb.min.x = body.max.x - kOverlapMargin;
		aabb.max.x = body.max.x + kPlayerBayonetReach;
	} else {
		aabb.min.x = body.min.x - kPlayerBayonetReach;
		aabb.max.x = body.min.x + kOverlapMargin;
	}
	aabb.min.y = centerY - kPlayerBayonetHeight * 0.5f;
	aabb.max.y = centerY + kPlayerBayonetHeight * 0.5f;
	aabb.min.z = body.min.z;
	aabb.max.z = body.max.z;
	return aabb;
}