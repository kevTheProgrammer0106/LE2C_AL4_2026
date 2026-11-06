#pragma once

#include "Global.h"
#include "KamataEngine.h"
#include <cstdint>
#include <memory>

/// Owns every model / texture / audio handle the gameplay scene uses.
/// GameScene calls Load() once, then reads the public members. Audio handles are only stored here;
/// GameScene::BindAssets() hands them to the classes that play them.
class GameAssets {
public:
	void Load();

	// --- Skydome ---
	std::unique_ptr<KamataEngine::Model> modelSkyDome;
	uint32_t skyDomeTex = 0;

	std::unique_ptr<KamataEngine::Model> modelPlayer;
	uint32_t playerBodyTex = 0;

	std::unique_ptr<KamataEngine::Model> modelEnemy;
	uint32_t enemyBodyTex = 0;

	std::unique_ptr<KamataEngine::Model> modelProjectile;
	uint32_t projectileTex = 0;

	// --- Audio: music ---
	uint32_t musicIngame = 0;

private:
	void LoadModelsAndTextures();
	void LoadAudio();
};