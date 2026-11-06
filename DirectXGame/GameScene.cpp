#define NOMINMAX

#include "GameScene.h"
#include "DebugCollisionDraw.h"
#include "GlobalVariables.h"
#include "InputManager.h"
#include <fstream>

using namespace KamataEngine;
// ============================================================
// [NEW] GlobalVariables registration for GameScene's static inline tunables
// ============================================================
namespace {
constexpr const char* kGroupName = "GameScene";
}


void GameScene::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
}

void GameScene::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv;
}

void GameScene::Initialize() {

	GlobalVariables::GetInstance()->LoadFiles();
	Player::RegisterGlobalVariables();
	GameScene::RegisterGlobalVariables();

	originPoint.Initialize();

	debugCamera_ = std::make_unique<DebugCamera>(WinApp::kWindowWidth, WinApp::kWindowHeight);
	PrimitiveDrawer::GetInstance()->SetCamera(&debugCamera_->GetCamera());
	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());


	camera_.farZ = 10000.0f;
	camera_.Initialize();

	// --- Assets: one load, everything lives in assets_ ---
	assets_.Load();
	//skyDome_ = std::make_unique<Skydome>();
	//skyDome_->Initialize({10.0f, 10.0f, 10.0f}, assets_.modelSkyDome.get(), assets_.skyDomeTex, &camera_, {0.0f, 0.0f, 0.0f});
	//skyDome_->RegisterGlobalVariables("Skydome");
	player_ = std::make_unique<Player>();
	player_->Initialize(assets_.modelPlayer.get(), assets_.playerBodyTex, &camera_, {0.0f, 0.0f, 0.0f});

	cameraController_ = std::make_unique<CameraController>();
	cameraController_->Initialize(&camera_);
	cameraController_->SetTarget(player_.get());
	cameraController_->Reset();
}

GameScene::~GameScene() {

}

void GameScene::ChangePhase() {

}

void GameScene::Update() {

	Player::ApplyGlobalVariables();
	GameScene::ApplyGlobalVariables();

	cameraController_->Update();
	player_->Update();

	//skyDome_->Update();

	ChangePhase();
#ifdef _DEBUG
	cameraController_->Perspective();
	if (isDebugCameraActive_) {
		PrimitiveDrawer::GetInstance()->SetCamera(&debugCamera_->GetCamera());
		// 座標軸表示を有効化
		AxisIndicator::GetInstance()->SetVisible(true);
		// 注視するカメラをセット
		AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());
	} else {
		PrimitiveDrawer::GetInstance()->SetCamera(&camera_);
		// 座標軸表示を有効化
		AxisIndicator::GetInstance()->SetVisible(true);
		// 注視するカメラをセット
		AxisIndicator::GetInstance()->SetTargetCamera(&camera_);
	}
	if (Input::GetInstance()->TriggerKey(DIK_P)) {
		if (isDebugCameraActive_) {
			// デバッグカメラを非アクティブ化
			isDebugCameraActive_ = false;
			cameraController_->Reset(); // カメラコントローラーをリセットしてプレイヤーに追従させる
		} else {
			isDebugCameraActive_ = true;
		}
	}
	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		// ビュープロジェクション行列の転送
		camera_.TransferMatrix();
	} else {
		// ビュープロジェクション行列の更新と転送
		camera_.UpdateMatrix();
	}
#endif
}

void GameScene::CheckAllCollisions() {
	AABB aabb1;
	aabb1 = player_->GetAABB();

	//for (const std::unique_ptr<BaseEnemy>& enemy : enemies_) {
	//	if (!enemy)
	//		continue;
	//	if (enemy->IsCollisionDisabled())
	//		continue;
	//	aabb2 = enemy->GetAABB();
	//	if (IsCollisionAABB(aabb1, aabb2)) {
	//	}
	//}
}

void GameScene::Draw() {
	//skyDome_->Draw();

	player_->Draw();
#ifdef _DEBUG
	DrawCollisionDebug();
#endif
}

void GameScene::DrawCollisionDebug() {
#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_F4))
		showCollisionDebug_ = !showCollisionDebug_;
	if (!showCollisionDebug_ || !player_)
		return;

	using namespace DebugDraw;

	// --- Player body + active melee swing ---
	if (!player_->isDead()) {
		Box(player_->GetAABB(), Color::kPlayer);
	}
	// --- Enemies: body + active weapon hitboxes ---
	//for (const std::unique_ptr<BaseEnemy>& enemy : enemies_) {
	//	if (!enemy || enemy->IsDead())
	//		continue;
	//	Box(enemy->GetAABB(), Color::kEnemy);
	//}
	
	// --- Projectiles ---
	//for (auto& bullet : playerBullets_)
	//	if (bullet && !bullet->IsDead())
	//		Box(bullet->GetAABB(), Color::kProjectile);
	////for (auto& bullet : enemyBullets_)
	////	if (bullet && !bullet->IsDead())
	////		Box(bullet->GetAABB(), Color::kProjectile);
#endif
}