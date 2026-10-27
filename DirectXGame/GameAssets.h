#pragma once

#include "Global.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include <cstdint>
#include <memory>

/// <summary>
/// 1つのタイルを描くためのモデル+テクスチャ
/// </summary>
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

	// --- Background Models --
	std::unique_ptr<KamataEngine::Model> modelBackgroundHill;
	std::unique_ptr<KamataEngine::Model> modelBackgroundTown;
	std::unique_ptr<KamataEngine::Model> modelBackgroundTrees;
	uint32_t hillTex = 0;
	uint32_t townTex = 0;
	uint32_t treesTex = 0;

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

	// --- Effects (Resources/spriteIngame) ---
	std::unique_ptr<KamataEngine::Model> modelSlashEffect; // quad used by every attack effect
	AttackEffectTextures attackEffectTextures;
	std::unique_ptr<KamataEngine::Model> hitEffectModel;   // "particle" quad (bursts / flash / muzzle flash / hit)
	std::unique_ptr<KamataEngine::Model> guardEffectModel; // ring
	uint32_t effectExplosionTex = 0;
	uint32_t effectFlashTex = 0;
	uint32_t effectFragmentsTex = 0;
	uint32_t effectSmokeTex = 0;
	uint32_t effectSparkTex = 0;
	std::unique_ptr<KamataEngine::Model> deathParticleModel;
	uint32_t deathParticleTex = 0;

	// --- 'B' blocks: each block has its OWN model and texture ---
	// linkID 0 grass, 1 dirt, 2 stone, 3 cobblestone, 4 leaves,
	// 5 crate, 6 iron, 7 wood planks, 8 wood support
	std::unique_ptr<KamataEngine::Model> modelBlockGrass;
	std::unique_ptr<KamataEngine::Model> modelBlockDirt;
	std::unique_ptr<KamataEngine::Model> modelBlockStone;
	std::unique_ptr<KamataEngine::Model> modelBlockCobblestone;
	std::unique_ptr<KamataEngine::Model> modelBlockLeaves;
	std::unique_ptr<KamataEngine::Model> modelBlockCrate;
	std::unique_ptr<KamataEngine::Model> modelBlockIron;
	std::unique_ptr<KamataEngine::Model> modelBlockWoodPlanks;
	std::unique_ptr<KamataEngine::Model> modelBlockWoodSupport;

	uint32_t blockGrassTex = 0;
	uint32_t blockDirtTex = 0;
	uint32_t blockStoneTex = 0;
	uint32_t blockCobblestoneTex = 0;
	uint32_t blockLeavesTex = 0;
	uint32_t blockCrateTex = 0;
	uint32_t blockIronTex = 0;
	uint32_t blockWoodPlanksTex = 0;
	uint32_t blockWoodSupportTex = 0;

	// --- 'T' platforms: each platform has its OWN model and texture ---
	// linkID 0 leaves, 1 dirt, 2 wood, 3 iron
	std::unique_ptr<KamataEngine::Model> modelBlockPlatformLeaves;
	std::unique_ptr<KamataEngine::Model> modelBlockPlatformDirt;
	std::unique_ptr<KamataEngine::Model> modelBlockPlatformWood;
	std::unique_ptr<KamataEngine::Model> modelBlockPlatformIron;

	uint32_t blockPlatformLeavesTex = 0;
	uint32_t blockPlatformDirtTex = 0;
	uint32_t blockPlatformWoodTex = 0;
	uint32_t blockPlatformIronTex = 0;

	// --- 'M' slow blocks: each slow block has its OWN model and texture ---
	// linkID 0 mud, 1 gravel
	std::unique_ptr<KamataEngine::Model> modelBlockSlowMud;
	std::unique_ptr<KamataEngine::Model> modelBlockSlowGravel;

	uint32_t blockSlowMudTex = 0;
	uint32_t blockSlowGravelTex = 0;

	// --- Special blocks: each has its OWN model, never shared with block types ---
	std::unique_ptr<KamataEngine::Model> modelGlass;     // 'Y'
	std::unique_ptr<KamataEngine::Model> modelBreakable; // 'V'
	std::unique_ptr<KamataEngine::Model> modelExplosive; // 'Z'
	std::unique_ptr<KamataEngine::Model> modelSpike;     // 'I'
	std::unique_ptr<KamataEngine::Model> modelFragile;   // '9'

	// モデルとテクスチャを別々に取得する。
	// linkIDから個別のモデルとテクスチャを選択する。
	KamataEngine::Model* ResolveTileModel(MapChipType type, uint8_t linkID) const;
	uint32_t ResolveTileTexture(MapChipType type, uint8_t linkID) const;

	// special blocks only: Y, V, Z, I, 9. stage is used by 'V' (0..2 damage stage)
	TileVisual ResolveSpecial(MapChipType type, int stage = 0) const;

	// --- Single-texture chips ---
	uint32_t blockSpikesTex = 0; // 'I' spike
	uint32_t blockGateTex = 0;   // '1'-'4' gates and '5'-'8' arena gates
	uint32_t blockGlassTex = 0;  // 'Y' glass
	uint32_t blockBarrelTex = 0; // 'Z' explosive
	uint32_t blockHealthTex = 0; // 'H' health block

	// --- 'V' breakable / '9' fragile (placeholders until real art exists) ---
	uint32_t breakableTex[3] = {0, 0, 0}; // 'V': [0] intact, [1] cracked, [2] nearly broken
	uint32_t fragileTex = 0;              // '9'

	// --- 'U' tutorial sign ---
	std::unique_ptr<KamataEngine::Model> modelTutorialBlock;
	uint32_t tutorialBlockTex = 0;

	// --- 'X' goal ---
	std::unique_ptr<KamataEngine::Model> modelGoal;
	uint32_t goalTex = 0;

	// --- '1'-'8' gates (model only; texture is blockGateTex above) ---
	std::unique_ptr<KamataEngine::Model> modelGate;

	// --- 'H' health block (model only; texture is blockHealthTex above) ---
	std::unique_ptr<KamataEngine::Model> modelHealthBlock;

	// --- 'W' switch = lever system (handle swings, support stays) ---
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
	uint32_t pauseButtonTex[3] = {0, 0, 0};   // play, retry, back
	uint32_t retryButtonTex[3] = {0, 0, 0};   // retry, restart, back
	uint32_t victoryButtonTex[3] = {0, 0, 0}; // next, restart, back
	uint32_t inGamePauseButtonTex = 0;
	uint32_t cursorTex[2] = {0, 0}; // [0] aim (play), [1] arrow (menus)

	// --- Banner(画面上部の文字)。順番は BannerUI::Kind: start, arena, paused, defeat, victory ---
	uint32_t bannerTex[6] = {0, 0, 0, 0, 0, 0};
	uint32_t screenFlashTex = 0;

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