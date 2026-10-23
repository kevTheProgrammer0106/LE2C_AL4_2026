#define NOMINMAX

#include "UIMenu.h"

using namespace KamataEngine;

void UIMenu::Initialize(uint32_t tex, KamataEngine::Camera* camera) {
	camera_ = camera;

	buttonTextures_.clear();
	buttonTextures_.push_back(tex);

	barTexture_ = TextureManager::Load("./Resources/white1x1.png");

	worldTransform_.Initialize();

	Vector2 targetPos = {worldTransform_.translation_.x, worldTransform_.translation_.y};

	cursorSprite_.reset(Sprite::Create(tex, targetPos));
	cursorSprite_->SetAnchorPoint({0.5f, 0.5f});
	cursorSprite_->SetSize({32.0f, 32.0f});

	if (clickSoundDataHandle_ == 0) {
		clickSoundDataHandle_ = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	}
}

void UIMenu::Initialize(const uint32_t* textures, size_t textureCount, KamataEngine::Camera* camera) {
	camera_ = camera;

	buttonTextures_.clear();
	if (textures != nullptr && textureCount > 0) {
		buttonTextures_.assign(textures, textures + textureCount);
	}

	barTexture_ = TextureManager::Load("./Resources/white1x1.png");

	worldTransform_.Initialize();

	uint32_t cursorTexture = 0;
	if (!buttonTextures_.empty()) {
		cursorTexture = buttonTextures_[0];
	}

	Vector2 targetPos = {worldTransform_.translation_.x, worldTransform_.translation_.y};

	cursorSprite_.reset(Sprite::Create(cursorTexture, targetPos));
	cursorSprite_->SetAnchorPoint({0.5f, 0.5f});
	cursorSprite_->SetSize({32.0f, 32.0f});

	if (clickSoundDataHandle_ == 0) {
		clickSoundDataHandle_ = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	}
}

/// <summary>
///  Used in kPlay, hides default cursor for custom one
/// </summary>
void UIMenu::UpdateCursor() {
	ShowCursor(false);
	Vector2 targetPos = {worldTransform_.translation_.x, worldTransform_.translation_.y};
	cursorSprite_->SetPosition(targetPos);
}

void UIMenu::Update() {
	// マウス座標/クリック状態はInputManagerから取得する。ここではKamataEngine::Inputへは直接触らない。
	InputManager* input = InputManager::GetInstance();

	float mouseX = static_cast<float>(input->GetMouseX());
	float mouseY = static_cast<float>(input->GetMouseY());

	worldTransform_.translation_ = {mouseX, mouseY, 0.0f};
	WorldTransformUpdate(worldTransform_);

	bool mouseDown = input->IsPrimaryHeld();
	bool mouseTriggeredDown = input->IsPrimaryTriggered();
	UpdateButtons(mouseX, mouseY, mouseDown, mouseTriggeredDown);
	DrawHoverLabel(mouseX, mouseY);

#ifdef _DEBUG
	ImGui::Begin("Cursor");
	ImGui::DragFloat3("translate", &worldTransform_.translation_.x);
	ImGui::End();
#endif
}

void UIMenu::Draw() {
	Sprite::PreDraw();
	for (Button& button : buttons_) {
		if (!button.visible) {
			continue;
		}
		// バー(奥) -> ボタン本体(手前)。登場/退場はスライドで表現するので、アルファは常に固定
		if (button.barSprite) {
			button.barSprite->Draw();
		}
		if (button.sprite) {
			button.sprite->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
			button.sprite->Draw();
		}
	}
	if (cursorVisible_) {
		cursorSprite_->Draw();
	}
	Sprite::PostDraw();
}

void UIMenu::StartButtonFade(bool fadeIn, float duration) {
	buttonFadeStatus_ = fadeIn ? ButtonFadeStatus::kFadeIn : ButtonFadeStatus::kFadeOut;
	// 登場はゆっくりイージング、退場は素早く。呼び出し側の値は下限/上限で補正する
	const float clamped = fadeIn ? std::max(duration, kSlideInDuration) : std::min(duration, kSlideOutDuration);
	buttonFadeDuration_ = std::max(clamped, 0.0001f);
	buttonFadeTimer_ = 0.0f;
	buttonFadeAlpha_ = fadeIn ? 0.0f : 1.0f;
	buttonSlide_ = fadeIn ? 0.0f : 1.0f;
}

void UIMenu::UpdateButtonFade() {
	if (buttonFadeStatus_ == ButtonFadeStatus::kNone) {
		return;
	}

	buttonFadeTimer_ += 1.0f / 60.0f;
	if (buttonFadeTimer_ >= buttonFadeDuration_) {
		buttonFadeTimer_ = buttonFadeDuration_;
	}

	float t = std::clamp(buttonFadeTimer_ / buttonFadeDuration_, 0.0f, 1.0f);
	const bool fadeIn = (buttonFadeStatus_ == ButtonFadeStatus::kFadeIn);
	buttonFadeAlpha_ = fadeIn ? t : (1.0f - t);
	// 登場: 減速しながら本来の位置へ / 退場: 加速しながら端へ
	buttonSlide_ = fadeIn ? EaseOut(0.0f, 1.0f, t) : (1.0f - EaseIn(0.0f, 1.0f, t));
}

void UIMenu::DrawHoverLabel(float mouseX, float mouseY) const {
	if (buttonFadeAlpha_ <= 0.01f) {
		return;
	}

	const Button* hovered = nullptr;
	for (const Button& button : buttons_) {
		if (button.visible && button.enabled && button.isHovered && !button.label.empty()) {
			hovered = &button;
			break;
		}
	}
	if (!hovered) {
		return;
	}

	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	ImFont* font = ImGui::GetFont();

	// CalcTextSize measures at the default font size, so scale it to kLabelFontSize.
	const float scale = kLabelFontSize / ImGui::GetFontSize();
	ImVec2 textSize = ImGui::CalcTextSize(hovered->label.c_str());
	textSize.x *= scale;
	textSize.y *= scale;

	const float boxWidth = textSize.x + kLabelPadX * 2.0f;
	const float boxHeight = textSize.y + kLabelPadY * 2.0f;

	// centered above the cursor, kept inside the screen
	const float screenW = static_cast<float>(WinApp::kWindowWidth);
	float left = std::clamp(mouseX - boxWidth * 0.5f, 4.0f, std::max(4.0f, screenW - boxWidth - 4.0f));
	float top = std::max(4.0f, mouseY - kLabelCursorGap - boxHeight);

	const float a = std::clamp(buttonFadeAlpha_, 0.0f, 1.0f);
	const ImVec2 p0 = {left, top};
	const ImVec2 p1 = {left + boxWidth, top + boxHeight};

	drawList->AddRectFilled(p0, p1, IM_COL32(0, 0, 0, static_cast<int>(200 * a)), 6.0f);
	drawList->AddRect(p0, p1, IM_COL32(255, 255, 255, static_cast<int>(180 * a)), 6.0f, 0, 1.5f);
	drawList->AddText(font, kLabelFontSize, {left + kLabelPadX, top + kLabelPadY}, IM_COL32(255, 255, 255, static_cast<int>(255 * a)), hovered->label.c_str());
}

#pragma region UIボタン(AABB)

bool UIMenu::IsInsideAABB(const ButtonAABB& aabb, float x, float y) { return x >= aabb.left && x <= aabb.right && y >= aabb.top && y <= aabb.bottom; }

UIMenu::SlideSide UIMenu::ResolveSide(const Button& button) const {
	if (button.side != SlideSide::kAuto) {
		return button.side;
	}
	const float centerX = (button.aabb.left + button.aabb.right) * 0.5f;
	return (centerX < static_cast<float>(WinApp::kWindowWidth) * 0.5f) ? SlideSide::kLeft : SlideSide::kRight;
}

// 完全に画面外へ出るぶんのずれ(hidden)に、(1 - スライド進行度)を掛ける。
// 左: バーの右端が画面の左端(x=0)に来る位置 / 右: ボタン本体の左端が画面の右端に来る位置
float UIMenu::ComputeSlideOffset(const Button& button) const {
	const float screenW = static_cast<float>(WinApp::kWindowWidth);
	const float hidden = (ResolveSide(button) == SlideSide::kLeft) ? -button.aabb.right : (screenW - button.aabb.left);
	return hidden * (1.0f - buttonSlide_);
}

// バー = 本来のボタン矩形を端側へ kSliderExtend だけ伸ばしたもの(スライド位置込み)
UIMenu::ButtonAABB UIMenu::ComputeBarRect(const Button& button) const {
	ButtonAABB rect = button.aabb;
	if (ResolveSide(button) == SlideSide::kLeft) {
		rect.left -= kSliderExtend;
	} else {
		rect.right += kSliderExtend;
	}
	const float offset = ComputeSlideOffset(button);
	rect.left += offset;
	rect.right += offset;
	return rect;
}

// ボタン本体 = 本来のボタン矩形(元のサイズ)をスライドぶんずらしたもの
UIMenu::ButtonAABB UIMenu::ComputeFaceRect(const Button& button) const {
	ButtonAABB rect = button.aabb;
	const float offset = ComputeSlideOffset(button);
	rect.left += offset;
	rect.right += offset;
	return rect;
}

UIMenu::ButtonID UIMenu::AddButton(uint32_t textureHandle, float x, float y, float width, float height, const std::string& label) {
	Button button;
	button.id = nextButtonID_++;
	button.aabb = {x, y, x + width, y + height};
	button.label = label;

	// バーは本体より先に描くので別スプライト。本体は元のサイズのまま
	button.barSprite.reset(Sprite::Create(barTexture_, {x, y}));
	button.barSprite->SetSize({width, height});
	button.sprite.reset(Sprite::Create(textureHandle, {x, y}));
	button.sprite->SetSize({width, height});

	buttons_.push_back(std::move(button));
	ApplyButtonVisual(buttons_.back());
	return buttons_.back().id;
}
UIMenu::ButtonID UIMenu::AddButton(size_t textureIndex, float x, float y, float width, float height, const std::string& label) {
	if (textureIndex >= buttonTextures_.size()) {
		return kInvalidButton;
	}
	return AddButton(buttonTextures_[textureIndex], x, y, width, height, label);
}

void UIMenu::ClearButtons() { buttons_.clear(); }

UIMenu::Button* UIMenu::FindButton(ButtonID id) {
	for (Button& button : buttons_) {
		if (button.id == id) {
			return &button;
		}
	}
	return nullptr;
}
const UIMenu::Button* UIMenu::FindButton(ButtonID id) const {
	for (const Button& button : buttons_) {
		if (button.id == id) {
			return &button;
		}
	}
	return nullptr;
}

void UIMenu::SetButtonPosition(ButtonID id, float x, float y) {
	Button* button = FindButton(id);
	if (!button) {
		return;
	}
	float width = button->aabb.right - button->aabb.left;
	float height = button->aabb.bottom - button->aabb.top;
	button->aabb = {x, y, x + width, y + height};
	ApplyButtonVisual(*button);
}
void UIMenu::SetButtonSize(ButtonID id, float width, float height) {
	Button* button = FindButton(id);
	if (!button) {
		return;
	}
	button->aabb.right = button->aabb.left + width;
	button->aabb.bottom = button->aabb.top + height;
	ApplyButtonVisual(*button);
}
void UIMenu::SetButtonSlideSide(ButtonID id, SlideSide side) {
	if (Button* button = FindButton(id)) {
		button->side = side;
		ApplyButtonVisual(*button);
	}
}
void UIMenu::SetButtonVisible(ButtonID id, bool visible) {
	if (Button* button = FindButton(id)) {
		button->visible = visible;
	}
}
void UIMenu::SetButtonEnabled(ButtonID id, bool enabled) {
	if (Button* button = FindButton(id)) {
		button->enabled = enabled;
		if (!enabled) {
			button->isHeld = false;
			button->isHovered = false;
			button->holdTimer = 0.0f;
			button->pressStartedInside = false;
		}
	}
}
void UIMenu::SetButtonTexture(ButtonID id, uint32_t textureHandle) {
	if (Button* button = FindButton(id)) {
		if (button->sprite) {
			button->sprite->SetTextureHandle(textureHandle);
		}
	}
}
void UIMenu::SetButtonOnClick(ButtonID id, std::function<void()> onClick) {
	if (Button* button = FindButton(id)) {
		button->onClick = std::move(onClick);
	}
}

bool UIMenu::IsButtonClicked(ButtonID id) const {
	const Button* button = FindButton(id);
	return button && button->wasClickedThisFrame;
}
bool UIMenu::IsButtonHeld(ButtonID id) const {
	const Button* button = FindButton(id);
	return button && button->isHeld;
}
float UIMenu::GetButtonHoldTime(ButtonID id) const {
	const Button* button = FindButton(id);
	return button ? button->holdTimer : 0.0f;
}
bool UIMenu::IsButtonHovered(ButtonID id) const {
	const Button* button = FindButton(id);
	return button && button->isHovered;
}

void UIMenu::UpdateButtons(float mouseX, float mouseY, bool mouseDown, bool mouseTriggeredDown) {
	for (Button& button : buttons_) {
		button.wasClickedThisFrame = false;

		if (!button.visible || !button.enabled) {
			button.isHovered = false;
			button.isHeld = false;
			button.holdTimer = 0.0f;
			button.pressStartedInside = false;
			button.scale = 1.0f;
			ApplyButtonVisual(button);
			continue;
		}

		// 判定は現在のスライド位置にあるバー全体(画面外にいる間はマウスが届かないので自然に無効)
		button.isHovered = IsInsideAABB(ComputeBarRect(button), mouseX, mouseY);

		if (button.isHeld) {
			if (mouseDown) {
				button.holdTimer += 1.0f / 60.0f;
			} else {
				if (button.pressStartedInside && button.isHovered) {
					button.wasClickedThisFrame = true;

					if (clickSoundDataHandle_ != 0) {
						AudioSettings::PlaySfx(clickSoundDataHandle_, false, clickSoundVolume_);
					}
					if (button.onClick) {
						button.onClick();
					}
				}
				button.isHeld = false;
				button.holdTimer = 0.0f;
				button.pressStartedInside = false;
			}
		} else if (button.isHovered && mouseTriggeredDown) {
			button.isHeld = true;
			button.pressStartedInside = true;
			button.holdTimer = 0.0f;
		}
		float targetScale = button.isHeld ? kPressScale : (button.isHovered ? kHoverScale : 1.0f);
		button.scale += (targetScale - button.scale) * kScaleLerp;
		ApplyButtonVisual(button);
	}
}

// バーとボタン本体を、それぞれの矩形の中心を保ったままscaleで拡縮してスプライトへ反映する。
// 判定矩形(ComputeBarRect)自体はscaleの影響を受けない
void UIMenu::ApplyButtonVisual(Button& button) const {
	auto apply = [&](Sprite* sprite, const ButtonAABB& rect) {
		if (!sprite) {
			return;
		}
		const float w = rect.right - rect.left;
		const float h = rect.bottom - rect.top;
		const float sw = w * button.scale;
		const float sh = h * button.scale;
		sprite->SetSize({sw, sh});
		sprite->SetPosition({rect.left - (sw - w) * 0.5f, rect.top - (sh - h) * 0.5f});
	};

	apply(button.barSprite.get(), ComputeBarRect(button));
	apply(button.sprite.get(), ComputeFaceRect(button));

	if (button.barSprite) {
		button.barSprite->SetColor(button.isHovered ? kBarHoverColor : kBarColor);
	}
}
#pragma endregion