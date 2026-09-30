#include "GameAssets.h"

using namespace KamataEngine;

void GameAssets::Load() {
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

	// --- Enemy: Cuirassier [NEW] dedicated assets (previously reused Rifleman's) ---
	modelCuirassierBody.reset(Model::CreateFromOBJ("cuirassierBody", true));
	modelCuirassierHead.reset(Model::CreateFromOBJ("cuirassierHead", true));
	modelCuirassierHat.reset(Model::CreateFromOBJ("cuirassierHat", true));
	cuirassierBodyTex = TextureManager::Load("./Resources/cuirassierBody/cuirassierTorso.png");
	cuirassierHeadTex = TextureManager::Load("./Resources/cuirassierHead/cuirassierHead.png");
	cuirassierHatTex = TextureManager::Load("./Resources/cuirassierHat/cuirassierHelmet.png");

	// --- Enemy: Fighter [NEW] dedicated assets (previously reused Rifleman's) ---
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
	hitEffectModel = Model::CreateFromOBJ("particle");

	guardEffectTex = TextureManager::Load("./Resources/ring/ring.png");
	guardEffectModel.reset(Model::CreateFromOBJ("ring"));

	deathParticleTex = TextureManager::Load("./Resources/deathParticle/white1x1.png");
	deathParticleModel.reset(Model::CreateFromOBJ("deathParticle", true));

	// --- Blocks ---
	blockTex = TextureManager::Load("./Resources/block/block.png");
	blockModel.reset(Model::CreateFromOBJ("block"));
	blockPlatformTex = TextureManager::Load("./Resources/platform/block.png");
	blockPlatformModel.reset(Model::CreateFromOBJ("platform"));

	// Additional block texture variants, selected per-tile via the map chip subID.
	// subID 1 = wood, 2 = brick, 3 = cobble, 4 = concrete, 5 = rubber.
	{
		static constexpr uint8_t kSubIDGrass = 0;
		static constexpr uint8_t kSubIDWood = 1;
		static constexpr uint8_t kSubIDBrick = 2;
		static constexpr uint8_t kSubIDCobble = 3;
		static constexpr uint8_t kSubIDConcrete = 4;
		static constexpr uint8_t kSubIDRubber = 5;

		blockTexVariants[kSubIDGrass] = TextureManager::Load("./Resources/block/grassBlock.png");
		blockTexVariants[kSubIDWood] = TextureManager::Load("./Resources/block/grassBlock.png");
		blockTexVariants[kSubIDBrick] = TextureManager::Load("./Resources/block/grassBlock.png");
		blockTexVariants[kSubIDCobble] = TextureManager::Load("./Resources/block/grassBlock.png");
		blockTexVariants[kSubIDConcrete] = TextureManager::Load("./Resources/block/grassBlock.png");
		blockTexVariants[kSubIDRubber] = TextureManager::Load("./Resources/block/grassBlock.png");

		platformTexVariants[kSubIDGrass] = TextureManager::Load("./Resources/block/grassBlock.png");
		platformTexVariants[kSubIDWood] = TextureManager::Load("./Resources/block/grassBlock.png");
		platformTexVariants[kSubIDBrick] = TextureManager::Load("./Resources/block/grassBlock.png");
		platformTexVariants[kSubIDCobble] = TextureManager::Load("./Resources/block/grassBlock.png");
		platformTexVariants[kSubIDConcrete] = TextureManager::Load("./Resources/block/grassBlock.png");
		platformTexVariants[kSubIDRubber] = TextureManager::Load("./Resources/block/grassBlock.png");
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
}