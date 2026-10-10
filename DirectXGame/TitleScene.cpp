#include "TitleScene.h"
#include "AudioSettings.h"
#include "InputManager.h"
#include "MusicManager.h"

using namespace KamataEngine;

TitleScene::~TitleScene() { delete fade_; }

void TitleScene::Initialize(bool withScreenFadeIn) {
	fade_ = new Fade();
	fade_->Initialize();
	camera_.Initialize();

	whiteTex_ = TextureManager::Load("./Resources/white1x1.png");
	buttonTex_[0] = TextureManager::Load("./Resources/UserInterface/buttonPlay.png");
	buttonTex_[1] = TextureManager::Load("./Resources/UserInterface/buttonLevelSelect.png");
	buttonTex_[2] = TextureManager::Load("./Resources/UserInterface/buttonLevel2.png");
	buttonTex_[3] = TextureManager::Load("./Resources/UserInterface/buttonLevel1.png");
	/*buttonTex_[2] = TextureManager::Load("./Resources/UserInterface/buttonSettings.png");
	buttonTex_[3] = TextureManager::Load("./Resources/UserInterface/buttonCredits.png");*/
	buttonBackTex_ = TextureManager::Load("./Resources/UserInterface/buttonBack.png");
	/*creditsTex_ = TextureManager::Load("./Resources/UserInterface/credits.png");*/
	creditsTex_ = TextureManager::Load("./Resources/UserInterface/buttonLevel3.png");
	cursorTex_ = TextureManager::Load("./Resources/UserInterface/directionArrow.png");

	menuInput_.Initialize(buttonTex_, 4, &camera_);
	menuInput_.SetButtonGroupAlpha(0.0f); // 画面フェードイン完了後にボタンをフェードインさせる

	constexpr float kButtonWidth = 220.0f;
	constexpr float kButtonHeight = 140.0f;
	constexpr float kSmallButtonWidth = 200.0f;
	constexpr float kSmallButtonHeight = 140.0f;
	constexpr float kColumnGap = 20.0f;
	float centerX = static_cast<float>(WinApp::kWindowWidth) / 2.0f;

	// Tune these to your art: two stacked buttons, then Settings / Credits side by side.
	startButton_ = menuInput_.AddButton(buttonTex_[0], centerX - kButtonWidth / 2.0f, 220.0f, kButtonWidth, kButtonHeight, "Start");
	levelSelectButton_ = menuInput_.AddButton(buttonTex_[1], centerX - kButtonWidth / 2.0f, 370.0f, kButtonWidth, kButtonHeight, "Level Select");
	settingsButton_ = menuInput_.AddButton(buttonTex_[2], centerX - kSmallButtonWidth - kColumnGap / 2.0f, 530.0f, kSmallButtonWidth, kSmallButtonHeight, "Settings");
	creditsButton_ = menuInput_.AddButton(buttonTex_[3], centerX + kColumnGap / 2.0f, 530.0f, kSmallButtonWidth, kSmallButtonHeight, "Credits");

	menuInput_.SetButtonOnClick(startButton_, [this]() { BeginExit(NextDestination::kGamePlay, true); });
	menuInput_.SetButtonOnClick(levelSelectButton_, [this]() { BeginExit(NextDestination::kLevelSelect, false); });
	menuInput_.SetButtonOnClick(settingsButton_, [this]() { OpenPanel(Panel::kSettings); });
	menuInput_.SetButtonOnClick(creditsButton_, [this]() { OpenPanel(Panel::kCredits); });
	menuInput_.SetCursorTexture(cursorTex_);

	InitializePanels();

	// メニューBGM
	menuMusicSoundDataHandle_ = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/YellowMarch.wav");
	musicManager_.InitializeMenuTrack(menuMusicSoundDataHandle_);

	if (withScreenFadeIn) {
		phase_ = Phase::kFadeIn;
		fade_->Start(Fade::Status::FadeIn, 1.0f);
	} else {
		fade_->Stop();
		phase_ = Phase::kButtonFadeIn;
		menuInput_.StartButtonFade(true, kButtonFadeDuration);
	}
}

void TitleScene::InitializePanels() {
	// panel menu: one Back button, used by both Settings and Credits
	panelMenu_.Initialize(buttonBackTex_, &camera_);
	panelMenu_.SetButtonGroupAlpha(0.0f);
	panelMenu_.SetCursorTexture(cursorTex_);

	constexpr float kBackWidth = 200.0f;
	constexpr float kBackHeight = 120.0f;
	float centerX = static_cast<float>(WinApp::kWindowWidth) / 2.0f;
	panelBackButton_ = panelMenu_.AddButton(buttonBackTex_, centerX - kBackWidth / 2.0f, static_cast<float>(WinApp::kWindowHeight) - kBackHeight - 30.0f, kBackWidth, kBackHeight, "Back");
	panelMenu_.SetButtonOnClick(panelBackButton_, [this]() { ClosePanel(); });

	// volume sliders (Master / Music / SFX), values read/written through AudioSettings
	sliders_[0].get = &AudioSettings::GetMasterVolume;
	sliders_[0].set = &AudioSettings::SetMasterVolume;
	sliders_[1].get = &AudioSettings::GetMusicSetting;
	sliders_[1].set = &AudioSettings::SetMusicVolume;
	sliders_[2].get = &AudioSettings::GetSfxSetting;
	sliders_[2].set = &AudioSettings::SetSfxVolume;

	float sliderX = centerX - kSliderWidth / 2.0f;
	for (size_t i = 0; i < sliders_.size(); ++i) {
		VolumeSlider& s = sliders_[i];
		s.x = sliderX;
		s.y = kSliderStartY + static_cast<float>(i) * kSliderSpacing;

		s.track.reset(Sprite::Create(whiteTex_, {s.x, s.y}));
		s.track->SetSize({kSliderWidth, kSliderHeight});

		s.fill.reset(Sprite::Create(whiteTex_, {s.x, s.y}));
		s.fill->SetSize({kSliderWidth, kSliderHeight});

		s.knob.reset(Sprite::Create(whiteTex_, {s.x, s.y}));
		s.knob->SetSize({kKnobWidth, kKnobHeight});
	}

	// credits image, centered
	constexpr float kCreditsWidth = 640.0f;
	constexpr float kCreditsHeight = 360.0f;
	creditsSprite_.reset(Sprite::Create(creditsTex_, {(static_cast<float>(WinApp::kWindowWidth) - kCreditsWidth) / 2.0f, 120.0f}));
	creditsSprite_->SetSize({kCreditsWidth, kCreditsHeight});
}

void TitleScene::BeginExit(NextDestination destination, bool screenFade) {
	musicManager_.StopMenuMusic();
	nextDestination_ = destination;
	if (screenFade) {
		phase_ = Phase::kFadeOut;
		fade_->Start(Fade::Status::FadeOut, 1.0f);
	} else {
		phase_ = Phase::kButtonFadeOut;
		menuInput_.StartButtonFade(false, kButtonFadeDuration);
	}
}

void TitleScene::OpenPanel(Panel panel) {
	pendingPanel_ = panel;
	phase_ = Phase::kMenuToPanel;
	menuInput_.StartButtonFade(false, kButtonFadeDuration);
}

void TitleScene::ClosePanel() {
	phase_ = Phase::kPanelToMenu;
	panelMenu_.StartButtonFade(false, kButtonFadeDuration);
	for (VolumeSlider& s : sliders_) {
		if (s.dragging) {
			s.dragging = false;
			AudioSettings::SaveToFile();
		}
	}
}

void TitleScene::UpdateSliders(float alpha) {
	InputManager* input = InputManager::GetInstance();
	const float mouseX = static_cast<float>(input->GetMouseX());
	const float mouseY = static_cast<float>(input->GetMouseY());
	const bool interactive = (phase_ == Phase::kPanel);

	for (VolumeSlider& s : sliders_) {
		float value = s.get();

		if (interactive) {
			bool hover = mouseX >= s.x - kKnobWidth && mouseX <= s.x + kSliderWidth + kKnobWidth && mouseY >= s.y - kKnobHeight * 0.5f && mouseY <= s.y + kSliderHeight + kKnobHeight * 0.5f;
			if (input->IsPrimaryTriggered() && hover) {
				s.dragging = true;
			}
			if (s.dragging) {
				if (input->IsPrimaryHeld()) {
					value = std::clamp((mouseX - s.x) / kSliderWidth, 0.0f, 1.0f);
					s.set(value);
				} else {
					s.dragging = false;
					AudioSettings::SaveToFile(); // write once on release, like the ImGui window
				}
			}
		}

		s.track->SetColor({0.15f, 0.15f, 0.15f, 0.9f * alpha});
		s.fill->SetSize({kSliderWidth * value, kSliderHeight});
		s.fill->SetColor({0.25f, 0.55f, 0.95f, alpha});
		s.knob->SetPosition({s.x + kSliderWidth * value - kKnobWidth * 0.5f, s.y + kSliderHeight * 0.5f - kKnobHeight * 0.5f});
		s.knob->SetColor({1.0f, 1.0f, 1.0f, alpha});
	}
}

void TitleScene::Update() {
	menuInput_.UpdateCursor();
	panelMenu_.UpdateCursor();
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
		if (phase_ == Phase::kMain && InputManager::GetInstance()->IsJumpTriggered()) {
			BeginExit(NextDestination::kGamePlay, true);
		}
		break;

	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true; // scene manager swaps to GameScene
		}
		break;

	case Phase::kButtonFadeOut:
		menuInput_.UpdateButtonFade();
		if (menuInput_.IsButtonFadeFinished()) {
			finished_ = true; // scene manager swaps to LevelSelectScene (no screen fade)
		}
		break;

	case Phase::kMenuToPanel:
		menuInput_.UpdateButtonFade();
		if (menuInput_.IsButtonFadeFinished()) {
			currentPanel_ = pendingPanel_;
			phase_ = Phase::kPanel;
			panelMenu_.StartButtonFade(true, kButtonFadeDuration);
		}
		break;

	case Phase::kPanel:
		panelMenu_.UpdateButtonFade();
		panelMenu_.Update();
		if (phase_ == Phase::kPanel && InputManager::GetInstance()->IsPauseTriggered()) {
			ClosePanel();
		}
		break;

	case Phase::kPanelToMenu:
		panelMenu_.UpdateButtonFade();
		if (panelMenu_.IsButtonFadeFinished()) {
			currentPanel_ = Panel::kNone;
			phase_ = Phase::kButtonFadeIn;
			menuInput_.StartButtonFade(true, kButtonFadeDuration);
		}
		break;
	}

	if (currentPanel_ == Panel::kSettings && (phase_ == Phase::kPanel || phase_ == Phase::kPanelToMenu)) {
		UpdateSliders(panelMenu_.GetButtonFadeAlpha());
	}
}

void TitleScene::DrawPanel() {
	const float alpha = panelMenu_.GetButtonFadeAlpha();

	Sprite::PreDraw();
	if (currentPanel_ == Panel::kSettings) {
		for (VolumeSlider& s : sliders_) {
			s.track->Draw();
			s.fill->Draw();
			s.knob->Draw();
		}
	} else if (currentPanel_ == Panel::kCredits && creditsSprite_) {
		creditsSprite_->SetColor({1.0f, 1.0f, 1.0f, alpha});
		creditsSprite_->Draw();
	}
	Sprite::PostDraw();

	panelMenu_.Draw(); // Back button + cursor
}

void TitleScene::Draw() {
	switch (phase_) {
	case Phase::kMain:
	case Phase::kButtonFadeIn:
	case Phase::kButtonFadeOut:
	case Phase::kMenuToPanel:
		menuInput_.Draw();
		return;
	case Phase::kPanel:
	case Phase::kPanelToMenu:
		DrawPanel();
		return;
	default:
		break;
	}
	fade_->Draw();
}