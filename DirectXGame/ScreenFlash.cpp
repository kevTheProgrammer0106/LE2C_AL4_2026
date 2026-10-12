#define NOMINMAX
#include "ScreenFlash.h"
#include "GlobalVariables.h"

#include <algorithm>

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "ScreenFlash";
constexpr float kDeltaTime = 1.0f / 60.0f;
} // namespace

void ScreenFlash::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "MaxAlpha", kMaxAlpha);
	gv->AddItem(kGroupName, "FlashDuration", kFlashDuration);
	gv->AddItem(kGroupName, "LowHpAlpha", kLowHpAlpha);
	gv->AddItem(kGroupName, "StunAlpha", kStunAlpha);
	gv->AddItem(kGroupName, "PersistentFade", kPersistentFade);
	gv->AddItem(kGroupName, "LowHpThreshold", kLowHpThreshold);
	gv->AddItem(kGroupName, "DamageColor", kDamageColor);
	gv->AddItem(kGroupName, "HealColor", kHealColor);
	gv->AddItem(kGroupName, "BlockColor", kBlockColor);
	gv->AddItem(kGroupName, "StunColor", kStunColor);
}

void ScreenFlash::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kMaxAlpha = gv->GetFloatValue(kGroupName, "MaxAlpha");
	kFlashDuration = std::max(gv->GetFloatValue(kGroupName, "FlashDuration"), 0.01f);
	kLowHpAlpha = gv->GetFloatValue(kGroupName, "LowHpAlpha");
	kStunAlpha = gv->GetFloatValue(kGroupName, "StunAlpha");
	kPersistentFade = gv->GetFloatValue(kGroupName, "PersistentFade");
	kLowHpThreshold = gv->GetIntValue(kGroupName, "LowHpThreshold");
	kDamageColor = gv->GetVector3Value(kGroupName, "DamageColor");
	kHealColor = gv->GetVector3Value(kGroupName, "HealColor");
	kBlockColor = gv->GetVector3Value(kGroupName, "BlockColor");
	kStunColor = gv->GetVector3Value(kGroupName, "StunColor");
}

void ScreenFlash::Initialize(uint32_t whiteTextureHandle) {
	// Fadeと同じ作り: white1x1 を画面全体に引き伸ばし、色と不透明度だけを変える
	sprite_.reset(Sprite::Create(whiteTextureHandle, Vector2(0.0f, 0.0f), Vector4(1.0f, 1.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f)));
	sprite_->SetSize(Vector2(static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)));
	flashTimer_ = 0.0f;
	persistentAlpha_ = 0.0f;
	stunned_ = false;
	lowHealth_ = false;
}

void ScreenFlash::Trigger(Type type) {
	flashType_ = type;
	flashTimer_ = kFlashDuration;
}

void ScreenFlash::Update() {
	if (!sprite_) {
		return;
	}

	// ---- 出続ける色: スタンが優先、次に低HP。出入りは滑らかに補間 ----
	float targetAlpha = 0.0f;
	if (stunned_) {
		persistentColor_ = kStunColor;
		targetAlpha = kStunAlpha;
	} else if (lowHealth_) {
		persistentColor_ = kDamageColor;
		targetAlpha = kLowHpAlpha;
	}
	persistentAlpha_ = Lerp(persistentAlpha_, targetAlpha, kPersistentFade);
	if (persistentAlpha_ < 0.001f) {
		persistentAlpha_ = 0.0f;
	}

	// ---- 一瞬のフラッシュ: 最大の不透明度から二乗で薄れる ----
	float flashAlpha = 0.0f;
	Vector3 flashColor = kDamageColor;
	if (flashTimer_ > 0.0f) {
		flashTimer_ = std::max(0.0f, flashTimer_ - kDeltaTime);
		const float t = flashTimer_ / kFlashDuration; // 1=出た直後 / 0=消えた
		flashAlpha = kMaxAlpha * t * t;
		switch (flashType_) {
		case Type::kDamage:
			flashColor = kDamageColor;
			break;
		case Type::kHeal:
			flashColor = kHealColor;
			break;
		case Type::kBlock:
			flashColor = kBlockColor;
			break;
		}
	}

	// 強いほうを表示する。低HP中の被弾は、赤い出続ける色の上に強い赤が点滅して戻る
	if (flashAlpha > persistentAlpha_) {
		sprite_->SetColor(Vector4(flashColor.x, flashColor.y, flashColor.z, flashAlpha));
	} else {
		sprite_->SetColor(Vector4(persistentColor_.x, persistentColor_.y, persistentColor_.z, persistentAlpha_));
	}
}

void ScreenFlash::Draw() {
	if (!sprite_ || (flashTimer_ <= 0.0f && persistentAlpha_ <= 0.0f)) {
		return;
	}
	Sprite::PreDraw();
	sprite_->Draw();
	Sprite::PostDraw();
}