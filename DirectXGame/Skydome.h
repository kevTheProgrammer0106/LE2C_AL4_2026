#pragma once
#include "KamataEngine.h"
/// <summary>
///	天球
/// </summary>
class Skydome 
{
public:

	~Skydome();

	void Initialize(KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera);
	void Update();
	void Draw();

private:
	KamataEngine::Camera* camera_ = nullptr;
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_{};
	uint32_t textureHandle_ = 0;
	KamataEngine::Model* model_ = nullptr;
};
