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
	modelSkyDome.reset(Model::CreateFromOBJ("skydome", true));
	skyDomeTex = TextureManager::Load("./Resources/skydome/skydome.png");

	modelPlayer.reset(Model::CreateFromOBJ("playerHat", true));
	playerBodyTex = TextureManager::Load("./Resources/playerHat/playerShako.png");

	modelEnemy.reset(Model::CreateFromOBJ("officeHat", true));
	enemyBodyTex = TextureManager::Load("./Resources/officeHat/officerHat.png");

	modelProjectile.reset(Model::CreateFromOBJ("firearmBall", true));
	projectileTex = TextureManager::Load("./Resources/firearmBall/firearmBall.png");
}

void GameAssets::LoadAudio() {
	Audio* audio = Audio::GetInstance();
	musicIngame = audio->LoadWave("./Resources/sfx_bgm/combatMusic.wav");
}