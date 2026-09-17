
#define NOMINMAX

#include "InputManager.h"

using namespace KamataEngine;

void InputManager::Initialize(uint32_t tex, KamataEngine::Camera* camera) {
	camera_ = camera;

	// 単一テクスチャの既存UIでは、一覧を1個だけ保持する。
	buttonTextures_.clear();
	buttonTextures_.push_back(tex);

	worldTransform_.Initialize();

	Vector2 targetPos = {worldTransform_.translation_.x, worldTransform_.translation_.y};

	aimSprite_.reset(Sprite::Create(tex, targetPos));
	aimSprite_->SetAnchorPoint({0.5f, 0.5f});
	aimSprite_->SetSize({32.0f, 32.0f});

	if (clickSoundDataHandle_ == 0) {
		clickSoundDataHandle_ = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	}
}

void InputManager::Initialize(const uint32_t* textures, size_t textureCount, KamataEngine::Camera* camera) {

	camera_ = camera;

	// 複数テクスチャをコピーして保持する。
	buttonTextures_.clear();

	if (textures != nullptr && textureCount > 0) {
		buttonTextures_.assign(textures, textures + textureCount);
	}
	
	worldTransform_.Initialize();

	// カーソル用スプライトは従来どおり先頭のテクスチャを使用する。
	uint32_t cursorTexture = 0;

	if (!buttonTextures_.empty()) {
		cursorTexture = buttonTextures_[0];
	}

	Vector2 targetPos = {worldTransform_.translation_.x, worldTransform_.translation_.y};

	aimSprite_.reset(Sprite::Create(cursorTexture, targetPos));
	aimSprite_->SetAnchorPoint({0.5f, 0.5f});
	aimSprite_->SetSize({32.0f, 32.0f});

	if (clickSoundDataHandle_ == 0) {
		clickSoundDataHandle_ = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	}
}

void InputManager::Update() {
	// Disables default cursor. do not remove for a while
	//ShowCursor(false);

	mouseMovement_ = Input::GetInstance()->GetMousePosition();

	float mouseX = static_cast<float>(mouseMovement_.x);
	float mouseY = static_cast<float>(mouseMovement_.y);

	worldTransform_.translation_ = {mouseX, mouseY, 0.0f};

	WorldTransformUpdate(worldTransform_);

	Vector2 targetPos = {worldTransform_.translation_.x, worldTransform_.translation_.y};
	aimSprite_->SetPosition(targetPos);
#ifdef _DEBUG
	ImGui::Begin("Cursor");
	ImGui::DragFloat3("translate", &worldTransform_.translation_.x);
	ImGui::End();
#endif

	// PushKey/TriggerKeyと同じ命名規則(Player.cpp等で確認済み)に合わせ、
	// マウスも IsPushMouse(押され続けている間ずっとtrue) / IsTriggerMouse(押した瞬間だけtrue) の
	// ペアがある前提で使う。ホールド判定にはIsPushMouseが必須。
	bool mouseDown = Input::GetInstance()->IsPressMouse(0);
	bool mouseTriggeredDown = Input::GetInstance()->IsTriggerMouse(0);
	UpdateButtons(mouseX, mouseY, mouseDown, mouseTriggeredDown);
}

void InputManager::Draw() {
	Sprite::PreDraw();
	for (Button& button : buttons_) {
		if (button.visible && button.sprite) {
			// ボタン群フェード用のアルファを、ボタン本来の色(白固定)に掛け合わせて適用する
			button.sprite->SetColor({1.0f, 1.0f, 1.0f, buttonFadeAlpha_});
			button.sprite->Draw();
		}
	}
	aimSprite_->Draw();
	Sprite::PostDraw();
}

void InputManager::StartButtonFade(bool fadeIn, float duration) {
	buttonFadeStatus_ = fadeIn ? ButtonFadeStatus::kFadeIn : ButtonFadeStatus::kFadeOut;
	buttonFadeDuration_ = std::max(duration, 0.0001f);
	buttonFadeTimer_ = 0.0f;
	buttonFadeAlpha_ = fadeIn ? 0.0f : 1.0f;
}

void InputManager::UpdateButtonFade() {
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

bool InputManager::IsInsideAABB(const ButtonAABB& aabb, float x, float y) { return x >= aabb.left && x <= aabb.right && y >= aabb.top && y <= aabb.bottom; }

InputManager::ButtonID InputManager::AddButton(uint32_t textureHandle, float x, float y, float width, float height, const std::string& label) {
	Button button;
	button.id = nextButtonID_++;
	button.aabb = {x, y, x + width, y + height};
	button.label = label;

	button.sprite.reset(Sprite::Create(textureHandle, {x, y}));
	button.sprite->SetSize({width, height});

	buttons_.push_back(std::move(button));
	return buttons_.back().id;
}
InputManager::ButtonID InputManager::AddButton(size_t textureIndex, float x, float y, float width, float height, const std::string& label) {

	// 無効な番号なら、ボタンを作成しない。
	if (textureIndex >= buttonTextures_.size()) {
		return kInvalidButton;
	}

	return AddButton(buttonTextures_[textureIndex], x, y, width, height, label);
}

void InputManager::ClearButtons() { buttons_.clear(); }

InputManager::Button* InputManager::FindButton(ButtonID id) {
	for (Button& button : buttons_) {
		if (button.id == id) {
			return &button;
		}
	}
	return nullptr;
}
const InputManager::Button* InputManager::FindButton(ButtonID id) const {
	for (const Button& button : buttons_) {
		if (button.id == id) {
			return &button;
		}
	}
	return nullptr;
}

void InputManager::SetButtonPosition(ButtonID id, float x, float y) {
	Button* button = FindButton(id);
	if (!button) {
		return;
	}
	float width = button->aabb.right - button->aabb.left;
	float height = button->aabb.bottom - button->aabb.top;
	button->aabb = {x, y, x + width, y + height};
	if (button->sprite) {
		button->sprite->SetPosition({x, y});
	}
}
void InputManager::SetButtonSize(ButtonID id, float width, float height) {
	Button* button = FindButton(id);
	if (!button) {
		return;
	}
	button->aabb.right = button->aabb.left + width;
	button->aabb.bottom = button->aabb.top + height;
	if (button->sprite) {
		button->sprite->SetSize({width, height});
	}
}
void InputManager::SetButtonVisible(ButtonID id, bool visible) {
	if (Button* button = FindButton(id)) {
		button->visible = visible;
	}
}
void InputManager::SetButtonEnabled(ButtonID id, bool enabled) {
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
void InputManager::SetButtonTexture(ButtonID id, uint32_t textureHandle) {
	if (Button* button = FindButton(id)) {
		if (button->sprite) {
			button->sprite->SetTextureHandle(textureHandle);
		}
	}
}
void InputManager::SetButtonOnClick(ButtonID id, std::function<void()> onClick) {
	if (Button* button = FindButton(id)) {
		button->onClick = std::move(onClick);
	}
}

bool InputManager::IsButtonClicked(ButtonID id) const {
	const Button* button = FindButton(id);
	return button && button->wasClickedThisFrame;
}
bool InputManager::IsButtonHeld(ButtonID id) const {
	const Button* button = FindButton(id);
	return button && button->isHeld;
}
float InputManager::GetButtonHoldTime(ButtonID id) const {
	const Button* button = FindButton(id);
	return button ? button->holdTimer : 0.0f;
}
bool InputManager::IsButtonHovered(ButtonID id) const {
	const Button* button = FindButton(id);
	return button && button->isHovered;
}

void InputManager::UpdateButtons(float mouseX, float mouseY, bool mouseDown, bool mouseTriggeredDown) {

	for (Button& button : buttons_) {
		button.wasClickedThisFrame = false;

		if (!button.visible || !button.enabled) {
			button.isHovered = false;
			button.isHeld = false;
			button.holdTimer = 0.0f;
			button.pressStartedInside = false;
			continue;
		}

		button.isHovered = IsInsideAABB(button.aabb, mouseX, mouseY);

		if (button.isHeld) {
			if (mouseDown) {
				button.holdTimer += 1.0f / 60.0f;
			} else {
				if (button.pressStartedInside && button.isHovered) {
					button.wasClickedThisFrame = true;

					// クリックSFXを再生。ハンドルが有効な時のみ。
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
	}
}

#pragma endregion