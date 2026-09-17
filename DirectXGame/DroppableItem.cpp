#include "DroppableItem.h"
#include "GlobalVariables.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "DroppableItem";
}

void DroppableItem::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "HalfWidth", kHalfWidth);
	gv->AddItem(kGroupName, "HalfHeight", kHalfHeight);
	gv->AddItem(kGroupName, "HalfDepth", kHalfDepth);
	gv->AddItem(kGroupName, "BobAmplitude", kBobAmplitude);
	gv->AddItem(kGroupName, "BobSpeed", kBobSpeed);
	gv->AddItem(kGroupName, "SpinSpeed", kSpinSpeed);
	gv->AddItem(kGroupName, "LifeTime", kLifeTime);
	gv->AddItem(kGroupName, "BlinkWarningDuration", kBlinkWarningDuration);
	gv->AddItem(kGroupName, "BlinkFrequency", kBlinkFrequency);
}

void DroppableItem::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kHalfWidth = gv->GetFloatValue(kGroupName, "HalfWidth");
	kHalfHeight = gv->GetFloatValue(kGroupName, "HalfHeight");
	kHalfDepth = gv->GetFloatValue(kGroupName, "HalfDepth");
	kBobAmplitude = gv->GetFloatValue(kGroupName, "BobAmplitude");
	kBobSpeed = gv->GetFloatValue(kGroupName, "BobSpeed");
	kSpinSpeed = gv->GetFloatValue(kGroupName, "SpinSpeed");
	kLifeTime = gv->GetFloatValue(kGroupName, "LifeTime");
	kBlinkWarningDuration = gv->GetFloatValue(kGroupName, "BlinkWarningDuration");
	kBlinkFrequency = gv->GetFloatValue(kGroupName, "BlinkFrequency");
}

void DroppableItem::Initialize(Model* model, uint32_t textureHandle, Camera* camera, const Vector3& position, DropItemCategory category, DropWeaponKind kind) {
#ifdef _DEBUG
	assert(model);
	assert(camera);
#endif
	model_ = model;
	textureHandle_ = textureHandle;
	camera_ = camera;
	category_ = category;
	kind_ = kind;

	basePosition_ = position;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {0.6f, 0.6f, 0.6f};

	bobTimer_ = 0.0f;
	isPickedUp_ = false;
	lifeTimer_ = 0.0f;
	isExpired_ = false;
	isPickedUp_ = false;
}

void DroppableItem::Update() {
	if (isPickedUp_ || isExpired_) {
		return;
	}

	lifeTimer_ += 1.0f / 60.0f;
	if (lifeTimer_ >= kLifeTime) {
		isExpired_ = true;
		return;
	}

	bobTimer_ += 1.0f / 60.0f;
	float bob = std::sin(bobTimer_ * kBobSpeed) * kBobAmplitude;
	worldTransform_.translation_ = basePosition_ + Vector3(0.0f, bob, 0.0f);
	worldTransform_.rotation_.y += kSpinSpeed * (1.0f / 60.0f);
	WorldTransformUpdate(worldTransform_);
}

void DroppableItem::Draw() {
	if (isPickedUp_ || isExpired_ || !model_) {
		return;
	}

	// 消える直前は点滅で警告する(obj/texは不要、単純に描画をスキップするだけ)
	float remaining = kLifeTime - lifeTimer_;
	if (remaining <= kBlinkWarningDuration) {
		int blinkPhase = static_cast<int>(lifeTimer_ * kBlinkFrequency) % 2;
		if (blinkPhase == 0) {
			return;
		}
	}

	Model::PreDraw();
	model_->Draw(worldTransform_, *camera_, textureHandle_);
	Model::PostDraw();
}

AABB DroppableItem::GetAABB() const {
	Vector3 pos = worldTransform_.translation_;
	AABB aabb;
	aabb.min = {pos.x - kHalfWidth, pos.y - kHalfHeight, pos.z - kHalfDepth};
	aabb.max = {pos.x + kHalfWidth, pos.y + kHalfHeight, pos.z + kHalfDepth};
	return aabb;
}