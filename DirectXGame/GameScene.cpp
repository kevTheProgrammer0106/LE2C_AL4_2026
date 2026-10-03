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
	BaseMelee::RegisterGlobalVariables();
	BaseFirearm::RegisterGlobalVariables();
	AnimationManager::RegisterGlobalVariables();
	CameraController::RegisterGlobalVariables();
	PlayerBehavior::RegisterGlobalVariables();
	EnemyRifleman::RegisterGlobalVariables();
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

	modelGoal_.reset(Model::CreateFromOBJ("block", true));
	goalTex_ = TextureManager::Load("./Resources/block/block.png");
	modelGate_.reset(Model::CreateFromOBJ("block", true));
	gateTex_ = TextureManager::Load("./Resources/block/block.png");
	modelSwitch_.reset(Model::CreateFromOBJ("block", true));
	switchTex_ = TextureManager::Load("./Resources/block/block.png");

	skyDome_ = std::make_unique<Skydome>();
	skyDomeTextureHandle_ = TextureManager::Load("./Resources/skydome/skydome.png");
	modelSkyDome_.reset(Model::CreateFromOBJ("skydome", true));
	skyDome_->Initialize(modelSkyDome_.get(), skyDomeTextureHandle_, &camera_);

	modelPlayerHead_.reset(Model::CreateFromOBJ("playerhead", true));
	modelPlayerHat_.reset(Model::CreateFromOBJ("playerhat", true));
	modelPlayerBody_.reset(Model::CreateFromOBJ("playerbody", true));
	playerBodyTex_ = TextureManager::Load("./Resources/playerBody/tex.png");
	playerHeadTex_ = TextureManager::Load("./Resources/playerHead/tex.png");
	playerHatTex_ = TextureManager::Load("./Resources/playerHat/tex.png");

	modelSabreLight_.reset(Model::CreateFromOBJ("sabreLight"));
	texSabreLight_ = TextureManager::Load("./Resources/sabreLight/sabre.png");
	modelSabreHeavy_.reset(Model::CreateFromOBJ("sabreHeavy"));
	texSabreHeavy_ = TextureManager::Load("./Resources/sabreHeavy/sabre.png");

	modelMusketLong_.reset(Model::CreateFromOBJ("musketLong", true));
	musketLongTex_ = TextureManager::Load("./Resources/musketLong/musketBody.png");
	modelMusketBayonet_.reset(Model::CreateFromOBJ("musketBayonet", true));
	musketBayonetTex_ = TextureManager::Load("./Resources/musketBayonet/musketBody.png");
	modelProjectileBall_.reset(Model::CreateFromOBJ("firearmBall", true));
	firearmBallTex_ = TextureManager::Load("./Resources/firearmBall/firearmBall.png");

	modelEnemyBody_.reset(Model::CreateFromOBJ("riflemanBody", true));
	modelEnemyHead_.reset(Model::CreateFromOBJ("riflemanHead", true));
	modelEnemyHat_.reset(Model::CreateFromOBJ("riflemanHat", true));
	enemyBodyTex_ = TextureManager::Load("./Resources/riflemanBody/riflemanTorso.png");
	enemyHeadTex_ = TextureManager::Load("./Resources/riflemanHead/riflemanHead.png");
	enemyHatTex_ = TextureManager::Load("./Resources/riflemanHat/riflemanShako.png");

	deathParticles_ = std::make_unique<DeathParticles>();
	deathParticleTextureHandle_ = TextureManager::Load("./Resources/deathParticle/white1x1.png");
	deathParticleModel.reset(Model::CreateFromOBJ("deathParticle", true));
	deathParticles_->Initialize(deathParticleModel.get(), deathParticleTextureHandle_, &camera_, {0.0f, 0.0f, 0.0f});

	slashEffectTex_ = TextureManager::Load("./Resources/hit_effect/hit_effect.png");
	modelSlashEffect_.reset(Model::CreateFromOBJ("hit_effect", true));
	hitEffectTex_ = TextureManager::Load("./Resources/particle/circle.png");
	hitEffectModel_ = Model::CreateFromOBJ("particle");
	HitEffect::SetModel(hitEffectModel_);
	HitEffect::SetCamera(&camera_);

	guardEffectTex_ = TextureManager::Load("./Resources/ring/ring.png");
	guardEffectModel_.reset(Model::CreateFromOBJ("ring"));
	GuardEffect::SetModel(guardEffectModel_.get());
	GuardEffect::SetCamera(&camera_);

	interactionPromptTex_ = TextureManager::Load("./Resources/UserInterface/InteractionPrompt.png");
	interactionPrompt_.Initialize(interactionPromptTex_);

	// --- SFX loading ---
	{
		uint32_t sfxFirearmFire = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/gunShot.wav");
		uint32_t sfxFirearmReload = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/gunReloading.wav");
		BaseFirearm::SetFireSoundHandle(sfxFirearmFire);
		BaseFirearm::SetReloadSoundHandle(sfxFirearmReload);

		uint32_t sfxMeleeSwingDefault = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/swing.wav");
		BaseMelee::SetDefaultSwingSoundHandle(sfxMeleeSwingDefault);

		uint32_t sfxSwingRight = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/swordSlashRight.wav");
		uint32_t sfxSwingSlashLeft = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/swordSlashLeft.wav");
		uint32_t sfxSwingDownward = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/swordSlashDownward.wav");

		BaseMelee::SetSwingSoundHandle(0, sfxSwingRight);
		BaseMelee::SetSwingSoundHandle(1, sfxSwingSlashLeft);
		BaseMelee::SetSwingSoundHandle(2, sfxSwingDownward);

		uint32_t sfxEnemyDeath0 = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/deathScream1.wav");
		uint32_t sfxEnemyDeath1 = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/deathScream2.wav");
		uint32_t sfxEnemyDeath2 = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/deathScream3.wav");
		BaseEnemy::SetDefaultDeathSoundHandle(0, sfxEnemyDeath0);
		BaseEnemy::SetDefaultDeathSoundHandle(1, sfxEnemyDeath1);
		BaseEnemy::SetDefaultDeathSoundHandle(2, sfxEnemyDeath2);

		uint32_t sfxPlayerHurt = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/DefaultHit.wav");
		uint32_t sfxPlayerDeath = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
		uint32_t sfxPlayerBlockStart = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/blockReady.wav");
		uint32_t sfxPlayerBlockImpact = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/blockSuccess.wav");
		PlayerBehavior::SetHurtSoundHandle(sfxPlayerHurt);
		PlayerBehavior::SetDeathSoundHandle(sfxPlayerDeath);
		PlayerBehavior::SetBlockStartSoundHandle(sfxPlayerBlockStart);
		PlayerBehavior::SetBlockImpactSoundHandle(sfxPlayerBlockImpact);

		// Cuirassier-specific overrides: unique melee swing/lunge, block, and movement (footstep) sounds.
		// All other enemies keep BaseEnemy/BaseMelee defaults for now.
		uint32_t sfxCuirassierSwing = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
		uint32_t sfxCuirassierLunge = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/swordLunge.wav");
		uint32_t sfxCuirassierMove = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/knightWalk.wav");
		WeaponHeavySabre::SetSwingSoundHandle(sfxCuirassierSwing);
		WeaponHeavySabre::SetLungeSoundHandle(sfxCuirassierLunge);
		EnemyCuirassier::SetMovementSoundHandle(sfxCuirassierMove);

		// Cuirassier-specific death SFX, 3 variations (falls back to
		// BaseEnemy's variations automatically if these are left at 0).
		uint32_t sfxCuirassierHurt = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/cuiHurt.wav");
		uint32_t sfxCuirassierDeath0 = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
		uint32_t sfxCuirassierDeath1 = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
		uint32_t sfxCuirassierDeath2 = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
		EnemyCuirassier::SetHurtSoundHandle(sfxCuirassierHurt);
		EnemyCuirassier::SetDeathSoundHandle(0, sfxCuirassierDeath0);
		EnemyCuirassier::SetDeathSoundHandle(1, sfxCuirassierDeath1);
		EnemyCuirassier::SetDeathSoundHandle(2, sfxCuirassierDeath2);

		sfxSwitchPressHandle_ = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
		sfxGateOpenHandle_ = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
		sfxGoalReachedHandle_ = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/victoryJingle.wav");

		uint32_t sfxBayonetLunge = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/swordLunge.wav");
		EnemyRifleman::SetBayonetLungeSoundHandle(sfxBayonetLunge);

		uint32_t sfxWeaponSwitch = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/weaponSwitch.wav");
		Player::SetWeaponSwitchSoundHandle(sfxWeaponSwitch);

		uint32_t sfxFootstep = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/FootStep.wav");
		uint32_t sfxJump = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/Jump.wav");
		uint32_t sfxLand = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/Landing.wav");
		BaseEnemy::SetDefaultFootstepSoundHandle(sfxFootstep);
		BaseEnemy::SetDefaultJumpSoundHandle(sfxJump);
		BaseEnemy::SetDefaultLandSoundHandle(sfxLand);
		Player::SetFootstepSoundHandle(sfxFootstep);
		Player::SetJumpSoundHandle(sfxJump);
		Player::SetLandSoundHandle(sfxLand);

		uint32_t sfxHealthRestore = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/hpHeal.wav");
		HealthBlock::SetConsumeSoundHandle(sfxHealthRestore);

		uint32_t sfxMusketLoaded = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/gunLoaded.wav");
		FirearmMusket::SetMusketLoadedSoundHandle(sfxMusketLoaded);
		;
	}

	{
		ingameMusicSoundDataHandle_ = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/combatMusic.wav");
		arenaMusicSoundDataHandle_ = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/ArenaMusic.wav");
		musicManager_.Initialize(ingameMusicSoundDataHandle_, arenaMusicSoundDataHandle_);
		musicManager_.StopMenuMusic();
	}
	blockTextureHandle = TextureManager::Load("./Resources/blockNormal/blockGrass.png");
	blockTexVariants_[0] = blockTextureHandle;
	platformTexVariants_[0] = blockPlatformTextureHandle;

	blockModel.reset(Model::CreateFromOBJ("blockNormal"));
	blockPlatformTextureHandle = TextureManager::Load("./Resources/platform/block.png");
	blockPlatformModel.reset(Model::CreateFromOBJ("platform"));

	// Additional block texture variants, selected per-tile via the map chip subID.
	// subID 1 = wood, 2 = brick, 3 = cobble, 4 = concrete, 5 = rubber.
	// Each variant reuses the existing block/blockHalfUp/blockHalfDown/platform OBJ shapes
	// (same models as subID 0) and only swaps the texture, following the ResolveBlockTexture
	// lookup pattern already used for subID 0.
	{
		static constexpr uint8_t kSubIDGrass = 0;
		static constexpr uint8_t kSubIDWood = 1;
		static constexpr uint8_t kSubIDBrick = 2;
		static constexpr uint8_t kSubIDCobble = 3;
		static constexpr uint8_t kSubIDConcrete = 4;
		static constexpr uint8_t kSubIDRubber = 5;

		blockTexVariants_[kSubIDGrass] = TextureManager::Load("./Resources/block/grassBlock.png");
		blockTexVariants_[kSubIDWood] = TextureManager::Load("./Resources/block/grassBlock.png");
		blockTexVariants_[kSubIDBrick] = TextureManager::Load("./Resources/block/grassBlock.png");
		blockTexVariants_[kSubIDCobble] = TextureManager::Load("./Resources/block/grassBlock.png");
		blockTexVariants_[kSubIDConcrete] = TextureManager::Load("./Resources/block/grassBlock.png");
		blockTexVariants_[kSubIDRubber] = TextureManager::Load("./Resources/block/grassBlock.png");

		platformTexVariants_[kSubIDGrass] = TextureManager::Load("./Resources/block/grassBlock.png");
		platformTexVariants_[kSubIDWood] = TextureManager::Load("./Resources/block/grassBlock.png");
		platformTexVariants_[kSubIDBrick] = TextureManager::Load("./Resources/block/grassBlock.png");
		platformTexVariants_[kSubIDCobble] = TextureManager::Load("./Resources/block/grassBlock.png");
		platformTexVariants_[kSubIDConcrete] = TextureManager::Load("./Resources/block/grassBlock.png");
		platformTexVariants_[kSubIDRubber] = TextureManager::Load("./Resources/block/grassBlock.png");

		// blockTexVariants_[kSubIDGrass] = TextureManager::Load("./Resources/block/grassBlock.png");
		// blockTexVariants_[kSubIDWood] = TextureManager::Load("./Resources/block/woodBlock.png");
		// blockTexVariants_[kSubIDBrick] = TextureManager::Load("./Resources/block/brickBlock.png");
		// blockTexVariants_[kSubIDCobble] = TextureManager::Load("./Resources/block/cobbleBlock.png");
		// blockTexVariants_[kSubIDConcrete] = TextureManager::Load("./Resources/block/concreteBlock.png");
		// blockTexVariants_[kSubIDRubber] = TextureManager::Load("./Resources/block/rubberBlock.png");

		//		platformTexVariants_[kSubIDGrass] = TextureManager::Load("./Resources/block/grassBlock.png");
		// platformTexVariants_[kSubIDWood] = TextureManager::Load("./Resources/platform/woodBlock.png");
		// platformTexVariants_[kSubIDBrick] = TextureManager::Load("./Resources/platform/brickBlock.png");
		// platformTexVariants_[kSubIDCobble] = TextureManager::Load("./Resources/platform/cobbleBlock.png");
		// platformTexVariants_[kSubIDConcrete] = TextureManager::Load("./Resources/platform/concreteBlock.png");
		// platformTexVariants_[kSubIDRubber] = TextureManager::Load("./Resources/platform/rubberBlock.png");
	}
	cursorTex_[0] = TextureManager::Load("./Resources/UserInterface/directionAim.png");
	cursorTex_[1] = TextureManager::Load("./Resources/UserInterface/directionArrow.png");
	cursor_.Initialize(cursorTex_, 2, &camera_);
	healthBarBackgroundTex_ = TextureManager::Load("./Resources/white1x1.png");
	healthBarFillTex_ = TextureManager::Load("./Resources/white1x1.png");
	playerHealthUI_.Initialize(healthBarBackgroundTex_, healthBarFillTex_);

	modelHealthBlock_.reset(Model::CreateFromOBJ("block"));
	healthBlockTex_ = TextureManager::Load("./Resources/block/block.png");

	enemyIconRiflemanTex_ = TextureManager::Load("./Resources/UserInterface/iconRifleman.png");
	enemyIconCuirassierTex_ = TextureManager::Load("./Resources/UserInterface/iconCuirassier.png");
	enemyIconFighterTex_ = TextureManager::Load("./Resources/UserInterface/iconFighter.png");
	enemyIconArrowTex_ = TextureManager::Load("./Resources/UserInterface/enemyDirectionArrow.png");
	enemyHUD_.Initialize(enemyIconArrowTex_, enemyIconFighterTex_);

	meleeIconTex_ = texSabreLight_;
	firearmIconTex_ = musketLongTex_;
	weaponStatusUI_.Initialize(meleeIconTex_, firearmIconTex_, healthBarBackgroundTex_, healthBarFillTex_);

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
		player_->SetWeaponMode(cp.weaponModeIsFirearm ? Player::WeaponMode::kFirearm : Player::WeaponMode::kMelee);

		cameraController_->SetTarget(player_.get());
		cameraController_->Reset();
		cameraController_->SetWorldBounds(static_cast<float>(loadedChunkCount_) * kChunkWorldWidth, mapChipField_->GetWorldHeight());
	}

	for (const ArenaCameraPoint& cp : arenaCameraPoints_) {
		GetOrCreateArena(cp.arenaID)->SetCameraFocusOverride(cp.position);
	}
	InitializePauseMenu();
	InitializeRetryMenu();
	InitializeInGamePauseButton();
	InitializeVictoryMenu();

	loadingCover_->Start(Fade::Status::FadeOut, kLoadingCoverFadeOutDuration);
}

void GameScene::InitializePauseMenu() {
	pauseButtonTex_[0] = TextureManager::Load("./Resources/UserInterface/buttonPlay.png");
	pauseButtonTex_[1] = TextureManager::Load("./Resources/UserInterface/buttonRetry.png");
	pauseButtonTex_[2] = TextureManager::Load("./Resources/UserInterface/buttonBack.png");

	pauseMenuInput_.Initialize(pauseButtonTex_, 3, &camera_);

	constexpr float kButtonWidth = 220.0f;
	constexpr float kButtonHeight = 160.0f;
	constexpr float kSpacing = 100.0f;
	float centerX = static_cast<float>(WinApp::kWindowWidth) / 2.0f - kButtonWidth / 2.0f;
	float startY = static_cast<float>(WinApp::kWindowHeight) / 2.0f - kSpacing;

	pauseResumeButton_ = pauseMenuInput_.AddButton(pauseButtonTex_[0], centerX, startY, kButtonWidth, kButtonHeight, "Resume");
	pauseMenuInput_.SetButtonOnClick(pauseResumeButton_, [this]() { phase_ = phaseBeforePause_; });

	pauseRetryButton_ = pauseMenuInput_.AddButton(pauseButtonTex_[1], centerX, startY + kSpacing, kButtonWidth, kButtonHeight, "Retry");
	pauseMenuInput_.SetButtonOnClick(pauseRetryButton_, [this]() { reloadRequested_ = true; });

	pauseMainMenuButton_ = pauseMenuInput_.AddButton(pauseButtonTex_[2], centerX, startY + kSpacing * 2.0f, kButtonWidth, kButtonHeight, "Main Menu");
	pauseMenuInput_.SetButtonOnClick(pauseMainMenuButton_, [this]() {
		pendingReturnToMenu_ = true;
		fade_->Start(Fade::Status::FadeOut, 1.0f);
	});
}

void GameScene::InitializeRetryMenu() {
	retryButtonTex_[0] = TextureManager::Load("./Resources/UserInterface/buttonRetry.png");
	retryButtonTex_[1] = TextureManager::Load("./Resources/UserInterface/buttonRestart.png");
	retryButtonTex_[2] = TextureManager::Load("./Resources/UserInterface/buttonBack.png");

	retryMenuInput_.Initialize(pauseButtonTex_, 3, &camera_);

	constexpr float kButtonWidth = 220.0f;
	constexpr float kButtonHeight = 160.0f;
	constexpr float kSpacing = 100.0f;
	float centerX = static_cast<float>(WinApp::kWindowWidth) / 2.0f - kButtonWidth / 2.0f;
	float startY = static_cast<float>(WinApp::kWindowHeight) / 2.0f - kSpacing * 0.5f;

	std::string retryLabel = stageManager_->HasCheckpoint() ? "Retry (Checkpoint)" : "Retry";
	retryButton_ = retryMenuInput_.AddButton(retryButtonTex_[0], centerX, startY, kButtonWidth, kButtonHeight, retryLabel);
	retryMenuInput_.SetButtonOnClick(retryButton_, [this]() { reloadRequested_ = true; });

	restartButton_ = retryMenuInput_.AddButton(retryButtonTex_[1], centerX, startY + kSpacing, kButtonWidth, kButtonHeight, "Restart");
	retryMenuInput_.SetButtonOnClick(restartButton_, [this]() { restartRequested_ = true; });

	retryMainMenuButton_ = retryMenuInput_.AddButton(retryButtonTex_[2], centerX, startY + kSpacing * 2.0f, kButtonWidth, kButtonHeight, "Main Menu");
	retryMenuInput_.SetButtonOnClick(retryMainMenuButton_, [this]() {
		pendingReturnToMenu_ = true;
		fade_->Start(Fade::Status::FadeOut, 1.0f);
	});
}

void GameScene::InitializeInGamePauseButton() {
	inGamePauseButtonTex_ = TextureManager::Load("./Resources/UserInterface/buttonIngamePause.png");
	gameplayHUDInput_.Initialize(inGamePauseButtonTex_, &camera_);

	constexpr float kButtonSize = 44.0f;
	constexpr float kMargin = 16.0f;
	float x = kMargin;
	float y = kMargin;

	inGamePauseButton_ = gameplayHUDInput_.AddButton(inGamePauseButtonTex_, x, y, kButtonSize, kButtonSize, "II");
	gameplayHUDInput_.SetButtonOnClick(inGamePauseButton_, [this]() {
		if (phase_ == Phase::kPlay) {
			phaseBeforePause_ = phase_;
			phase_ = Phase::kPause;
			pauseMenuInput_.StartButtonFade(true, kButtonFadeDuration);
		}
	});
}

void GameScene::InitializeVictoryMenu() {
	victoryButtonTex_[0] = TextureManager::Load("./Resources/UserInterface/buttonRestart.png");
	victoryButtonTex_[1] = TextureManager::Load("./Resources/UserInterface/buttonBack.png");

	victoryMenuInput_.Initialize(victoryButtonTex_, 2, &camera_);
	victoryMenuInput_.SetButtonGroupAlpha(0.0f);

	// dim backdrop behind the buttons, faded in lockstep with the buttons below
	victoryOverlay_.reset(Sprite::Create(healthBarBackgroundTex_, {0.0f, 0.0f}));
	victoryOverlay_->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
	victoryOverlay_->SetColor({0.0f, 0.0f, 0.0f, 0.0f});

	constexpr float kButtonWidth = 220.0f;
	constexpr float kButtonHeight = 160.0f;
	constexpr float kSpacing = 100.0f;
	float centerX = static_cast<float>(WinApp::kWindowWidth) / 2.0f - kButtonWidth / 2.0f;
	float startY = static_cast<float>(WinApp::kWindowHeight) / 2.0f - kSpacing * 0.5f;

	victoryRestartButton_ = victoryMenuInput_.AddButton(victoryButtonTex_[0], centerX, startY, kButtonWidth, kButtonHeight, "Play Again");
	victoryMenuInput_.SetButtonOnClick(victoryRestartButton_, [this]() { restartRequested_ = true; });

	victoryMainMenuButton_ = victoryMenuInput_.AddButton(victoryButtonTex_[1], centerX, startY + kSpacing, kButtonWidth, kButtonHeight, "Back to Menu");
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
	HitEffect* newHitEffect = HitEffect::Create(hitEffectTex_, spawnLocation, rotationZ);
	visualEffects_.push_back(std::unique_ptr<BaseEffect>(newHitEffect));
}
void GameScene::CreateGuardEffect(Vector3& spawnLocation) {
	GuardEffect* newGuardEffect = GuardEffect::Create(guardEffectTex_, spawnLocation);
	visualEffects_.push_back(std::unique_ptr<BaseEffect>(newGuardEffect));
}
void GameScene::CreateMuzzleFlash(Vector3& spawnLocation) {
	HitEffect* flash = HitEffect::Create(hitEffectTex_, spawnLocation);
	visualEffects_.push_back(std::unique_ptr<BaseEffect>(flash));
}
void GameScene::CreateBlockSparkEffect(Vector3& spawnLocation) {
	GuardEffect* spark = GuardEffect::Create(guardEffectTex_, spawnLocation);
	visualEffects_.push_back(std::unique_ptr<BaseEffect>(spark));
}
void GameScene::CreateEnemyDeathParticles(const Vector3& position) {
	auto particles = std::make_unique<DeathParticles>();
	particles->Initialize(deathParticleModel.get(), deathParticleTextureHandle_, &camera_, position);
	enemyDeathParticles_.push_back(std::move(particles));
}
void GameScene::CreateFirearmProjectile(Owner owner, const KamataEngine::Vector3& position, const KamataEngine::Vector2& aimDirection) {
	auto projectile = std::make_unique<ProjectileBall>();
	projectile->InitializeAimed(modelProjectileBall_.get(), firearmBallTex_, &camera_, position, aimDirection, owner);
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
			if (deathParticles_)
				deathParticles_->Initialize(deathParticleModel.get(), deathParticleTextureHandle_, &camera_, deathParticlesPosition);
		}

		for (std::unique_ptr<BaseEnemy>& enemy : enemies_) {
			if (!enemy)
				continue;
			if (!IsWithinActiveRange(enemy->GetWorldPosition(), kEntityCullDistance))
				continue;
			enemy->Update();
		}
#ifdef _DEBUG
		// enemyTest->Update();
#endif
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
			if (activeArenaLockingCamera_) {
				uint8_t activeArenaID = activeArenaLockingCamera_->GetArenaID();
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

			bool isFirearmMode = (player_->GetWeaponMode() == Player::WeaponMode::kFirearm);
			bool isReloading = false;
			float reloadProgress = 1.0f;
			if (FirearmMusket* firearm = player_->GetCurrentFirearm()) {
				isReloading = firearm->IsReloading();
				reloadProgress = firearm->GetReloadProgress01();
			}
			weaponStatusUI_.Update(isFirearmMode, isReloading, reloadProgress);
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
		if (deathParticleModel && deathParticles_->IsFinished())
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
		return; // [FIX] idempotent — closes the duplication bug
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

				Model* shapeModel = blockModel.get();
				uint32_t tex = blockTextureHandle;
				switch (mapChipType) {
				case MapChipType::kBlockHalfUp:
				//	shapeModel = blockHalfUpModel.get();
				//	tex = ResolveBlockTexture(blockHalfUpTexVariants_, subID, blockHalfUpHandle);
				//	break;
				case MapChipType::kBlockHalfDown:
				//	shapeModel = blockHalfDownModel.get();
				//	tex = ResolveBlockTexture(blockHalfDownTexVariants_, subID, blockHalfDownHandle);
				//	break;
				default: // kBlock / kSlowBlock keep the cube shape
					tex = ResolveBlockTexture(blockTexVariants_, subID, blockTextureHandle);
					break;
				}
				worldTransformBlocks_[i][j] = {worldTransform, shapeModel, tex};
			} break;
			case MapChipType::kPlatform: {
				uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
				delete worldTransformBlocks_[i][j].transform;
				WorldTransform* worldTransform = new WorldTransform;
				worldTransform->Initialize();
				worldTransform->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				uint32_t tex = ResolveBlockTexture(platformTexVariants_, subID, blockPlatformTextureHandle);
				worldTransformBlocks_[i][j] = {worldTransform, blockPlatformModel.get(), tex};
			} break;
			case MapChipType::kArenaCamera: {
				uint8_t arenaID = mapChipField_->GetMapChipLinkIDByIndex(j, i);
				Vector3 camPos = mapChipField_->GetMapChipPositionByIndex(j, i);

				arenaCameraPoints_.push_back({camPos, arenaID});

				// ストリーミングで生成されたカメラポイントも、その場でArenaManagerへ登録する。
				// これで初回ロード時でも、後からロードされたFタイルでも同じフォーカスを使える。
				GetOrCreateArena(arenaID)->SetCameraFocusOverride(camPos);
			} break;
			case MapChipType::kPlayer: {
				player_ = std::make_unique<Player>();
				player_->SetMapChipField(mapChipField_.get());
				player_->SetGameScene(this);
				Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(j, i);
				std::array<PartResources, 3> playerParts = {
				    {{modelPlayerBody_.get(), playerBodyTex_}, {modelPlayerHead_.get(), playerHeadTex_}, {modelPlayerHat_.get(), playerHatTex_}}
                };
				player_->Initialize(playerParts, modelSlashEffect_.get(), slashEffectTex_, &camera_, playerPosition);
				player_->SetMusketModel(modelMusketLong_.get(), musketLongTex_);
				player_->SetMeleeModel(modelSabreLight_.get(), texSabreLight_);
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
				goal_->Initialize(modelGoal_.get(), goalTex_, &camera_, goalPosition);
			} break;
			case MapChipType::kGateUp:
			case MapChipType::kGateDown:
			case MapChipType::kGateLeft:
			case MapChipType::kGateRight: {
				uint8_t buttonID = mapChipField_->GetMapChipLinkIDByIndex(j, i);
				uint8_t textureVariant = mapChipField_->GetMapChipSubIDByIndex(j, i);
				Vector3 gatePosition = mapChipField_->GetMapChipPositionByIndex(j, i);
				auto gate = std::make_unique<SlidingGate>();
				gate->Initialize(modelGate_.get(), gateTex_, &camera_, gatePosition, mapChipType, textureVariant, buttonID);
				GetOrCreateArena(buttonID)->RegisterGate(gate.get()); // [NEW] same ID doubles as arena tag
				gates_.push_back(std::move(gate));
			} break;
			case MapChipType::kSwitch: {
				uint8_t buttonID = mapChipField_->GetMapChipSubIDByIndex(j, i);
				Vector3 switchPosition = mapChipField_->GetMapChipPositionByIndex(j, i);
				auto sw = std::make_unique<SwitchBlock>();
				sw->Initialize(modelSwitch_.get(), switchTex_, &camera_, switchPosition, buttonID);
				switches_.push_back(std::move(sw));
			} break;
			case MapChipType::kArenaTrigger: { // [NEW]
				uint8_t arenaID = mapChipField_->GetMapChipLinkIDByIndex(j, i);
				Vector3 triggerPos = mapChipField_->GetMapChipPositionByIndex(j, i);
				arenaTriggers_.push_back({triggerPos, arenaID, false});
				ArenaManager* arena = GetOrCreateArena(arenaID);

				// [NEW]
				Vector3 offsetOverride{};
				if (GetArenaCameraOffsetOverride(stageManager_->GetCurrentStageIndex(), arenaID, offsetOverride)) {
					arena->SetCameraOffsetOverride(offsetOverride);
				}
			} break;
			case MapChipType::kEnemySpawner: {
				uint8_t enemySubID = mapChipField_->GetMapChipSubIDByIndex(j, i); // wave slot/order: S00->0, S10->1, S20->2...
				uint8_t arenaID = mapChipField_->GetMapChipLinkIDByIndex(j, i);
				Vector3 spawnerPos = mapChipField_->GetMapChipPositionByIndex(j, i);

				// [FIX] Pull the FULL wave config (enemy type, spawn interval,
				// mode, and max count) out of the spawn table instead of
				// hard-coding interval=5.0f/kLimited/3 and only ever applying
				// the resolved enemy type afterward. Previously every
				// per-arena override of interval/mode/maxSpawnCount in
				// GetArenaFieldConfigs() was computed by ResolveArenaSpawnEntry
				// and then silently discarded -- this is why wave customization
				// never actually took effect in-game.
				ArenaSpawnEntry waveEntry = ResolveArenaSpawnEntry(stageManager_->GetCurrentStageIndex(), arenaID, enemySubID);

				auto spawner = std::make_unique<EnemySpawner>();
				spawner->Initialize(spawnerPos, enemySubID, waveEntry.interval, waveEntry.mode, waveEntry.maxSpawnCount);
				spawner->SetArenaID(arenaID);
				spawner->SetActive(false); // ArenaManager turns on only the current wave's spawner (see ArenaManager::ActivateCurrentWave)

				spawner->SetResolvedEnemyType(waveEntry.enemyType);
				spawner->SetSpawnedEnemiesOmniscient(true);

				ArenaManager* arena = GetOrCreateArena(arenaID);
				arena->RegisterSpawner(spawner.get());

				// Apply hard-coded per-arena camera offset override, once per arena.
				Vector3 offsetOverride{};
				if (GetArenaCameraOffsetOverride(stageManager_->GetCurrentStageIndex(), arenaID, offsetOverride)) {
					arena->SetCameraOffsetOverride(offsetOverride);
				}
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
				healthBlock->Initialize(modelHealthBlock_.get(), healthBlockTex_, &camera_, healthBlockPos, mode);
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
			uint32_t tex = ResolveBlockTexture(blockTexVariants_, subID, blockTextureHandle);

			worldTransformBackgroundBlocks_[i][j] = {worldTransform, blockModel.get(), tex};
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
	Player::ApplyGlobalVariables();
	HitEffect::ApplyGlobalVariables();
	GuardEffect::ApplyGlobalVariables();
	DeathParticles::ApplyGlobalVariables();
	WeaponSabre::ApplyGlobalVariables();
	FirearmMusket::ApplyGlobalVariables();
	DroppableItem::ApplyGlobalVariables();
	WeaponHeavySabre::ApplyGlobalVariables();
	FirearmMusketBayonet::ApplyGlobalVariables();
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
		} else {
			cameraController_->Update();
			if (phase_ == Phase::kPlay)
				UpdateInGamePauseButton();
		}
	}

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

	if (player_->isAttack()) {
		WeaponSabre* melee = static_cast<WeaponSabre*>(player_->GetCurrentMelee());
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
				if (const BaseMelee* pm = player_->GetCurrentMelee())
					power = pm->GetImpactPower(player_->GetComboIndex());
				enemy->RequestKnockback(dir, power);

				player_->OnCollision(enemy.get());
				if (player_->GetCurrentFirearm())
					player_->GetCurrentFirearm()->NotifyDealtDamage();
			}
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
				if (resolveAttackOnPlayer(fighter->GetSwingAABB(), fighter->GetWorldPosition(), melee->GetDamage(comboIdx, Owner::kEnemy), melee->GetImpactPower(comboIdx), fighter))
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
				float power = lunge ? melee->GetLungeImpactPower() : melee->GetImpactPower(comboIdx);
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
		cursor_.SetCursorTexture(cursorTex_[0]);
		DrawInGamePauseButton();
	} else {
		cursor_.SetCursorTexture(cursorTex_[1]);
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
		std::array<PartResources, 3> enemyParts = {
		    {{modelEnemyBody_.get(), enemyBodyTex_}, {modelEnemyHead_.get(), enemyHeadTex_}, {modelEnemyHat_.get(), enemyHatTex_}}
        };
		auto newEnemy = std::make_unique<EnemyRifleman>();
		newEnemy->SetMapChipField(mapChipField_.get());
		newEnemy->SetGameScene(this);
		newEnemy->Initialize(enemyParts, &camera_, position);
		newEnemy->SetMusketModel(modelMusketBayonet_.get(), musketBayonetTex_);
		enemies_.push_back(std::move(newEnemy));
	} break;
	case 1: {
		std::array<PartResources, 3> enemyParts = {
		    {{modelEnemyBody_.get(), enemyBodyTex_}, {modelEnemyHead_.get(), enemyHeadTex_}, {modelEnemyHat_.get(), enemyHatTex_}}
        };
		auto newEnemy = std::make_unique<EnemyCuirassier>();
		newEnemy->SetMapChipField(mapChipField_.get());
		newEnemy->SetGameScene(this);
		newEnemy->Initialize(enemyParts, &camera_, position);
		newEnemy->SetMeleeModel(modelSabreHeavy_.get(), texSabreHeavy_);
		enemies_.push_back(std::move(newEnemy));
	} break;
	case 2: {
		std::array<PartResources, 3> enemyParts = {
		    {{modelEnemyBody_.get(), enemyBodyTex_}, {modelEnemyHead_.get(), enemyHeadTex_}, {modelEnemyHat_.get(), enemyHatTex_}}
        };
		auto newEnemy = std::make_unique<EnemyFighter>();
		newEnemy->SetMapChipField(mapChipField_.get());
		newEnemy->SetGameScene(this);
		newEnemy->Initialize(enemyParts, &camera_, position);
		newEnemy->SetMeleeModel(modelSabreLight_.get(), texSabreLight_);
		enemies_.push_back(std::move(newEnemy));
	} break;
	default:
		break;
	}

	if (enemies_.size() == enemyCountBefore)
		return nullptr;

	BaseEnemy* spawned = enemies_.back().get();
	// BaseEnemy::Initialize starts with rotation_.y facing left while lrDirection_ is kRight (Cuirassier never
	// overrides it), and FaceDirection(kRight) early-outs, so a right-facing enemy kept a left-facing body
	// with right-facing weapons. Snap body + direction together on spawn.
	spawned->GetBehaviorModule().SnapFacing(static_cast<int>(BaseEnemy::LRDirection::kRight));
	return spawned;
}

void GameScene::ApplyEnemyPlacementState(BaseEnemy* enemy, uint8_t placementState) {
	if (!enemy)
		return;
	switch (static_cast<EnemyPlacementState>(placementState)) {
	case EnemyPlacementState::kHoldLeft:
		enemy->GetBehaviorModule().SetPostMode(BaseEnemyBehavior::PostMode::kHoldLeft);
		break;
	case EnemyPlacementState::kHoldRight:
		enemy->GetBehaviorModule().SetPostMode(BaseEnemyBehavior::PostMode::kHoldRight);
		break;
	case EnemyPlacementState::kOmniscient:
		// Same setup EnemySpawner uses for arena enemies.
		enemy->SetIgnoreEquipStates(true);
		if (player_)
			enemy->ReceiveAlert(player_->GetWorldPosition());
		enemy->SetPersistentAwareness(true);
		break;
	case EnemyPlacementState::kWander:
	default: // 3 and anything else: original behavior
		break;
	}
}

void GameScene::UpdateEnemySpawning() {
	if (!player_)
		return;
	Vector3 playerPos = player_->GetWorldPosition();
	for (EnemySpawnPoint& spawnPoint : enemySpawnPoints_) {
		if (spawnPoint.spawned)
			continue;
		bool shouldSpawn = false;
		if (spawnPoint.placementState == static_cast<uint8_t>(EnemyPlacementState::kOmniscient)) {
			// Omniscient enemies spawn as soon as their chunk is loaded, regardless of distance.
			uint32_t chunk = static_cast<uint32_t>(std::max(0.0f, spawnPoint.position.x) / kChunkWorldWidth);
			shouldSpawn = chunk >= oldestLoadedChunk_ && chunk < loadedChunkCount_;
		} else {
			shouldSpawn = std::abs(spawnPoint.position.x - playerPos.x) <= kEnemySpawnActivateDistance;
		}
		if (shouldSpawn) {
			BaseEnemy* spawned = SpawnEnemyAt(spawnPoint.position, spawnPoint.subID);
			ApplyEnemyPlacementState(spawned, spawnPoint.placementState);
			spawnPoint.spawned = true;
		}
	}
}

bool GameScene::IsWithinActiveRange(const Vector3& position, float range) const {
	if (!player_)
		return true;
	float dx = std::abs(position.x - player_->GetWorldPosition().x);
	return dx <= range;
}

void GameScene::CreateDroppableItem(const Vector3& position, DropItemCategory category, DropWeaponKind kind) {
	Model* model = nullptr;
	uint32_t tex = 0;
	switch (kind) {
	case DropWeaponKind::kSabreLight:
		model = modelSabreLight_.get();
		tex = texSabreLight_;
		break;
	case DropWeaponKind::kSabreHeavy:
		model = modelSabreHeavy_.get();
		tex = texSabreHeavy_;
		break;
	case DropWeaponKind::kMusketLong:
		model = modelMusketLong_.get();
		tex = musketLongTex_;
		break;
	case DropWeaponKind::kMusketBayonet:
		model = modelMusketBayonet_.get();
		tex = musketBayonetTex_;
		break;
	default:
		break;
	}
	if (!model)
		return;
	auto item = std::make_unique<DroppableItem>();
	item->Initialize(model, tex, &camera_, position, category, kind);
	droppableItems_.push_back(std::move(item));
}

void GameScene::ApplyItemPickup(const DroppableItem& item) {
	Vector3 dropPos = player_->GetWorldPosition();

	if (sfxItemPickupHandle_ != 0) {
		Audio::GetInstance()->PlayWave(sfxItemPickupHandle_, false, 1.0f);
	}

	switch (item.GetWeaponKind()) {
	case DropWeaponKind::kSabreLight:
		CreateDroppableItem(dropPos, DropItemCategory::kMelee, player_->GetCurrentMeleeKind());
		player_->SetMeleeModel(modelSabreLight_.get(), texSabreLight_);
		player_->SetCurrentMeleeKind(DropWeaponKind::kSabreLight);
		player_->SetWeaponMode(Player::WeaponMode::kMelee);
		break;
	case DropWeaponKind::kSabreHeavy:
		CreateDroppableItem(dropPos, DropItemCategory::kMelee, player_->GetCurrentMeleeKind());
		player_->SetMeleeModel(modelSabreHeavy_.get(), texSabreHeavy_);
		player_->SetCurrentMeleeKind(DropWeaponKind::kSabreHeavy);
		player_->SetWeaponMode(Player::WeaponMode::kMelee);
		break;
	case DropWeaponKind::kMusketLong:
		CreateDroppableItem(dropPos, DropItemCategory::kFirearm, player_->GetCurrentFirearmKind());
		player_->SetMusketModel(modelMusketLong_.get(), musketLongTex_);
		player_->SetCurrentFirearmKind(DropWeaponKind::kMusketLong);
		player_->SetWeaponMode(Player::WeaponMode::kFirearm);
		break;
	case DropWeaponKind::kMusketBayonet:
		CreateDroppableItem(dropPos, DropItemCategory::kFirearm, player_->GetCurrentFirearmKind());
		player_->SetMusketModel(modelMusketBayonet_.get(), musketBayonetTex_);
		player_->SetCurrentFirearmKind(DropWeaponKind::kMusketBayonet);
		player_->SetWeaponMode(Player::WeaponMode::kFirearm);
		break;
	default:
		break;
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

	// [NEW] clear the generation-tracking entries for this chunk across all vertical rows
	for (uint32_t row = 0; row < MapChipField::kMaxVerticalChunks; ++row) {
		generatedChunkRows_.erase({chunkIndex, row});
	}

	float chunkMinX = static_cast<float>(chunkIndex) * kChunkWorldWidth;
	float chunkMaxX = chunkMinX + kChunkWorldWidth;
	auto inRange = [&](float x) { return x >= chunkMinX && x < chunkMaxX; };

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

	if (sfxGoalReachedHandle_ != 0) {
		Audio::GetInstance()->PlayWave(sfxGoalReachedHandle_, false, 1.0f);
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
		if (sfxSwitchPressHandle_ != 0) {
			Audio::GetInstance()->PlayWave(sfxSwitchPressHandle_, false, 1.0f);
		}
		uint8_t buttonID = nearestInteractableSwitch_->GetButtonID();
		for (auto& gate : gates_) {
			if (gate && gate->GetButtonID() == buttonID) {
				gate->Open();
				if (sfxGateOpenHandle_ != 0) {
					Audio::GetInstance()->PlayWave(sfxGateOpenHandle_, false, 1.0f);
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

			ArenaManager* arena = GetOrCreateArena(trigger.arenaID);

			arena->Activate();
			musicManager_.StartArenaMusic(); // arena BGM crossfades in on trigger touch

			player_->SetControlLocked(true);
			Vector3 focusPoint = arena->GetCameraFocusPoint() + arena->GetEffectiveCameraOffset();
			cameraController_->StartArenaFocus(focusPoint, 0.8f);
			activeArenaLockingCamera_ = arena;
		}
	}
}

void GameScene::UpdateArenas() {
	if (activeArenaLockingCamera_ && player_ && player_->IsControlLocked() && cameraController_->IsArenaFocusHolding()) {
		player_->SetControlLocked(false);
	}

	for (auto& arena : arenas_) {
		if (!arena) {
			continue;
		}
		ArenaManager::State prevState = arena->GetState();
		arena->Update(this);

		if (prevState == ArenaManager::State::kActive && arena->GetState() == ArenaManager::State::kCleared && activeArenaLockingCamera_ == arena.get()) {
			if (player_) {
				player_->SetControlLocked(true);
			}
			cameraController_->EndArenaFocus(0.8f);
			musicManager_.StopArenaMusic();
			arenaReturningToFollow_ = true; // [FIX] only now is a return-to-follow actually in flight
		}
	}

	if (activeArenaLockingCamera_ && arenaReturningToFollow_ && cameraController_->GetMode() == CameraController::Mode::kFollow) {
		if (player_) {
			player_->SetControlLocked(false);
		}
		activeArenaLockingCamera_ = nullptr;
		arenaReturningToFollow_ = false;
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
	    // --- Stage 1, Arena 0 ---
	    ArenaFieldConfig{
	                     /*stageIndex*/ 1,
	                     /*arenaID*/ 0,
	                     /*spawnTable*/
	        {
	            {0, 2, 3.0f, EnemySpawner::SpawnMode::kLimited, 1},
	            {1, 2, 3.0f, EnemySpawner::SpawnMode::kLimited, 1}, // wave 1: Fighter
	            {2, 2, 3.0f, EnemySpawner::SpawnMode::kLimited, 2}, // wave 2: Fighter
	            {3, 1, 5.0f, EnemySpawner::SpawnMode::kLimited, 1}, // wave 3 Cuirassier
	        },	                         /*hasCameraOffsetOverride*/ false,
	                     /*cameraOffsetOverride*/ {},
	                     },

	    // --- Stage 2, Arena 0 ---
	    ArenaFieldConfig{
	                     /*stageIndex*/ 2,
	                     /*arenaID*/ 2,
	                     /*spawnTable*/
	        {
	            {0, 0, 2.0f, EnemySpawner::SpawnMode::kLimited, 1},
	            {1, 2, 1.0f, EnemySpawner::SpawnMode::kLimited, 3},
	            {2, 0, 4.0f, EnemySpawner::SpawnMode::kLimited, 3},
	            {3, 2, 1.0f, EnemySpawner::SpawnMode::kLimited, 2},
	            {4, 2, 0.5f, EnemySpawner::SpawnMode::kLimited, 1},
	            {5, 0, 0.2f, EnemySpawner::SpawnMode::kLimited, 2},
	        }, /*hasCameraOffsetOverride*/ true,
	                     /*cameraOffsetOverride*/ {0.0f, 0.0f, -35.0f},
	                     },
	    // --- Stage 2, Arena 1 ---
	    ArenaFieldConfig{
	                     /*stageIndex*/ 2,
	                     /*arenaID*/ 7,
	                     /*spawnTable*/
	        {
	            {0, 2, 0.5f, EnemySpawner::SpawnMode::kLimited, 1}, // fighter followed by rifleman
	            {1, 0, 3.0f, EnemySpawner::SpawnMode::kLimited, 3},
	            {2, 1, 1.0f, EnemySpawner::SpawnMode::kLimited, 1}, // cuirassier will join the fray
	            {3, 2, 5.0f, EnemySpawner::SpawnMode::kLimited, 4}, // fighter invades from the left
	            {4, 0, 5.0f, EnemySpawner::SpawnMode::kLimited, 4}, // rifleman flanks at right
	            {5, 2, 0.5f, EnemySpawner::SpawnMode::kLimited, 2}, // FIGHTEERRR
	            {6, 0, 0.5f, EnemySpawner::SpawnMode::kLimited, 2}, // RIFLEMANNN
	            {7, 1, 5.0f, EnemySpawner::SpawnMode::kLimited, 1}, // CUIRASSIERRR
	        },	            /*hasCameraOffsetOverride*/ true,
	                     /*cameraOffsetOverride*/ {0.0f, 0.0f, -35.0f},
	                     },
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
bool GameScene::GetArenaCameraOffsetOverride(int32_t stageIndex, uint8_t arenaID, Vector3& outOffset) {
	for (const ArenaFieldConfig& config : GetArenaFieldConfigs()) {
		if (config.stageIndex == stageIndex && config.arenaID == arenaID && config.hasCameraOffsetOverride) {
			outOffset = config.cameraOffsetOverride;
			return true;
		}
	}
	return false;
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

		if (player_->isAttack()) {
			// dynamic_cast: the player may be holding WeaponHeavySabre, which has no player-side swing AABB
			if (WeaponSabre* sabre = dynamic_cast<WeaponSabre*>(player_->GetCurrentMelee())) {
				if (player_->GetAttackAnimState().phase == Player::AttackPhase::kAttack) {
					Box(sabre->GetSwingAABB(*player_, player_->GetComboIndex()), Color::kAttack);
				}
			}
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