#include "KamataEngine.h"
#include <Windows.h>

#include "GameScene.h"
#include "GlobalVariables.h"
#include "InputManager.h"

GameScene* gameScene = nullptr;

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>

enum class Scene { kUnknown = 0, kGame };
Scene scene = Scene::kUnknown;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	using namespace KamataEngine;
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	KamataEngine::Initialize(L"LE2C_29_メンドーザ_ケビン_ラルフ_パレルモ_RailwayShooter");

	DirectXCommon* dxCommonPtr = DirectXCommon::GetInstance();
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	GlobalVariables::GetInstance()->LoadFiles();


	scene = Scene::kGame;
	gameScene = new GameScene();
	gameScene->Initialize();

	while (true) {
		if (KamataEngine::Update())
			break;

		InputManager::GetInstance()->Update();

		imguiManager->Begin();


		GlobalVariables::GetInstance()->Update();
		gameScene->Update();

		imguiManager->End();

		dxCommonPtr->PreDraw();
		gameScene->Draw();
		imguiManager->Draw();
		dxCommonPtr->PostDraw();
	}

	KamataEngine::Finalize();

	delete gameScene;
	gameScene = nullptr;

	return 0;
}