#define NOMINMAX

#include "GameScene.h"
#include "DebugCollisionDraw.h"
#include "GlobalVariables.h"
#include "InputManager.h"
#include "StageManager.h"
#include <fstream>

using namespace KamataEngine;
// ============================================================
// [NEW] GlobalVariables registration for GameScene's static inline tunables
// ============================================================
namespace {
constexpr const char* kGroupName = "GameScene";
}

namespace {
bool ResolveWeaponVisual(const GameAssets& a, DropWeaponKind kind, Model*& outModel, uint32_t& outTex) {
	switch (kind) {
	case DropWeaponKind::kSabreLight:
		outModel = a.modelSabreLight.get();
		outTex = a.texSabreLight;
		return true;
	case DropWeaponKind::kSabreHeavy:
		outModel = a.modelSabreHeavy.get();
		outTex = a.texSabreHeavy;
		return true;
	case DropWeaponKind::kMusketLong:
		outModel = a.modelMusketLong.get();
		outTex = a.musketLongTex;
		return true;
	case DropWeaponKind::kMusketBayonet:
		outModel = a.modelMusketBayonet.get();
		outTex = a.musketBayonetTex;
		return true;
	case DropWeaponKind::kShotgun:
		outModel = a.modelMusketLong.get();
		outTex = a.musketLongTex;
		return true;
	default:
		return false;
	}
}
}

void GameScene::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "ChunkLoadTriggerMargin", kChunkLoadTriggerMargin);
	gv->AddItem(kGroupName, "LoadingCoverFadeOutDuration", kLoadingCoverFadeOutDuration);
	gv->AddItem(kGroupName, "EnemySpawnActivateDistance", kEnemySpawnActivateDistance);
	gv->AddItem(kGroupName, "EntityCullDistance", kEntityCullDistance);
	gv->AddItem(kGroupName, "BackgroundDepthOffset", kBackgroundDepthOffset);
	gv->AddItem(kGroupName, "VerticalChunkLoadTriggerMargin", kVerticalChunkLoadTriggerMargin);
	gv->AddItem(kGroupName, "InteractionRange", kInteractionRange);
}

void GameScene::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kChunkLoadTriggerMargin = gv->GetFloatValue(kGroupName, "ChunkLoadTriggerMargin");
	kLoadingCoverFadeOutDuration = gv->GetFloatValue(kGroupName, "LoadingCoverFadeOutDuration");
	kEnemySpawnActivateDistance = gv->GetFloatValue(kGroupName, "EnemySpawnActivateDistance");
	kEntityCullDistance = gv->GetFloatValue(kGroupName, "EntityCullDistance");
	kBackgroundDepthOffset = gv->GetFloatValue(kGroupName, "BackgroundDepthOffset");
	kVerticalChunkLoadTriggerMargin = gv->GetFloatValue(kGroupName, "VerticalChunkLoadTriggerMargin");
	kInteractionRange = gv->GetFloatValue(kGroupName, "InteractionRange");
}
void GameScene::debugMenu() {
#ifdef _DEBUG
	debugMenu_.Draw(this);
#endif
}
void GameScene::BindAssets() {
	BaseFirearm::SetFireSoundHandle(assets_.sfxFirearmFire);
	FirearmMusket::SetMusketLoadedSoundHandle(assets_.sfxMusketLoaded);

	BaseMelee::SetDefaultSwingSoundHandle(assets_.sfxMeleeSwingDefault);
	BaseMelee::SetSwingSoundHandle(0, assets_.sfxSwingRight);
	BaseMelee::SetSwingSoundHandle(1, assets_.sfxSwingLeft);
	BaseMelee::SetSwingSoundHandle(2, assets_.sfxSwingDownward);

	for (int i = 0; i < 3; ++i)
		BaseEnemy::SetDefaultDeathSoundHandle(i, assets_.sfxEnemyDeath[i]);
	BaseEnemy::SetDefaultFootstepSoundHandle(assets_.sfxFootstep);
	BaseEnemy::SetDefaultJumpSoundHandle(assets_.sfxJump);
	BaseEnemy::SetDefaultLandSoundHandle(assets_.sfxLand);

	PlayerBehavior::SetHurtSoundHandle(assets_.sfxPlayerHurt);
	PlayerBehavior::SetDeathSoundHandle(assets_.sfxPlayerDeath);
	PlayerBehavior::SetBlockStartSoundHandle(assets_.sfxPlayerBlockStart);
	PlayerBehavior::SetBlockImpactSoundHandle(assets_.sfxPlayerBlockImpact);
	Player::SetWeaponSwitchSoundHandle(assets_.sfxWeaponSwitch);
	Player::SetFootstepSoundHandle(assets_.sfxFootstep);
	Player::SetJumpSoundHandle(assets_.sfxJump);
	Player::SetLandSoundHandle(assets_.sfxLand);

	FirearmMusketBayonet::SetLungeSoundHandle(assets_.sfxBayonetLunge);

	WeaponHeavySabre::SetSwingSoundHandle(assets_.sfxCuirassierSwing);
	WeaponHeavySabre::SetLungeSoundHandle(assets_.sfxCuirassierLunge);
	EnemyCuirassier::SetMovementSoundHandle(assets_.sfxCuirassierMove);
	EnemyCuirassier::SetHurtSoundHandle(assets_.sfxCuirassierHurt);
	for (int i = 0; i < 3; ++i)
		EnemyCuirassier::SetDeathSoundHandle(i, assets_.sfxCuirassierDeath[i]);

	EnemyRifleman::SetBayonetLungeSoundHandle(assets_.sfxBayonetLunge);
	HealthBlock::SetConsumeSoundHandle(assets_.sfxHealthRestore);

	ArenaManager::SetArenaStartSoundHandle(assets_.sfxArenaStart);
	ArenaManager::SetWaveStartSoundHandle(assets_.sfxWaveStart);
	ArenaManager::SetWaveEndSoundHandle(assets_.sfxWaveEnd);
	ArenaManager::SetArenaClearSoundHandle(assets_.sfxArenaClear);
}
void GameScene::Initialize(StageManager* stageDataManager) {
	loadingCover_ = std::make_unique<Fade>();
	loadingCover_->Initialize();
	loadingCover_->Start(Fade::Status::FadeIn, 0.0001f);
	loadingCover_->Update();
	loadingCoverActive_ = true;

	stageManager_ = stageDataManager;

	GlobalVariables::GetInstance()->LoadFiles();

	Goal::RegisterGlobalVariables();
	SlidingGate::RegisterGlobalVariables();
	BaseEnemy::RegisterGlobalVariables();
	Player::RegisterGlobalVariables();
	HitEffect::RegisterGlobalVariables();
	GuardEffect::RegisterGlobalVariables();
	DeathParticles::RegisterGlobalVariables();
	WeaponSabre::RegisterGlobalVariables();
	FirearmMusket::RegisterGlobalVariables();
	DroppableItem::RegisterGlobalVariables();
	WeaponHeavySabre::RegisterGlobalVariables();
	FirearmMusketBayonet::RegisterGlobalVariables();
	FirearmShotgun::RegisterGlobalVariables(); 
	BaseMelee::RegisterGlobalVariables();
	BaseFirearm::RegisterGlobalVariables();
	AnimationManager::RegisterGlobalVariables();
	CameraController::RegisterGlobalVariables();
	PlayerBehavior::RegisterGlobalVariables();
	EnemyRifleman::RegisterGlobalVariables();
	EnemyFighter::RegisterGlobalVariables();
	EnemyCuirassier::RegisterGlobalVariables();
	VisionSensor::RegisterGlobalVariables();
	EnemySpawner::RegisterGlobalVariables();
	EnemyHUD::RegisterGlobalVariables();
	ArenaManager::RegisterGlobalVariables();
	HealthBlock::RegisterGlobalVariables();
	GameScene::RegisterGlobalVariables();
	MusicManager::RegisterGlobalVariables();

	originPoint.Initialize();

	debugCamera_ = std::make_unique<DebugCamera>(WinApp::kWindowWidth, WinApp::kWindowHeight);
	PrimitiveDrawer::GetInstance()->SetCamera(&debugCamera_->GetCamera());
	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());

	mapChipField_ = std::make_unique<MapChipField>();

	int32_t currentStageIndex = stageManager_->GetCurrentStageIndex();
	maxFieldChunks_ = static_cast<uint32_t>(stageManager_->GetFieldCount(currentStageIndex));

	mapChipField_->Initialize(maxFieldChunks_, MapChipField::kMaxVerticalChunks);
	mapChipField_->LoadChunkCsv(0, stageManager_->GetFieldChunkFileName(currentStageIndex, 0));
	loadedChunkCount_ = 1;
	oldestLoadedChunk_ = 0;
	loadedTopChunkRow_ = MapChipField::kGroundChunkRow;
	loadedBottomChunkRow_ = MapChipField::kGroundChunkRow;
	levelCleared_ = false;

	backgroundMapChipField_ = std::make_unique<MapChipField>();
	backgroundMapChipField_->Initialize(maxFieldChunks_, MapChipField::kMaxVerticalChunks);
	{
		std::string bgPath = stageManager_->GetBackgroundFieldChunkFileName(currentStageIndex, 0);
		std::ifstream bgTest(bgPath);
		if (bgTest.good())
			backgroundMapChipField_->LoadChunkCsv(0, bgPath);
	}

	camera_.farZ = 10000.0f;
	camera_.Initialize();

	phase_ = Phase::kFadeIn;

	fade_ = std::make_unique<Fade>();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);
	// --- Assets: one load, everything lives in assets_ ---
	assets_.Load();
	BindAssets();

	skyDome_ = std::make_unique<Skydome>();
	skyDome_->Initialize(assets_.modelSkyDome.get(), assets_.skyDomeTex, &camera_);

	deathParticles_ = std::make_unique<DeathParticles>();
	deathParticles_->Initialize(assets_.deathParticleModel.get(), assets_.deathParticleTex, &camera_, {0.0f, 0.0f, 0.0f});

	HitEffect::SetModel(assets_.hitEffectModel.get());
	HitEffect::SetCamera(&camera_);
	GuardEffect::SetModel(assets_.guardEffectModel.get());
	GuardEffect::SetCamera(&camera_);

	interactionPrompt_.Initialize(assets_.interactionPromptTex);

	musicManager_.Initialize(assets_.musicIngame, assets_.musicArena);
	musicManager_.StopMenuMusic();

	cursor_.Initialize(assets_.cursorTex, 2, &camera_);
	playerHealthUI_.Initialize(assets_.healthBarBackgroundTex, assets_.healthBarFillTex);
	enemyHUD_.Initialize(assets_.enemyIconArrowTex, assets_.enemyIconFighterTex);
	{
		WeaponStatusUI::Textures icons;
		icons.meleeLight = assets_.meleeIconLightTex;
		icons.meleeHeavy = assets_.meleeIconHeavyTex;
		icons.firearmLong = assets_.firearmIconLongTex;
		icons.firearmBayonet = assets_.firearmIconBayonetTex;
		icons.bayonetLunge = assets_.bayonetLungeIconTex;
		weaponStatusUI_.Initialize(icons);
	}
	playerBlockUI_.Initialize(assets_.healthBarBackgroundTex, assets_.healthBarFillTex);

	cameraController_ = std::make_unique<CameraController>();
	cameraController_->Initialize(&camera_);

	GenerateChunkTileMap(0);
	GenerateBackgroundChunkTileMap(0, MapChipField::kGroundChunkRow);

	if (stageManager_->HasCheckpoint() && player_) {
		const StageManager::CheckpointData& cp = stageManager_->GetCheckpoint();

		for (uint32_t chunk = 1; chunk < cp.loadedChunkCount; ++chunk) {
			mapChipField_->LoadChunkCsv(chunk, stageManager_->GetFieldChunkFileName(currentStageIndex, chunk));
			GenerateChunkTileMap(chunk);
			if (backgroundMapChipField_) {
				std::string bgPath = stageManager_->GetBackgroundFieldChunkFileName(currentStageIndex, chunk);
				std::ifstream bgTest(bgPath);
				if (bgTest.good())
					backgroundMapChipField_->LoadChunkCsv(chunk, bgPath);
				GenerateBackgroundChunkTileMap(chunk, MapChipField::kGroundChunkRow);
			}
		}
		loadedChunkCount_ = cp.loadedChunkCount;
		oldestLoadedChunk_ = cp.oldestLoadedChunk;
		loadedTopChunkRow_ = cp.loadedTopChunkRow;
		loadedBottomChunkRow_ = cp.loadedBottomChunkRow;

		for (uint32_t row = loadedTopChunkRow_; row <= loadedBottomChunkRow_; ++row) {
			if (row == MapChipField::kGroundChunkRow)
				continue;
			LoadVerticalRow(row);
		}

		WorldTransform wt;
		wt.translation_ = cp.playerPosition;
		player_->SetWorldTransform(wt);
		player_->ResetVelocity();
		player_->SetHP(cp.playerHP);
		{
			Model* m = nullptr;
			uint32_t t = 0;
			DropWeaponKind meleeKind = static_cast<DropWeaponKind>(cp.meleeKind);
			DropWeaponKind firearmKind = static_cast<DropWeaponKind>(cp.firearmKind);
			if (ResolveWeaponVisual(assets_, meleeKind, m, t))
				player_->SetMeleeModel(m, t, meleeKind);
			if (ResolveWeaponVisual(assets_, firearmKind, m, t))
				player_->SetMusketModel(m, t, firearmKind);
		}
		player_->SetWeaponMode(cp.weaponModeIsFirearm ? Player::WeaponMode::kFirearm : Player::WeaponMode::kMelee);

		cameraController_->SetTarget(player_.get());
		cameraController_->Reset();
		cameraController_->SetWorldBounds(static_cast<float>(loadedChunkCount_) * kChunkWorldWidth, mapChipField_->GetWorldHeight());
	}
	InitializePauseMenu();
	InitializeRetryMenu();
	InitializeInGamePauseButton();
	InitializeVictoryMenu();

	loadingCover_->Start(Fade::Status::FadeOut, kLoadingCoverFadeOutDuration);
}

void GameScene::InitializePauseMenu() {
	pauseMenuInput_.Initialize(assets_.pauseButtonTex, 3, &camera_);

	constexpr float kButtonWidth = 220.0f;
	constexpr float kButtonHeight = 160.0f;
	constexpr float kSpacing = 100.0f;
	float centerX = static_cast<float>(WinApp::kWindowWidth) / 2.0f - kButtonWidth / 2.0f;
	float startY = static_cast<float>(WinApp::kWindowHeight) / 2.0f - kSpacing;

	pauseResumeButton_ = pauseMenuInput_.AddButton(assets_.pauseButtonTex[0], centerX, startY, kButtonWidth, kButtonHeight, "Resume");
	pauseMenuInput_.SetButtonOnClick(pauseResumeButton_, [this]() { phase_ = phaseBeforePause_; });

	pauseRetryButton_ = pauseMenuInput_.AddButton(assets_.pauseButtonTex[1], centerX, startY + kSpacing, kButtonWidth, kButtonHeight, "Retry");
	pauseMenuInput_.SetButtonOnClick(pauseRetryButton_, [this]() { reloadRequested_ = true; });

	pauseMainMenuButton_ = pauseMenuInput_.AddButton(assets_.pauseButtonTex[2], centerX, startY + kSpacing * 2.0f, kButtonWidth, kButtonHeight, "Main Menu");
	pauseMenuInput_.SetButtonOnClick(pauseMainMenuButton_, [this]() {
		pendingReturnToMenu_ = true;
		fade_->Start(Fade::Status::FadeOut, 1.0f);
	});
}

void GameScene::InitializeRetryMenu() {
	retryMenuInput_.Initialize(assets_.pauseButtonTex, 3, &camera_);

	constexpr float kButtonWidth = 220.0f;
	constexpr float kButtonHeight = 160.0f;
	constexpr float kSpacing = 100.0f;
	float centerX = static_cast<float>(WinApp::kWindowWidth) / 2.0f - kButtonWidth / 2.0f;
	float startY = static_cast<float>(WinApp::kWindowHeight) / 2.0f - kSpacing * 0.5f;

	std::string retryLabel = stageManager_->HasCheckpoint() ? "Retry (Checkpoint)" : "Retry";
	retryButton_ = retryMenuInput_.AddButton(assets_.retryButtonTex[0], centerX, startY, kButtonWidth, kButtonHeight, retryLabel);
	retryMenuInput_.SetButtonOnClick(retryButton_, [this]() { reloadRequested_ = true; });

	restartButton_ = retryMenuInput_.AddButton(assets_.retryButtonTex[1], centerX, startY + kSpacing, kButtonWidth, kButtonHeight, "Restart");
	retryMenuInput_.SetButtonOnClick(restartButton_, [this]() { restartRequested_ = true; });

	retryMainMenuButton_ = retryMenuInput_.AddButton(assets_.retryButtonTex[2], centerX, startY + kSpacing * 2.0f, kButtonWidth, kButtonHeight, "Main Menu");
	retryMenuInput_.SetButtonOnClick(retryMainMenuButton_, [this]() {
		pendingReturnToMenu_ = true;
		fade_->Start(Fade::Status::FadeOut, 1.0f);
	});
}

void GameScene::InitializeInGamePauseButton() {
	gameplayHUDInput_.Initialize(assets_.inGamePauseButtonTex, &camera_);

	constexpr float kButtonSize = 44.0f;
	constexpr float kMargin = 16.0f;
	float x = kMargin;
	float y = kMargin;

	inGamePauseButton_ = gameplayHUDInput_.AddButton(assets_.inGamePauseButtonTex, x, y, kButtonSize, kButtonSize, "Pause");
	gameplayHUDInput_.SetButtonOnClick(inGamePauseButton_, [this]() {
		if (phase_ == Phase::kPlay) {
			phaseBeforePause_ = phase_;
			phase_ = Phase::kPause;
			pauseMenuInput_.StartButtonFade(true, kButtonFadeDuration);
		}
	});
}

void GameScene::InitializeVictoryMenu() {
	victoryMenuInput_.Initialize(assets_.victoryButtonTex, 2, &camera_);
	victoryMenuInput_.SetButtonGroupAlpha(0.0f);

	victoryOverlay_.reset(Sprite::Create(assets_.healthBarBackgroundTex, {0.0f, 0.0f}));
	victoryOverlay_->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
	victoryOverlay_->SetColor({0.0f, 0.0f, 0.0f, 0.0f});

	constexpr float kButtonWidth = 220.0f;
	constexpr float kButtonHeight = 160.0f;
	constexpr float kSpacing = 100.0f;
	float centerX = static_cast<float>(WinApp::kWindowWidth) / 2.0f - kButtonWidth / 2.0f;
	float startY = static_cast<float>(WinApp::kWindowHeight) / 2.0f - kSpacing * 0.5f;

	victoryRestartButton_ = victoryMenuInput_.AddButton(assets_.victoryButtonTex[0], centerX, startY, kButtonWidth, kButtonHeight, "Play Again");
	victoryMenuInput_.SetButtonOnClick(victoryRestartButton_, [this]() { restartRequested_ = true; });

	victoryMainMenuButton_ = victoryMenuInput_.AddButton(assets_.victoryButtonTex[1], centerX, startY + kSpacing, kButtonWidth, kButtonHeight, "Back to Menu");
	victoryMenuInput_.SetButtonOnClick(victoryMainMenuButton_, [this]() {
		pendingReturnToMenu_ = true;
		fade_->Start(Fade::Status::FadeOut, 1.0f);
	});
}

void GameScene::UpdatePauseMenu() {
	pauseMenuInput_.UpdateButtonFade();
	pauseMenuInput_.Update();
}
void GameScene::UpdateRetryMenu() {
	retryMenuInput_.UpdateButtonFade();
	retryMenuInput_.Update();
}
void GameScene::DrawPauseMenu() { pauseMenuInput_.Draw(); }
void GameScene::DrawRetryMenu() { retryMenuInput_.Draw(); }
void GameScene::UpdateInGamePauseButton() { gameplayHUDInput_.Update(); }
void GameScene::DrawInGamePauseButton() { gameplayHUDInput_.Draw(); }
void GameScene::UpdateVictoryMenu() {
	victoryMenuInput_.UpdateButtonFade();
	victoryMenuInput_.Update();
	if (victoryOverlay_) {
		float alpha = victoryMenuInput_.GetButtonFadeAlpha() * 0.6f;
		victoryOverlay_->SetColor({0.0f, 0.0f, 0.0f, alpha});
	}
}

void GameScene::DrawVictoryMenu() {
	if (victoryOverlay_) {
		Sprite::PreDraw();
		victoryOverlay_->Draw();
		Sprite::PostDraw();
	}
	victoryMenuInput_.Draw();
}

GameScene::~GameScene() {
	if (returnToMenuRequested_) {
		musicManager_.Reset();
	}
	for (std::vector<BlockInstance>& line : worldTransformBlocks_)
		for (BlockInstance& block : line)
			delete block.transform;
	for (std::vector<BlockInstance>& line : worldTransformBackgroundBlocks_)
		for (BlockInstance& block : line)
			delete block.transform;
	enemies_.clear();
	visualEffects_.clear();
	worldTransformBlocks_.clear();
	worldTransformBackgroundBlocks_.clear();
}
                   
void GameScene::CreateHitEffect(Vector3& spawnLocation, float rotationZ) {
	HitEffect* newHitEffect = HitEffect::Create(assets_.hitEffectTex, spawnLocation, rotationZ);  
	visualEffects_.push_back(std::unique_ptr<BaseEffect>(newHitEffect));
}
void GameScene::CreateGuardEffect(Vector3& spawnLocation) {
	GuardEffect* newGuardEffect = GuardEffect::Create(assets_.guardEffectTex, spawnLocation);          
	visualEffects_.push_back(std::unique_ptr<BaseEffect>(newGuardEffect));
}
void GameScene::CreateMuzzleFlash(Vector3& spawnLocation) {
	HitEffect* flash = HitEffect::Create(assets_.hitEffectTex, spawnLocation);
	visualEffects_.push_back(std::unique_ptr<BaseEffect>(flash));
}
void GameScene::CreateBlockSparkEffect(Vector3& spawnLocation) {
	HitEffect* spark = HitEffect::Create(assets_.hitEffectTex, spawnLocation);
	visualEffects_.push_back(std::unique_ptr<BaseEffect>(spark));
}
void GameScene::CreateEnemyDeathParticles(const Vector3& position) {
	auto particles = std::make_unique<DeathParticles>();
	particles->Initialize(assets_.deathParticleModel.get(), assets_.deathParticleTex, &camera_, position);    
	enemyDeathParticles_.push_back(std::move(particles));
}
void GameScene::CreateFirearmProjectile(Owner owner, const KamataEngine::Vector3& position, const KamataEngine::Vector2& aimDirection, int damage, int32_t lifeFrames) {
	auto projectile = std::make_unique<ProjectileBall>();
	projectile->InitializeAimed(assets_.modelProjectileBall.get(), assets_.firearmBallTex, &camera_, position, aimDirection, owner);
	if (damage >= 0)
		projectile->SetDamage(damage); // 銃ごとのダメージ
	if (lifeFrames > 0)
		projectile->SetLifeFrames(lifeFrames);
	if (owner == Owner::kPlayer)
		playerBullets_.push_back(std::move(projectile));
	else if (owner == Owner::kEnemy)
		enemyBullets_.push_back(std::move(projectile));
}

void GameScene::ChangePhase() {
	switch (phase_) {
	case Phase::kFadeIn:
		// fade_->Update();
		// if (fade_->IsFinished()) {
		phase_ = Phase::kPlay;
		//	fade_->Stop();
		//}
		break;
	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			if (levelCleared_) {
				phase_ = Phase::kVictory;
				victoryMenuInput_.StartButtonFade(true, kButtonFadeDuration);
			} else {
				finished_ = true;
			}
		}
		break;
	case Phase::kPlay: {
		fade_->Update();
		if (fade_->IsFinished()) {
			fade_->Stop();
		}
		if (goal_)
			goal_->Update();
		for (auto& sw : switches_)
			if (sw)
				sw->Update();
		for (auto& gate : gates_)
			if (gate)
				gate->Update();

		if (!player_->isDead()) {
			if (cameraController_->GetMode() == CameraController::Mode::kForcedScroll) {
				player_->SetScrollBoundary(cameraController_->GetScrollLeftBoundary(), true);
			} else {
				player_->SetScrollBoundary(0.0f, false);
			}
			player_->Update();
		} else {
			phase_ = Phase::kDeath;
			const Vector3& deathParticlesPosition = player_->GetWorldPosition();
			if (deathParticles_) {
				deathParticles_->Initialize(assets_.deathParticleModel.get(), assets_.deathParticleTex, &camera_, deathParticlesPosition);
			}
		}

		for (std::unique_ptr<BaseEnemy>& enemy : enemies_) {
			if (!enemy)
				continue;
			if (!IsWithinActiveRange(enemy->GetWorldPosition(), kEntityCullDistance))
				continue;
			enemy->Update();
		}
		for (auto& item : droppableItems_) {
			if (!item)
				continue;
			if (!IsWithinActiveRange(item->GetWorldPosition(), kEntityCullDistance))
				continue;
			item->Update();
		}
		droppableItems_.erase(
		    std::remove_if(droppableItems_.begin(), droppableItems_.end(), [](const std::unique_ptr<DroppableItem>& item) { return !item || item->IsPickedUp() || item->IsExpired(); }),
		    droppableItems_.end());

		for (auto& bullet : playerBullets_)
			bullet->Update();
		playerBullets_.erase(std::remove_if(playerBullets_.begin(), playerBullets_.end(), [](const std::unique_ptr<ProjectileBall>& bullet) { return bullet->IsDead(); }), playerBullets_.end());

		for (std::unique_ptr<BaseEffect>& effect : visualEffects_) {
			if (!effect)
				continue;
			effect->Update();
		}

		for (auto& dp : enemyDeathParticles_) {
			if (dp)
				dp->Update();
		}
		enemyDeathParticles_.erase(
		    std::remove_if(enemyDeathParticles_.begin(), enemyDeathParticles_.end(), [](const std::unique_ptr<DeathParticles>& dp) { return !dp || dp->IsFinished(); }), enemyDeathParticles_.end());

		for (auto& bullet : enemyBullets_)
			bullet->Update();
		enemyBullets_.erase(std::remove_if(enemyBullets_.begin(), enemyBullets_.end(), [](const std::unique_ptr<ProjectileBall>& bullet) { return bullet->IsDead(); }), enemyBullets_.end());

		visualEffects_.erase(std::remove_if(visualEffects_.begin(), visualEffects_.end(), [](const std::unique_ptr<BaseEffect>& effect) { return effect->IsFinished(); }), visualEffects_.end());
		enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(), [](const std::unique_ptr<BaseEnemy>& enemy) { return enemy->IsDead(); }), enemies_.end());

		{
			std::vector<BaseEnemy*> arenaEnemies;
			if (ArenaManager* activeArena = FindActiveArena()) {
				uint8_t activeArenaID = activeArena->GetArenaID();
				arenaEnemies.reserve(enemies_.size());
				for (std::unique_ptr<BaseEnemy>& enemy : enemies_) {
					if (enemy && enemy->GetArenaID() == activeArenaID) {
						arenaEnemies.push_back(enemy.get());
					}
				}
			}
			enemyHUD_.Update(arenaEnemies, camera_, WinApp::kWindowWidth, WinApp::kWindowHeight);
		}
		if (player_) {
			playerHealthUI_.Update(player_->GetHP(), player_->GetMaxHP());

			WeaponStatusUI::State ws;
			ws.isFirearmMode = (player_->GetWeaponMode() == Player::WeaponMode::kFirearm);
			ws.meleeKind = player_->GetCurrentMeleeKind();
			ws.firearmKind = player_->GetCurrentFirearmKind();
			ws.isMeleeCooling = player_->IsMeleeLungeCooling();
			ws.meleeCooldownProgress01 = player_->GetMeleeLungeProgress01();
			ws.isLungeCooling = player_->IsBayonetLungeCooling();
			ws.lungeCooldownProgress01 = player_->GetBayonetLungeProgress01();
			if (BaseFirearm* firearm = player_->GetCurrentFirearm()) {
				ws.isReloading = firearm->IsReloading();
				ws.reloadProgress01 = firearm->GetReloadProgress01();
			}
			weaponStatusUI_.Update(ws);
			playerBlockUI_.Update(player_->GetBlockGauge(), player_->GetMaxBlockGauge(), player_->IsBlockBroken(), player_->GetBlockRecoverThreshold());
		}
		for (std::vector<BlockInstance>& line : worldTransformBlocks_) {
			for (BlockInstance& block : line) {
				if (!block.transform)
					continue;
				WorldTransformUpdate(*block.transform);
			}
		}

		CheckAllCollisions();

		for (DroppableItem* item : pendingPickups_) {
			if (item)
				ApplyItemPickup(*item);
		}
		pendingPickups_.clear();
		break;
	}
	case Phase::kDeath:
		for (std::unique_ptr<BaseEnemy>& enemy : enemies_) {
			if (!enemy)
				continue;
			enemy->Update();
		}
		if (deathParticles_)
			deathParticles_->Update();
		if (deathParticles_ && deathParticles_->IsFinished())
			phase_ = Phase::kRetry;
		retryMenuInput_.StartButtonFade(true, kButtonFadeDuration);
		cameraController_->Update();
		for (std::vector<BlockInstance>& line : worldTransformBlocks_) {
			for (BlockInstance& block : line) {
				if (!block.transform)
					continue;
				WorldTransformUpdate(*block.transform);
			}
		}
		break;
	case Phase::kRetry:
		break;
	case Phase::kPause:
		break;
	case Phase::kVictory:
		break;
	}
}

void GameScene::AlertNearbyEnemies(BaseEnemy* sourceEnemy, const Vector3& alertPosition, float radius) {
	for (std::unique_ptr<BaseEnemy>& enemy : enemies_) {
		if (!enemy || enemy.get() == sourceEnemy)
			continue;
		if (enemy->IsDead())
			continue;
		Vector3 diff = enemy->GetWorldPosition() - sourceEnemy->GetWorldPosition();
		float distanceSq = diff.x * diff.x + diff.y * diff.y;
		if (distanceSq <= radius * radius)
			enemy->ReceiveAlert(alertPosition);
	}
}

ArenaManager* GameScene::GetOrCreateArena(uint8_t arenaID) {
	for (auto& arena : arenas_)
		if (arena && arena->GetArenaID() == arenaID)
			return arena.get();
	auto newArena = std::make_unique<ArenaManager>();
	newArena->Initialize(arenaID);
	arenas_.push_back(std::move(newArena));
	return arenas_.back().get();
}

void GameScene::AddEnemySpawner(const Vector3& position, uint8_t enemySubID, float interval, EnemySpawner::SpawnMode mode, int32_t maxSpawnCount, bool omniscient) {
	auto spawner = std::make_unique<EnemySpawner>();
	spawner->Initialize(position, enemySubID, interval, mode, maxSpawnCount);
	spawner->SetSpawnedEnemiesOmniscient(omniscient);
	enemySpawners_.push_back(std::move(spawner));
}

void GameScene::SetCheckpoint(const Vector3& position) {
	StageManager::CheckpointData data;
	data.hasCheckpoint = true;
	data.playerPosition = position;
	data.playerHP = player_ ? player_->GetHP() : 100;
	data.weaponModeIsFirearm = (player_ && player_->GetWeaponMode() == Player::WeaponMode::kFirearm) ? 1 : 0;
	data.meleeKind = player_ ? static_cast<int32_t>(player_->GetCurrentMeleeKind()) : 0;
	data.firearmKind = player_ ? static_cast<int32_t>(player_->GetCurrentFirearmKind()) : 0;
	data.loadedChunkCount = loadedChunkCount_;
	data.oldestLoadedChunk = oldestLoadedChunk_;
	data.loadedTopChunkRow = loadedTopChunkRow_;
	data.loadedBottomChunkRow = loadedBottomChunkRow_;
	stageManager_->SetCheckpoint(data);
}

void GameScene::GenerateChunkTileMap(uint32_t chunkIndex) { GenerateChunkTileMapRow(chunkIndex, MapChipField::kGroundChunkRow); }

void GameScene::GenerateChunkTileMapRow(uint32_t chunkIndex, uint32_t chunkRow) {
	auto key = std::make_pair(chunkIndex, chunkRow);
	if (generatedChunkRows_.contains(key))
		return;
	generatedChunkRows_.insert(key);

	uint32_t numVertical = mapChipField_->GetNumBlockVertical();
	uint32_t numHorizontalTotal = mapChipField_->GetNumBlockHorizontal();

	if (worldTransformBlocks_.size() != numVertical) {
		worldTransformBlocks_.resize(numVertical);
		for (auto& row : worldTransformBlocks_)
			row.resize(numHorizontalTotal);
	}
	cameraController_->SetWorldBounds(static_cast<float>(loadedChunkCount_) * kChunkWorldWidth, mapChipField_->GetWorldHeight());

	uint32_t jStart = chunkIndex * MapChipField::kChunkBlockSize;
	uint32_t jEnd = std::min(jStart + MapChipField::kChunkBlockSize, numHorizontalTotal);
	uint32_t iStart = chunkRow * MapChipField::kChunkBlockSize; // [FIX] row-scoped
	uint32_t iEnd = std::min(iStart + MapChipField::kChunkBlockSize, numVertical);

	for (uint32_t i = iStart; i < iEnd; ++i) {
		for (uint32_t j = jStart; j < jEnd; ++j) {
			MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(j, i);
			switch (mapChipType) {
			case MapChipType::kBlock:
			case MapChipType::kBlockHalfUp:
			case MapChipType::kBlockHalfDown:
			case MapChipType::kSlowBlock: {
				uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
				delete worldTransformBlocks_[i][j].transform;
				WorldTransform* worldTransform = new WorldTransform;
				worldTransform->Initialize();
				worldTransform->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);

				Model* shapeModel = assets_.blockModel.get();
				uint32_t tex = assets_.blockTex;
				tex = assets_.ResolveBlockTexture(assets_.blockTexVariants, subID, assets_.blockTex);
				worldTransformBlocks_[i][j] = {worldTransform, shapeModel, tex};
			} break;
			case MapChipType::kPlatform: {
				uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
				delete worldTransformBlocks_[i][j].transform;
				WorldTransform* worldTransform = new WorldTransform;
				worldTransform->Initialize();
				worldTransform->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				uint32_t tex = assets_.ResolveBlockTexture(assets_.platformTexVariants, subID, assets_.blockPlatformTex);
				worldTransformBlocks_[i][j] = {worldTransform, assets_.blockPlatformModel.get(), tex};
			} break;
			case MapChipType::kArenaCamera: { // 'F': zoom point and trigger area. Sub ID = preset
				uint8_t presetID = mapChipField_->GetMapChipSubIDByIndex(j, i);
				cameraController_->AddZone(mapChipField_->GetMapChipPositionByIndex(j, i), CameraController::GetCameraConfig(stageManager_->GetCurrentStageIndex(), presetID));
			} break;
			case MapChipType::kPlayer: {
				player_ = std::make_unique<Player>();
				player_->SetMapChipField(mapChipField_.get());
				player_->SetGameScene(this);
				Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(j, i);
				std::array<PartResources, 3> playerParts = {
					{{assets_.modelPlayerBody.get(), assets_.playerBodyTex}, {assets_.modelPlayerHead.get(), assets_.playerHeadTex}, {assets_.modelPlayerHat.get(), assets_.playerHatTex}}
                };
				player_->Initialize(playerParts, assets_.modelSlashEffect.get(), assets_.slashEffectTex, &camera_, playerPosition);
				player_->SetMusketModel(assets_.modelMusketLong.get(), assets_.musketLongTex);
				player_->SetMeleeModel(assets_.modelSabreLight.get(), assets_.texSabreLight);
				cameraController_->SetTarget(player_.get());
				cameraController_->Reset();
			} break;
			case MapChipType::kEnemy: {
				uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
				Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(j, i);
				uint8_t placementState = mapChipField_->GetMapChipLinkIDByIndex(j, i); // 0 wander, 1 hold left, 2 hold right, 4 omniscient
				enemySpawnPoints_.push_back({enemyPosition, subID, false, placementState});
			} break;
			case MapChipType::kGoal: {
				Vector3 goalPosition = mapChipField_->GetMapChipPositionByIndex(j, i);
				goal_ = std::make_unique<Goal>();
				goal_->Initialize(assets_.modelGoal.get(), assets_.goalTex, &camera_, goalPosition);
			} break;
			case MapChipType::kGateUp:
			case MapChipType::kGateDown:
			case MapChipType::kGateLeft:
			case MapChipType::kGateRight:
			case MapChipType::kArenaGateUp:
			case MapChipType::kArenaGateDown:
			case MapChipType::kArenaGateLeft:
			case MapChipType::kArenaGateRight: {
				uint8_t buttonID = mapChipField_->GetMapChipLinkIDByIndex(j, i);
				uint8_t textureVariant = mapChipField_->GetMapChipSubIDByIndex(j, i);
				Vector3 gatePosition = mapChipField_->GetMapChipPositionByIndex(j, i);
				auto gate = std::make_unique<SlidingGate>();
				gate->Initialize(assets_.modelGate.get(), assets_.gateTex, &camera_, gatePosition, mapChipType, textureVariant, buttonID);
				GetOrCreateArena(buttonID)->RegisterGate(gate.get()); // [NEW] same ID doubles as arena tag
				gates_.push_back(std::move(gate));
			} break;
			case MapChipType::kSwitch: {
				uint8_t buttonID = mapChipField_->GetMapChipSubIDByIndex(j, i);
				Vector3 switchPosition = mapChipField_->GetMapChipPositionByIndex(j, i);
				auto sw = std::make_unique<SwitchBlock>();
				sw->Initialize(assets_.modelSwitch.get(), assets_.switchTex, &camera_, switchPosition, buttonID);
				switches_.push_back(std::move(sw));
			} break;
			case MapChipType::kArenaTrigger: {
				uint8_t arenaID = mapChipField_->GetMapChipLinkIDByIndex(j, i);
				Vector3 triggerPos = mapChipField_->GetMapChipPositionByIndex(j, i);
				arenaTriggers_.push_back({triggerPos, arenaID, false});
				GetOrCreateArena(arenaID);
			} break;
			case MapChipType::kEnemySpawner: {
				uint8_t enemySubID = mapChipField_->GetMapChipSubIDByIndex(j, i);
				uint8_t arenaID = mapChipField_->GetMapChipLinkIDByIndex(j, i);
				Vector3 spawnerPos = mapChipField_->GetMapChipPositionByIndex(j, i);

				ArenaSpawnEntry waveEntry = ResolveArenaSpawnEntry(stageManager_->GetCurrentStageIndex(), arenaID, enemySubID);

				auto spawner = std::make_unique<EnemySpawner>();
				spawner->Initialize(spawnerPos, enemySubID, waveEntry.interval, waveEntry.mode, waveEntry.maxSpawnCount);
				spawner->SetArenaID(arenaID);
				spawner->SetActive(false);
				spawner->SetResolvedEnemyType(waveEntry.enemyType);
				spawner->SetSpawnedEnemiesOmniscient(true);

				GetOrCreateArena(arenaID)->RegisterSpawner(spawner.get());
				enemySpawners_.push_back(std::move(spawner));
			} break;
			case MapChipType::kCheckpoint: {
				Vector3 cpPos = mapChipField_->GetMapChipPositionByIndex(j, i);
				checkpointTriggers_.push_back({cpPos, false});
			} break;
			case MapChipType::kHealthBlock: {
				uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
				Vector3 healthBlockPos = mapChipField_->GetMapChipPositionByIndex(j, i);
				HealthBlock::Mode mode = (subID == 0) ? HealthBlock::Mode::kLimited : HealthBlock::Mode::kTimer;
				auto healthBlock = std::make_unique<HealthBlock>();
				healthBlock->Initialize(assets_.modelHealthBlock.get(), assets_.healthBlockTex, &camera_, healthBlockPos, mode);
				healthBlocks_.push_back(std::move(healthBlock));
			} break;
			default:
				break;
			}
		}
	}
}

void GameScene::GenerateBackgroundChunkTileMap(uint32_t chunkX, uint32_t chunkY) {
	if (!backgroundMapChipField_)
		return;
	uint32_t numVertical = backgroundMapChipField_->GetNumBlockVertical();
	uint32_t numHorizontalTotal = backgroundMapChipField_->GetNumBlockHorizontal();

	if (worldTransformBackgroundBlocks_.size() != numVertical) {
		worldTransformBackgroundBlocks_.resize(numVertical);
		for (auto& row : worldTransformBackgroundBlocks_)
			row.resize(numHorizontalTotal);
	}

	uint32_t jStart = chunkX * MapChipField::kChunkBlockSize;
	uint32_t jEnd = std::min(jStart + MapChipField::kChunkBlockSize, numHorizontalTotal);
	uint32_t iStart = chunkY * MapChipField::kChunkBlockSize;
	uint32_t iEnd = std::min(iStart + MapChipField::kChunkBlockSize, numVertical);

	for (uint32_t i = iStart; i < iEnd; ++i) {
		for (uint32_t j = jStart; j < jEnd; ++j) {
			MapChipType type = backgroundMapChipField_->GetMapChipTypeByIndex(j, i);
			if (type != MapChipType::kBlock && type != MapChipType::kSlowBlock && type != MapChipType::kPlatform) {
				if (worldTransformBackgroundBlocks_[i][j].transform)
					delete worldTransformBackgroundBlocks_[i][j].transform;
				worldTransformBackgroundBlocks_[i][j] = {};
				continue;
			}
			WorldTransform* worldTransform = worldTransformBackgroundBlocks_[i][j].transform;
			if (!worldTransform) {
				worldTransform = new WorldTransform;
				worldTransform->Initialize();
			}
			worldTransform->translation_ = backgroundMapChipField_->GetMapChipPositionByIndex(j, i);
			worldTransform->translation_.z += kBackgroundDepthOffset;

			WorldTransformUpdate(*worldTransform);

			uint8_t subID = backgroundMapChipField_->GetMapChipSubIDByIndex(j, i);
			uint32_t tex = assets_.ResolveBlockTexture(assets_.blockTexVariants, subID, assets_.blockTex);
			worldTransformBackgroundBlocks_[i][j] = {worldTransform, assets_.blockModel.get(), tex};
		}
	}
}
void GameScene::Update() {
	cursor_.UpdateCursor();

	if (loadingCoverActive_ && loadingCover_) {
		loadingCover_->Update();
		if (loadingCover_->IsFinished()) {
			loadingCoverActive_ = false;
			loadingCover_->Stop();
		}
	}

	Goal::ApplyGlobalVariables();
	EnemyRifleman::ApplyGlobalVariables();
	EnemyCuirassier::ApplyGlobalVariables();
	EnemyFighter::ApplyGlobalVariables();
	Player::ApplyGlobalVariables();
	HitEffect::ApplyGlobalVariables();
	GuardEffect::ApplyGlobalVariables();
	DeathParticles::ApplyGlobalVariables();
	WeaponSabre::ApplyGlobalVariables();
	FirearmMusket::ApplyGlobalVariables();
	DroppableItem::ApplyGlobalVariables();
	WeaponHeavySabre::ApplyGlobalVariables();
	FirearmMusketBayonet::ApplyGlobalVariables();
	FirearmShotgun::ApplyGlobalVariables();    
	BaseMelee::ApplyGlobalVariables();
	BaseFirearm::ApplyGlobalVariables();
	AnimationManager::ApplyGlobalVariables();
	CameraController::ApplyGlobalVariables();
	PlayerBehavior::ApplyGlobalVariables();
	VisionSensor::ApplyGlobalVariables();
	EnemySpawner::ApplyGlobalVariables();
	EnemyHUD::ApplyGlobalVariables();
	ArenaManager::ApplyGlobalVariables();
	HealthBlock::ApplyGlobalVariables();
	GameScene::ApplyGlobalVariables();
	MusicManager::ApplyGlobalVariables();

	debugMenu();

	cursor_.Update();
	if (phase_ != Phase::kDeath) {
		cursor_.UpdateCursor();
	}
	if (InputManager::GetInstance()->IsPauseTriggered()) {
		if (phase_ == Phase::kPlay) {
			phaseBeforePause_ = phase_;
			phase_ = Phase::kPause;
			pauseMenuInput_.StartButtonFade(true, kButtonFadeDuration);
		} else if (phase_ == Phase::kPause) {
			phase_ = phaseBeforePause_;
		}
	}
	if (phase_ == Phase::kPlay)
		UpdateInGamePauseButton();
	if (phase_ == Phase::kPause)
		UpdatePauseMenu();
	else if (phase_ == Phase::kRetry)
		UpdateRetryMenu();
	else if (phase_ == Phase::kVictory)
		UpdateVictoryMenu();
	else
		cameraController_->Update();

	ChangePhase();
	if (pendingReturnToMenu_) {
		fade_->Update();
		if (fade_->IsFinished()) {
			returnToMenuRequested_ = true;
		}
	}

	UpdateEnemySpawning();
	UpdateFieldStreaming();
	UpdateInteractionPrompt();
	TryInteract();
	UpdateArenaTriggers();
	if (player_ && (phase_ == Phase::kPlay || phase_ == Phase::kDeath))
		cameraController_->UpdateZones(player_->GetAABB(), phase_ == Phase::kPlay && !player_->isDead());
	UpdateArenas();
	UpdateCheckpointTriggers();
	UpdatePeriodicEnemySpawners();
	UpdateHealthBlocks();
	musicManager_.Update();
#ifdef _DEBUG
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
	AABB aabb1, aabb2;
	aabb1 = player_->GetAABB();

	for (const std::unique_ptr<BaseEnemy>& enemy : enemies_) {
		if (!enemy)
			continue;
		if (enemy->IsCollisionDisabled())
			continue;
		aabb2 = enemy->GetAABB();
		if (IsCollisionAABB(aabb1, aabb2)) {
		}
	}

	if (player_->IsMeleeSwingActive()) {
		BaseMelee* melee = player_->GetCurrentMelee();
		AABB meleeAABB = melee->GetSwingAABB(*player_, player_->GetComboIndex());

		float hitRotZ = 0.0f;
		switch (player_->GetAttackDirection()) {
		case Player::AttackDirection::kDown:
			hitRotZ = -std::numbers::pi_v<float> / 2.0f;
			break;
		default:
			hitRotZ = 0.0f;
			break;
		}

		for (const std::unique_ptr<BaseEnemy>& enemy : enemies_) {
			if (!enemy || enemy->IsCollisionDisabled())
				continue;
			if (melee->HasHit(enemy.get()))
				continue;
			if (IsCollisionAABB(meleeAABB, enemy->GetAABB())) {
				enemy->OnCollision(player_.get());
				melee->MarkHit(enemy.get());

				if (enemy->WasBlocked()) {
					// guarded: no hit effect / knockback on the enemy, the player recoils instead
					player_->OnCollision(enemy.get());
					continue;
				}

				Vector3 effectPos = (enemy->GetWorldPosition() + player_->GetWorldPosition()) / 2.0f;
				CreateHitEffect(effectPos, hitRotZ);

				Vector3 dir = enemy->GetWorldPosition() - player_->GetWorldPosition();
				float len = std::sqrtf(dir.x * dir.x + dir.y * dir.y);
				if (len > 0.0001f) {
					dir.x /= len;
					dir.y /= len;
				} else {
					dir = {player_->isRight() ? 1.0f : -1.0f, 0.0f, 0.0f};
				}
				float power = 1.0f;
				if (const BaseMelee* pm = player_->GetCurrentMelee()) {
					power = pm->GetImpactPower(player_->GetComboIndex(), Owner::kPlayer); // プレイヤーのサーベル
				}
				enemy->RequestKnockback(dir, power);

				player_->OnCollision(enemy.get());
				if (player_->GetCurrentFirearm())
					player_->GetCurrentFirearm()->NotifyDealtDamage();
			}
		}
	}
	if (player_->IsLungeHitActive()) {
		const AABB lungeAABB = player_->GetLungeAABB();
		const LungeSpec spec = player_->GetLungeSpec(player_->GetCurrentLungeSource());
		for (const std::unique_ptr<BaseEnemy>& enemy : enemies_) {
			if (!enemy || enemy->IsDead() || enemy->IsCollisionDisabled())
				continue;
			if (player_->HasLungeHit(enemy.get()))
				continue;
			if (!IsCollisionAABB(lungeAABB, enemy->GetAABB()))
				continue;
			player_->MarkLungeHit(enemy.get()); // one hit per target per lunge

			// guarded: no damage, the player recoils (same as a blocked sabre swing)
			if (enemy->TryBlockAttack(player_->GetWorldPosition())) {
				player_->OnCollision(enemy.get());
				continue;
			}

			Vector3 dir = enemy->GetWorldPosition() - player_->GetWorldPosition();
			float len = std::sqrtf(dir.x * dir.x + dir.y * dir.y);
			if (len > 0.0001f) {
				dir.x /= len;
				dir.y /= len;
			} else {
				dir = {player_->isRight() ? 1.0f : -1.0f, 0.0f, 0.0f};
			}
			Vector3 hitDir = dir;
			hitDir.y += 0.4f;
			enemy->TakeDamage(spec.damage, hitDir, spec.impactPower);
			enemy->RequestKnockback(dir, spec.impactPower);

			if (player_->GetCurrentFirearm())
				player_->GetCurrentFirearm()->NotifyDealtDamage();
		}
	}
	for (auto& bullet : playerBullets_) {
		if (bullet->IsDead())
			continue;
		AABB bulletAABB = bullet->GetAABB();
		for (const std::unique_ptr<BaseEnemy>& enemy : enemies_) {
			if (!enemy || enemy->IsCollisionDisabled())
				continue;
			if (IsCollisionAABB(bulletAABB, enemy->GetAABB())) {
				enemy->OnCollision(bullet.get());
				bullet->OnCollision();
				break; 
			}
		}
	}

	auto touchesKillBlock = [&](const AABB& aabb) -> bool {
		Vector3 corners[4] = {
		    {aabb.min.x, aabb.min.y, 0.0f},
            {aabb.max.x, aabb.min.y, 0.0f},
            {aabb.min.x, aabb.max.y, 0.0f},
            {aabb.max.x, aabb.max.y, 0.0f}
        };
		for (const Vector3& corner : corners)
			if (mapChipField_->IsKillAtPosition(corner))
				return true;
		return false;
	};
	if (!player_->isDead() && touchesKillBlock(player_->GetAABB()))
		player_->KillInstant();
	for (const std::unique_ptr<BaseEnemy>& enemy : enemies_) {
		if (!enemy || enemy->IsDead() || enemy->IsCollisionDisabled())
			continue;
		if (touchesKillBlock(enemy->GetAABB()))
			enemy->KillInstant();
	}
	auto resolveGateCollision = [&](const AABB& entityAABB, KamataEngine::WorldTransform* wtToPush) {
		for (auto& gate : gates_) {
			if (!gate || !gate->IsBlocking())
				continue;
			AABB gateAABB = gate->GetAABB();
			if (!IsCollisionAABB(entityAABB, gateAABB))
				continue;
			float overlapLeft = entityAABB.max.x - gateAABB.min.x;
			float overlapRight = gateAABB.max.x - entityAABB.min.x;
			float overlapBottom = entityAABB.max.y - gateAABB.min.y;
			float overlapTop = gateAABB.max.y - entityAABB.min.y;
			float minOverlapX = std::min(overlapLeft, overlapRight);
			float minOverlapY = std::min(overlapBottom, overlapTop);
			Vector3 push{};
			if (minOverlapX < minOverlapY)
				push.x = (overlapLeft < overlapRight) ? -overlapLeft : overlapRight;
			else
				push.y = (overlapBottom < overlapTop) ? -overlapBottom : overlapTop;
			wtToPush->translation_ += push;
			WorldTransformUpdate(*wtToPush);
		}
	};
	if (!player_->isDead())
		resolveGateCollision(player_->GetAABB(), &const_cast<KamataEngine::WorldTransform&>(player_->GetWorldTransform()));
	for (const std::unique_ptr<BaseEnemy>& enemy : enemies_) {
		if (!enemy || enemy->IsDead() || enemy->IsCollisionDisabled())
			continue;
		resolveGateCollision(enemy->GetAABB(), &const_cast<KamataEngine::WorldTransform&>(enemy->GetWorldTransform()));
	}

	auto hitDirTo = [&](const Vector3& attackerPos) {
		Vector3 dir = player_->GetWorldPosition() - attackerPos;
		float len = std::sqrtf(dir.x * dir.x + dir.y * dir.y);
		if (len > 0.0001f) {
			dir.x /= len;
			dir.y /= len;
		}
		dir.y += 0.3f;
		return dir;
	};

	auto resolveAttackOnPlayer = [&](const AABB& attackBox, const Vector3& attackerPos, int damage, float power, BaseEnemy* attacker) -> bool {
		Vector3 dir = hitDirTo(attackerPos);
		if (player_->IsBlockingAttack(attackBox, attackerPos)) {
			player_->OnBlocked(dir, power, attacker);
			return true;
		}
		if (!IsCollisionAABB(attackBox, aabb1))
			return false;
		player_->TakeDamage(damage, dir, power, attacker);
		return true;
	};

	auto checkEnemyWeaponVsPlayer = [&](BaseEnemy* enemyPtr) {
		if (!enemyPtr || enemyPtr->IsDead() || enemyPtr->IsCollisionDisabled())
			return;
		if (EnemyFighter* fighter = dynamic_cast<EnemyFighter*>(enemyPtr)) {
			WeaponSabre* melee = fighter->GetCurrentMelee();
			if (fighter->IsSwingActive() && melee && !melee->HasHit(player_.get())) {
				int comboIdx = fighter->GetCurrentComboIndex();
				if (resolveAttackOnPlayer(fighter->GetSwingAABB(), fighter->GetWorldPosition(), melee->GetDamage(comboIdx, Owner::kEnemy), melee->GetImpactPower(comboIdx, Owner::kEnemy), fighter))
					melee->MarkHit(player_.get());
			}
		} else if (EnemyCuirassier* cuirassier = dynamic_cast<EnemyCuirassier*>(enemyPtr)) {
			WeaponHeavySabre* melee = cuirassier->GetCurrentMelee();
			bool active = cuirassier->IsSwingActive() || cuirassier->IsLungeActive();
			if (active && melee && !melee->HasHit(player_.get())) {
				bool lunge = cuirassier->IsLungeActive();
				AABB weaponAABB = lunge ? cuirassier->GetLungeAABB() : cuirassier->GetSwingAABB();
				int comboIdx = cuirassier->GetCurrentComboIndex();
				int damage = lunge ? melee->GetLungeDamage() : melee->GetDamage(comboIdx, Owner::kEnemy);
				float power = lunge ? melee->GetLungeImpactPower() : melee->GetImpactPower(comboIdx, Owner::kEnemy);
				if (resolveAttackOnPlayer(weaponAABB, cuirassier->GetWorldPosition(), damage, power, cuirassier))
					melee->MarkHit(player_.get());
			}
		} else if (EnemyRifleman* rifleman = dynamic_cast<EnemyRifleman*>(enemyPtr)) {
			if (rifleman->IsBayonetActive() && !rifleman->HasHitPlayerThisSwing()) {
				FirearmMusketBayonet* firearm = rifleman->GetCurrentFirearm();
				int damage = firearm ? firearm->GetLungeDamage() : 10;
				float power = firearm ? firearm->GetLungeImpactPower() : 1.8f;
				if (resolveAttackOnPlayer(rifleman->GetBayonetLungeAABB(), rifleman->GetWorldPosition(), damage, power, rifleman))
					rifleman->MarkHitPlayerThisSwing();
			}
		}
	};

	for (const std::unique_ptr<BaseEnemy>& enemy : enemies_)
		checkEnemyWeaponVsPlayer(enemy.get());
#ifdef _DEBUG
	// checkEnemyWeaponVsPlayer(enemyTest.get());
#endif

	for (auto& bullet : enemyBullets_) {
		if (bullet->IsDead())
			continue;
		if (resolveAttackOnPlayer(bullet->GetAABB(), bullet->GetWorldPosition(), bullet->GetDamage(bullet->GetOwner()), bullet->GetImpactPower(), nullptr))
			bullet->OnCollision();
	}

	if (goal_ && !player_->isDead() && phase_ == Phase::kPlay) {
		if (goal_->CheckCollision(aabb1))
			TriggerLevelClear();
	}
}

void GameScene::Draw() {
	skyDome_->Draw();

	if (!worldTransformBackgroundBlocks_.empty()) {
		static KamataEngine::ObjectColor backgroundTint;
		static bool backgroundTintInitialized = false;
		if (!backgroundTintInitialized) {
			backgroundTint.Initialize();
			backgroundTint.SetColor({0.55f, 0.55f, 0.6f, 1.0f});
			backgroundTintInitialized = true;
		}
		std::unordered_map<Model*, std::vector<BlockInstance*>> byModelBg;
		for (auto& line : worldTransformBackgroundBlocks_) {
			for (auto& block : line) {
				if (!block.transform)
					continue;
				byModelBg[block.model].push_back(&block);
			}
		}
		for (auto& [model, instances] : byModelBg) {
			model->PreDraw();
			for (BlockInstance* b : instances)
				model->Draw(*b->transform, camera_, b->textureHandle);
			model->PostDraw();
		}
	}

	player_->Draw();
	for (auto& bullet : playerBullets_)
		bullet->Draw();
	if (goal_)
		goal_->Draw();
	for (auto& gate : gates_)
		if (gate)
			gate->Draw();
	for (auto& sw : switches_)
		if (sw)
			sw->Draw();
	for (auto& hb : healthBlocks_)
		if (hb)
			hb->Draw();
	for (std::unique_ptr<BaseEnemy>& enemy : enemies_) {
		if (!enemy)
			continue;
		if (!IsWithinActiveRange(enemy->GetWorldPosition(), kEntityCullDistance))
			continue;
		enemy->Draw();
	}
	for (auto& bullet : enemyBullets_)
		bullet->Draw();
	for (auto& item : droppableItems_) {
		if (!item || item->IsPickedUp())
			continue;
		if (!IsWithinActiveRange(item->GetWorldPosition(), kEntityCullDistance))
			continue;
		item->Draw();
	}

	std::unordered_map<Model*, std::vector<BlockInstance*>> byModel;
	for (auto& line : worldTransformBlocks_) {
		for (auto& block : line) {
			if (!block.transform)
				continue;
			byModel[block.model].push_back(&block);
		}
	}
	for (auto& [model, instances] : byModel) {
		model->PreDraw();
		for (BlockInstance* b : instances) {
			model->Draw(*b->transform, camera_, b->textureHandle);
		}
		model->PostDraw();
	}

#ifdef _DEBUG
	DrawCollisionDebug();
#endif
	enemyHUD_.Draw();
	interactionPrompt_.Draw();

	playerHealthUI_.Draw();
	weaponStatusUI_.Draw();
	playerBlockUI_.Draw();

	if (deathParticles_) {
		deathParticles_->Draw();
	}
	for (auto& dp : enemyDeathParticles_) {
		if (dp)
			dp->Draw();
	}
	for (std::unique_ptr<BaseEffect>& effect : visualEffects_) {
		if (!effect)
			continue;
		effect->Draw();
	}

	if (phase_ == Phase::kPause) {
		DrawPauseMenu();
	} else if (phase_ == Phase::kRetry) {
		DrawRetryMenu();
	} else if (phase_ == Phase::kVictory) {
		DrawVictoryMenu();
	}

	if (phase_ == Phase::kPlay) {
		cursor_.SetCursorTexture(assets_.cursorTex[0]);
		DrawInGamePauseButton();
	} else {
		cursor_.SetCursorTexture(assets_.cursorTex[1]);
	}
	cursor_.Draw();
	if (fade_) {
		fade_->Draw();
	}

	if (loadingCoverActive_ && loadingCover_) {
		loadingCover_->Draw();
	}
}

BaseEnemy* GameScene::SpawnEnemyAt(const Vector3& position, uint8_t subID) {
	const size_t enemyCountBefore = enemies_.size();
	switch (subID) {
	case 0: {
		std::array<PartResources, 3> parts = {
		    {{assets_.modelRiflemanBody.get(), assets_.riflemanBodyTex}, {assets_.modelRiflemanHead.get(), assets_.riflemanHeadTex}, {assets_.modelRiflemanHat.get(), assets_.riflemanHatTex}}
        };
		auto newEnemy = std::make_unique<EnemyRifleman>();
		newEnemy->SetMapChipField(mapChipField_.get());
		newEnemy->SetGameScene(this);
		newEnemy->Initialize(parts, &camera_, position);
		newEnemy->SetMusketModel(assets_.modelMusketBayonet.get(), assets_.musketBayonetTex);
		newEnemy->SetHUDIconTexture(assets_.enemyIconRiflemanTex);
		enemies_.push_back(std::move(newEnemy));
	} break;
	case 1: {
		std::array<PartResources, 3> parts = {
		    {{assets_.modelCuirassierBody.get(), assets_.cuirassierBodyTex},
		     {assets_.modelCuirassierHead.get(), assets_.cuirassierHeadTex},
		     {assets_.modelCuirassierHat.get(), assets_.cuirassierHatTex}}
        };
		auto newEnemy = std::make_unique<EnemyCuirassier>();
		newEnemy->SetMapChipField(mapChipField_.get());
		newEnemy->SetGameScene(this);
		newEnemy->Initialize(parts, &camera_, position);
		newEnemy->SetMeleeModel(assets_.modelSabreHeavy.get(), assets_.texSabreHeavy);
		newEnemy->SetHUDIconTexture(assets_.enemyIconCuirassierTex);
		enemies_.push_back(std::move(newEnemy));
	} break;
	case 2: {
		std::array<PartResources, 3> parts = {
		    {{assets_.modelFighterBody.get(), assets_.fighterBodyTex}, {assets_.modelFighterHead.get(), assets_.fighterHeadTex}, {assets_.modelFighterHat.get(), assets_.fighterHatTex}}
        };
		auto newEnemy = std::make_unique<EnemyFighter>();
		newEnemy->SetMapChipField(mapChipField_.get());
		newEnemy->SetGameScene(this);
		newEnemy->Initialize(parts, &camera_, position);
		newEnemy->SetMeleeModel(assets_.modelSabreLight.get(), assets_.texSabreLight);
		newEnemy->SetHUDIconTexture(assets_.enemyIconFighterTex);
		enemies_.push_back(std::move(newEnemy));
	} break;
	default:
		break;
	}

	if (enemies_.size() == enemyCountBefore)
		return nullptr;

	BaseEnemy* spawned = enemies_.back().get();
	spawned->GetBehaviorModule().SnapFacing(static_cast<int>(BaseEnemy::LRDirection::kRight));
	return spawned;
}

void GameScene::UpdateEnemySpawning() {
	if (!player_ || phase_ != Phase::kPlay)
		return;

	const Vector3 playerPos = player_->GetWorldPosition();
	for (EnemySpawnPoint& sp : enemySpawnPoints_) {
		if (sp.spawned)
			continue;

		bool omniscient = (sp.placementState & static_cast<uint8_t>(EnemyPlacementState::kOmniscient)) != 0;
		bool shouldSpawn = false;
		if (omniscient) {
			// spawns as soon as its chunk is loaded, regardless of distance
			uint32_t chunk = static_cast<uint32_t>(std::max(0.0f, sp.position.x) / kChunkWorldWidth);
			shouldSpawn = chunk >= oldestLoadedChunk_ && chunk < loadedChunkCount_;
		} else {
			float dx = std::abs(sp.position.x - playerPos.x);
			float dy = std::abs(sp.position.y - playerPos.y);
			shouldSpawn = dx <= kEnemySpawnActivateDistance && dy <= kEnemySpawnActivateDistance;
		}

		if (shouldSpawn) {
			BaseEnemy* spawned = SpawnEnemyAt(sp.position, sp.subID);
			ApplyEnemyPlacementState(spawned, sp.placementState);
			sp.spawned = true;
		}
	}
}

void GameScene::ApplyEnemyPlacementState(BaseEnemy* enemy, uint8_t state) {
	if (!enemy)
		return;
	auto has = [state](EnemyPlacementState flag) { return (state & static_cast<uint8_t>(flag)) != 0; };

	// 1 = hold left, 2 = hold right (both set = original behaviour), 4 = omniscient. Flags can be combined.
	bool holdLeft = has(EnemyPlacementState::kHoldLeft);
	bool holdRight = has(EnemyPlacementState::kHoldRight);
	if (holdLeft && !holdRight)
		enemy->GetBehaviorModule().SetPostMode(BaseEnemyBehavior::PostMode::kHoldLeft);
	else if (holdRight && !holdLeft)
		enemy->GetBehaviorModule().SetPostMode(BaseEnemyBehavior::PostMode::kHoldRight);

	if (has(EnemyPlacementState::kOmniscient)) {
		enemy->SetIgnoreEquipStates(true);
		if (player_)
			enemy->ReceiveAlert(player_->GetWorldPosition());
		enemy->SetPersistentAwareness(true);
	}
}

bool GameScene::IsWithinActiveRange(const Vector3& position, float range) const {
	if (!player_)
		return true;
	float dx = std::abs(position.x - player_->GetWorldPosition().x);
	return dx <= range;
}

DroppableItem* GameScene::CreateDroppableItem(const Vector3& position, DropItemCategory category, DropWeaponKind kind, DropItemState state) {
	Model* model = nullptr;
	uint32_t tex = 0;
	if (!ResolveWeaponVisual(assets_, kind, model, tex) || !model)
		return nullptr;
	auto item = std::make_unique<DroppableItem>();
	item->Initialize(model, tex, &camera_, position, category, kind, state);
	item->SetMapChipField(mapChipField_.get());
	DroppableItem* raw = item.get();
	droppableItems_.push_back(std::move(item));
	return raw; // 次の push_back まで有効。直後に Launch するだけに使う
}

void GameScene::ApplyItemPickup(const DroppableItem& item) {
	// Copy everything we need FIRST: CreateDroppableItem() below pushes into droppableItems_,
	// which can reallocate and leave `item` dangling.
	const DropWeaponKind pickedKind = item.GetWeaponKind();
	const DropItemCategory pickedCategory = item.GetCategory();
	const DropItemState pickedState = item.GetState();
	Vector3 dropPos = player_->GetWorldPosition();

	if (assets_.sfxItemPickup != 0) {
		AudioSettings::PlaySfx(assets_.sfxItemPickup, false, 1.0f);
	}

	Model* model = nullptr;
	uint32_t tex = 0;
	if (!ResolveWeaponVisual(assets_, pickedKind, model, tex))
		return;

		// 手放す武器はプレイヤーの向いている方向へ放り出す
	const Vector3 throwVelocity = DroppableItem::MakeThrowVelocity(player_->isRight());

	if (pickedCategory == DropItemCategory::kMelee) {
		if (DroppableItem* dropped = CreateDroppableItem(dropPos, DropItemCategory::kMelee, player_->GetCurrentMeleeKind(), player_->GetCurrentMeleeState()))
			dropped->Launch(throwVelocity);
		CreateDroppableItem(dropPos, DropItemCategory::kMelee, player_->GetCurrentMeleeKind(), player_->GetCurrentMeleeState());
		player_->SetMeleeModel(model, tex, pickedKind);
		player_->RestoreMeleeState(pickedState); // premature re-pickup: cooldown restarts
		player_->SetWeaponMode(Player::WeaponMode::kMelee);
	} else if (pickedCategory == DropItemCategory::kFirearm) {
		if (DroppableItem* dropped = CreateDroppableItem(dropPos, DropItemCategory::kFirearm, player_->GetCurrentFirearmKind(), player_->GetCurrentFirearmState()))
			dropped->Launch(throwVelocity);
		CreateDroppableItem(dropPos, DropItemCategory::kFirearm, player_->GetCurrentFirearmKind(), player_->GetCurrentFirearmState());
		player_->SetMusketModel(model, tex, pickedKind);
		player_->RestoreFirearmState(pickedState);
		player_->SetWeaponMode(Player::WeaponMode::kFirearm);
	}
}

void GameScene::UpdateFieldStreaming() {
	if (!player_ || phase_ != Phase::kPlay)
		return;

	float playerX = std::max(0.0f, player_->GetWorldPosition().x);
	uint32_t playerChunk = static_cast<uint32_t>(playerX / kChunkWorldWidth);
	playerChunk = std::min(playerChunk, maxFieldChunks_ - 1);

	// Chunk we need loaded: the current one, or the next one once we're inside the trigger margin.
	uint32_t targetChunk = playerChunk;
	float chunkEndX = static_cast<float>(playerChunk + 1) * kChunkWorldWidth;
	float margin = std::min(kChunkLoadTriggerMargin, kChunkWorldWidth);
	if (playerX >= chunkEndX - margin) {
		targetChunk = playerChunk + 1;
	}
	targetChunk = std::min(targetChunk, maxFieldChunks_ - 1);

	// Load every missing chunk up to the target (handles fast movement / teleports / checkpoints).
	bool loadedAny = false;
	while (loadedChunkCount_ <= targetChunk) {
		uint32_t nextChunk = loadedChunkCount_;
		int32_t stageIndex = stageManager_->GetCurrentStageIndex();

		std::string fgPath = stageManager_->GetFieldChunkFileName(stageIndex, nextChunk);
		if (std::ifstream(fgPath).good()) {
			mapChipField_->LoadChunkCsv(nextChunk, fgPath);
		}
		GenerateChunkTileMapRow(nextChunk, MapChipField::kGroundChunkRow);

		if (backgroundMapChipField_) {
			std::string bgPath = stageManager_->GetBackgroundFieldChunkFileName(stageIndex, nextChunk);
			if (std::ifstream(bgPath).good()) {
				backgroundMapChipField_->LoadChunkCsv(nextChunk, bgPath);
			}
			GenerateBackgroundChunkTileMap(nextChunk, MapChipField::kGroundChunkRow);
		}

		loadedChunkCount_ = nextChunk + 1;
		loadedAny = true;
	}

	if (loadedAny) {
		cameraController_->SetWorldBounds(static_cast<float>(loadedChunkCount_) * kChunkWorldWidth, mapChipField_->GetWorldHeight());
		for (uint32_t row = loadedTopChunkRow_; row <= loadedBottomChunkRow_; ++row) {
			if (row == MapChipField::kGroundChunkRow)
				continue;
			LoadVerticalRow(row);
		}
	}

	while (oldestLoadedChunk_ + 1 < playerChunk) {
		UnloadChunkVisuals(oldestLoadedChunk_);
		mapChipField_->UnloadChunk(oldestLoadedChunk_);
		if (backgroundMapChipField_)
			backgroundMapChipField_->UnloadChunk(oldestLoadedChunk_);
		++oldestLoadedChunk_;
	}

	float playerY = player_->GetWorldPosition().y;
	float loadedTopWorldY = static_cast<float>(MapChipField::kMaxVerticalChunks - loadedTopChunkRow_) * kChunkWorldHeight;
	float loadedBottomWorldY = static_cast<float>(MapChipField::kMaxVerticalChunks - 1 - loadedBottomChunkRow_) * kChunkWorldHeight;

	if (loadedTopChunkRow_ > 0 && playerY >= loadedTopWorldY - kVerticalChunkLoadTriggerMargin) {
		--loadedTopChunkRow_;
		LoadVerticalRow(loadedTopChunkRow_);
	}
	if (loadedBottomChunkRow_ < MapChipField::kMaxVerticalChunks - 1 && playerY <= loadedBottomWorldY + kVerticalChunkLoadTriggerMargin) {
		++loadedBottomChunkRow_;
		LoadVerticalRow(loadedBottomChunkRow_);
	}
}

void GameScene::LoadVerticalRow(uint32_t row) {
	int32_t stageIndex = stageManager_->GetCurrentStageIndex();
	int32_t rowOffset = static_cast<int32_t>(MapChipField::kGroundChunkRow) - static_cast<int32_t>(row);

	for (uint32_t chunkX = oldestLoadedChunk_; chunkX < loadedChunkCount_; ++chunkX) {
		std::string fgPath = stageManager_->GetVerticalFieldChunkFileName(stageIndex, chunkX, rowOffset);
		std::ifstream fgTest(fgPath);
		if (fgTest.good()) {
			mapChipField_->LoadChunkCsv(chunkX, row, fgPath);
		}
		GenerateChunkTileMapRow(chunkX, row); // [FIX] row-scoped, idempotent — was GenerateChunkTileMap(chunkX)

		if (backgroundMapChipField_) {
			std::string bgPath = stageManager_->GetBackgroundFieldChunkFileName(stageIndex, chunkX);
			size_t dot = bgPath.rfind(".csv");
			std::string bgRowPath = bgPath;
			if (rowOffset != 0 && dot != std::string::npos) {
				std::string suffix = (rowOffset > 0) ? ("_u" + std::to_string(rowOffset)) : ("_d" + std::to_string(-rowOffset));
				bgRowPath = bgPath;
				bgRowPath.insert(dot, suffix);
			}
			std::ifstream bgTest(bgRowPath);
			if (bgTest.good()) {
				backgroundMapChipField_->LoadChunkCsv(chunkX, row, bgRowPath);
			}
			GenerateBackgroundChunkTileMap(chunkX, row);
		}
	}
}

void GameScene::UnloadChunkVisuals(uint32_t chunkIndex) {
	uint32_t xStart = chunkIndex * MapChipField::kChunkBlockSize;
	uint32_t xEnd = xStart + MapChipField::kChunkBlockSize;

	for (auto& row : worldTransformBlocks_) {
		for (uint32_t j = xStart; j < xEnd && j < row.size(); ++j) {
			delete row[j].transform;
			row[j] = {};
		}
	}
	for (auto& row : worldTransformBackgroundBlocks_) {
		for (uint32_t j = xStart; j < xEnd && j < row.size(); ++j) {
			delete row[j].transform;
			row[j] = {};
		}
	}

	// clear the generation-tracking entries for this chunk across all vertical rows
	for (uint32_t row = 0; row < MapChipField::kMaxVerticalChunks; ++row) {
		generatedChunkRows_.erase({chunkIndex, row});
	}

	float chunkMinX = static_cast<float>(chunkIndex) * kChunkWorldWidth;
	float chunkMaxX = chunkMinX + kChunkWorldWidth;
	auto inRange = [&](float x) { return x >= chunkMinX && x < chunkMaxX; };

	cameraController_->RemoveZoneDataInRange(chunkMinX, chunkMaxX);

	enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(), [&](const std::unique_ptr<BaseEnemy>& e) { return !e || inRange(e->GetWorldPosition().x); }), enemies_.end());

	droppableItems_.erase(
	    std::remove_if(droppableItems_.begin(), droppableItems_.end(), [&](const std::unique_ptr<DroppableItem>& item) { return !item || inRange(item->GetWorldPosition().x); }),
	    droppableItems_.end());

	gates_.erase(std::remove_if(gates_.begin(), gates_.end(), [&](const std::unique_ptr<SlidingGate>& g) { return !g || inRange(g->GetAABB().min.x); }), gates_.end());

	switches_.erase(std::remove_if(switches_.begin(), switches_.end(), [&](const std::unique_ptr<SwitchBlock>& s) { return !s || inRange(s->GetAABB().min.x); }), switches_.end());

	healthBlocks_.erase(
	    std::remove_if(healthBlocks_.begin(), healthBlocks_.end(), [&](const std::unique_ptr<HealthBlock>& hb) { return !hb || inRange(hb->GetWorldPosition().x); }), healthBlocks_.end());

	for (EnemySpawnPoint& sp : enemySpawnPoints_) {
		if (inRange(sp.position.x))
			sp.spawned = true;
	}

	enemySpawners_.erase(std::remove_if(enemySpawners_.begin(), enemySpawners_.end(), [&](const std::unique_ptr<EnemySpawner>& s) { return !s || inRange(s->GetPosition().x); }), enemySpawners_.end());
}

void GameScene::TriggerLevelClear() {
	if (levelCleared_)
		return;
	levelCleared_ = true;

	phase_ = Phase::kVictory;
	victoryMenuInput_.StartButtonFade(true, kButtonFadeDuration);

	if (assets_.sfxGoalReached != 0) {
		AudioSettings::PlaySfx(assets_.sfxGoalReached, false, 1.0f); // 1.0f -> sSfxVolume_
	}
	if (stageManager_)
		stageManager_->ClearCheckpoint();
}

void GameScene::UpdateInteractionPrompt() {
	interactionPrompt_.Update();

	if (!player_ || player_->isDead() || phase_ != Phase::kPlay) {
		interactionPrompt_.Hide();
		nearestInteractableItem_ = nullptr;
		nearestInteractableSwitch_ = nullptr;
		return;
	}

	Vector3 playerPos = player_->GetWorldPosition();
	float bestDistSq = kInteractionRange * kInteractionRange;
	InteractionPrompt::Kind bestKind = InteractionPrompt::Kind::kNone;
	nearestInteractableItem_ = nullptr;
	nearestInteractableSwitch_ = nullptr;

	for (auto& item : droppableItems_) {
		if (!item || item->IsPickedUp() || item->IsExpired())
			continue;
		Vector3 diff = item->GetWorldPosition() - playerPos;
		float distSq = diff.x * diff.x + diff.y * diff.y;
		if (distSq <= bestDistSq) {
			bestDistSq = distSq;
			bestKind = InteractionPrompt::Kind::kWeaponPickup;
			nearestInteractableItem_ = item.get();
			nearestInteractableSwitch_ = nullptr;
		}
	}
	for (auto& sw : switches_) {
		if (!sw || sw->IsPressed()) // [FIX] already-pressed switches stop prompting
			continue;
		AABB swAABB = sw->GetAABB();
		Vector3 swPos = {(swAABB.min.x + swAABB.max.x) * 0.5f, (swAABB.min.y + swAABB.max.y) * 0.5f, 0.0f};
		Vector3 diff = swPos - playerPos;
		float distSq = diff.x * diff.x + diff.y * diff.y;
		if (distSq <= bestDistSq) {
			bestDistSq = distSq;
			bestKind = InteractionPrompt::Kind::kSwitch;
			nearestInteractableSwitch_ = sw.get();
			nearestInteractableItem_ = nullptr;
		}
	}

	if (bestKind == InteractionPrompt::Kind::kNone) {
		interactionPrompt_.Hide();
		return;
	}

	interactionPrompt_.Show(bestKind);
}

void GameScene::TryInteract() {
	if (!player_ || player_->isDead() || phase_ != Phase::kPlay)
		return;
	if (player_->IsControlLocked())
		return;
	if (!Input::GetInstance()->TriggerKey(DIK_F))
		return;

	if (nearestInteractableItem_ && !nearestInteractableItem_->IsPickedUp() && !nearestInteractableItem_->IsExpired()) {
		nearestInteractableItem_->MarkPickedUp();
		ApplyItemPickup(*nearestInteractableItem_);
		nearestInteractableItem_ = nullptr;
		interactionPrompt_.Hide();
		return;
	}

	if (nearestInteractableSwitch_ && !nearestInteractableSwitch_->IsPressed()) {
		nearestInteractableSwitch_->SetPressed(true);
		if (assets_.sfxSwitchPress != 0) {
			AudioSettings::PlaySfx(assets_.sfxSwitchPress, false, 1.0f); // 1.0f -> sSfxVolume_
		}
		uint8_t buttonID = nearestInteractableSwitch_->GetButtonID();
		for (auto& gate : gates_) {
			if (gate && !gate->IsArenaGate() && gate->GetButtonID() == buttonID) {
				gate->Open();
				if (assets_.sfxGateOpen != 0) {
					AudioSettings::PlaySfx(assets_.sfxGateOpen, false, 1.0f); // 1.0f -> sSfxVolume_
				}
			}
		}
		nearestInteractableSwitch_ = nullptr;
		interactionPrompt_.Hide();
	}
}

void GameScene::UpdateArenaTriggers() {
	if (!player_ || player_->isDead() || phase_ != Phase::kPlay)
		return;

	Vector3 playerPos = player_->GetWorldPosition();
	constexpr float kTriggerRadius = 1.0f;

	for (ArenaTriggerPoint& trigger : arenaTriggers_) {
		if (trigger.consumed)
			continue;
		Vector3 diff = playerPos - trigger.position;
		float distSq = diff.x * diff.x + diff.y * diff.y;
		if (distSq <= kTriggerRadius * kTriggerRadius) {
			trigger.consumed = true;
			GetOrCreateArena(trigger.arenaID)->Activate(); // plays arena-start SFX; no camera, no control lock
			musicManager_.StartArenaMusic();
		}
	}
}

ArenaManager* GameScene::FindActiveArena() const {
	for (const auto& arena : arenas_) {
		if (arena && arena->GetState() == ArenaManager::State::kActive)
			return arena.get();
	}
	return nullptr;
}

void GameScene::UpdateArenas() {
	for (auto& arena : arenas_) {
		if (!arena)
			continue;
		ArenaManager::State prevState = arena->GetState();
		arena->Update(this);

		if (prevState == ArenaManager::State::kActive && arena->GetState() == ArenaManager::State::kCleared) {
			if (!FindActiveArena())
				musicManager_.StopArenaMusic();
		}
	}
}

void GameScene::UpdatePeriodicEnemySpawners() {
	if (!player_ || phase_ != Phase::kPlay)
		return;
	Vector3 playerPos = player_->GetWorldPosition();
	for (auto& spawner : enemySpawners_) {
		if (spawner)
			spawner->Update(this, playerPos);
	}
}

void GameScene::UpdateCheckpointTriggers() {
	if (!player_ || player_->isDead() || phase_ != Phase::kPlay)
		return;

	constexpr float kCheckpointRadius = 1.0f;
	Vector3 playerPos = player_->GetWorldPosition();

	for (CheckpointTriggerPoint& cp : checkpointTriggers_) {
		if (cp.consumed)
			continue;
		Vector3 diff = playerPos - cp.position;
		float distSq = diff.x * diff.x + diff.y * diff.y;
		if (distSq <= kCheckpointRadius * kCheckpointRadius) {
			cp.consumed = true;
			SetCheckpoint(cp.position);
		}
	}
}

void GameScene::UpdateHealthBlocks() {
	for (std::unique_ptr<HealthBlock>& block : healthBlocks_) {
		if (block)
			block->Update();
	}

	if (!player_ || player_->isDead() || phase_ != Phase::kPlay)
		return;
	if (player_->GetHP() >= player_->GetMaxHP())
		return;

	AABB playerAABB = player_->GetAABB();
	for (std::unique_ptr<HealthBlock>& block : healthBlocks_) {
		if (!block || !block->IsActive())
			continue;
		if (IsCollisionAABB(playerAABB, block->GetAABB())) {
			player_->SetHP(player_->GetMaxHP());
			block->Consume();
		}
	}
}

const std::vector<GameScene::ArenaFieldConfig>& GameScene::GetArenaFieldConfigs() {
	static const std::vector<ArenaFieldConfig> configs = {
	    ArenaFieldConfig{/*stageIndex*/ 1,
	                     /*arenaID*/ 0,
	                     {
	                         {0, 2, 3.0f, EnemySpawner::SpawnMode::kLimited, 1},
	                         {1, 2, 3.0f, EnemySpawner::SpawnMode::kLimited, 1},
	                         {2, 2, 3.0f, EnemySpawner::SpawnMode::kLimited, 2},
	                         {3, 1, 5.0f, EnemySpawner::SpawnMode::kLimited, 1},
	                     }},
	    ArenaFieldConfig{/*stageIndex*/ 2,
	                     /*arenaID*/ 2,
	                     {
	                         {0, 0, 2.0f, EnemySpawner::SpawnMode::kLimited, 1},
	                         {1, 2, 1.0f, EnemySpawner::SpawnMode::kLimited, 3},
	                         {2, 0, 4.0f, EnemySpawner::SpawnMode::kLimited, 3},
	                         {3, 2, 1.0f, EnemySpawner::SpawnMode::kLimited, 2},
	                         {4, 2, 0.5f, EnemySpawner::SpawnMode::kLimited, 1},
	                         {5, 0, 0.2f, EnemySpawner::SpawnMode::kLimited, 2},
	                     }},
	    ArenaFieldConfig{/*stageIndex*/ 2,
	                     /*arenaID*/ 7,
	                     {
	                         {0, 2, 0.5f, EnemySpawner::SpawnMode::kLimited, 1},
	                         {1, 0, 3.0f, EnemySpawner::SpawnMode::kLimited, 3},
	                         {2, 1, 1.0f, EnemySpawner::SpawnMode::kLimited, 1},
	                         {3, 2, 5.0f, EnemySpawner::SpawnMode::kLimited, 4},
	                         {4, 0, 5.0f, EnemySpawner::SpawnMode::kLimited, 4},
	                         {5, 2, 0.5f, EnemySpawner::SpawnMode::kLimited, 2},
	                         {6, 0, 0.5f, EnemySpawner::SpawnMode::kLimited, 2},
	                         {7, 1, 5.0f, EnemySpawner::SpawnMode::kLimited, 1},
	                     }},
	};
	return configs;
}

GameScene::ArenaSpawnEntry GameScene::ResolveArenaSpawnEntry(int32_t stageIndex, uint8_t arenaID, uint8_t subID) {
	for (const ArenaFieldConfig& config : GetArenaFieldConfigs()) {
		if (config.stageIndex != stageIndex || config.arenaID != arenaID) {
			continue;
		}
		for (const ArenaSpawnEntry& entry : config.spawnTable) {
			if (entry.subIDSlot == subID) {
				return entry;
			}
		}
		break; // matched the (stage, arena) but this subID slot wasn't declared -- fall through to defaults
	}

	// subID 1-3: vanilla default waves, no table entry required.
	if (subID >= 1 && subID <= 3) {
		return ArenaSpawnEntry{subID, static_cast<uint8_t>(subID - 1), 5.0f, EnemySpawner::SpawnMode::kLimited, 3};
	}
	// subID 0: legacy literal fallback (old maps used 0/1/2 directly as enemy type).
	if (subID == 0) {
		return ArenaSpawnEntry{subID, subID, 5.0f, EnemySpawner::SpawnMode::kLimited, 3};
	}
	// subID >= 4 with no matching table entry: inert custom slot -- don't guess.
	return ArenaSpawnEntry{subID, 0xFF, 5.0f, EnemySpawner::SpawnMode::kLimited, 0};
}

void GameScene::DrawCollisionDebug() {
#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_F1))
		showCollisionDebug_ = !showCollisionDebug_;
	if (!showCollisionDebug_ || !player_)
		return;

	using namespace DebugDraw;

	// --- Player body + active melee swing ---
	if (!player_->isDead()) {
		Box(player_->GetAABB(), Color::kPlayer);
		if (player_->IsBlockActive())
			Box(player_->GetBlockAABB(), Color::kBlock);

		if (player_->IsMeleeSwingActive()) {
			if (BaseMelee* melee = player_->GetCurrentMelee()) {
				if (player_->GetAttackAnimState().phase == Player::AttackPhase::kAttack) {
					Box(melee->GetSwingAABB(*player_, player_->GetComboIndex()), Color::kAttack);
				}
			}
		}
		if (player_->IsLungeHitActive())
			Box(player_->GetLungeAABB(), Color::kAttack);
	}
	// --- Camera zones (owned by CameraController) ---
	for (const auto& [key, zone] : cameraController_->GetZones()) {
		bool active = (cameraController_->GetActiveZoneID() == static_cast<int32_t>(key));
		Box(CameraController::GetZoneArea(zone), active ? Color::kCameraZoneActive : Color::kCameraZone);

		BoxAt(zone.focusPoint, 0.5f, 0.5f, 0.5f, Color::kCameraFocus);
		Vector3 camPos = zone.focusPoint + zone.config.offset;
		BoxAt(camPos, 0.3f, 0.3f, 0.3f, Color::kCameraFocus);
		Arrow(camPos, zone.focusPoint, Color::kCameraFocus);

		float halfH = std::tan(camera_.fovAngleY * 0.5f) * std::abs(zone.config.offset.z);
		float halfW = halfH * camera_.aspectRatio;
		Vector3 c[4] = {
		    {zone.focusPoint.x - halfW, zone.focusPoint.y + halfH, zone.focusPoint.z},
		    {zone.focusPoint.x + halfW, zone.focusPoint.y + halfH, zone.focusPoint.z},
		    {zone.focusPoint.x + halfW, zone.focusPoint.y - halfH, zone.focusPoint.z},
		    {zone.focusPoint.x - halfW, zone.focusPoint.y - halfH, zone.focusPoint.z},
		};
		for (int k = 0; k < 4; ++k) {
			Line(c[k], c[(k + 1) % 4], Color::kCameraFocus);
			Line(camPos, c[k], Color::kCameraFocus);
		}
	}
	// --- Enemies: body + active weapon hitboxes ---
	for (const std::unique_ptr<BaseEnemy>& enemy : enemies_) {
		if (!enemy || enemy->IsDead())
			continue;
		if (!IsWithinActiveRange(enemy->GetWorldPosition(), kEntityCullDistance))
			continue;

		Box(enemy->GetAABB(), Color::kEnemy);
		VisionCone(enemy->GetVisionSensor(), enemy->IsPlayerSpotted());

		if (EnemyFighter* fighter = dynamic_cast<EnemyFighter*>(enemy.get())) {
			if (fighter->IsSwingActive())
				Box(fighter->GetSwingAABB(), Color::kAttack);
		} else if (EnemyCuirassier* cuirassier = dynamic_cast<EnemyCuirassier*>(enemy.get())) {
			if (cuirassier->IsLungeActive())
				Box(cuirassier->GetLungeAABB(), Color::kAttack);
			else if (cuirassier->IsSwingActive())
				Box(cuirassier->GetSwingAABB(), Color::kAttack);
		} else if (EnemyRifleman* rifleman = dynamic_cast<EnemyRifleman*>(enemy.get())) {
			if (rifleman->IsBayonetActive())
				Box(rifleman->GetBayonetLungeAABB(), Color::kAttack);
		}
	}

	// --- Projectiles ---
	for (auto& bullet : playerBullets_)
		if (bullet && !bullet->IsDead())
			Box(bullet->GetAABB(), Color::kProjectile);
	for (auto& bullet : enemyBullets_)
		if (bullet && !bullet->IsDead())
			Box(bullet->GetAABB(), Color::kProjectile);

	// --- Interactables ---
	for (auto& item : droppableItems_)
		if (item && !item->IsPickedUp() && !item->IsExpired())
			Box(item->GetAABB(), Color::kObject);
	for (auto& hb : healthBlocks_)
		if (hb && hb->IsActive())
			Box(hb->GetAABB(), Color::kObject);
	for (auto& sw : switches_)
		if (sw)
			Box(sw->GetAABB(), Color::kObject);
	for (auto& gate : gates_)
		if (gate && gate->IsBlocking())
			Box(gate->GetAABB(), Color::kObject);
#endif
}