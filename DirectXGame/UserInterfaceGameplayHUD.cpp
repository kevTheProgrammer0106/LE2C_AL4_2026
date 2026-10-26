#include "UserInterfaceGameplayHUD.h"

using namespace KamataEngine;

namespace {
// Shared by every flat-colour bar (player health, block gauge, boss bar).
std::unique_ptr<Sprite> MakeFlatSprite(uint32_t texture, const Vector2& pos, const Vector2& size, const Vector4& color) {
	std::unique_ptr<Sprite> s(Sprite::Create(texture, pos));
	s->SetSize(size);
	s->SetColor(color);
	return s;
}
} // namespace

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
	border_ = MakeFlatSprite(
	    backgroundTextureHandle, {kBarX - kBorderThickness, kBarY - kBorderThickness}, {kBarWidth + kBorderThickness * 2.0f, kBarHeight + kBorderThickness * 2.0f}, {0.0f, 0.0f, 0.0f, 0.9f});
	background_ = MakeFlatSprite(backgroundTextureHandle, {kBarX, kBarY}, {kBarWidth, kBarHeight}, {0.2f, 0.05f, 0.05f, 0.9f});
	fill_ = MakeFlatSprite(fillTextureHandle, {kBarX, kBarY}, {kBarWidth, kBarHeight}, {0.2f, 0.8f, 0.2f, 1.0f});
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

// --- PlayerBlockUI ---
void PlayerBlockUI::Initialize(uint32_t backgroundTextureHandle, uint32_t fillTextureHandle) {
	barX_ = (static_cast<float>(WinApp::kWindowWidth) - kBarWidth) * 0.5f;
	barY_ = static_cast<float>(WinApp::kWindowHeight) - kBottomMargin - kBarHeight;

	border_ = MakeFlatSprite(
	    backgroundTextureHandle, {barX_ - kBorderThickness, barY_ - kBorderThickness}, {kBarWidth + kBorderThickness * 2.0f, kBarHeight + kBorderThickness * 2.0f}, {0.0f, 0.0f, 0.0f, 0.9f});
	background_ = MakeFlatSprite(backgroundTextureHandle, {barX_, barY_}, {kBarWidth, kBarHeight}, {0.05f, 0.07f, 0.15f, 0.9f});
	fill_ = MakeFlatSprite(fillTextureHandle, {barX_, barY_}, {kBarWidth, kBarHeight}, {0.25f, 0.55f, 0.95f, 1.0f});
	marker_ = MakeFlatSprite(fillTextureHandle, {barX_, barY_ - 4.0f}, {3.0f, kBarHeight + 8.0f}, {1.0f, 1.0f, 1.0f, 0.9f});
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

// --- BossHealthBar ---
void BossHealthBar::Initialize(uint32_t backgroundTextureHandle, uint32_t fillTextureHandle) {
	barX_ = static_cast<float>(WinApp::kWindowWidth) - kMarginRight - kBarWidth;
	const Vector2 size{kBarWidth, kBarHeight};

	border_ = MakeFlatSprite(
	    backgroundTextureHandle, {barX_ - kBorderThickness, kBarY - kBorderThickness}, {kBarWidth + kBorderThickness * 2.0f, kBarHeight + kBorderThickness * 2.0f}, {0.0f, 0.0f, 0.0f, 0.9f});
	background_ = MakeFlatSprite(backgroundTextureHandle, {barX_, kBarY}, size, {0.15f, 0.04f, 0.04f, 0.9f});
	chip_ = MakeFlatSprite(fillTextureHandle, {barX_, kBarY}, size, {0.95f, 0.85f, 0.75f, 1.0f});
	fill_ = MakeFlatSprite(fillTextureHandle, {barX_, kBarY}, size, {0.8f, 0.12f, 0.12f, 1.0f});
	alpha_ = 0.0f;
}

void BossHealthBar::Update(float currentHP, float maxHP, bool visible) {
	if (!fill_ || !chip_ || !background_ || !border_) {
		return;
	}
	constexpr float dt = 1.0f / 60.0f;

	// while hidden keep the last values so the bar fades out where it was
	if (visible) {
		targetFill_ = maxHP > 0.0f ? std::clamp(currentHP / maxHP, 0.0f, 1.0f) : 0.0f;
	}
	alpha_ += ((visible ? 1.0f : 0.0f) - alpha_) * kFadeRate;
	if (!visible && alpha_ < 0.002f) {
		alpha_ = 0.0f;
	}

	shownFill_ += (targetFill_ - shownFill_) * kFillRate;
	if (std::fabs(targetFill_ - shownFill_) < 0.0005f) {
		shownFill_ = targetFill_;
	}
	// chip: snaps up with healing, trails down at a fixed speed after damage
	if (chipFill_ < shownFill_) {
		chipFill_ = shownFill_;
	} else {
		chipFill_ = std::max(shownFill_, chipFill_ - kChipDropPerSecond * dt);
	}

	fill_->SetSize({kBarWidth * shownFill_, kBarHeight});
	chip_->SetSize({kBarWidth * chipFill_, kBarHeight});

	border_->SetColor({0.0f, 0.0f, 0.0f, 0.9f * alpha_});
	background_->SetColor({0.15f, 0.04f, 0.04f, 0.9f * alpha_});
	chip_->SetColor({0.95f, 0.85f, 0.75f, alpha_});
	fill_->SetColor({0.8f, 0.12f, 0.12f, alpha_});
}

void BossHealthBar::Draw() {
	if (!IsVisible() || !fill_) {
		return;
	}
	Sprite::PreDraw();
	border_->Draw();
	background_->Draw();
	chip_->Draw();
	fill_->Draw();
	Sprite::PostDraw();
}

// --- SwitchPointer ---
void SwitchPointer::Initialize(uint32_t arrowTextureHandle) {
	arrowTexture_ = arrowTextureHandle;
	targets_.clear();
	active_ = false;
}

void SwitchPointer::Start(const Vector3& switchWorldPos, const std::vector<Vector3>& gateWorldPositions) {
	switchPos_ = switchWorldPos;
	targets_.clear();
	for (const Vector3& gate : gateWorldPositions) {
		Target t;
		t.gatePos = gate;
		t.sprite.reset(Sprite::Create(arrowTexture_, {0.0f, 0.0f}));
		t.sprite->SetSize({kIconSize, kIconSize});
		t.sprite->SetAnchorPoint({0.5f, 0.5f});
		targets_.push_back(std::move(t));
	}
	timer_ = 0.0f;
	active_ = !targets_.empty();
}

void SwitchPointer::Update(const Camera& camera) {
	if (!active_) {
		return;
	}
	timer_ += 1.0f / 60.0f;
	if (timer_ >= kDuration) {
		active_ = false;
		targets_.clear();
		return;
	}

	float alpha = 1.0f;
	if (timer_ < kFadeIn) {
		alpha = timer_ / kFadeIn;
	} else if (timer_ > kDuration - kFadeOut) {
		alpha = (kDuration - timer_) / kFadeOut;
	}
	alpha = std::clamp(alpha, 0.0f, 1.0f);

	const float w = static_cast<float>(WinApp::kWindowWidth);
	const float h = static_cast<float>(WinApp::kWindowHeight);
	const float bob = std::sin(timer_ * kBobSpeed) * kBobAmplitude;
	// same projection StatusIndicator uses for the enemy icons
	const Matrix4x4 matViewport = MakeViewportMatrix(0.0f, 0.0f, w, h, 0.0f, 1.0f);
	const Matrix4x4 matVPV = Multiply(camera.matView, Multiply(camera.matProjection, matViewport));
	auto toScreen = [&](const Vector3& world) {
		const Vector3 p = TransformCoordinate(world, matVPV);
		return Vector2{p.x, p.y};
	};
	const Vector2 switchScreen = toScreen(switchPos_);

	for (Target& t : targets_) {
		const Vector2 gateScreen = toScreen(t.gatePos);
		const bool onScreen = gateScreen.x >= 0.0f && gateScreen.x <= w && gateScreen.y >= 0.0f && gateScreen.y <= h;

		Vector2 pos;
		float rotation;
		if (onScreen) {
			// float above the gate, pointing down at it
			pos = {gateScreen.x, gateScreen.y - kAboveGatePx + bob};
			rotation = std::numbers::pi_v<float> * 0.5f;
		} else {
			// midpoint between switch and gate, kept inside the screen, pointing at the gate
			pos = {(switchScreen.x + gateScreen.x) * 0.5f, (switchScreen.y + gateScreen.y) * 0.5f};
			pos.x = std::clamp(pos.x, kEdgeMargin, w - kEdgeMargin);
			pos.y = std::clamp(pos.y, kEdgeMargin, h - kEdgeMargin);
			rotation = std::atan2(gateScreen.y - pos.y, gateScreen.x - pos.x);
			const float dx = std::cos(rotation) * bob;
			const float dy = std::sin(rotation) * bob;
			pos.x += dx;
			pos.y += dy;
		}
		t.sprite->SetPosition(pos);
		t.sprite->SetRotation(rotation);
		t.sprite->SetColor({1.0f, 0.9f, 0.3f, alpha});
	}
}

void SwitchPointer::Draw() {
	if (!active_) {
		return;
	}
	Sprite::PreDraw();
	for (Target& t : targets_) {
		t.sprite->Draw();
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
	firearmPistols_.Initialize(tex.firearmDualPistols, {firearmX, iconY});
	firearmShotgun_.Initialize(tex.firearmBlunderbuss, {firearmX, iconY});
	bayonetLunge_.Initialize(tex.bayonetLunge, {lungeX, iconY});
	meleeAxe_.Initialize(tex.meleeAxe, {meleeX, iconY});
	meleeSpear_.Initialize(tex.meleeSpear, {meleeX, iconY});
}

void WeaponStatusUI::Update(const State& s) {
	shownMelee_ = s.meleeKind;
	shownFirearm_ = s.firearmKind;

	const bool heavy = (s.meleeKind == DropWeaponKind::kSabreHeavy);
	const bool bayonet = (s.firearmKind == DropWeaponKind::kMusketBayonet);
	const bool dual = (s.firearmKind == DropWeaponKind::kDualPistols);
	const Vector4& meleeBase = s.isFirearmMode ? kInactiveColor : kActiveColor;
	const Vector4& firearmBase = s.isFirearmMode ? kActiveColor : kInactiveColor;

	// only the held weapon's icon gets the live cooldown
	meleeLight_.Update(meleeBase, false, 1.0f);
	meleeHeavy_.Update(meleeBase, heavy && s.isMeleeCooling, s.meleeCooldownProgress01);
	meleeAxe_.Update(meleeBase, false, 1.0f);
	meleeSpear_.Update(meleeBase, false, 1.0f);

	firearmLong_.Update(firearmBase, !bayonet && !dual && s.isReloading, s.reloadProgress01);
	firearmPistols_.Update(firearmBase, dual && s.isReloading, s.reloadProgress01);
	firearmBayonet_.Update(firearmBase, bayonet && s.isReloading, s.reloadProgress01);
	bayonetLunge_.Update(firearmBase, bayonet && s.isLungeCooling, s.lungeCooldownProgress01);
	firearmShotgun_.Update(firearmBase, !bayonet && s.isReloading, s.reloadProgress01);
}

void WeaponStatusUI::Draw() {
	Sprite::PreDraw();
	switch (shownMelee_) {
	case DropWeaponKind::kSabreHeavy:
		meleeHeavy_.Draw();
		break;
	case DropWeaponKind::kLongAxe:
		meleeAxe_.Draw();
		break;
	case DropWeaponKind::kSpear:
		meleeSpear_.Draw();
		break;
	default:
		meleeLight_.Draw();
		break;
	}

	switch (shownFirearm_) {
	case DropWeaponKind::kMusketBayonet:
		firearmBayonet_.Draw();
		bayonetLunge_.Draw();
		break;
	case DropWeaponKind::kShotgun:
		firearmShotgun_.Draw();
		break;
	case DropWeaponKind::kDualPistols:
		firearmPistols_.Draw();
		break;
	default:
		firearmLong_.Draw();
		break;
	}
	Sprite::PostDraw();
}