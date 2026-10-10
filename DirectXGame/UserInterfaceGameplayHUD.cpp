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
// --- PlayerBlockUI ---
void PlayerBlockUI::Initialize(uint32_t backgroundTextureHandle, uint32_t fillTextureHandle) {
	barX_ = (static_cast<float>(WinApp::kWindowWidth) - kBarWidth) * 0.5f;
	barY_ = static_cast<float>(WinApp::kWindowHeight) - kBottomMargin - kBarHeight;

	border_.reset(Sprite::Create(backgroundTextureHandle, {barX_ - kBorderThickness, barY_ - kBorderThickness}));
	border_->SetSize({kBarWidth + kBorderThickness * 2.0f, kBarHeight + kBorderThickness * 2.0f});
	border_->SetColor({0.0f, 0.0f, 0.0f, 0.9f});

	background_.reset(Sprite::Create(backgroundTextureHandle, {barX_, barY_}));
	background_->SetSize({kBarWidth, kBarHeight});
	background_->SetColor({0.05f, 0.07f, 0.15f, 0.9f});

	fill_.reset(Sprite::Create(fillTextureHandle, {barX_, barY_}));
	fill_->SetSize({kBarWidth, kBarHeight});
	fill_->SetColor({0.25f, 0.55f, 0.95f, 1.0f});

	marker_.reset(Sprite::Create(fillTextureHandle, {barX_, barY_ - 4.0f}));
	marker_->SetSize({3.0f, kBarHeight + 8.0f});
	marker_->SetColor({1.0f, 1.0f, 1.0f, 0.9f});
}

void PlayerBlockUI::Update(float gauge, float maxGauge, bool broken, float recoverThreshold) {
	if (!fill_ || !marker_) {
		return;
	}
	broken_ = broken;
	float t = maxGauge > 0.0f ? std::clamp(gauge / maxGauge, 0.0f, 1.0f) : 0.0f;
	fill_->SetSize({kBarWidth * t, kBarHeight});
	fill_->SetColor(broken ? Vector4{0.85f, 0.2f, 0.2f, 1.0f} : Vector4{0.25f, 0.55f, 0.95f, 1.0f});

	float markerT = maxGauge > 0.0f ? std::clamp(recoverThreshold / maxGauge, 0.0f, 1.0f) : 0.0f;
	marker_->SetPosition({barX_ + kBarWidth * markerT - 1.5f, barY_ - 4.0f});
}

void PlayerBlockUI::Draw() {
	if (!border_ || !background_ || !fill_ || !marker_) {
		return;
	}
	Sprite::PreDraw();
	border_->Draw();
	background_->Draw();
	fill_->Draw();
	if (broken_) {
		marker_->Draw();
	}
	Sprite::PostDraw();
}

// --- WeaponStatusUI ---
void WeaponStatusUI::CooldownIcon::Initialize(uint32_t tex, const Vector2& position) {
	pos = position;

	dark.reset(Sprite::Create(tex, pos));
	texSize = dark->GetSize(); // Create() defaults to the texture size
	dark->SetSize({kIconSize, kIconSize});

	bright.reset(Sprite::Create(tex, pos));
	bright->SetSize({kIconSize, kIconSize});

	dark->SetColor(kInactiveColor);
	bright->SetColor(kInactiveColor);
}

void WeaponStatusUI::CooldownIcon::Update(const Vector4& base, bool cooling, float progress01) {
	if (!dark || !bright) {
		return;
	}

	// cooldown just finished -> white flash + enlarge pop
	if (wasCooling && !cooling) {
		flashTimer = kFlashDuration;
		popTimer = kPopDuration;
	}
	wasCooling = cooling;
	constexpr float dt = 1.0f / 60.0f;
	flashTimer = std::max(0.0f, flashTimer - dt);
	popTimer = std::max(0.0f, popTimer - dt);

	float t = cooling ? std::clamp(progress01, 0.0f, 1.0f) : 1.0f;

	float flashT = (kFlashDuration > 0.0f) ? flashTimer / kFlashDuration : 0.0f;
	float boost = 1.0f + kFlashBoost * flashT;
	bright->SetColor({base.x * boost, base.y * boost, base.z * boost, base.w});
	dark->SetColor({base.x * kDarkFactor, base.y * kDarkFactor, base.z * kDarkFactor, base.w});

	// pop: 0 -> peak -> 0 over kPopDuration
	float popT = (kPopDuration > 0.0f) ? 1.0f - popTimer / kPopDuration : 1.0f;
	float scale = 1.0f + kPopScale * ((popTimer > 0.0f) ? std::sin(popT * std::numbers::pi_v<float>) : 0.0f);
	float size = kIconSize * scale;
	float extra = (size - kIconSize) * 0.5f; // grow from the icon's centre
	float left = pos.x - extra;
	float top = pos.y - extra;

	darkVisible = (t < 1.0f);
	brightVisible = (t > 0.01f);

	dark->SetPosition({left, top});
	dark->SetSize({size, size});

	if (brightVisible) {
		// reveal the bottom t of the icon (lightens upward)
		bright->SetTextureRect({0.0f, texSize.y * (1.0f - t)}, {texSize.x, texSize.y * t});
		bright->SetPosition({left, top + size * (1.0f - t)});
		bright->SetSize({size, size * t});
	}
}

void WeaponStatusUI::CooldownIcon::Draw() {
	if (!dark || !bright) {
		return;
	}
	if (darkVisible) {
		dark->Draw();
	}
	if (brightVisible) {
		bright->Draw();
	}
}

void WeaponStatusUI::Initialize(const Textures& tex) {
	float iconY = PlayerHealthUI::GetBarBottomY() + kTopOffsetFromHealthBar;
	float meleeX = PlayerHealthUI::GetBarX();
	float firearmX = meleeX + kIconSize + kIconSpacing;
	float lungeX = firearmX + kIconSize + kIconSpacing; // next to the musket icon

	meleeLight_.Initialize(tex.meleeLight, {meleeX, iconY});
	meleeHeavy_.Initialize(tex.meleeHeavy, {meleeX, iconY});
	firearmLong_.Initialize(tex.firearmLong, {firearmX, iconY});
	firearmBayonet_.Initialize(tex.firearmBayonet, {firearmX, iconY});
	bayonetLunge_.Initialize(tex.bayonetLunge, {lungeX, iconY});
}

void WeaponStatusUI::Update(const State& s) {
	shownMelee_ = s.meleeKind;
	shownFirearm_ = s.firearmKind;

	const bool heavy = (s.meleeKind == DropWeaponKind::kSabreHeavy);
	const bool bayonet = (s.firearmKind == DropWeaponKind::kMusketBayonet);
	const Vector4& meleeBase = s.isFirearmMode ? kInactiveColor : kActiveColor;
	const Vector4& firearmBase = s.isFirearmMode ? kActiveColor : kInactiveColor;

	// only the held weapon's icon gets the live cooldown
	meleeLight_.Update(meleeBase, false, 1.0f);
	meleeHeavy_.Update(meleeBase, heavy && s.isMeleeCooling, s.meleeCooldownProgress01);
	firearmLong_.Update(firearmBase, !bayonet && s.isReloading, s.reloadProgress01);
	firearmBayonet_.Update(firearmBase, bayonet && s.isReloading, s.reloadProgress01);
	bayonetLunge_.Update(firearmBase, bayonet && s.isLungeCooling, s.lungeCooldownProgress01);
}

void WeaponStatusUI::Draw() {
	Sprite::PreDraw();
	(shownMelee_ == DropWeaponKind::kSabreHeavy ? meleeHeavy_ : meleeLight_).Draw();
	if (shownFirearm_ == DropWeaponKind::kMusketBayonet) {
		firearmBayonet_.Draw();
		bayonetLunge_.Draw();
	} else {
		firearmLong_.Draw();
	}
	Sprite::PostDraw();
}