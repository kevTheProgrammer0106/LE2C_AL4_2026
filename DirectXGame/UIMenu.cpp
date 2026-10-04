#define NOMINMAX

#include "UIMenu.h"

using namespace KamataEngine;

void UIMenu::Initialize(uint32_t tex, KamataEngine::Camera* camera) {
	camera_ = camera;

	buttonTextures_.clear();
	buttonTextures_.push_back(tex);

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
	// マウス座標/クリック状態はInputManagerから取得する。ここではKamataEngine::Inputへ直接触らない。
	InputManager* input = InputManager::GetInstance();

	float mouseX = static_cast<float>(input->GetMouseX());
	float mouseY = static_cast<float>(input->GetMouseY());

	worldTransform_.translation_ = {mouseX, mouseY, 0.0f};
	WorldTransformUpdate(worldTransform_);

	bool mouseDown = input->IsPrimaryHeld();
	bool mouseTriggeredDown = input->IsPrimaryTriggered();
	UpdateButtons(mouseX, mouseY, mouseDown, mouseTriggeredDown);

#ifdef _DEBUG
	ImGui::Begin("Cursor");
	ImGui::DragFloat3("translate", &worldTransform_.translation_.x);
	ImGui::End();
#endif
}

void UIMenu::Draw() {
	Sprite::PreDraw();
	for (Button& button : buttons_) {
		if (button.visible && button.sprite) {
			button.sprite->SetColor({1.0f, 1.0f, 1.0f, buttonFadeAlpha_});
			button.sprite->Draw();
		}
	}
	cursorSprite_->Draw();
	Sprite::PostDraw();
}

void UIMenu::StartButtonFade(bool fadeIn, float duration) {
	buttonFadeStatus_ = fadeIn ? ButtonFadeStatus::kFadeIn : ButtonFadeStatus::kFadeOut;
	buttonFadeDuration_ = std::max(duration, 0.0001f);
	buttonFadeTimer_ = 0.0f;
	buttonFadeAlpha_ = fadeIn ? 0.0f : 1.0f;
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
	buttonFadeAlpha_ = (buttonFadeStatus_ == ButtonFadeStatus::kFadeIn) ? t : (1.0f - t);
}

#pragma region UIボタン(AABB)

bool UIMenu::IsInsideAABB(const ButtonAABB& aabb, float x, float y) { return x >= aabb.left && x <= aabb.right && y >= aabb.top && y <= aabb.bottom; }

UIMenu::ButtonID UIMenu::AddButton(uint32_t textureHandle, float x, float y, float width, float height, const std::string& label) {
	Button button;
	button.id = nextButtonID_++;
	button.aabb = {x, y, x + width, y + height};
	button.label = label;

	button.sprite.reset(Sprite::Create(textureHandle, {x, y}));
	button.sprite->SetSize({width, height});

	buttons_.push_back(std::move(button));
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

		button.isHovered = IsInsideAABB(button.aabb, mouseX, mouseY);

		if (button.isHeld) {
			if (mouseDown) {
				button.holdTimer += 1.0f / 60.0f;
			} else {
				if (button.pressStartedInside && button.isHovered) {
					button.wasClickedThisFrame = true;

					if (clickSoundDataHandle_ != 0) {
						Audio::GetInstance()->PlayWave(clickSoundDataHandle_, false, clickSoundVolume_);
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
void UIMenu::ApplyButtonVisual(Button& button) {
	if (!button.sprite) {
		return;
	}
	float w = button.aabb.right - button.aabb.left;
	float h = button.aabb.bottom - button.aabb.top;
	float sw = w * button.scale;
	float sh = h * button.scale;
	button.sprite->SetSize({sw, sh});
	button.sprite->SetPosition({button.aabb.left - (sw - w) * 0.5f, button.aabb.top - (sh - h) * 0.5f});
}
#pragma endregion