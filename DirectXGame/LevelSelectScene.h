#pragma once

#include "Fade.h"
#include "InputManager.h"
#include "KamataEngine.h"

class StageManager;

/// <summary>
/// レベルセレクト(ステージ選択)シーン。
/// StageManagerに読み込まれている全ステージを、1ステージにつき1ボタンで一覧表示する。
/// ボタンをクリックすると StageManager::SetCurrentStageIndex を呼んだ上でゲームプレイへ遷移する。
/// </summary>
class LevelSelectScene {
public:
	void Initialize(StageManager* stageManager);
	void Update();
	void Draw();

	~LevelSelectScene();

	enum class Phase {
		kFadeIn,       // 画面フェードイン
		kButtonFadeIn, // ボタンのみフェードイン
		kMain,
		kFadeOut,       // 画面フェードアウト(ステージ選択→ゲームプレイ用)
		kButtonFadeOut, // ボタンのみフェードアウト(戻る→タイトル用)
	};

	bool isFinished() const { return finished_; }
	// trueなら「戻る」でメインメニューへ、falseなら選択したステージでゲームプレイへ
	bool isReturningToMenu() const { return returningToMenu_; }

private:
	bool finished_ = false;
	bool returningToMenu_ = false;

	Fade* fade_ = nullptr;
	Phase phase_ = Phase::kFadeIn;

	StageManager* stageManager_ = nullptr;

	InputManager menuInput_;
	uint32_t buttonTex_[3] = {0, 0, 0};
	uint32_t buttonBackTex_ = 0;
	uint32_t cursorTex_ = 0;
	std::vector<InputManager::ButtonID> stageButtons_;
	InputManager::ButtonID backButton_ = InputManager::kInvalidButton;

	// ボタンフェードの速さ(画面フェードより素早く)
	static inline float kButtonFadeDuration = 0.25f;

	KamataEngine::Camera camera_{};
};