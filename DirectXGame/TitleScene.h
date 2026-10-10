#pragma once

#include "Fade.h"
#include "InputManager.h"
#include "KamataEngine.h"
#include "MusicManager.h"
#include "UIMenu.h"
#include <memory>

/// <summary>
/// タイトルシーン(メインメニュー)
/// Start / Level Select / Settings / Credits。Settings と Credits は同じシーン内のパネルとして表示する。
/// </summary>
class TitleScene {
public:
	void Initialize(bool withScreenFadeIn = true);
	void Update();
	void Draw();

	~TitleScene();

	enum class Phase {
		kFadeIn,        // 画面フェードイン
		kButtonFadeIn,  // ボタンのみフェードイン
		kMain,          // メイン処理
		kFadeOut,       // 画面フェードアウト(Play遷移用)
		kButtonFadeOut, // ボタンのみフェードアウト(Level Select遷移用)
		kMenuToPanel,   // メインボタンをフェードアウトしてパネルへ
		kPanel,         // Settings / Credits 表示中
		kPanelToMenu,   // パネルをフェードアウトしてメインへ戻る
	};

	enum class NextDestination { kNone, kGamePlay, kLevelSelect };

	bool isFinished() const { return finished_; }
	NextDestination GetNextDestination() const { return nextDestination_; }

private:
	enum class Panel { kNone, kSettings, kCredits };

	// Play / Level Select への遷移を一本化(BGM停止 + 行き先設定 + フェード開始)
	void BeginExit(NextDestination destination, bool screenFade);
	// Settings / Credits 共通の開閉
	void OpenPanel(Panel panel);
	void ClosePanel();

	void InitializePanels();
	void UpdateSliders(float alpha);
	void DrawPanel();

	bool finished_ = false;

	Fade* fade_ = nullptr;
	Phase phase_ = Phase::kFadeIn;
	NextDestination nextDestination_ = NextDestination::kNone;

	// --- main menu ---
	UIMenu menuInput_;
	uint32_t buttonTex_[4] = {0, 0, 0, 0}; // play, level select, settings, credits
	uint32_t cursorTex_ = 0;

	UIMenu::ButtonID startButton_ = UIMenu::kInvalidButton;
	UIMenu::ButtonID levelSelectButton_ = UIMenu::kInvalidButton;
	UIMenu::ButtonID settingsButton_ = UIMenu::kInvalidButton;
	UIMenu::ButtonID creditsButton_ = UIMenu::kInvalidButton;

	// --- panels (shared Back button) ---
	UIMenu panelMenu_;
	UIMenu::ButtonID panelBackButton_ = UIMenu::kInvalidButton;
	uint32_t buttonBackTex_ = 0;
	Panel currentPanel_ = Panel::kNone;
	Panel pendingPanel_ = Panel::kNone;

	// --- settings sliders ---
	struct VolumeSlider {
		float (*get)() = nullptr;
		void (*set)(float) = nullptr;
		float x = 0.0f;
		float y = 0.0f;
		bool dragging = false;
		std::unique_ptr<KamataEngine::Sprite> track;
		std::unique_ptr<KamataEngine::Sprite> fill;
		std::unique_ptr<KamataEngine::Sprite> knob;
	};
	std::array<VolumeSlider, 3> sliders_; // master, music, sfx
	static inline float kSliderWidth = 400.0f;
	static inline float kSliderHeight = 16.0f;
	static inline float kKnobWidth = 20.0f;
	static inline float kKnobHeight = 36.0f;
	static inline float kSliderSpacing = 90.0f;
	static inline float kSliderStartY = 220.0f;

	// --- credits ---
	uint32_t creditsTex_ = 0;
	std::unique_ptr<KamataEngine::Sprite> creditsSprite_;

	uint32_t whiteTex_ = 0;

	static inline float kButtonFadeDuration = 0.25f;

	KamataEngine::Camera camera_{};

	MusicManager musicManager_;
	uint32_t menuMusicSoundDataHandle_ = 0;
};