#define NOMINMAX
#include "FirearmPistols.h"
#include "GlobalVariables.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "FirearmPistols";
}

void FirearmPistols::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);

	gv->AddItem(kGroupName, "OffsetScale", pistolOffset_.scale);
	gv->AddItem(kGroupName, "OffsetRotate", pistolOffset_.rotation);
	gv->AddItem(kGroupName, "OffsetTranslate", pistolOffset_.translation);
	gv->AddItem(kGroupName, "OffsetHolsterScale", pistolHolsterOffset_.scale);
	gv->AddItem(kGroupName, "OffsetHolsterRotate", pistolHolsterOffset_.rotation);
	gv->AddItem(kGroupName, "OffsetHolsterTranslate", pistolHolsterOffset_.translation);
	gv->AddItem(kGroupName, "OffsetReloadScale", pistolReloadOffset_.scale);
	gv->AddItem(kGroupName, "OffsetReloadRotate", pistolReloadOffset_.rotation);
	gv->AddItem(kGroupName, "OffsetReloadTranslate", pistolReloadOffset_.translation);
	gv->AddItem(kGroupName, "SecondPistolDelta", kSecondDelta);

	gv->AddItem(kGroupName, "ReloadDurationPlayer", kReloadDurationPlayer);
	gv->AddItem(kGroupName, "ReloadHitBonusPlayer", kReloadHitBonusPlayer);
	gv->AddItem(kGroupName, "ReloadDurationMinPlayer", kReloadDurationMinPlayer);
	gv->AddItem(kGroupName, "ReloadDurationEnemy", kReloadDurationEnemy);
	gv->AddItem(kGroupName, "ReloadHitBonusEnemy", kReloadHitBonusEnemy);
	gv->AddItem(kGroupName, "ReloadDurationMinEnemy", kReloadDurationMinEnemy);

	gv->AddItem(kGroupName, "DamageToEnemy", kDamageToEnemy);
	gv->AddItem(kGroupName, "DamageToPlayer", kDamageToPlayer);
}

void FirearmPistols::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();

	pistolOffset_.scale = gv->GetVector3Value(kGroupName, "OffsetScale");
	pistolOffset_.rotation = gv->GetVector3Value(kGroupName, "OffsetRotate");
	pistolOffset_.translation = gv->GetVector3Value(kGroupName, "OffsetTranslate");
	pistolHolsterOffset_.scale = gv->GetVector3Value(kGroupName, "OffsetHolsterScale");
	pistolHolsterOffset_.rotation = gv->GetVector3Value(kGroupName, "OffsetHolsterRotate");
	pistolHolsterOffset_.translation = gv->GetVector3Value(kGroupName, "OffsetHolsterTranslate");
	pistolReloadOffset_.scale = gv->GetVector3Value(kGroupName, "OffsetReloadScale");
	pistolReloadOffset_.rotation = gv->GetVector3Value(kGroupName, "OffsetReloadRotate");
	pistolReloadOffset_.translation = gv->GetVector3Value(kGroupName, "OffsetReloadTranslate");
	kSecondDelta = gv->GetVector3Value(kGroupName, "SecondPistolDelta");

	kReloadDurationPlayer = gv->GetFloatValue(kGroupName, "ReloadDurationPlayer");
	kReloadHitBonusPlayer = gv->GetFloatValue(kGroupName, "ReloadHitBonusPlayer");
	kReloadDurationMinPlayer = gv->GetFloatValue(kGroupName, "ReloadDurationMinPlayer");
	kReloadDurationEnemy = gv->GetFloatValue(kGroupName, "ReloadDurationEnemy");
	kReloadHitBonusEnemy = gv->GetFloatValue(kGroupName, "ReloadHitBonusEnemy");
	kReloadDurationMinEnemy = gv->GetFloatValue(kGroupName, "ReloadDurationMinEnemy");

	kDamageToEnemy = gv->GetIntValue(kGroupName, "DamageToEnemy");
	kDamageToPlayer = gv->GetIntValue(kGroupName, "DamageToPlayer");
}

void FirearmPistols::Initialize(Model* model, uint32_t textureHandle, Camera* camera) {
	BaseFirearm::Initialize(model, textureHandle, camera); // ammo = 2, reload state reset
	worldTransformSecond_.Initialize();
	colorSlot_[0].Initialize();
	colorSlot_[1].Initialize();
	loaded_[0] = loaded_[1] = true;
	reloadQueue_.clear();
	nextSlot_ = 0;
	lastFiredSlot_ = 0;
	UpdateColors();
}

void FirearmPistols::UpdateColors() {
	for (int i = 0; i < 2; ++i) {
		colorSlot_[i].SetColor(loaded_[i] ? Vector4{1.0f, 1.0f, 1.0f, 1.0f} : kUnloadedColor);
	}
}

bool FirearmPistols::TryFire() {
	if (currentAmmo_ <= 0) {
		return false;
	}
	// alternate pistols, falling back to whichever is loaded
	int slot = loaded_[nextSlot_] ? nextSlot_ : 1 - nextSlot_;
	if (!loaded_[slot]) {
		return false;
	}
	loaded_[slot] = false;
	reloadQueue_.push_back(slot); // reloaded in the order they were fired
	lastFiredSlot_ = slot;
	nextSlot_ = 1 - slot;
	--currentAmmo_;
	if (!isReloading_) {
		isReloading_ = true;
		reloadTimer_ = 0.0f;
	}
	UpdateColors();
	return true;
}

// One pistol per reload duration, so the full set takes twice as long.
void FirearmPistols::TickReload() {
	if (currentAmmo_ < GetMaxAmmo()) {
		reloadTimer_ += 1.0f / 60.0f;
		if (reloadTimer_ >= reloadDurationCurrent_ * reloadTimeScale_) {
			int slot = -1;
			if (!reloadQueue_.empty()) {
				slot = reloadQueue_.front();
				reloadQueue_.erase(reloadQueue_.begin());
			} else {
				slot = loaded_[0] ? 1 : 0; // safety: queue lost (should not happen)
			}
			loaded_[slot] = true;
			++currentAmmo_;
			reloadTimer_ = 0.0f;
			reloadDurationCurrent_ = GetBaseReloadDuration();
			PlaySfxHandle(sLoadedSoundHandle_);
			UpdateColors();
		}
	}
	isReloading_ = (currentAmmo_ < GetMaxAmmo());
}

float FirearmPistols::GetReloadProgress01() const {
	if (!isReloading_) {
		return 1.0f;
	}
	const float total = reloadDurationCurrent_ * reloadTimeScale_;
	const float slotT = (total > 0.0f) ? std::clamp(reloadTimer_ / total, 0.0f, 1.0f) : 1.0f;
	return std::clamp((static_cast<float>(currentAmmo_) + slotT) / static_cast<float>(GetMaxAmmo()), 0.0f, 1.0f);
}

void FirearmPistols::RestoreDropState(DropItemState state) {
	if (state != DropItemState::kOnCooldown) {
		return;
	}
	BaseFirearm::RestoreDropState(state); // ammo 0, reload restarts from zero
	loaded_[0] = loaded_[1] = false;
	reloadQueue_ = {0, 1};
	UpdateColors();
}

void FirearmPistols::UpdateFollow(const WorldTransform& parentTransform, float localAimAngle, bool facingRight) {
	BaseFirearm::UpdateFollow(parentTransform, localAimAngle, facingRight); // first pistol (+ ticks reload / equip once)

	// Second pistol = first pistol shifted in the parent's local space. Own WorldTransform (never copy one: shared constant buffer).
	Matrix4x4 rotMatrix = Multiply(MakeRotateXMatrix(parentTransform.rotation_.x), Multiply(MakeRotateYMatrix(parentTransform.rotation_.y), MakeRotateZMatrix(parentTransform.rotation_.z)));
	Vector3 delta = TransformNormal(Vector3{kSecondDelta.x * equipFactor_, kSecondDelta.y * equipFactor_, kSecondDelta.z * equipFactor_}, rotMatrix);

	worldTransformSecond_.translation_ = worldTransform_.translation_ + delta;
	worldTransformSecond_.rotation_ = worldTransform_.rotation_;
	worldTransformSecond_.scale_ = worldTransform_.scale_;
	WorldTransformUpdate(worldTransformSecond_);
}

void FirearmPistols::Draw() {
	if (!model_ || !camera_) {
		return;
	}
	Model::PreDraw();
	model_->Draw(worldTransform_, *camera_, textureHandle_, &colorSlot_[0]);
	model_->Draw(worldTransformSecond_, *camera_, textureHandle_, &colorSlot_[1]);
	Model::PostDraw();
}

Vector3 FirearmPistols::GetFiredMuzzleWorldPosition() const {
	// slot 0 = first pistol, slot 1 = shifted pistol
	const WorldTransform& wt = (lastFiredSlot_ == 0) ? worldTransform_ : worldTransformSecond_;
	return TransformCoordinate(GetMuzzleLocalOffset(), wt.matWorld_);
}