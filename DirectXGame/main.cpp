#include "KamataEngine.h"
#include <Windows.h>

#include "GameScene.h"
#include "GlobalVariables.h"
#include "InputManager.h"
#include "LevelSelectScene.h"
#include "StageManager.h"
#include "TitleScene.h"

GameScene* gameScene = nullptr;
TitleScene* titleScene = nullptr;
LevelSelectScene* levelSelectScene = nullptr;
StageManager* stageManager = nullptr;

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>

void LoadDebugSettings() {
	const std::string filePath = "DebugSettings.ini";
	std::ifstream file(filePath);
	if (!file.is_open())
		return;

	std::stringstream ss;
	ss << file.rdbuf();
	file.close();

	std::string line;
	while (std::getline(ss, line)) {
		if (line.empty())
			continue;
		std::istringstream lineStream(line);
		std::string key;
		std::string value;
		lineStream >> key >> value;
		if (key == "InitialStage") {
			stageManager->SetCurrentStageIndexByName(value);
		}
	}
}

enum class Scene { kUnknown = 0, kTitle, kLevelSelect, kGame };
Scene scene = Scene::kUnknown;

void ChangeScene() {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene->isFinished()) {
			TitleScene::NextDestination dest = titleScene->GetNextDestination();
			delete titleScene;
			titleScene = nullptr;

			if (dest == TitleScene::NextDestination::kLevelSelect) {
				scene = Scene::kLevelSelect;
				levelSelectScene = new LevelSelectScene();
				levelSelectScene->Initialize(stageManager);
			} else {
				scene = Scene::kGame;
				gameScene = new GameScene();
				gameScene->Initialize(stageManager);
			}
		}
		break;
	case Scene::kLevelSelect:
		if (levelSelectScene->isFinished()) {
			bool backToMenu = levelSelectScene->isReturningToMenu();
			delete levelSelectScene;
			levelSelectScene = nullptr;

			if (backToMenu) {
				scene = Scene::kTitle;
				titleScene = new TitleScene();
				titleScene->Initialize();
			} else {
				scene = Scene::kGame;
				gameScene = new GameScene();
				gameScene->Initialize(stageManager);
			}
		}
		break;
	case Scene::kGame:

		if (gameScene->isFinished() || gameScene->isReturnToMenuRequested()) {
			bool cleared = gameScene->isLevelCleared();
			bool wentToMenu = gameScene->isReturnToMenuRequested();
			delete gameScene;
			gameScene = nullptr;

			if (wentToMenu) {
				stageManager->ClearCheckpoint();
			}

			if (cleared) {
				scene = Scene::kLevelSelect;
				levelSelectScene = new LevelSelectScene();
				levelSelectScene->Initialize(stageManager);
			} else {
				scene = Scene::kTitle;
				titleScene = new TitleScene();
				titleScene->Initialize();
			}
		} else if (gameScene->isRestartRequested()) { // [NEW] Restart — wipes checkpoint
			stageManager->ClearCheckpoint();
			delete gameScene;
			gameScene = nullptr;
			gameScene = new GameScene();
			gameScene->Initialize(stageManager);
		} else if (gameScene->isReloadRequested()) { // Retry — checkpoint preserved
			delete gameScene;
			gameScene = nullptr;
			gameScene = new GameScene();
			gameScene->Initialize(stageManager);
		}
		break;
	}
}

void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kLevelSelect:
		levelSelectScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	default:
		break;
	}
}

void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;
	case Scene::kLevelSelect:
		levelSelectScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	default:
		break;
	}
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	using namespace KamataEngine;
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	KamataEngine::Initialize(L"歩兵ハリー");

	DirectXCommon* dxCommonPtr = DirectXCommon::GetInstance();
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	stageManager = new StageManager;
	stageManager->LoadStageDataFile();

#ifdef _DEBUG
	LoadDebugSettings();
	scene = Scene::kGame;
	gameScene = new GameScene();
	gameScene->Initialize(stageManager);
#else
	scene = Scene::kTitle;
	titleScene = new TitleScene();
	titleScene->Initialize();
#endif

	while (true) {
		if (KamataEngine::Update())
			break;

		InputManager::GetInstance()->Update();

		imguiManager->Begin();

		ChangeScene();
		GlobalVariables::GetInstance()->Update();
		UpdateScene();

		imguiManager->End();

		dxCommonPtr->PreDraw();
		DrawScene();
		imguiManager->Draw();
		dxCommonPtr->PostDraw();
	}

	KamataEngine::Finalize();

	delete gameScene;
	gameScene = nullptr;
	delete titleScene;
	titleScene = nullptr;
	delete levelSelectScene;
	levelSelectScene = nullptr;
	delete stageManager;
	stageManager = nullptr;

	return 0;
}