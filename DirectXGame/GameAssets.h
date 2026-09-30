#pragma once

#include "KamataEngine.h"
#include <cstdint>
#include <memory>
#include <unordered_map>

/// <summary>
/// GameSceneが持っていたモデル/テクスチャのロードと所有権をここへ集約する。
/// GameSceneはこのクラスの Load() を1回呼び、以降は公開メンバー経由で
/// 各モデル/テクスチャハンドルを参照する。
///
/// [NEW] EnemyCuirassier / EnemyFighter は元々 Rifleman と同じ body/head/hat
/// (modelEnemyBody_ 等) を使い回していたが、それぞれ専用のモデル/テクスチャを
/// 個別に読み込むようにした (modelCuirassierBody 等 / modelFighterBody 等)。
/// </summary>
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
	std::unique_ptr<KamataEngine::Model> modelMusketLong;
	uint32_t musketLongTex = 0;
	std::unique_ptr<KamataEngine::Model> modelMusketBayonet;
	uint32_t musketBayonetTex = 0;
	std::unique_ptr<KamataEngine::Model> modelProjectileBall;
	uint32_t firearmBallTex = 0;

	// --- Enemy: Rifleman ---
	std::unique_ptr<KamataEngine::Model> modelRiflemanBody;
	std::unique_ptr<KamataEngine::Model> modelRiflemanHead;
	std::unique_ptr<KamataEngine::Model> modelRiflemanHat;
	uint32_t riflemanBodyTex = 0;
	uint32_t riflemanHeadTex = 0;
	uint32_t riflemanHatTex = 0;

	// --- Enemy: Cuirassier [NEW] own body/head/hat instead of reusing Rifleman's ---
	std::unique_ptr<KamataEngine::Model> modelCuirassierBody;
	std::unique_ptr<KamataEngine::Model> modelCuirassierHead;
	std::unique_ptr<KamataEngine::Model> modelCuirassierHat;
	uint32_t cuirassierBodyTex = 0;
	uint32_t cuirassierHeadTex = 0;
	uint32_t cuirassierHatTex = 0;

	// --- Enemy: Fighter [NEW] own body/head/hat instead of reusing Rifleman's ---
	std::unique_ptr<KamataEngine::Model> modelFighterBody;
	std::unique_ptr<KamataEngine::Model> modelFighterHead;
	std::unique_ptr<KamataEngine::Model> modelFighterHat;
	uint32_t fighterBodyTex = 0;
	uint32_t fighterHeadTex = 0;
	uint32_t fighterHatTex = 0;

	// --- Effects ---
	std::unique_ptr<KamataEngine::Model> modelSlashEffect;
	uint32_t slashEffectTex = 0;
	KamataEngine::Model* hitEffectModel = nullptr; // HitEffect::SetModel が所有権を持たない生ポインタで受け取る既存仕様に合わせる
	uint32_t hitEffectTex = 0;
	std::unique_ptr<KamataEngine::Model> guardEffectModel;
	uint32_t guardEffectTex = 0;
	std::unique_ptr<KamataEngine::Model> deathParticleModel;
	uint32_t deathParticleTex = 0;

	// --- Blocks ---
	std::unique_ptr<KamataEngine::Model> blockModel;
	uint32_t blockTex = 0;
	std::unique_ptr<KamataEngine::Model> blockPlatformModel;
	uint32_t blockPlatformTex = 0;
	std::unordered_map<uint8_t, uint32_t> blockTexVariants;
	std::unordered_map<uint8_t, uint32_t> platformTexVariants;

	// --- Goal / Gate / Switch / HealthBlock ---
	std::unique_ptr<KamataEngine::Model> modelGoal;
	uint32_t goalTex = 0;
	std::unique_ptr<KamataEngine::Model> modelGate;
	uint32_t gateTex = 0;
	std::unique_ptr<KamataEngine::Model> modelSwitch;
	uint32_t switchTex = 0;
	std::unique_ptr<KamataEngine::Model> modelHealthBlock;
	uint32_t healthBlockTex = 0;

	// --- Gameplay HUD ---
	uint32_t enemyIconRiflemanTex = 0;
	uint32_t enemyIconCuirassierTex = 0;
	uint32_t enemyIconFighterTex = 0;
	uint32_t enemyIconArrowTex = 0;
	uint32_t healthBarBackgroundTex = 0;
	uint32_t healthBarFillTex = 0;
	uint32_t meleeIconTex = 0;
	uint32_t firearmIconTex = 0;
	uint32_t interactionPromptTex = 0;

	uint32_t ResolveBlockTexture(const std::unordered_map<uint8_t, uint32_t>& table, uint8_t subID, uint32_t defaultTex) const {
		auto it = table.find(subID);
		return it != table.end() ? it->second : defaultTex;
	}
};