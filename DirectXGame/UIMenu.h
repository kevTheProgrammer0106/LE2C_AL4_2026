#pragma once

#include "Global.h"
#include "InputManager.h"
#include <functional>
#include <string>
#include <vector>

/// <summary>
/// UIボタン群とマウスカーソルの表示・当たり判定・フェードだけを扱うクラス。
/// ゲームプレイ入力(移動/ジャンプ/攻撃等)は一切扱わない — それはInputManagerの役目。
/// マウス座標・クリック状態はInputManager経由で取得する(KamataEngine::Inputへは直接触らない)。
/// </summary>
class UIMenu {
public:
	// 既存の単一テクスチャ用。今までのUIはこのまま使える。
	void Initialize(uint32_t tex, KamataEngine::Camera* camera);

	// 複数テクスチャ用。ボタンごとに使用するテクスチャを選択できる。
	void Initialize(const uint32_t* textures, size_t textureCount, KamataEngine::Camera* camera);

	void Update();
	void UpdateCursor();

	void Draw();

	struct ButtonAABB {
		float left = 0.0f;
		float top = 0.0f;
		float right = 0.0f;
		float bottom = 0.0f;
	};

	using ButtonID = int32_t;
	static constexpr ButtonID kInvalidButton = -1;

	// 既存の単一テクスチャ用
	ButtonID AddButton(uint32_t textureHandle, float x, float y, float width, float height, const std::string& label = "");

	// 複数テクスチャ用。textureIndexで使用するテクスチャを指定する。
	ButtonID AddButton(size_t textureIndex, float x, float y, float width, float height, const std::string& label = "");

	void ClearButtons();

	void SetButtonPosition(ButtonID id, float x, float y);
	void SetButtonSize(ButtonID id, float width, float height);
	void SetButtonVisible(ButtonID id, bool visible);
	void SetButtonEnabled(ButtonID id, bool enabled);
	void SetButtonTexture(ButtonID id, uint32_t textureHandle);
	void SetButtonOnClick(ButtonID id, std::function<void()> onClick);

	bool IsButtonClicked(ButtonID id) const;
	bool IsButtonHeld(ButtonID id) const;
	float GetButtonHoldTime(ButtonID id) const;
	bool IsButtonHovered(ButtonID id) const;

	void SetClickSoundVolume(float volume) { clickSoundVolume_ = volume; }
	void SetClickSoundHandle(uint32_t soundDataHandle) { clickSoundDataHandle_ = soundDataHandle; }

	void StartButtonFade(bool fadeIn, float duration);
	void UpdateButtonFade();
	bool IsButtonFadeFinished() const { return buttonFadeStatus_ == ButtonFadeStatus::kNone || buttonFadeTimer_ >= buttonFadeDuration_; }
	float GetButtonFadeAlpha() const { return buttonFadeAlpha_; }
	void SetButtonGroupAlpha(float alpha) { buttonFadeAlpha_ = alpha; }

	void SetCursorTexture(uint32_t textureHandle) {
		if (cursorSprite_) {
			cursorSprite_->SetTextureHandle(textureHandle);
		}
	}

private:
	struct Button {
		ButtonID id = kInvalidButton;
		ButtonAABB aabb{};
		std::unique_ptr<KamataEngine::Sprite> sprite;
		std::string label;
		bool visible = true;
		bool enabled = true;

		bool isHovered = false;
		bool isHeld = false;
		bool wasClickedThisFrame = false;
		float holdTimer = 0.0f;
		bool pressStartedInside = false;

		std::function<void()> onClick;
	};

	Button* FindButton(ButtonID id);
	const Button* FindButton(ButtonID id) const;
	static bool IsInsideAABB(const ButtonAABB& aabb, float x, float y);
	void UpdateButtons(float mouseX, float mouseY, bool mouseDown, bool mouseTriggeredDown);

	std::vector<Button> buttons_;
	ButtonID nextButtonID_ = 0;

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Camera* camera_ = nullptr;

	std::unique_ptr<KamataEngine::Sprite> cursorSprite_ = nullptr;

	// 複数ボタン用のテクスチャ一覧
	std::vector<uint32_t> buttonTextures_;

	enum class ButtonFadeStatus { kNone, kFadeIn, kFadeOut };
	ButtonFadeStatus buttonFadeStatus_ = ButtonFadeStatus::kNone;
	float buttonFadeDuration_ = 0.0f;
	float buttonFadeTimer_ = 0.0f;
	float buttonFadeAlpha_ = 1.0f;

	uint32_t clickSoundDataHandle_ = 0;
	float clickSoundVolume_ = 1.0f;
};