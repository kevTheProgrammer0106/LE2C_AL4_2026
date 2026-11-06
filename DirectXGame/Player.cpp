#define NOMINMAX
#include "Player.h"
#include "GameScene.h"

#include "GlobalVariables.h"


using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "Player";
}

#pragma region GlobalVariables
void Player::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "Acceleration", kAcceleration);
	gv->AddItem(kGroupName, "Attenuation", kAttenuation);
	gv->AddItem(kGroupName, "LimitRunSpeed", kLimitRunSpeed);
	gv->AddItem(kGroupName, "GravityAcceleration", kGravityAcceleration);
	gv->AddItem(kGroupName, "Width", kWidth);
	gv->AddItem(kGroupName, "Height", kHeight);
	gv->AddItem(kGroupName, "Blank", kBlank);

	gv->AddItem(kGroupName, "Player Scale", currentSRT.scale);
	gv->AddItem(kGroupName, "Player Rotation", currentSRT.rotation);
	gv->AddItem(kGroupName, "Player Translation", currentSRT.translation);
}
void Player::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kAcceleration = gv->GetFloatValue(kGroupName, "Acceleration");
	kAttenuation = gv->GetFloatValue(kGroupName, "Attenuation");
	kLimitRunSpeed = gv->GetFloatValue(kGroupName, "LimitRunSpeed");
	kGravityAcceleration = gv->GetFloatValue(kGroupName, "GravityAcceleration");
	kBlank = gv->GetFloatValue(kGroupName, "Blank");
	kWidth = gv->GetFloatValue(kGroupName, "Width");
	kHeight = gv->GetFloatValue(kGroupName, "Height");

	currentSRT.scale = gv->GetVector3Value(kGroupName, "Player Scale");
	currentSRT.rotation = gv->GetVector3Value(kGroupName, "Player Rotation");
	currentSRT.translation = gv->GetVector3Value(kGroupName, "Player Translation");
}
#pragma endregion

Player::~Player() {}

void Player::Initialize(KamataEngine::Model* modelPlayer, uint32_t texturePlayer, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
#ifdef _DEBUG
	assert(camera);
	assert(modelPlayer);
#endif
	camera_ = camera;
	velocity_ = {0, 0, 0};

	modelPlayer_ = modelPlayer;
	playerTex_ = texturePlayer;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_ = currentSRT.rotation;
	worldTransform_.translation_ = position;

	velocity_ = {0.0f, 0.0f, 0.0f};
}

void Player::Update() {
	ApplyGlobalVariables();

	if (isDead_) {
		return;
	}
	GroundMovement();
	currentSRT = {worldTransform_.scale_, worldTransform_.rotation_, GetWorldPosition()};
	WorldTransformUpdate(worldTransform_);
}
void Player::Draw() {
	Model::PreDraw();
	modelPlayer_->Draw(worldTransform_, *camera_, playerTex_);
	Model::PostDraw();
}

const WorldTransform& Player::GetWorldTransform() const { return worldTransform_; }
void Player::SetWorldTransform(const KamataEngine::WorldTransform& worldTransform) {
	worldTransform_.translation_ = worldTransform.translation_;
	worldTransform_.rotation_ = worldTransform.rotation_;
	worldTransform_.scale_ = worldTransform.scale_;
}
Vector3 Player::GetWorldPosition() const { return {worldTransform_.translation_.x, worldTransform_.translation_.y, worldTransform_.translation_.z}; }

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};
	return aabb;
}
void Player::GroundMovement() {
	if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_A)) {
		Vector3 acceleration = {};
		if (Input::GetInstance()->PushKey(DIK_D)) {
			if ( velocity_.x < 0.0f ) {
				velocity_.x *= (1.0f - kAttenuation);
			}
			acceleration.x += kAcceleration;
		} else if (Input::GetInstance()->PushKey(DIK_A)) {
			if (velocity_.x > 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}
			acceleration.x -= kAcceleration;
		}
		velocity_ += acceleration;
		velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
	} else {
		velocity_.x *= (1.0f - kAttenuation);
	}

	 // 速度をプレイヤーの座標に反映する
	worldTransform_.translation_ += velocity_;
}