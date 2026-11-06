#pragma once
#include "KamataEngine.h"

#include <array>
#include <vector>

#include "Global.h"
#include "InputManager.h"

#include "Player.h"
#include "CameraController.h"
#include "GameAssets.h"
#include "InputManager.h"

class StageManager;

struct BlockVisual {
	KamataEngine::Model* model = nullptr;
	uint32_t textureHandle = 0;
};

struct BlockInstance {
	KamataEngine::WorldTransform* transform = nullptr;
	KamataEngine::Model* model = nullptr;
	uint32_t textureHandle = 0;
};

class GameScene final {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	~GameScene();

	void CheckAllCollisions();

	void ChangePhase();

	void Initialize();
	void Update();
	void Draw();

	Player* GetPlayer() const { return player_.get(); }
	InputManager* GetInputManager() const { return input_.get(); }


private:
	std::unique_ptr<InputManager> input_ = nullptr;

	GameAssets assets_;

	bool isDebugCameraActive_ = false;
	KamataEngine::Camera camera_{};
	std::unique_ptr<CameraController> cameraController_ = nullptr;
	std::unique_ptr<KamataEngine::DebugCamera> debugCamera_ = nullptr;
	KamataEngine::WorldTransform originPoint{};


	std::unique_ptr<Player> player_ = nullptr;

	void DrawCollisionDebug();
	bool showCollisionDebug_ = true;
};