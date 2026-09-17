#include "UserInterfaceGameplayHUD.h"

using namespace KamataEngine;

void InteractionPrompt::Initialize(uint32_t iconTextureHandle) {
	BaseSprite::Initialize(iconTextureHandle, {kScreenAnchorX, kScreenAnchorY});
	SetSize({200.0f, 200.0f});
	isVisible_ = false;
}

void InteractionPrompt::Show(Kind kind) {
	kind_ = kind;
	isVisible_ = true;
}

void InteractionPrompt::Hide() {
	isVisible_ = false;
	kind_ = Kind::kNone;
}

void InteractionPrompt::Update() {
	bobTimer_ += 1.0f / 60.0f;
	if (!isVisible_)
		return;
	float bob = std::sin(bobTimer_ * kBobSpeed) * kBobAmplitude;
	SetPosition({kScreenAnchorX, kScreenAnchorY - bob});
	BaseSprite::Update();
}

void InteractionPrompt::Draw() {
	if (!isVisible_)
		return;
	BaseSprite::Draw();
}

// --- PlayerHealthUI ---
void PlayerHealthUI::Initialize(uint32_t backgroundTextureHandle, uint32_t fillTextureHandle) {
	border_.reset(Sprite::Create(backgroundTextureHandle, {kBarX - kBorderThickness, kBarY - kBorderThickness}));
	border_->SetSize({kBarWidth + kBorderThickness * 2.0f, kBarHeight + kBorderThickness * 2.0f});
	border_->SetColor({0.0f, 0.0f, 0.0f, 0.9f});

	background_.reset(Sprite::Create(backgroundTextureHandle, {kBarX, kBarY}));
	background_->SetSize({kBarWidth, kBarHeight});
	background_->SetColor({0.2f, 0.05f, 0.05f, 0.9f});

	fill_.reset(Sprite::Create(fillTextureHandle, {kBarX, kBarY}));
	fill_->SetSize({kBarWidth, kBarHeight});
	fill_->SetColor({0.2f, 0.8f, 0.2f, 1.0f});
}

void PlayerHealthUI::Update(int hp, int maxHp) {
	if (!fill_) {
		return;
	}
	float t = maxHp > 0 ? std::clamp(static_cast<float>(hp) / static_cast<float>(maxHp), 0.0f, 1.0f) : 0.0f;

	fill_->SetSize({kBarWidth * t, kBarHeight});

	Vector4 color;
	if (t > 0.5f) {
		color = {0.2f, 0.8f, 0.2f, 1.0f};
	} else if (t > 0.25f) {
		color = {0.9f, 0.75f, 0.1f, 1.0f};
	} else {
		color = {0.85f, 0.15f, 0.15f, 1.0f};
	}
	fill_->SetColor(color);
}

void PlayerHealthUI::Draw() {
	if (!background_ || !fill_ || !border_) {
		return;
	}
	Sprite::PreDraw();
	border_->Draw();
	background_->Draw();
	fill_->Draw();
	Sprite::PostDraw();
}

// --- WeaponStatusUI ---
void WeaponStatusUI::Initialize(uint32_t meleeIconTexture, uint32_t firearmIconTexture, uint32_t barBackgroundTexture, uint32_t barFillTexture) {
	float iconY = PlayerHealthUI::GetBarBottomY() + kTopOffsetFromHealthBar;
	float meleeX = PlayerHealthUI::GetBarX();
	float firearmX = meleeX + kIconSize + kIconSpacing;

	meleeIcon_.reset(Sprite::Create(meleeIconTexture, {meleeX, iconY}));
	meleeIcon_->SetSize({kIconSize, kIconSize});
	meleeIcon_->SetColor(kInactiveColor);

	firearmIcon_.reset(Sprite::Create(firearmIconTexture, {firearmX, iconY}));
	firearmIcon_->SetSize({kIconSize, kIconSize});
	firearmIcon_->SetColor(kInactiveColor);

	float ammoBarY = iconY + kIconSize + kAmmoBarTopOffset;

	ammoBarBorder_.reset(Sprite::Create(barBackgroundTexture, {firearmX - kAmmoBarBorderThickness, ammoBarY - kAmmoBarBorderThickness}));
	ammoBarBorder_->SetSize({kIconSize + kAmmoBarBorderThickness * 2.0f, kAmmoBarHeight + kAmmoBarBorderThickness * 2.0f});
	ammoBarBorder_->SetColor({0.0f, 0.0f, 0.0f, 0.9f});

	ammoBarBackground_.reset(Sprite::Create(barBackgroundTexture, {firearmX, ammoBarY}));
	ammoBarBackground_->SetSize({kIconSize, kAmmoBarHeight});
	ammoBarBackground_->SetColor({0.15f, 0.15f, 0.05f, 0.9f});

	ammoBarFill_.reset(Sprite::Create(barFillTexture, {firearmX, ammoBarY}));
	ammoBarFill_->SetSize({kIconSize, kAmmoBarHeight});
	ammoBarFill_->SetColor({0.9f, 0.8f, 0.2f, 1.0f});
}

void WeaponStatusUI::Update(bool isFirearmMode, bool isReloading, float reloadProgress01) {
	if (!meleeIcon_ || !firearmIcon_ || !ammoBarFill_) {
		return;
	}

	meleeIcon_->SetColor(isFirearmMode ? kInactiveColor : kActiveColor);
	firearmIcon_->SetColor(isFirearmMode ? kActiveColor : kInactiveColor);

	float t = std::clamp(reloadProgress01, 0.0f, 1.0f);
	ammoBarFill_->SetSize({kIconSize * t, kAmmoBarHeight});

	//  same visual language as HP: yellow/orange while reloading (draining/refilling),
	// steady green once ready, so "reloading" reads the same way "low HP" does.
	Vector4 color = isReloading ? Vector4{0.9f, 0.6f, 0.15f, 1.0f} : Vector4{0.3f, 0.85f, 0.3f, 1.0f};
	ammoBarFill_->SetColor(color);
}

void WeaponStatusUI::Draw() {
	if (!meleeIcon_ || !firearmIcon_ || !ammoBarBorder_ || !ammoBarBackground_ || !ammoBarFill_) {
		return;
	}
	Sprite::PreDraw();
	meleeIcon_->Draw();
	firearmIcon_->Draw();
	ammoBarBorder_->Draw();
	ammoBarBackground_->Draw();
	ammoBarFill_->Draw();
	Sprite::PostDraw();
}