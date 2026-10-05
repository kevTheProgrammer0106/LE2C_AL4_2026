#include "GameAssets.h"

using namespace KamataEngine;

void GameAssets::Load() {
	LoadModelsAndTextures();
	LoadAudio();
}

void GameAssets::LoadModelsAndTextures() {
	// --- Skydome ---
	modelSkyDome.reset(Model::CreateFromOBJ("skydome", true));
	skyDomeTex = TextureManager::Load("./Resources/skydome/skydome.png");

	// --- Player ---
	modelPlayerHead.reset(Model::CreateFromOBJ("playerhead", true));
	modelPlayerHat.reset(Model::CreateFromOBJ("playerhat", true));
	modelPlayerBody.reset(Model::CreateFromOBJ("playerbody", true));
	playerBodyTex = TextureManager::Load("./Resources/playerBody/tex.png");
	playerHeadTex = TextureManager::Load("./Resources/playerHead/tex.png");
	playerHatTex = TextureManager::Load("./Resources/playerHat/tex.png");

	// --- Weapons ---
	modelSabreLight.reset(Model::CreateFromOBJ("sabreLight"));
	texSabreLight = TextureManager::Load("./Resources/sabreLight/sabre.png");
	modelSabreHeavy.reset(Model::CreateFromOBJ("sabreHeavy"));
	texSabreHeavy = TextureManager::Load("./Resources/sabreHeavy/sabre.png");

	modelMusketLong.reset(Model::CreateFromOBJ("musketLong", true));
	musketLongTex = TextureManager::Load("./Resources/musketLong/musketBody.png");
	modelMusketBayonet.reset(Model::CreateFromOBJ("musketBayonet", true));
	musketBayonetTex = TextureManager::Load("./Resources/musketBayonet/musketBody.png");
	modelProjectileBall.reset(Model::CreateFromOBJ("firearmBall", true));
	firearmBallTex = TextureManager::Load("./Resources/firearmBall/firearmBall.png");

	// --- Enemy: Rifleman ---
	modelRiflemanBody.reset(Model::CreateFromOBJ("riflemanBody", true));
	modelRiflemanHead.reset(Model::CreateFromOBJ("riflemanHead", true));
	modelRiflemanHat.reset(Model::CreateFromOBJ("riflemanHat", true));
	riflemanBodyTex = TextureManager::Load("./Resources/riflemanBody/riflemanTorso.png");
	riflemanHeadTex = TextureManager::Load("./Resources/riflemanHead/riflemanHead.png");
	riflemanHatTex = TextureManager::Load("./Resources/riflemanHat/riflemanShako.png");

	// --- Enemy: Cuirassier ---
	modelCuirassierBody.reset(Model::CreateFromOBJ("cuirassierBody", true));
	modelCuirassierHead.reset(Model::CreateFromOBJ("cuirassierHead", true));
	modelCuirassierHat.reset(Model::CreateFromOBJ("cuirassierHat", true));
	cuirassierBodyTex = TextureManager::Load("./Resources/cuirassierBody/cuirassierTorso.png");
	cuirassierHeadTex = TextureManager::Load("./Resources/cuirassierHead/cuirassierHead.png");
	cuirassierHatTex = TextureManager::Load("./Resources/cuirassierHat/cuirassierHelmet.png");

	// --- Enemy: Fighter ---
	modelFighterBody.reset(Model::CreateFromOBJ("fighterBody", true));
	modelFighterHead.reset(Model::CreateFromOBJ("fighterHead", true));
	modelFighterHat.reset(Model::CreateFromOBJ("fighterHat", true));
	fighterBodyTex = TextureManager::Load("./Resources/fighterBody/fighterTorso.png");
	fighterHeadTex = TextureManager::Load("./Resources/fighterHead/fighterHead.png");
	fighterHatTex = TextureManager::Load("./Resources/fighterHat/fighterCap.png");

	// --- Effects ---
	slashEffectTex = TextureManager::Load("./Resources/hit_effect/hit_effect.png");
	modelSlashEffect.reset(Model::CreateFromOBJ("hit_effect", true));
	hitEffectTex = TextureManager::Load("./Resources/particle/circle.png");
	hitEffectModel.reset(Model::CreateFromOBJ("particle"));
	guardEffectTex = TextureManager::Load("./Resources/ring/ring.png");
	guardEffectModel.reset(Model::CreateFromOBJ("ring"));
	deathParticleTex = TextureManager::Load("./Resources/deathParticle/white1x1.png");
	deathParticleModel.reset(Model::CreateFromOBJ("deathParticle", true));

	// --- Blocks ---
	blockTex = TextureManager::Load("./Resources/blockNormal/blockGrass.png");
	blockModel.reset(Model::CreateFromOBJ("blockNormal"));
	blockPlatformTex = TextureManager::Load("./Resources/platform/block.png");
	blockPlatformModel.reset(Model::CreateFromOBJ("platform"));

	// Per-tile texture variants selected by the map chip subID (0 grass, 1 wood, 2 brick, 3 cobble, 4 concrete, 5 rubber).
	// All point at grassBlock.png for now; swap individual lines when the real textures exist.
	{
		uint32_t grass = TextureManager::Load("./Resources/block/grassBlock.png");
		for (uint8_t subID = 0; subID <= 5; ++subID) {
			blockTexVariants[subID] = grass;
			platformTexVariants[subID] = grass;
		}
	}

	// --- Goal / Gate / Switch / HealthBlock ---
	modelGoal.reset(Model::CreateFromOBJ("block", true));
	goalTex = TextureManager::Load("./Resources/block/block.png");
	modelGate.reset(Model::CreateFromOBJ("block", true));
	gateTex = TextureManager::Load("./Resources/block/block.png");
	modelSwitch.reset(Model::CreateFromOBJ("block", true));
	switchTex = TextureManager::Load("./Resources/block/block.png");
	modelHealthBlock.reset(Model::CreateFromOBJ("block"));
	healthBlockTex = TextureManager::Load("./Resources/block/block.png");

	// --- Gameplay HUD ---
	healthBarBackgroundTex = TextureManager::Load("./Resources/white1x1.png");
	healthBarFillTex = TextureManager::Load("./Resources/white1x1.png");
	enemyIconRiflemanTex = TextureManager::Load("./Resources/UserInterface/iconRifleman.png");
	enemyIconCuirassierTex = TextureManager::Load("./Resources/UserInterface/iconCuirassier.png");
	enemyIconFighterTex = TextureManager::Load("./Resources/UserInterface/iconFighter.png");
	enemyIconArrowTex = TextureManager::Load("./Resources/UserInterface/enemyDirectionArrow.png");
	meleeIconTex = texSabreLight;
	firearmIconTex = musketLongTex;
	interactionPromptTex = TextureManager::Load("./Resources/UserInterface/InteractionPrompt.png");

	// --- Menus / cursor ---
	pauseButtonTex[0] = TextureManager::Load("./Resources/UserInterface/buttonPlay.png");
	pauseButtonTex[1] = TextureManager::Load("./Resources/UserInterface/buttonRetry.png");
	pauseButtonTex[2] = TextureManager::Load("./Resources/UserInterface/buttonBack.png");
	retryButtonTex[0] = TextureManager::Load("./Resources/UserInterface/buttonRetry.png");
	retryButtonTex[1] = TextureManager::Load("./Resources/UserInterface/buttonRestart.png");
	retryButtonTex[2] = TextureManager::Load("./Resources/UserInterface/buttonBack.png");
	victoryButtonTex[0] = TextureManager::Load("./Resources/UserInterface/buttonRestart.png");
	victoryButtonTex[1] = TextureManager::Load("./Resources/UserInterface/buttonBack.png");
	inGamePauseButtonTex = TextureManager::Load("./Resources/UserInterface/buttonIngamePause.png");
	cursorTex[0] = TextureManager::Load("./Resources/UserInterface/directionAim.png");
	cursorTex[1] = TextureManager::Load("./Resources/UserInterface/directionArrow.png");
}

void GameAssets::LoadAudio() {
	Audio* audio = Audio::GetInstance();

	musicIngame = audio->LoadWave("./Resources/sfx_bgm/combatMusic.wav");
	musicArena = audio->LoadWave("./Resources/sfx_bgm/ArenaMusic.wav");

	sfxFirearmFire = audio->LoadWave("./Resources/sfx_bgm/gunShot.wav");
	sfxFirearmReload = audio->LoadWave("./Resources/sfx_bgm/gunReloading.wav");
	sfxMusketLoaded = audio->LoadWave("./Resources/sfx_bgm/gunLoaded.wav");
	sfxMeleeSwingDefault = audio->LoadWave("./Resources/sfx_bgm/swing.wav");
	sfxSwingRight = audio->LoadWave("./Resources/sfx_bgm/swordSlashRight.wav");
	sfxSwingLeft = audio->LoadWave("./Resources/sfx_bgm/swordSlashLeft.wav");
	sfxSwingDownward = audio->LoadWave("./Resources/sfx_bgm/swordSlashDownward.wav");
	sfxBayonetLunge = audio->LoadWave("./Resources/sfx_bgm/swordLunge.wav");

	sfxPlayerHurt = audio->LoadWave("./Resources/sfx_bgm/DefaultHit.wav");
	sfxPlayerDeath = audio->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	sfxPlayerBlockStart = audio->LoadWave("./Resources/sfx_bgm/blockReady.wav");
	sfxPlayerBlockImpact = audio->LoadWave("./Resources/sfx_bgm/blockSuccess.wav");
	sfxWeaponSwitch = audio->LoadWave("./Resources/sfx_bgm/weaponSwitch.wav");

	sfxEnemyDeath[0] = audio->LoadWave("./Resources/sfx_bgm/deathScream1.wav");
	sfxEnemyDeath[1] = audio->LoadWave("./Resources/sfx_bgm/deathScream2.wav");
	sfxEnemyDeath[2] = audio->LoadWave("./Resources/sfx_bgm/deathScream3.wav");

	sfxCuirassierSwing = audio->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	sfxCuirassierLunge = audio->LoadWave("./Resources/sfx_bgm/swordLunge.wav");
	sfxCuirassierMove = audio->LoadWave("./Resources/sfx_bgm/knightWalk.wav");
	sfxCuirassierHurt = audio->LoadWave("./Resources/sfx_bgm/cuiHurt.wav");
	for (int i = 0; i < 3; ++i) {
		sfxCuirassierDeath[i] = audio->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	}

	sfxFootstep = audio->LoadWave("./Resources/sfx_bgm/FootStep.wav");
	sfxJump = audio->LoadWave("./Resources/sfx_bgm/Jump.wav");
	sfxLand = audio->LoadWave("./Resources/sfx_bgm/Landing.wav");
	sfxHealthRestore = audio->LoadWave("./Resources/sfx_bgm/hpHeal.wav");
	sfxSwitchPress = audio->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	sfxGateOpen = audio->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	sfxGoalReached = audio->LoadWave("./Resources/sfx_bgm/victoryJingle.wav");
}