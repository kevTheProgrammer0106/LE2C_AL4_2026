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

	// 既存の銃剣ダメージ値を登録する。
	gv->AddItem(kGroupName, "LungeDamage", kLungeDamage);
	gv->AddItem(kGroupName, "LungeImpactPower", kLungeImpactPower);

	// 銃剣攻撃そのもののフェーズ・移動・間合いを登録する。
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

	// 既存の銃剣ダメージ値を読み込む。
	kLungeDamage = gv->GetIntValue(kGroupName, "LungeDamage");

	kLungeImpactPower = gv->GetFloatValue(kGroupName, "LungeImpactPower");

	// 銃剣攻撃パターンを読み込む。
	kBayonetAnticipateDuration = gv->GetFloatValue(kGroupName, "BayonetAnticipateDuration");

	kBayonetAttackDuration = gv->GetFloatValue(kGroupName, "BayonetAttackDuration");

	kBayonetRecoveryDuration = gv->GetFloatValue(kGroupName, "BayonetRecoveryDuration");

	kBayonetAttackSpeed = gv->GetFloatValue(kGroupName, "BayonetAttackSpeed");

	kBayonetReach = gv->GetFloatValue(kGroupName, "BayonetReach");

	kBayonetSwingHeight = gv->GetFloatValue(kGroupName, "BayonetSwingHeight");
}

void FirearmMusketBayonet::UpdateFollow(const WorldTransform& parentTransform, float localAimAngle, bool facingRight) {

	equipFactor_ = Lerp(equipFactor_, equipTarget_, kEquipLerpSpeed);

	//bool isHolstered = equipTarget_ < 0.5f;
	if (currentAmmo_ < kMaxAmmo) {
			isReloading_ = true;
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

	Matrix4x4 rotMatrix = Multiply(MakeRotateXMatrix(parentTransform.rotation_.x), Multiply(MakeRotateYMatrix(parentTransform.rotation_.y), MakeRotateZMatrix(parentTransform.rotation_.z)));

	float facingSign = facingRight ? 1.0f : -1.0f;
	float correctedAimAngle = localAimAngle * facingSign;

	constexpr float kModelYaw = std::numbers::pi_v<float> / 2.0f;

	// [FIX] リロード中の静止ポーズ表示が有効(isShowingReloadPose_)なら、収納側のブレンド先を
	// musketHolsterOffset_ ではなく musketReloadOffset_ にする。isReloading_(弾薬不足で実際に
	// リロード処理が進行中かどうか)とは独立させてあり、見た目はRiflemanBehavior側が
	// SetReloading()で明示的に制御する(kReloading状態の間だけtrue)。
	const SRTOffset& stowedOffset = isShowingReloadPose_ ? musketReloadOffset_ : musketHolsterOffset_;

	Vector3 holsterRotation = stowedOffset.rotation;

	if (!facingRight) {
		holsterRotation.y = -holsterRotation.y;
		holsterRotation.z = -holsterRotation.z;
	}

	float holsterRotY = parentTransform.rotation_.y + holsterRotation.y;
	float holsterRotZ = holsterRotation.z;

	float combatRotY = parentTransform.rotation_.y + kModelYaw + musketOffset_.rotation.y;

	float combatRotZ = correctedAimAngle + musketOffset_.rotation.z;

	Vector3 blendedLocalOffset = Lerp(stowedOffset.translation, musketOffset_.translation, equipFactor_);

	float blendedRotY = Lerp(holsterRotY, combatRotY, equipFactor_);
	float blendedRotZ = Lerp(holsterRotZ, combatRotZ, equipFactor_);

	Vector3 blendedScale = Lerp(stowedOffset.scale, musketOffset_.scale, equipFactor_);

	Vector3 rotatedOffset = TransformNormal(blendedLocalOffset, rotMatrix);

	worldTransform_.translation_ = parentTransform.translation_ + rotatedOffset;

	worldTransform_.rotation_.x = parentTransform.rotation_.x + Lerp(stowedOffset.rotation.x, musketOffset_.rotation.x, equipFactor_);

	worldTransform_.rotation_.y = blendedRotY;
	worldTransform_.rotation_.z = blendedRotZ;
	worldTransform_.scale_ = blendedScale;

#ifdef _DEBUG
	ImGui::Begin("Debug Follow");
	ImGui::Text("Aim: %.2f", localAimAngle);
	ImGui::Text("Corrected Aim: %.2f", correctedAimAngle);
	ImGui::Text("Facing Right: %s", facingRight ? "true" : "false");
	ImGui::Text("Parent Y: %.2f", parentTransform.rotation_.y);
	ImGui::Text("Weapon Z: %.2f", worldTransform_.rotation_.z);
	ImGui::Text("Showing Reload Pose: %s", isShowingReloadPose_ ? "true" : "false");
	ImGui::End();
#endif

	WorldTransformUpdate(worldTransform_);
}
bool FirearmMusketBayonet::TryFire() {
	if (currentAmmo_ <= 0) {
		return false;
	}
	--currentAmmo_;
	return true;
}

void FirearmMusketBayonet::NotifyDealtDamage() { reloadDurationCurrent_ = std::max(reloadDurationCurrent_ - kReloadHitBonus, kReloadDurationMin); }