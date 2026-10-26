#define NOMINMAX
#include "SpriteTutorial.h"
#include "GlobalVariables.h"
#include "Player.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "SpriteTutorial";
}

void SpriteTutorial::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "FadeInDistance", kFadeInDistance);
	gv->AddItem(kGroupName, "FadeOutDistance", kFadeOutDistance);
	gv->AddItem(kGroupName, "HoverOffsetY", kHoverOffsetY);
	gv->AddItem(kGroupName, "FadeSpeed", kFadeSpeed);
	gv->AddItem(kGroupName, "SpriteWidth", kSpriteWidth);
	gv->AddItem(kGroupName, "SpriteHeight", kSpriteHeight);
}

void SpriteTutorial::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kFadeInDistance = gv->GetFloatValue(kGroupName, "FadeInDistance");
	kFadeOutDistance = std::max(gv->GetFloatValue(kGroupName, "FadeOutDistance"), kFadeInDistance); // keeps the hysteresis band valid
	kHoverOffsetY = gv->GetFloatValue(kGroupName, "HoverOffsetY");
	kFadeSpeed = gv->GetFloatValue(kGroupName, "FadeSpeed");
	kSpriteWidth = gv->GetFloatValue(kGroupName, "SpriteWidth");
	kSpriteHeight = gv->GetFloatValue(kGroupName, "SpriteHeight");
}

void SpriteTutorial::LoadSharedTextures() {
	if (sTexturesLoaded_)
		return;
	// Order must match TutorialTextureType
	static constexpr const char* kFiles[] = {
	    "./Resources/UserInterface/tutorialMove.png",         "./Resources/UserInterface/tutorialJump.png",          "./Resources/UserInterface/tutorialPlatform.png",      "./Resources/UserInterface/tutorialCheckpointExplain.png",
	    "./Resources/UserInterface/tutorialAttack.png",       "./Resources/UserInterface/tutorialBlock.png",         "./Resources/UserInterface/tutorialBlockExplain.png",  "./Resources/UserInterface/tutorialPickup.png",
	    "./Resources/UserInterface/tutorialWeaponSwitch.png", "./Resources/UserInterface/tutorialAttackSpecial.png", "./Resources/UserInterface/tutorialPickupExplain.png", "./Resources/UserInterface/tutorialGate.png",
	    "./Resources/UserInterface/tutorialObjective.png",
	};
	static_assert(std::size(kFiles) == static_cast<size_t>(TutorialTextureType::kCount), "one file per TutorialTextureType");
	for (size_t i = 0; i < std::size(kFiles); ++i)
		sTextures_[i] = TextureManager::Load(kFiles[i]);
	sTexturesLoaded_ = true;
}

void SpriteTutorial::Initialize(Model* blockModel, uint32_t blockTexture, Camera* camera, const Vector3& position, TutorialTextureType type) {
#ifdef _DEBUG
	assert(blockModel);
	assert(camera);
#endif
	model_ = blockModel;
	blockTexture_ = blockTexture;
	camera_ = camera;
	type_ = (type < TutorialTextureType::kCount) ? type : TutorialTextureType::kMove;
	currentAlpha_ = 0.0f;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	WorldTransformUpdate(worldTransform_);

	LoadSharedTextures();
	sprite_.reset(Sprite::Create(sTextures_[static_cast<size_t>(type_)], {0.0f, 0.0f}));
	sprite_->SetSize({kSpriteWidth, kSpriteHeight});
	sprite_->SetAnchorPoint({0.5f, 1.0f}); // bottom-centre, so it sits above the head
	sprite_->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
}

void SpriteTutorial::Update(Player* player) {
	if (!player || !camera_ || !sprite_)
		return;

	const Vector3 playerPos = player->GetWorldPosition();
	const float distance = std::hypot(playerPos.x - worldTransform_.translation_.x, playerPos.y - worldTransform_.translation_.y);

	if (distance <= kFadeInDistance)
		currentAlpha_ = std::min(currentAlpha_ + kFadeSpeed, 1.0f);
	else if (distance >= kFadeOutDistance)
		currentAlpha_ = std::max(currentAlpha_ - kFadeSpeed, 0.0f);
	// between the two distances the current alpha is kept (hysteresis)

	if (currentAlpha_ <= 0.0f)
		return; // fully hidden: skip the projection

	// Shared world -> screen helper (Global.h) instead of a private matrix multiply
	const Vector3 screen = WorldToScreen(*camera_, playerPos + Vector3(0.0f, kHoverOffsetY, 0.0f));
	sprite_->SetSize({kSpriteWidth, kSpriteHeight});
	sprite_->SetPosition({screen.x, screen.y});
	sprite_->SetColor({1.0f, 1.0f, 1.0f, currentAlpha_});
}

void SpriteTutorial::DrawBlock() {
	if (!model_ || !camera_)
		return;
	Model::PreDraw();
	model_->Draw(worldTransform_, *camera_, blockTexture_);
	Model::PostDraw();
}

void SpriteTutorial::DrawSprite() {
	if (!sprite_ || currentAlpha_ <= 0.0f)
		return;
	Sprite::PreDraw();
	sprite_->Draw();
	Sprite::PostDraw();
}