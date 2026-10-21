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
	modelSabreLight.reset(Model::CreateFromOBJ("weaponSabre"));
	texSabreLight = TextureManager::Load("./Resources/weaponSabre/weaponSabre.png");
	modelSabreHeavy.reset(Model::CreateFromOBJ("weaponHeavySword"));
	texSabreHeavy = TextureManager::Load("./Resources/weaponHeavySword/weaponHeavySword.png");
	modelSpear.reset(Model::CreateFromOBJ("weaponSpontoon"));
	texSpear = TextureManager::Load("./Resources/weaponSpontoon/weaponSpontoon.png");
	modelAxe.reset(Model::CreateFromOBJ("weaponAxe"));
	texAxe = TextureManager::Load("./Resources/weaponAxe/weaponAxe.png");

	modelMusketLong.reset(Model::CreateFromOBJ("musketLong", true));
	musketLongTex = TextureManager::Load("./Resources/musketLong/musketLong.png");
	modelMusketBayonet.reset(Model::CreateFromOBJ("musketBayonet", true));
	musketBayonetTex = TextureManager::Load("./Resources/musketBayonet/musketBayonet.png");
	modelShotgun.reset(Model::CreateFromOBJ("musketBlunderbuss", true));
	shotgunTex = TextureManager::Load("./Resources/musketBlunderbuss/musketSkin.png");
	modelPistol.reset(Model::CreateFromOBJ("musketPistol", true));
	pistolTex = TextureManager::Load("./Resources/musketBlunderbuss/musketSkin.png");


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
	fighterHatTex = TextureManager::Load("./Resources/fighterHat/fighterShako.png");

	// --- Enemy: Carabinier ---
	//modelCarabinierBody.reset(Model::CreateFromOBJ("carabinierBody", true));
	//modelCarabinierHead.reset(Model::CreateFromOBJ("carabinierHead", true));
	//modelCarabinierHat.reset(Model::CreateFromOBJ("carabinierHat", true));
	//carabinierBodyTex = TextureManager::Load("./Resources/carabinierBody/carabinierTorso.png");
	//carabinierHeadTex = TextureManager::Load("./Resources/carabinierHead/carabinierHead.png");
	//carabinierHatTex = TextureManager::Load("./Resources/carabinierHat/carabinierHelmet.png");
	modelCarabinierBody.reset(Model::CreateFromOBJ("cuirassierBody", true));
	modelCarabinierHead.reset(Model::CreateFromOBJ("cuirassierHead", true));
	modelCarabinierHat.reset(Model::CreateFromOBJ("cuirassierHat", true));
	carabinierBodyTex = TextureManager::Load("./Resources/cuirassierBody/cuirassierTorso.png");
	carabinierHeadTex = TextureManager::Load("./Resources/cuirassierHead/cuirassierHead.png");
	carabinierHatTex = TextureManager::Load("./Resources/cuirassierHat/cuirassierHelmet.png");

	// --- Enemy: Shotgunner ---
	//modelShotgunnerBody.reset(Model::CreateFromOBJ("shotgunnerBody", true));
	//modelShotgunnerHead.reset(Model::CreateFromOBJ("shotgunnerHead", true));
	//modelShotgunnerHat.reset(Model::CreateFromOBJ("shotgunnerHat", true));
	//shotgunnerBodyTex = TextureManager::Load("./Resources/shotgunnerBody/shotgunnerTorso.png");
	//shotgunnerHeadTex = TextureManager::Load("./Resources/shotgunnerHead/shotgunnerHead.png");
	//shotgunnerHatTex = TextureManager::Load("./Resources/shotgunnerHat/shotgunnerShako.png");

	modelShotgunnerBody.reset(Model::CreateFromOBJ("riflemanBody", true));
	modelShotgunnerHead.reset(Model::CreateFromOBJ("riflemanHead", true));
	modelShotgunnerHat.reset(Model::CreateFromOBJ("riflemanHat", true));
	shotgunnerBodyTex = TextureManager::Load("./Resources/riflemanBody/riflemanTorso.png");
	shotgunnerHeadTex = TextureManager::Load("./Resources/riflemanHead/riflemanHead.png");
	shotgunnerHatTex = TextureManager::Load("./Resources/riflemanHat/riflemanShako.png");

	// --- Enemy: Grenadier ---
	// modelGrenadierBody.reset(Model::CreateFromOBJ("grenadierBody", true));
	// modelGrenadierHead.reset(Model::CreateFromOBJ("grenadierHead", true));
	// modelGrenadierHat.reset(Model::CreateFromOBJ("grenadierHat", true));
	// grenadierBodyTex = TextureManager::Load("./Resources/grenadierBody/grenadierTorso.png");
	// grenadierHeadTex = TextureManager::Load("./Resources/grenadierHead/grenadierHead.png");
	// grenadierHatTex = TextureManager::Load("./Resources/grenadierHat/grenadierShako.png");

	modelGrenadierBody.reset(Model::CreateFromOBJ("fighterBody", true));
	modelGrenadierHead.reset(Model::CreateFromOBJ("riflemanHead", true));
	modelGrenadierHat.reset(Model::CreateFromOBJ("riflemanHat", true));
	grenadierBodyTex = TextureManager::Load("./Resources/fighterBody/fighterTorso.png");
	grenadierHeadTex = TextureManager::Load("./Resources/riflemanHead/riflemanHead.png");
	grenadierHatTex = TextureManager::Load("./Resources/riflemanHat/riflemanShako.png");

		// --- Enemy: Officer ---
	// modelOfficerBody.reset(Model::CreateFromOBJ("officerBody", true));
	// modelOfficerHead.reset(Model::CreateFromOBJ("officerHead", true));
	// modelOfficerHat.reset(Model::CreateFromOBJ("officerHat", true));
	// officerBodyTex = TextureManager::Load("./Resources/officerBody/officerTorso.png");
	// officerHeadTex = TextureManager::Load("./Resources/officerHead/officerHead.png");
	// officerHatTex = TextureManager::Load("./Resources/officerHat/officerShako.png");

	modelOfficerBody.reset(Model::CreateFromOBJ("fighterBody", true));
	modelOfficerHead.reset(Model::CreateFromOBJ("riflemanHead", true));
	modelOfficerHat.reset(Model::CreateFromOBJ("riflemanHat", true));
	officerBodyTex = TextureManager::Load("./Resources/fighterBody/fighterTorso.png");
	officerHeadTex = TextureManager::Load("./Resources/riflemanHead/riflemanHead.png");
	officerHatTex = TextureManager::Load("./Resources/riflemanHat/riflemanShako.png");

	// --- Enemy: Sapper (仮: Cuirassierのモデルを流用) ---
	modelSapperBody.reset(Model::CreateFromOBJ("cuirassierBody", true));
	modelSapperHead.reset(Model::CreateFromOBJ("cuirassierHead", true));
	modelSapperHat.reset(Model::CreateFromOBJ("cuirassierHat", true));
	sapperBodyTex = TextureManager::Load("./Resources/cuirassierBody/cuirassierTorso.png");
	sapperHeadTex = TextureManager::Load("./Resources/cuirassierHead/cuirassierHead.png");
	sapperHatTex = TextureManager::Load("./Resources/cuirassierHat/cuirassierHelmet.png");


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
	modelGoal.reset(Model::CreateFromOBJ("blockNormal", true));
	goalTex = TextureManager::Load("./Resources/blockNormal/blockGrass.png");
	modelGate.reset(Model::CreateFromOBJ("blockNormal", true));
	gateTex = TextureManager::Load("./Resources/blockNormal/blockGrass.png");
	modelSwitch.reset(Model::CreateFromOBJ("blockNormal", true));
	switchTex = TextureManager::Load("./Resources/blockNormal/blockGrass.png");
	modelHealthBlock.reset(Model::CreateFromOBJ("blockNormal"));
	healthBlockTex = TextureManager::Load("./Resources/blockNormal/blockGrass.png");
	modelTutorialBlock.reset(Model::CreateFromOBJ("blockNormal", true));

	// --- Gameplay HUD ---
	healthBarBackgroundTex = TextureManager::Load("./Resources/white1x1.png");
	healthBarFillTex = TextureManager::Load("./Resources/white1x1.png");
	enemyIconRiflemanTex = TextureManager::Load("./Resources/UserInterface/iconRifleman.png");
	enemyIconCuirassierTex = TextureManager::Load("./Resources/UserInterface/iconCuirassier.png");
	enemyIconFighterTex = TextureManager::Load("./Resources/UserInterface/iconFighter.png");
	enemyIconCarabinierTex = TextureManager::Load("./Resources/UserInterface/iconCarabinier.png");
	enemyIconGrenadierTex = TextureManager::Load("./Resources/UserInterface/iconGrenadier.png");
	enemyIconOfficerTex = TextureManager::Load("./Resources/UserInterface/iconOfficer.png");
	enemyIconSapperTex = TextureManager::Load("./Resources/UserInterface/iconSapper.png");

	enemyIconArrowTex = TextureManager::Load("./Resources/UserInterface/enemyDirectionArrow.png");

	// Placeholder icons: reuse the weapon textures until real icon art exists.
	// Swap each line for TextureManager::Load("./Resources/UserInterface/iconXxx.png") when the files are ready.
	meleeIconLightTex = TextureManager::Load("./Resources/UserInterface/weaponSabre.png");
	meleeIconHeavyTex = TextureManager::Load("./Resources/UserInterface/weaponSabreHeavy.png");
	meleeIconAxeTex = TextureManager::Load("./Resources/UserInterface/weaponAxe.png");
	meleeIconSpearTex = TextureManager::Load("./Resources/UserInterface/weaponSpontoon.png");

	firearmIconLongTex = TextureManager::Load("./Resources/UserInterface/weaponMusket.png");
	firearmIconBayonetTex = TextureManager::Load("./Resources/UserInterface/weaponMusketBayonet.png");
	firearmIconShotgunTex = TextureManager::Load("./Resources/UserInterface/weaponBlunderbuss.png");
	firearmIconPistolTex = TextureManager::Load("./Resources/UserInterface/weaponPistols.png");
	
	
	bayonetLungeIconTex = TextureManager::Load("./Resources/UserInterface/weaponBayonet.png");
	
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

	// glassTex = loadOr("./Resources/block/glass.png", blockTex);
	// breakableTex[0] = loadOr("./Resources/block/breakable.png", blockTex);
	// breakableTex[1] = loadOr("./Resources/block/breakable_cracked1.png", breakableTex[0]);
	// breakableTex[2] = loadOr("./Resources/block/breakable_cracked2.png", breakableTex[1]);
	// explosiveTex = loadOr("./Resources/block/explosive.png", blockTex);
	// spikeTex = loadOr("./Resources/block/spike.png", blockTex);
	// slowBlockTex = loadOr("./Resources/block/slowBlock.png", blockTexVariants[0]);
	// fragileTex = loadOr("./Resources/block/fragile.png", blockTex);

	glassTex = TextureManager::Load("./Resources/blockNormal/blockGrass.png");
	breakableTex[0] = TextureManager::Load("./Resources/blockNormal/blockGrass.png");
	breakableTex[1] = TextureManager::Load("./Resources/blockNormal/blockGrass.png");
	breakableTex[2] = TextureManager::Load("./Resources/blockNormal/blockGrass.png");
	explosiveTex = TextureManager::Load("./Resources/blockNormal/blockGrass.png");
	spikeTex = TextureManager::Load("./Resources/blockNormal/blockGrass.png");
	slowBlockTex = TextureManager::Load("./Resources/blockNormal/blockGrass.png");
	fragileTex = TextureManager::Load("./Resources/blockNormal/blockGrass.png");
	tutorialBlockTex = TextureManager::Load("./Resources/blockNormal/blockGrass.png");
}

void GameAssets::LoadAudio() {
	Audio* audio = Audio::GetInstance();

	musicIngame = audio->LoadWave("./Resources/sfx_bgm/combatMusic.wav");
	musicArena = audio->LoadWave("./Resources/sfx_bgm/ArenaMusic.wav");

	sfxFirearmFire = audio->LoadWave("./Resources/sfx_bgm/gunShot.wav");
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

	sfxGlassBreak = audio->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	sfxBlockHit = audio->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	sfxBlockBreak = audio->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	sfxExplosion = audio->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	sfxSpikeHit = audio->LoadWave("./Resources/sfx_bgm/defaultSound.wav");

	// Arena SFX (placeholders — swap in real wavs; handle 0 = silent)
	sfxArenaStart = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	sfxWaveStart = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	sfxWaveEnd = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	sfxArenaClear = Audio::GetInstance()->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
}