#include "GameAssets.h"
#include <algorithm>
#include <fstream>
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

	modelBackgroundHill.reset(Model::CreateFromOBJ("backgroundPlain"));
	modelBackgroundTown.reset(Model::CreateFromOBJ("backgroundTown"));
	modelBackgroundTrees.reset(Model::CreateFromOBJ("backgroundTrees"));
	hillTex = TextureManager::Load("./Resources/backgroundPlain/grassTexture.png");
	townTex = TextureManager::Load("./Resources/backgroundTown/grayTex.png");
	treesTex = TextureManager::Load("./Resources/backgroundTrees/trees.png");

	// --- Player ---
	modelPlayerHead.reset(Model::CreateFromOBJ("playerhead", true));
	modelPlayerHat.reset(Model::CreateFromOBJ("playerhat", true));
	modelPlayerBody.reset(Model::CreateFromOBJ("playerbody", true));
	playerBodyTex = TextureManager::Load("./Resources/playerBody/playerBody.png");
	playerHeadTex = TextureManager::Load("./Resources/playerHead/playerHead.png");
	playerHatTex = TextureManager::Load("./Resources/playerHat/playerShako.png");

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
	modelCarabinierBody.reset(Model::CreateFromOBJ("cuirassierBody", true));
	modelCarabinierHead.reset(Model::CreateFromOBJ("cuirassierHead", true));
	modelCarabinierHat.reset(Model::CreateFromOBJ("carabinierHat", true));
	carabinierBodyTex = TextureManager::Load("./Resources/cuirassierBody/cuirassierTorso.png");
	carabinierHeadTex = TextureManager::Load("./Resources/cuirassierHead/cuirassierHead.png");
	carabinierHatTex = TextureManager::Load("./Resources/carabinierHat/carabinierHat.png");

	// --- Enemy: Shotgunner  ---
	modelShotgunnerBody.reset(Model::CreateFromOBJ("fighterBody", true));
	modelShotgunnerHead.reset(Model::CreateFromOBJ("fighterHead", true));
	modelShotgunnerHat.reset(Model::CreateFromOBJ("shotgunnerHat", true));
	shotgunnerBodyTex = TextureManager::Load("./Resources/fighterBody/fighterTorso.png");
	shotgunnerHeadTex = TextureManager::Load("./Resources/fighterHead/fighterHead.png");
	shotgunnerHatTex = TextureManager::Load("./Resources/shotgunnerHat/shotgunnerHat.png");

	// --- Enemy: Grenadier ---
	modelGrenadierBody.reset(Model::CreateFromOBJ("officerBody", true));
	modelGrenadierHead.reset(Model::CreateFromOBJ("riflemanHead", true));
	modelGrenadierHat.reset(Model::CreateFromOBJ("grenadierHat", true));
	grenadierBodyTex = TextureManager::Load("./Resources/officerBody/grenadierTorso.png");
	grenadierHeadTex = TextureManager::Load("./Resources/riflemanHead/riflemanHead.png");
	grenadierHatTex = TextureManager::Load("./Resources/grenadierHat/grenadierHat.png");

	// --- Enemy: Officer ---
	modelOfficerBody.reset(Model::CreateFromOBJ("officerBody", true));
	modelOfficerHead.reset(Model::CreateFromOBJ("officerHead", true));
	modelOfficerHat.reset(Model::CreateFromOBJ("officeHat", true));
	officerBodyTex = TextureManager::Load("./Resources/officerBody/grenadierTorso.png");
	officerHeadTex = TextureManager::Load("./Resources/officerHead/riflemanHead.png");
	officerHatTex = TextureManager::Load("./Resources/officeHat/officerHat.png");

	// --- Enemy: Sapper ---
	modelSapperBody.reset(Model::CreateFromOBJ("sapperBody", true));
	modelSapperHead.reset(Model::CreateFromOBJ("sapperHead", true));
	modelSapperHat.reset(Model::CreateFromOBJ("sapperHat", true));
	sapperBodyTex = TextureManager::Load("./Resources/sapperBody/sapperBody.png");
	sapperHeadTex = TextureManager::Load("./Resources/sapperHead/riflemanHead.png");
	sapperHatTex = TextureManager::Load("./Resources/sapperHat/sapperHat.png");

	// --- Effects (Resources/spriteIngame) ---
	const auto fx = [](const char* file) { return TextureManager::Load(std::string("./Resources/spriteIngame/") + file); };

	modelSlashEffect.reset(Model::CreateFromOBJ("attackQuad", true));
	attackEffectTextures.Set(AttackEffectType::kSlash, fx("attackSlash.png"));
	attackEffectTextures.Set(AttackEffectType::kSlashOpposite, fx("attackSlashOpposite.png"));
	attackEffectTextures.Set(AttackEffectType::kSlashDownward, fx("attackSlashDownward.png"));
	attackEffectTextures.Set(AttackEffectType::kSlashAlt, fx("attackSlashAlt.png"));
	attackEffectTextures.Set(AttackEffectType::kThrust, fx("attackThrust.png"));
	attackEffectTextures.Set(AttackEffectType::kSmash, fx("attackSmash.png"));

	effectExplosionTex = fx("effectExplosion.png");
	effectFlashTex = fx("effectFlash.png");
	effectFragmentsTex = fx("effectFragments.png");
	effectSmokeTex = fx("effectSmoke.png");
	effectSparkTex = fx("effectSpark.png");

	hitEffectModel.reset(Model::CreateFromOBJ("quad"));
	guardEffectModel.reset(Model::CreateFromOBJ("quad"));
	deathParticleModel.reset(Model::CreateFromOBJ("quad", true));

		// --- Individual block models: B ---
	// OBJファイル名とテクスチャファイル名を同じ名前にする。
	modelBlockGrass.reset(Model::CreateFromOBJ("blockGrass"));
	blockGrassTex = TextureManager::Load("./Resources/blockGrass/blockGrass.png");

	modelBlockDirt.reset(Model::CreateFromOBJ("blockDirt"));
	blockDirtTex = TextureManager::Load("./Resources/blockDirt/blockDirt.png");

	modelBlockStone.reset(Model::CreateFromOBJ("blockStone"));
	blockStoneTex = TextureManager::Load("./Resources/blockStone/blockStone.png");

	modelBlockCobblestone.reset(Model::CreateFromOBJ("blockCobblestone"));
	blockCobblestoneTex = TextureManager::Load("./Resources/blockCobblestone/blockCobblestone.png");

	modelBlockLeaves.reset(Model::CreateFromOBJ("blockLeaves"));
	blockLeavesTex = TextureManager::Load("./Resources/blockLeaves/blockLeaves.png");

	modelBlockCrate.reset(Model::CreateFromOBJ("blockCrate"));
	blockCrateTex = TextureManager::Load("./Resources/blockCrate/blockCrate.png");

	modelBlockIron.reset(Model::CreateFromOBJ("blockIron"));
	blockIronTex = TextureManager::Load("./Resources/blockIron/blockIron.png");

	modelBlockWoodPlanks.reset(Model::CreateFromOBJ("blockWoodPlanks"));
	blockWoodPlanksTex = TextureManager::Load("./Resources/blockWoodPlanks/blockWoodPlanks.png");

	modelBlockWoodSupport.reset(Model::CreateFromOBJ("blockWoodSupport"));
	blockWoodSupportTex = TextureManager::Load("./Resources/blockWoodSupport/blockWoodSupport.png");

	// --- Individual platform models: T ---
	modelBlockPlatformLeaves.reset(Model::CreateFromOBJ("blockPlatformLeaves"));
	blockPlatformLeavesTex = TextureManager::Load("./Resources/blockPlatformLeaves/blockPlatformLeaves.png");

	modelBlockPlatformDirt.reset(Model::CreateFromOBJ("blockPlatformDirt"));
	blockPlatformDirtTex = TextureManager::Load("./Resources/blockPlatformDirt/blockPlatformDirt.png");

	modelBlockPlatformWood.reset(Model::CreateFromOBJ("blockPlatformWood"));
	blockPlatformWoodTex = TextureManager::Load("./Resources/blockPlatformWood/blockPlatformWood.png");

	modelBlockPlatformIron.reset(Model::CreateFromOBJ("blockPlatformIron"));
	blockPlatformIronTex = TextureManager::Load("./Resources/blockPlatformIron/blockPlatformIron.png");

	// --- Individual slow-block models: M ---
	modelBlockSlowMud.reset(Model::CreateFromOBJ("blockSlowMud"));
	blockSlowMudTex = TextureManager::Load("./Resources/blockSlowMud/blockSlowMud.png");

	modelBlockSlowGravel.reset(Model::CreateFromOBJ("blockSlowGravel"));
	blockSlowGravelTex = TextureManager::Load("./Resources/blockSlowGravel/blockSlowGravel.png");

	// ===== Single-texture chips =====
	blockSpikesTex = TextureManager::Load("./Resources/blockSpikes/blockSpikes.png"); // 'I' spike
	blockGateTex = TextureManager::Load("./Resources/blockGate/blockGate.png");       // '1'-'8' gates
	blockGlassTex = TextureManager::Load("./Resources/blockGlass/blockGlass.png");    // 'Y' glass
	blockBarrelTex = TextureManager::Load("./Resources/blockBarrel/blockBarrel.png"); // 'Z' explosive
	blockHealthTex = TextureManager::Load("./Resources/blockHealth/blockHealth.png"); // 'H' health block

	// Special blocks: separate Model instances. Swap the OBJ names when you have dedicated art
	// (e.g. "blockGlass", "blockBreakable", "blockBarrel", "blockSpikes", "blockFragile").
	modelGlass.reset(Model::CreateFromOBJ("blockGlass"));
	modelBreakable.reset(Model::CreateFromOBJ("blockCrate"));
	modelExplosive.reset(Model::CreateFromOBJ("blockBarrel"));
	modelSpike.reset(Model::CreateFromOBJ("blockSpikes"));
	modelFragile.reset(Model::CreateFromOBJ("blockLeaves"));

	// ===== 'V' breakable / '9' fragile (placeholders: crate / wood planks) =====
	breakableTex[0] = breakableTex[1] = breakableTex[2] = blockCrateTex; // 'V'
	fragileTex = blockLeavesTex; // '9'

	// ===== 'U' tutorial sign =====
	modelTutorialBlock.reset(Model::CreateFromOBJ("blockSign", true));
	tutorialBlockTex = TextureManager::Load("./Resources/blockSign/blockSign.png");

	// ===== 'X' goal =====
	modelGoal.reset(Model::CreateFromOBJ("blockGoal", true));
	goalTex = TextureManager::Load("./Resources/blockGoal/blockGoal.png");

	// ===== '1'-'8' gates (texture = blockGateTex) =====
	modelGate.reset(Model::CreateFromOBJ("blockGate", true));

	// ===== 'H' health block (texture = blockHealthTex) =====
	modelHealthBlock.reset(Model::CreateFromOBJ("blockHealth"));

	// ===== 'W' switch: lever handle + support base =====
	modelLever.reset(Model::CreateFromOBJ("blockLever"));
	modelLeverSupport.reset(Model::CreateFromOBJ("blockLeverSupport"));
	blockLeverTex = TextureManager::Load("./Resources/blockLever/blockLever.png");
	blockLeverSupportTex = TextureManager::Load("./Resources/blockLeverSupport/blockLeverSupport.png");

	// Special blocks: separate Model instances. Swap the OBJ names when you have dedicated art
	// (e.g. "blockGlass", "blockBreakable", "blockBarrel", "blockSpikes", "blockFragile").
	modelGlass.reset(Model::CreateFromOBJ("blockGlass"));
	modelBreakable.reset(Model::CreateFromOBJ("blockTypeTwo"));
	modelExplosive.reset(Model::CreateFromOBJ("blockBarrel"));
	modelSpike.reset(Model::CreateFromOBJ("blockSpikes"));
	modelFragile.reset(Model::CreateFromOBJ("blockTypeOne"));

	// ===== 'V' breakable / '9' fragile (placeholders: crate / wood planks) =====
	breakableTex[0] = breakableTex[1] = breakableTex[2] = blockCrateTex; // 'V'
	fragileTex = blockLeavesTex;                                         // '9'

	// ===== 'U' tutorial sign =====
	modelTutorialBlock.reset(Model::CreateFromOBJ("blockSign", true));
	tutorialBlockTex = TextureManager::Load("./Resources/blockSign/blockSign.png");

	// ===== 'X' goal =====
	modelGoal.reset(Model::CreateFromOBJ("blockGoal", true));
	goalTex = TextureManager::Load("./Resources/blockGoal/blockGoal.png");

	// ===== '1'-'8' gates (texture = blockGateTex) =====
	modelGate.reset(Model::CreateFromOBJ("blockGate", true));

	// ===== 'H' health block (texture = blockHealthTex) =====
	modelHealthBlock.reset(Model::CreateFromOBJ("blockHealth"));

	// ===== 'W' switch: lever handle + support base =====
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

	// Next-level button. Uses buttonNext.png if you have drawn it, otherwise the Play icon.
	{
		const char* nextPath = "./Resources/UserInterface/buttonPlay.png";
		victoryButtonTex[0] = TextureManager::Load(std::ifstream(nextPath).good() ? nextPath : "./Resources/UserInterface/buttonPlay.png");
	}
	victoryButtonTex[1] = TextureManager::Load("./Resources/UserInterface/buttonRestart.png");
	victoryButtonTex[2] = TextureManager::Load("./Resources/UserInterface/buttonBack.png");

	inGamePauseButtonTex = TextureManager::Load("./Resources/UserInterface/buttonIngamePause.png");
	cursorTex[0] = TextureManager::Load("./Resources/UserInterface/iconDirectionCrosshair.png");
	cursorTex[1] = TextureManager::Load("./Resources/UserInterface/iconCursor.png");
	screenFlashTex = TextureManager::Load("./Resources/UserInterface/flashScreen.png");
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

// CSVのlinkIDから個別のブロックモデルを選択する。
Model* GameAssets::ResolveTileModel(MapChipType type, uint8_t linkID) const {
	switch (type) {
	case MapChipType::kBlock: // 'B'
		switch (linkID) {
		case 0:
			return modelBlockGrass.get();
		case 1:
			return modelBlockDirt.get();
		case 2:
			return modelBlockStone.get();
		case 3:
			return modelBlockCobblestone.get();
		case 4:
			return modelBlockLeaves.get();
		case 5:
			return modelBlockCrate.get();
		case 6:
			return modelBlockIron.get();
		case 7:
			return modelBlockWoodPlanks.get();
		case 8:
			return modelBlockWoodSupport.get();
		default:
			return nullptr;
		}

	case MapChipType::kPlatform: // 'T'
		switch (linkID) {
		case 0:
			return modelBlockPlatformLeaves.get();
		case 1:
			return modelBlockPlatformDirt.get();
		case 2:
			return modelBlockPlatformWood.get();
		case 3:
			return modelBlockPlatformIron.get();
		default:
			return nullptr;
		}

	case MapChipType::kSlowBlock: // 'M'
		switch (linkID) {
		case 0:
			return modelBlockSlowMud.get();
		case 1:
			return modelBlockSlowGravel.get();
		default:
			return nullptr;
		}

	default:
		return nullptr;
	}
}

// CSVのlinkIDから個別のブロックテクスチャを選択する。
uint32_t GameAssets::ResolveTileTexture(MapChipType type, uint8_t linkID) const {
	switch (type) {
	case MapChipType::kBlock: // 'B'
		switch (linkID) {
		case 0:
			return blockGrassTex;
		case 1:
			return blockDirtTex;
		case 2:
			return blockStoneTex;
		case 3:
			return blockCobblestoneTex;
		case 4:
			return blockLeavesTex;
		case 5:
			return blockCrateTex;
		case 6:
			return blockIronTex;
		case 7:
			return blockWoodPlanksTex;
		case 8:
			return blockWoodSupportTex;
		default:
			return 0;
		}

	case MapChipType::kPlatform: // 'T'
		switch (linkID) {
		case 0:
			return blockPlatformLeavesTex;
		case 1:
			return blockPlatformDirtTex;
		case 2:
			return blockPlatformWoodTex;
		case 3:
			return blockPlatformIronTex;
		default:
			return 0;
		}

	case MapChipType::kSlowBlock: // 'M'
		switch (linkID) {
		case 0:
			return blockSlowMudTex;
		case 1:
			return blockSlowGravelTex;
		default:
			return 0;
		}

	default:
		return 0;
	}
}

// special blocks only: Y, V, Z, I, 9. stage is used by 'V' (0..2 damage stage)
TileVisual GameAssets::ResolveSpecial(MapChipType type, int stage) const {
	switch (type) {
	case MapChipType::kGlass:
		return {modelGlass.get(), blockGlassTex};
	case MapChipType::kBreakable:
		return {modelBreakable.get(), breakableTex[std::clamp(stage, 0, 2)]};
	case MapChipType::kExplosive:
		return {modelExplosive.get(), blockBarrelTex};
	case MapChipType::kSpike:
		return {modelSpike.get(), blockSpikesTex};
	case MapChipType::kFragile:
		return {modelFragile.get(), fragileTex};
	default:
		return {nullptr, 0};
	}
}