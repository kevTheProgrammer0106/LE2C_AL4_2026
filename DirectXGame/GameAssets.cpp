#include "GameAssets.h"
#include <algorithm>
#include <initializer_list>

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

	// --- Enemy: Carabinier (placeholder: Cuirassier art) ---
	modelCarabinierBody.reset(Model::CreateFromOBJ("cuirassierBody", true));
	modelCarabinierHead.reset(Model::CreateFromOBJ("cuirassierHead", true));
	modelCarabinierHat.reset(Model::CreateFromOBJ("cuirassierHat", true));
	carabinierBodyTex = TextureManager::Load("./Resources/cuirassierBody/cuirassierTorso.png");
	carabinierHeadTex = TextureManager::Load("./Resources/cuirassierHead/cuirassierHead.png");
	carabinierHatTex = TextureManager::Load("./Resources/cuirassierHat/cuirassierHelmet.png");

	// --- Enemy: Shotgunner (placeholder: Rifleman art) ---
	modelShotgunnerBody.reset(Model::CreateFromOBJ("riflemanBody", true));
	modelShotgunnerHead.reset(Model::CreateFromOBJ("riflemanHead", true));
	modelShotgunnerHat.reset(Model::CreateFromOBJ("riflemanHat", true));
	shotgunnerBodyTex = TextureManager::Load("./Resources/riflemanBody/riflemanTorso.png");
	shotgunnerHeadTex = TextureManager::Load("./Resources/riflemanHead/riflemanHead.png");
	shotgunnerHatTex = TextureManager::Load("./Resources/riflemanHat/riflemanShako.png");

	// --- Enemy: Grenadier (placeholder) ---
	modelGrenadierBody.reset(Model::CreateFromOBJ("fighterBody", true));
	modelGrenadierHead.reset(Model::CreateFromOBJ("riflemanHead", true));
	modelGrenadierHat.reset(Model::CreateFromOBJ("riflemanHat", true));
	grenadierBodyTex = TextureManager::Load("./Resources/fighterBody/fighterTorso.png");
	grenadierHeadTex = TextureManager::Load("./Resources/riflemanHead/riflemanHead.png");
	grenadierHatTex = TextureManager::Load("./Resources/riflemanHat/riflemanShako.png");

	// --- Enemy: Officer (placeholder) ---
	modelOfficerBody.reset(Model::CreateFromOBJ("fighterBody", true));
	modelOfficerHead.reset(Model::CreateFromOBJ("riflemanHead", true));
	modelOfficerHat.reset(Model::CreateFromOBJ("riflemanHat", true));
	officerBodyTex = TextureManager::Load("./Resources/fighterBody/fighterTorso.png");
	officerHeadTex = TextureManager::Load("./Resources/riflemanHead/riflemanHead.png");
	officerHatTex = TextureManager::Load("./Resources/riflemanHat/riflemanShako.png");

	// --- Enemy: Sapper (placeholder: Cuirassier art) ---
	modelSapperBody.reset(Model::CreateFromOBJ("cuirassierBody", true));
	modelSapperHead.reset(Model::CreateFromOBJ("cuirassierHead", true));
	modelSapperHat.reset(Model::CreateFromOBJ("cuirassierHat", true));
	sapperBodyTex = TextureManager::Load("./Resources/cuirassierBody/cuirassierTorso.png");
	sapperHeadTex = TextureManager::Load("./Resources/cuirassierHead/cuirassierHead.png");
	sapperHatTex = TextureManager::Load("./Resources/cuirassierHat/cuirassierHelmet.png");

	// --- Effects ---
	slashEffectTex = TextureManager::Load("./Resources/spriteIngame/attackSlash.png");
	modelSlashEffect.reset(Model::CreateFromOBJ("hit_effect", true));
	hitEffectTex = TextureManager::Load("./Resources/spriteIngame/effectExplosion.png");
	hitEffectModel.reset(Model::CreateFromOBJ("particle"));
	guardEffectTex = TextureManager::Load("./Resources/spriteIngame/effectSpark.png");
	guardEffectModel.reset(Model::CreateFromOBJ("ring"));
	deathParticleTex = TextureManager::Load("./Resources/deathParticle/white1x1.png");
	deathParticleModel.reset(Model::CreateFromOBJ("deathParticle", true));

	// --- Blocks ---
	blockModel.reset(Model::CreateFromOBJ("blockNormal"));
	blockPlatformModel.reset(Model::CreateFromOBJ("blockPlatform"));

	modelBlockTypeOne.reset(Model::CreateFromOBJ("blockTypeOne"));
	modelBlockTypeTwo.reset(Model::CreateFromOBJ("blockTypeTwo"));
	modelBlockTypeThree.reset(Model::CreateFromOBJ("blockTypeThree"));
	modelBlockTypeFour.reset(Model::CreateFromOBJ("blockTypeFour"));
	modelBlockWoodPlanks.reset(Model::CreateFromOBJ("blockWoodPlanks"));
	modelBlockSupport.reset(Model::CreateFromOBJ("blockWoodSupport"));

	blockGrassTex = TextureManager::Load("./Resources/blockTypeOne/blockGrass.png");
	blockDirtTex = TextureManager::Load("./Resources/blockTypeOne/blockDirt.png");
	blockStoneTex = TextureManager::Load("./Resources/blockTypeOne/blockStone.png");
	blockCobblestoneTex = TextureManager::Load("./Resources/blockTypeOne/blockCobblestone.png");
	blockLeavesTex = TextureManager::Load("./Resources/blockTypeOne/blockLeaves.png");
	blockSandTex = TextureManager::Load("./Resources/blockTypeOne/blockSand.png");

	blockCrateTex = TextureManager::Load("./Resources/blockTypeTwo/blockCrate.png");
	blockIronTex = TextureManager::Load("./Resources/blockTypeTwo/blockIron.png");

	blockShinglesTex = TextureManager::Load("./Resources/blockTypeThree/blockShingles.png");
	blockThatchTex = TextureManager::Load("./Resources/blockTypeThree/blockThatch.png");
	blockTilesTex = TextureManager::Load("./Resources/blockTypeThree/blockTiles.png");

	blockWoodTex = TextureManager::Load("./Resources/blockTypeFour/blockWood.png");
	blockWoodPlanksTex = TextureManager::Load("./Resources/blockWoodPlanks/blockWoodPlanks.png");
	blockSupportTex = TextureManager::Load("./Resources/blockWoodSupport/blockWoodSupport.png");

	blockPlatformLeavesTex = TextureManager::Load("./Resources/blockPlatform/blockPlatformLeaves.png");
	blockPlatformDirtTex = TextureManager::Load("./Resources/blockPlatform/blockPlatformDirt.png");
	blockPlatformWoodTex = TextureManager::Load("./Resources/blockPlatform/blockPlatformWood.png");
	blockPlatformIronTex = TextureManager::Load("./Resources/blockPlatform/blockPlatformIron.png");

	blockMudTex = TextureManager::Load("./Resources/blockSlow/blockMud.png");
	blockGravelTex = TextureManager::Load("./Resources/blockSlow/blockGravel.png");
	blockHeavySandTex = TextureManager::Load("./Resources/blockSlow/blockSandHeavy.png");

	blockSpikesTex = TextureManager::Load("./Resources/blockSpikes/blockSpikes.png");
	blockGateTex = TextureManager::Load("./Resources/blockGate/blockGate.png");
	blockGlassTex = TextureManager::Load("./Resources/blockGlass/blockGlass.png");
	blockBarrelTex = TextureManager::Load("./Resources/blockBarrel/blockBarrel.png");
	blockHealthTex = TextureManager::Load("./Resources/blockHealth/blockHealth.png");

	// placeholders until destructible art exists
	breakableTex[0] = breakableTex[1] = breakableTex[2] = blockCrateTex;
	fragileTex = blockWoodPlanksTex;

	modelTutorialBlock.reset(Model::CreateFromOBJ("blockSign", true));
	tutorialBlockTex = TextureManager::Load("./Resources/blockSign/blockSign.png");

	// --- Goal / Gate / HealthBlock ---
	modelGoal.reset(Model::CreateFromOBJ("blockGoal", true));
	goalTex = TextureManager::Load("./Resources/blockGoal/blockGoal.png");
	modelGate.reset(Model::CreateFromOBJ("blockGate", true));
	modelHealthBlock.reset(Model::CreateFromOBJ("blockHealth"));

	// --- Lever (handle + support) ---
	modelLever.reset(Model::CreateFromOBJ("blockLever"));
	modelLeverSupport.reset(Model::CreateFromOBJ("blockLeverSupport"));
	blockLeverTex = TextureManager::Load("./Resources/blockLever/blockLever.png");
	blockLeverSupportTex = TextureManager::Load("./Resources/blockLeverSupport/blockLeverSupport.png");

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
	cursorTex[0] = TextureManager::Load("./Resources/UserInterface/iconDirectionCrosshair.png");
	cursorTex[1] = TextureManager::Load("./Resources/UserInterface/iconCursor.png");
	// --- Banner(2048x512) ---
	bannerTex[0] = TextureManager::Load("./Resources/UserInterface/bannerObjective.png");
	bannerTex[1] = TextureManager::Load("./Resources/UserInterface/bannerArena.png");
	bannerTex[2] = TextureManager::Load("./Resources/UserInterface/bannerPaused.png");
	bannerTex[3] = TextureManager::Load("./Resources/UserInterface/bannerDefeat.png");
	bannerTex[4] = TextureManager::Load("./Resources/UserInterface/bannerVictory.png");
	bannerTex[5] = TextureManager::Load("./Resources/UserInterface/bannerBuffed.png");
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

	// Arena SFX (placeholders; handle 0 = silent)
	sfxArenaStart = audio->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	sfxWaveStart = audio->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	sfxWaveEnd = audio->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
	sfxArenaClear = audio->LoadWave("./Resources/sfx_bgm/defaultSound.wav");
}

// B / T / M chips -> model + texture. Out-of-range subID falls back to the last entry of the group.
TileVisual GameAssets::ResolveTile(MapChipType type, uint8_t subID, uint8_t linkID) const {
	auto pick = [subID](std::initializer_list<uint32_t> list) {
		const size_t i = std::min<size_t>(subID, list.size() - 1);
		return *(list.begin() + i);
	};

	switch (type) {
	case MapChipType::kBlock:
		switch (linkID) {
		default:
		case 0:
			return {modelBlockTypeOne.get(), pick({blockGrassTex, blockDirtTex, blockStoneTex, blockCobblestoneTex, blockLeavesTex, blockSandTex})};
		case 1:
			return {modelBlockTypeTwo.get(), pick({blockCrateTex, blockIronTex})};
		case 2:
			return {modelBlockTypeThree.get(), pick({blockShinglesTex, blockThatchTex, blockTilesTex})};
		case 3:
			return {modelBlockTypeFour.get(), blockWoodTex};
		case 4:
			return {modelBlockWoodPlanks.get(), blockWoodPlanksTex};
		case 5:
			return {modelBlockSupport.get(), blockSupportTex};
		}
	case MapChipType::kPlatform:
		return {blockPlatformModel.get(), pick({blockPlatformLeavesTex, blockPlatformDirtTex, blockPlatformWoodTex, blockPlatformIronTex})};
	case MapChipType::kSlowBlock:
		return {blockModel.get(), pick({blockMudTex, blockGravelTex, blockHeavySandTex})};
	default:
		return {blockModel.get(), blockGrassTex};
	}
}