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

	kReloadDuration = gv->GetFloatValue(kGroupName, "ReloadDuration");
	kReloadHitBonus = gv->GetFloatValue(kGroupName, "ReloadHitBonus");
	kReloadDurationMin = gv->GetFloatValue(kGroupName, "ReloadDurationMin");
}

void FirearmMusket::UpdateFollow(const WorldTransform& parentTransform, float localAimAngle, bool facingRight) {

	equipFactor_ = Lerp(equipFactor_, equipTarget_, kEquipLerpSpeed);

	// リロードは収納中(holstered)のみ進行する。装備中は一時停止する。
	//bool isHolstered = equipTarget_ < 0.5f;
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
	

	Matrix4x4 rotMatrix = Multiply(MakeRotateXMatrix(parentTransform.rotation_.x), Multiply(MakeRotateYMatrix(parentTransform.rotation_.y), MakeRotateZMatrix(parentTransform.rotation_.z)));


	// While equipped: ease the facing sign continuously so it stays in sync with the
	// body's turn animation (see previous fix). While holstered: snap instantly,
	// same philosophy as WeaponSabre::SetWeaponState snapping its state immediately.
	float facingSign = facingRight ? 1.0f : -1.0f;
	float correctedAimAngle = localAimAngle * facingSign;

	constexpr float kModelYaw = std::numbers::pi_v<float> / 2.0f;

Vector3 holsterRotation = musketHolsterOffset_.rotation;

	if (!facingRight) {
		holsterRotation.y = -holsterRotation.y;
		holsterRotation.z = -holsterRotation.z;
	}

	float holsterRotY = parentTransform.rotation_.y + holsterRotation.y;
	float holsterRotZ = holsterRotation.z;

	float combatRotY = parentTransform.rotation_.y + kModelYaw + musketOffset_.rotation.y;

	/*float combatRotY = parentTransform.rotation_.y + std::numbers::pi_v<float> / 2.0f + musketOffset_.rotation.y;*/
	float combatRotZ = correctedAimAngle + musketOffset_.rotation.z;

	Vector3 blendedLocalOffset = Lerp(musketHolsterOffset_.translation, musketOffset_.translation, equipFactor_);


	float blendedRotY = Lerp(holsterRotY, combatRotY, equipFactor_);
	float blendedRotZ = Lerp(holsterRotZ, combatRotZ, equipFactor_);

	Vector3 blendedScale = Lerp(musketHolsterOffset_.scale, musketOffset_.scale, equipFactor_);

	Vector3 rotatedOffset = TransformNormal(blendedLocalOffset, rotMatrix);

	worldTransform_.translation_ = parentTransform.translation_ + rotatedOffset;

	worldTransform_.rotation_.x = parentTransform.rotation_.x + Lerp(musketHolsterOffset_.rotation.x, musketOffset_.rotation.x, equipFactor_);

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
	ImGui::End();
#endif

	WorldTransformUpdate(worldTransform_);
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
	reloadDurationCurrent_ = max(reloadDurationCurrent_ - kReloadHitBonus, kReloadDurationMin);
}