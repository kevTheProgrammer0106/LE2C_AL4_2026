#include "Skydome.h"

using namespace KamataEngine;

void Skydome::Initialize(KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera) {
#ifdef _DEBUG
	assert(model);
	assert(textureHandle);
	assert(camera);
#endif
	model_ = model;	
	textureHandle_ = textureHandle;
	camera_ = camera;

	worldTransform_.Initialize();
}

Skydome::~Skydome() { }


void Skydome::Update() 
{
	// ワールド変換の更新
	worldTransform_.TransferMatrix();
}

void Skydome::Draw() 
{
	model_->PreDraw();
	model_->Draw(worldTransform_, *camera_, textureHandle_);
	model_->PostDraw();
}
