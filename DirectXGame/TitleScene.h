#pragma once

#include "Fade.h"
#include "InputManager.h"
#include "KamataEngine.h"
#include "MusicManager.h"
#include "UIMenu.h"

/// <summary>
/// タイトルシーン(メインメニュー)
/// 「はじめから(Start)」でゲームプレイへ、「ステージ選択(Level Select)」でレベルセレクトへ遷移する。
/// </summary>
class TitleScene {
public:
	void Initialize();
	void Update();
	void Draw();

	~TitleScene();

	// シーンのフェーズ
	enum class Phase {
		kFadeIn,        // フェードイン(画面)
		kButtonFadeIn,  // ボタンのみフェードイン(画面フェードは行わない)
		kMain,          // メイン処理
		kFadeOut,       // フェードアウト(画面。Playへの遷移用)
		kButtonFadeOut, // ボタンのみフェードアウト(Level Selectへの遷移用。画面フェードは行わない)
	};

	// 遷移先の選択(FadeOut完了後にmain.cppが参照する)
	enum class NextDestination { kNone, kGamePlay, kLevelSelect };

	// デスフラグのgetter
	bool isFinished() const { return finished_; }
	NextDestination GetNextDestination() const { return nextDestination_; }

private:
	// 終了フラグ
	bool finished_ = false;

	Fade* fade_ = nullptr;

	// 現在のフェーズ
	Phase phase_ = Phase::kFadeIn;

	NextDestination nextDestination_ = NextDestination::kNone;

	UIMenu menuInput_;
	uint32_t buttonTex_[2] = {0, 0};
	uint32_t cursorTex_ = 0;

	UIMenu::ButtonID startButton_ = UIMenu::kInvalidButton;
	UIMenu::ButtonID levelSelectButton_ = UIMenu::kInvalidButton;

	// ボタンフェードの速さ(画面フェードより素早く)
	static inline float kButtonFadeDuration = 0.25f;

	KamataEngine::Camera camera_{};

	MusicManager musicManager_;
	uint32_t menuMusicSoundDataHandle_ = 0;
};