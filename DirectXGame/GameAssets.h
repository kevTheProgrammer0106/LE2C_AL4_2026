#pragma once

#include "KamataEngine.h"
#include "MapChipField.h"
#include <cstdint>
#include <memory>

/// 1つのタイルを描くためのモデル+テクスチャ
struct TileVisual {
	KamataEngine::Model* model = nullptr;
	uint32_t tex = 0;
};

/// Owns every model / texture / audio handle the gameplay scene uses.
/// GameScene calls Load() once, then reads the public members. Audio handles are only stored here;
/// GameScene::BindAssets() hands them to the classes that play them.
class GameAssets {
public:
	void Load();

	// --- Skydome ---
	std::unique_ptr<KamataEngine::Model> modelSkyDome;
	uint32_t skyDomeTex = 0;

	// --- Player ---
	std::unique_ptr<KamataEngine::Model> modelPlayerBody;
	std::unique_ptr<KamataEngine::Model> modelPlayerHead;
	std::unique_ptr<KamataEngine::Model> modelPlayerHat;
	uint32_t playerBodyTex = 0;
	uint32_t playerHeadTex = 0;
	uint32_t playerHatTex = 0;

	// --- Weapons ---
	std::unique_ptr<KamataEngine::Model> modelSabreLight;
	uint32_t texSabreLight = 0;
	std::unique_ptr<KamataEngine::Model> modelSabreHeavy;
	uint32_t texSabreHeavy = 0;
	std::unique_ptr<KamataEngine::Model> modelAxe;
	uint32_t texAxe = 0;
	std::unique_ptr<KamataEngine::Model> modelSpear;
	uint32_t texSpear = 0;
	std::unique_ptr<KamataEngine::Model> modelMusketLong;
	uint32_t musketLongTex = 0;
	std::unique_ptr<KamataEngine::Model> modelMusketBayonet;
	uint32_t musketBayonetTex = 0;
	std::unique_ptr<KamataEngine::Model> modelShotgun;
	uint32_t shotgunTex = 0;
	std::unique_ptr<KamataEngine::Model> modelPistol;
	uint32_t pistolTex = 0;

	std::unique_ptr<KamataEngine::Model> modelProjectileBall;
	uint32_t firearmBallTex = 0;

	// --- Enemy: Rifleman ---
	std::unique_ptr<KamataEngine::Model> modelRiflemanBody;
	std::unique_ptr<KamataEngine::Model> modelRiflemanHead;
	std::unique_ptr<KamataEngine::Model> modelRiflemanHat;
	uint32_t riflemanBodyTex = 0;
	uint32_t riflemanHeadTex = 0;
	uint32_t riflemanHatTex = 0;

	// --- Enemy: Cuirassier ---
	std::unique_ptr<KamataEngine::Model> modelCuirassierBody;
	std::unique_ptr<KamataEngine::Model> modelCuirassierHead;
	std::unique_ptr<KamataEngine::Model> modelCuirassierHat;
	uint32_t cuirassierBodyTex = 0;
	uint32_t cuirassierHeadTex = 0;
	uint32_t cuirassierHatTex = 0;

	// --- Enemy: Fighter ---
	std::unique_ptr<KamataEngine::Model> modelFighterBody;
	std::unique_ptr<KamataEngine::Model> modelFighterHead;
	std::unique_ptr<KamataEngine::Model> modelFighterHat;
	uint32_t fighterBodyTex = 0;
	uint32_t fighterHeadTex = 0;
	uint32_t fighterHatTex = 0;

	// --- Enemy: Carabinier ---
	std::unique_ptr<KamataEngine::Model> modelCarabinierBody;
	std::unique_ptr<KamataEngine::Model> modelCarabinierHead;
	std::unique_ptr<KamataEngine::Model> modelCarabinierHat;
	uint32_t carabinierBodyTex = 0;
	uint32_t carabinierHeadTex = 0;
	uint32_t carabinierHatTex = 0;

	// --- Enemy: Shotgunner ---
	std::unique_ptr<KamataEngine::Model> modelShotgunnerBody;
	std::unique_ptr<KamataEngine::Model> modelShotgunnerHead;
	std::unique_ptr<KamataEngine::Model> modelShotgunnerHat;
	uint32_t shotgunnerBodyTex = 0;
	uint32_t shotgunnerHeadTex = 0;
	uint32_t shotgunnerHatTex = 0;

	// --- Enemy: Grenadier ---
	std::unique_ptr<KamataEngine::Model> modelGrenadierBody;
	std::unique_ptr<KamataEngine::Model> modelGrenadierHead;
	std::unique_ptr<KamataEngine::Model> modelGrenadierHat;
	uint32_t grenadierBodyTex = 0;
	uint32_t grenadierHeadTex = 0;
	uint32_t grenadierHatTex = 0;

	// --- Enemy: Sapper ---
	std::unique_ptr<KamataEngine::Model> modelSapperBody;
	std::unique_ptr<KamataEngine::Model> modelSapperHead;
	std::unique_ptr<KamataEngine::Model> modelSapperHat;
	uint32_t sapperBodyTex = 0;
	uint32_t sapperHeadTex = 0;
	uint32_t sapperHatTex = 0;

	// --- Enemy: Officer ---
	std::unique_ptr<KamataEngine::Model> modelOfficerBody;
	std::unique_ptr<KamataEngine::Model> modelOfficerHead;
	std::unique_ptr<KamataEngine::Model> modelOfficerHat;
	uint32_t officerBodyTex = 0;
	uint32_t officerHeadTex = 0;
	uint32_t officerHatTex = 0;

	// --- Effects ---
	std::unique_ptr<KamataEngine::Model> modelSlashEffect;
	uint32_t slashEffectTex = 0;
	std::unique_ptr<KamataEngine::Model> hitEffectModel;
	uint32_t hitEffectTex = 0;
	std::unique_ptr<KamataEngine::Model> guardEffectModel;
	uint32_t guardEffectTex = 0;
	std::unique_ptr<KamataEngine::Model> deathParticleModel;
	uint32_t deathParticleTex = 0;

	// --- Blocks ---
	std::unique_ptr<KamataEngine::Model> blockModel; // generic cube: slow, glass, spikes, barrel, breakable, fragile
	std::unique_ptr<KamataEngine::Model> blockPlatformModel;

	// mapChip B models, picked by linkID
	std::unique_ptr<KamataEngine::Model> modelBlockTypeOne;    // linkID 0
	std::unique_ptr<KamataEngine::Model> modelBlockTypeTwo;    // linkID 1
	std::unique_ptr<KamataEngine::Model> modelBlockTypeThree;  // linkID 2
	std::unique_ptr<KamataEngine::Model> modelBlockTypeFour;   // linkID 3
	std::unique_ptr<KamataEngine::Model> modelBlockWoodPlanks; // linkID 4
	std::unique_ptr<KamataEngine::Model> modelBlockSupport;    // linkID 5
	std::unique_ptr<KamataEngine::Model> modelBlockTest;       // linkID 6

	// TypeOne (subID 0-5)
	uint32_t blockGrassTex = 0, blockDirtTex = 0, blockStoneTex = 0, blockCobblestoneTex = 0, blockLeavesTex = 0, blockSandTex = 0;
	// TypeTwo (subID 0-1)
	uint32_t blockCrateTex = 0, blockIronTex = 0;
	// TypeThree (subID 0-2)
	uint32_t blockShinglesTex = 0, blockThatchTex = 0, blockTilesTex = 0;
	// TypeFour / WoodPlanks / Support / Test
	uint32_t blockWoodTex = 0, blockWoodPlanksTex = 0, blockSupportTex = 0, blockTestTex = 0;

	// platform (subID 0-3)
	uint32_t blockPlatformLeavesTex = 0, blockPlatformDirtTex = 0, blockPlatformWoodTex = 0, blockPlatformIronTex = 0;
	// slow block (subID 0-2)
	uint32_t blockMudTex = 0, blockGravelTex = 0, blockHeavySandTex = 0;

	// same mapChips as before, renamed
	uint32_t blockSpikesTex = 0, blockGateTex = 0, blockGlassTex = 0, blockBarrelTex = 0, blockHealthTex = 0;

	uint32_t breakableTex[3] = {0, 0, 0}; // intact, cracked, nearly broken (placeholder: all the same)
	uint32_t fragileTex = 0;

	// B / T / M chips -> model + texture
	TileVisual ResolveTile(MapChipType type, uint8_t subID, uint8_t linkID) const;

	std::unique_ptr<KamataEngine::Model> modelTutorialBlock;
	uint32_t tutorialBlockTex = 0;

	// --- Goal / Gate / HealthBlock ---
	std::unique_ptr<KamataEngine::Model> modelGoal;
	uint32_t goalTex = 0;
	std::unique_ptr<KamataEngine::Model> modelGate;
	std::unique_ptr<KamataEngine::Model> modelHealthBlock;

	// --- Lever system (handle + support base) ---
	std::unique_ptr<KamataEngine::Model> modelLever;
	std::unique_ptr<KamataEngine::Model> modelLeverSupport;
	uint32_t blockLeverTex = 0;
	uint32_t blockLeverSupportTex = 0;

	// --- Gameplay HUD ---
	uint32_t enemyIconRiflemanTex = 0;
	uint32_t enemyIconCuirassierTex = 0;
	uint32_t enemyIconFighterTex = 0;
	uint32_t enemyIconCarabinierTex = 0;
	uint32_t enemyIconGrenadierTex = 0;
	uint32_t enemyIconOfficerTex = 0;
	uint32_t enemyIconSapperTex = 0;

	uint32_t enemyIconArrowTex = 0;
	uint32_t healthBarBackgroundTex = 0;
	uint32_t healthBarFillTex = 0;

	uint32_t meleeIconLightTex = 0;
	uint32_t meleeIconHeavyTex = 0;
	uint32_t meleeIconAxeTex = 0;
	uint32_t meleeIconSpearTex = 0;

	uint32_t firearmIconLongTex = 0;
	uint32_t firearmIconBayonetTex = 0;
	uint32_t firearmIconShotgunTex = 0;
	uint32_t firearmIconPistolTex = 0;

	uint32_t bayonetLungeIconTex = 0;

	uint32_t interactionPromptTex = 0;

	// --- Menus / cursor ---
	uint32_t pauseButtonTex[3] = {0, 0, 0}; // play, retry, back
	uint32_t retryButtonTex[3] = {0, 0, 0}; // retry, restart, back
	uint32_t victoryButtonTex[2] = {0, 0};  // restart, back
	uint32_t inGamePauseButtonTex = 0;
	uint32_t cursorTex[2] = {0, 0}; // [0] aim (play), [1] arrow (menus)
	// --- Banner(画面上部の文字)。順番は BannerUI::Kind: start, arena, paused, defeat, victory ---
	uint32_t bannerTex[6] = {0, 0, 0, 0, 0, 0};

	// --- Audio: music ---
	uint32_t musicIngame = 0;
	uint32_t musicArena = 0;

	// --- Audio: weapons ---
	uint32_t sfxFirearmFire = 0;
	uint32_t sfxMusketLoaded = 0;
	uint32_t sfxMeleeSwingDefault = 0;
	uint32_t sfxSwingRight = 0;
	uint32_t sfxSwingLeft = 0;
	uint32_t sfxSwingDownward = 0;
	uint32_t sfxBayonetLunge = 0;

	// --- Audio: player ---
	uint32_t sfxPlayerHurt = 0;
	uint32_t sfxPlayerDeath = 0;
	uint32_t sfxPlayerBlockStart = 0;
	uint32_t sfxPlayerBlockImpact = 0;
	uint32_t sfxWeaponSwitch = 0;

	// --- Audio: enemies ---
	uint32_t sfxEnemyDeath[3] = {0, 0, 0};
	uint32_t sfxCuirassierSwing = 0;
	uint32_t sfxCuirassierLunge = 0;
	uint32_t sfxCuirassierMove = 0;
	uint32_t sfxCuirassierHurt = 0;
	uint32_t sfxCuirassierDeath[3] = {0, 0, 0};

	// --- Audio: movement / world ---
	uint32_t sfxFootstep = 0;
	uint32_t sfxJump = 0;
	uint32_t sfxLand = 0;
	uint32_t sfxHealthRestore = 0;
	uint32_t sfxSwitchPress = 0;
	uint32_t sfxGateOpen = 0;
	uint32_t sfxGoalReached = 0;
	uint32_t sfxItemPickup = 0; // not loaded anywhere yet; set it here when you have a file
	uint32_t sfxGlassBreak = 0;
	uint32_t sfxBlockHit = 0;
	uint32_t sfxBlockBreak = 0;
	uint32_t sfxExplosion = 0;
	uint32_t sfxSpikeHit = 0;

	// --- Audio: arena / wave ---
	uint32_t sfxArenaStart = 0;
	uint32_t sfxWaveStart = 0;
	uint32_t sfxWaveEnd = 0;
	uint32_t sfxArenaClear = 0;

private:
	void LoadModelsAndTextures();
	void LoadAudio();
};