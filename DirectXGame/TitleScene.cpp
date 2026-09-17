#include "TitleScene.h"
#include "MusicManager.h"

using namespace KamataEngine;

TitleScene::~TitleScene() { delete fade_; }

void TitleScene::Initialize() {
	// 初期化処理
	fade_ = new Fade();
	fade_->Initialize();

	fade_->Start(Fade::Status::FadeIn, 1.0f); // フェードインを開始（1秒間）

	camera_.Initialize();

	buttonTex_ = TextureManager::Load("./Resources/UserInterface/directionArrow.png");
	menuInput_.Initialize(buttonTex_, &camera_);
	menuInput_.SetButtonGroupAlpha(0.0f); // 画面フェードイン完了後にボタンをフェードインさせるので、最初は非表示にしておく

	constexpr float kButtonWidth = 320.0f;
	constexpr float kButtonHeight = 80.0f;
	float centerX = static_cast<float>(WinApp::kWindowWidth) / 2.0f - kButtonWidth / 2.0f;

	startButton_ = menuInput_.AddButton(buttonTex_, centerX, 320.0f, kButtonWidth, kButtonHeight, "Start");
	menuInput_.SetButtonOnClick(startButton_, [this]() {
		// Playへの遷移: 画面フェードのみ(既存動作のまま)。メニューBGMもここでフェードアウトさせる。
		musicManager_.StopMenuMusic();
		nextDestination_ = NextDestination::kGamePlay;
		phase_ = Phase::kFadeOut;
		fade_->Start(Fade::Status::FadeOut, 1.0f);
	});

	levelSelectButton_ = menuInput_.AddButton(buttonTex_, centerX, 420.0f, kButtonWidth, kButtonHeight, "Level Select");
	menuInput_.SetButtonOnClick(levelSelectButton_, [this]() {
		// Level Selectシーンへの遷移: 画面フェードは行わず、ボタンだけ素早くフェードアウトする
		musicManager_.StopMenuMusic();
		nextDestination_ = NextDestination::kLevelSelect;
		phase_ = Phase::kButtonFadeOut;
		menuInput_.StartButtonFade(false, kButtonFadeDuration);
	});

	// メニューBGM。GameSceneへ遷移してもMusicManagerの内部状態はstaticで共有されているため、
	// 一度Playへ入ってメニューへ戻ってきた際もここで再度呼べばゼロから鳴らし直せる。
	menuMusicSoundDataHandle_ = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/YellowMarch.wav");
	musicManager_.InitializeMenuTrack(menuMusicSoundDataHandle_);
}

void TitleScene::Update() {
	musicManager_.Update();

	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			fade_->Stop();
			phase_ = Phase::kButtonFadeIn;
			menuInput_.StartButtonFade(true, kButtonFadeDuration);
		}
		break;

	case Phase::kButtonFadeIn:
		menuInput_.UpdateButtonFade();
		menuInput_.Update();
		if (menuInput_.IsButtonFadeFinished()) {
			phase_ = Phase::kMain;
		}
		break;

	case Phase::kMain:
		menuInput_.Update();
		// SPACEキーでも(デバッグ/コントローラー無しの簡易操作用に)ゲーム開始できるようにしておく
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			musicManager_.StopMenuMusic();
			nextDestination_ = NextDestination::kGamePlay;
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}
		break;

	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true; // scene manager will swap to GameScene / LevelSelectScene
		}
		break;

	case Phase::kButtonFadeOut:
		menuInput_.UpdateButtonFade();
		if (menuInput_.IsButtonFadeFinished()) {
			finished_ = true; // scene manager will swap to LevelSelectScene (no screen fade)
		}
		break;
	}
}

void TitleScene::Draw() {
	// 描画処理
	switch (phase_) {
	case Phase::kMain:
	case Phase::kButtonFadeIn:
	case Phase::kButtonFadeOut:
		menuInput_.Draw();
		return;
	default:
		break;
	}
	fade_->Draw();
}