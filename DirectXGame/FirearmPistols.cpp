#define NOMINMAX
#include "FirearmPistols.h"
#include "GlobalVariables.h"
#include "PoseGV.h"

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

	// ---- 2丁目のホルスター / リアクションの構え ----
	PoseGV::Register(kGroupName, "Holster2", pistolHolsterOffset2_);
	PoseGV::Register(kGroupName, "Knockback", pistolKnockbackOffset_);
	PoseGV::Register(kGroupName, "Stun", pistolStunOffset_);
	PoseGV::Register(kGroupName, "Block", pistolBlockOffset_);

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

	PoseGV::Apply(kGroupName, "Holster2", pistolHolsterOffset2_);
	PoseGV::Apply(kGroupName, "Knockback", pistolKnockbackOffset_);
	PoseGV::Apply(kGroupName, "Stun", pistolStunOffset_);
	PoseGV::Apply(kGroupName, "Block", pistolBlockOffset_);

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
			PlaySfxHandle(sLoadedSoundHandle_); // the "loaded" jingle plays once per pistol
			cycleTimer_[slot] = kCycleDuration; // ...together with the cycling kick
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

	// 収納中だけ、2丁目のホルスター(pistolHolsterOffset2_)と1丁目のホルスターの差分を足す。
	// 基底の合成は Lerp(収納, 戦闘, equipFactor_) なので、2丁目は (1 - equipFactor_) * (holster2 - holster1) だけずれる。
	// リロードポーズ中は2丁とも同じ収納ポーズなので差分なし。
	const float stow = 1.0f - equipFactor_;
	Vector3 holsterTranslation = {0.0f, 0.0f, 0.0f};
	Vector3 holsterRotation = {0.0f, 0.0f, 0.0f};
	Vector3 holsterScale = {0.0f, 0.0f, 0.0f};
	if (!isShowingReloadPose_) {
		const SRTOffset& h1 = pistolHolsterOffset_;
		const SRTOffset& h2 = pistolHolsterOffset2_;
		const float mirror = facingRight ? 1.0f : -1.0f; // 基底と同じく、左向きはy/zを反転
		holsterTranslation = {(h2.translation.x - h1.translation.x) * stow, (h2.translation.y - h1.translation.y) * stow, (h2.translation.z - h1.translation.z) * stow};
		holsterRotation = {(h2.rotation.x - h1.rotation.x) * stow, (h2.rotation.y - h1.rotation.y) * stow * mirror, (h2.rotation.z - h1.rotation.z) * stow * mirror};
		holsterScale = {(h2.scale.x - h1.scale.x) * stow, (h2.scale.y - h1.scale.y) * stow, (h2.scale.z - h1.scale.z) * stow};
	}

	Vector3 delta = TransformNormal(
	    Vector3{kSecondDelta.x * equipFactor_ + holsterTranslation.x, kSecondDelta.y * equipFactor_ + holsterTranslation.y, kSecondDelta.z * equipFactor_ + holsterTranslation.z}, rotMatrix);

	worldTransformSecond_.translation_ = worldTransform_.translation_ + delta;
	worldTransformSecond_.rotation_ = {worldTransform_.rotation_.x + holsterRotation.x, worldTransform_.rotation_.y + holsterRotation.y, worldTransform_.rotation_.z + holsterRotation.z};
	worldTransformSecond_.scale_ = {worldTransform_.scale_.x + holsterScale.x, worldTransform_.scale_.y + holsterScale.y, worldTransform_.scale_.z + holsterScale.z};

	// Cycling kick: when a pistol finishes loading its muzzle flicks up and eases back (like pumping a shotgun).
	// Applied after the second pistol was derived, so each pistol kicks on its own.
	constexpr float kDt = 1.0f / 60.0f;
	const float sign = facingRight ? 1.0f : -1.0f;
	WorldTransform* transforms[2] = {&worldTransform_, &worldTransformSecond_};
	for (int i = 0; i < 2; ++i) {
		if (cycleTimer_[i] > 0.0f) {
			cycleTimer_[i] = std::max(0.0f, cycleTimer_[i] - kDt);
		}
		const float t = 1.0f - cycleTimer_[i] / std::max(kCycleDuration, 0.001f); // 0 -> 1
		const float w = (cycleTimer_[i] > 0.0f) ? std::sin(std::numbers::pi_v<float> * t) * equipFactor_ : 0.0f;
		if (w > 0.0f) {
			transforms[i]->rotation_.z += sign * kCycleKickAngle * w;
			transforms[i]->translation_.y += kCycleRise * w;
		}
		WorldTransformUpdate(*transforms[i]); // refreshes matWorld_ (the muzzle position reads it)
	}
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