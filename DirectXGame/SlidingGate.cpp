#define NOMINMAX
#include "SlidingGate.h"
#include "GlobalVariables.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "SlidingGate";
}

void SlidingGate::Initialize(Model* model, uint32_t textureHandle, Camera* camera, const Vector3& closedPosition, MapChipType gateType, uint8_t textureVariant, uint8_t buttonID) {
#ifdef _DEBUG
	assert(model);
	assert(camera);
#endif
	model_ = model;
	textureHandle_ = textureHandle;
	camera_ = camera;

	closedPosition_ = closedPosition;
	direction_ = DirectionFromType(gateType);
	textureVariant_ = textureVariant;
	buttonID_ = buttonID;

	worldTransform_.Initialize();
	worldTransform_.translation_ = closedPosition_;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	isOpenTarget_ = false;
	openFactor_ = 0.0f;
}

SlidingGate::SlideDirection SlidingGate::DirectionFromType(MapChipType type) {
	switch (type) {
	case MapChipType::kGateUp:
	default:
		return SlideDirection::kUp;
	case MapChipType::kGateDown:
		return SlideDirection::kDown;
	case MapChipType::kGateLeft:
		return SlideDirection::kLeft;
	case MapChipType::kGateRight:
		return SlideDirection::kRight;
	}
}

void SlidingGate::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);

	gv->AddItem(kGroupName, "HalfWidth", kHalfWidth);
	gv->AddItem(kGroupName, "HalfHeight", kHalfHeight);
	gv->AddItem(kGroupName, "HalfDepth", kHalfDepth);
	gv->AddItem(kGroupName, "SlideDistance", kSlideDistance);
	gv->AddItem(kGroupName, "SlideSpeed", kSlideSpeed);
	gv->AddItem(kGroupName, "BlockingThreshold", kBlockingThreshold);
}

void SlidingGate::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();

	kHalfWidth = gv->GetFloatValue(kGroupName, "HalfWidth");
	kHalfHeight = gv->GetFloatValue(kGroupName, "HalfHeight");
	kHalfDepth = gv->GetFloatValue(kGroupName, "HalfDepth");
	kSlideDistance = gv->GetFloatValue(kGroupName, "SlideDistance");
	kSlideSpeed = gv->GetFloatValue(kGroupName, "SlideSpeed");
	kBlockingThreshold = gv->GetFloatValue(kGroupName, "BlockingThreshold");
}

void SlidingGate::Update() {
	float target = isOpenTarget_ ? 1.0f : 0.0f;
	if (openFactor_ < target) {
		openFactor_ = std::min(openFactor_ + kSlideSpeed, target);
	} else if (openFactor_ > target) {
		openFactor_ = std::max(openFactor_ - kSlideSpeed, target);
	}

	Vector3 slideOffset{};
	switch (direction_) {
	case SlideDirection::kUp:
		slideOffset = {0.0f, kSlideDistance * openFactor_, 0.0f};
		break;
	case SlideDirection::kDown:
		slideOffset = {0.0f, -kSlideDistance * openFactor_, 0.0f};
		break;
	case SlideDirection::kLeft:
		slideOffset = {-kSlideDistance * openFactor_, 0.0f, 0.0f};
		break;
	case SlideDirection::kRight:
		slideOffset = {kSlideDistance * openFactor_, 0.0f, 0.0f};
		break;
	}

	worldTransform_.translation_ = closedPosition_ + slideOffset;
	WorldTransformUpdate(worldTransform_);
}

AABB SlidingGate::GetAABB() const {
	Vector3 pos = worldTransform_.translation_;
	AABB aabb;
	aabb.min = {pos.x - kHalfWidth, pos.y - kHalfHeight, pos.z - kHalfDepth};
	aabb.max = {pos.x + kHalfWidth, pos.y + kHalfHeight, pos.z + kHalfDepth};
	return aabb;
}

void SlidingGate::Draw() {
	if (model_) {
		Model::PreDraw();
		model_->Draw(worldTransform_, *camera_, textureHandle_);
		Model::PostDraw();
	}
}