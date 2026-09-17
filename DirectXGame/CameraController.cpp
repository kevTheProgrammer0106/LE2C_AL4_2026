#include "CameraController.h"

#include "Global.h"
#include "GlobalVariables.h"
#include "Player.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "CameraController";
}

void CameraController::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "InterpolationRate", kInterpolationRate);
	gv->AddItem(kGroupName, "VelocityBias", kVelocityBias);
}
void CameraController::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kInterpolationRate = gv->GetFloatValue(kGroupName, "InterpolationRate");
	kVelocityBias = gv->GetFloatValue(kGroupName, "VelocityBias");
}

void CameraController::Initialize(Camera* camera) {
	assert(camera);
	camera_ = camera;
	camera_->Initialize();
}

void CameraController::Update() {
	if (mode_ == Mode::kArenaFocus) {
		UpdateArenaFocusCamera();
		camera_->UpdateMatrix();
		return;
	}

	const WorldTransform& targetWT = target_->GetWorldTransform();

	targetPos.x = targetWT.translation_.x + targetOffset_.x + target_->GetVelocity().x * kVelocityBias;
	targetPos.y = targetWT.translation_.y + targetOffset_.y + target_->GetVelocity().y * kVelocityBias;
	targetPos.z = targetWT.translation_.z + targetOffset_.z + target_->GetVelocity().z * kVelocityBias;

	switch (mode_) {
	case Mode::kFollow:
		UpdateCameraPerspectives(targetWT);
		break;
	case Mode::kForcedScroll:
		camera_->rotation_.x = 0.0f;
		camera_->rotation_.y = 0.0f;
		camera_->translation_ = Lerp(camera_->translation_, targetPos, kInterpolationRate);
		scrollX_ += scrollSpeed_;
		scrollX_ = std::clamp(scrollX_, movableArea_.left, movableArea_.right);
		camera_->translation_.x = scrollX_;
		camera_->translation_.x = std::clamp(camera_->translation_.x, scrollX_ + margin.left, scrollX_ + margin.right);
		camera_->translation_.y = std::clamp(camera_->translation_.y, targetWT.translation_.y + margin.bottom, targetWT.translation_.y + margin.top);
		camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea_.left, movableArea_.right);
		camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea_.bottom, movableArea_.top);
		break;
	default:
		break;
	}

	camera_->UpdateMatrix();
}

void CameraController::StartArenaFocus(const Vector3& focusPoint, float lerpDuration) {
	arenaFocusPoint_ = focusPoint;
	arenaFocusStartPos_ = camera_->translation_;
	arenaFocusDuration_ = std::max(lerpDuration, 0.0001f);
	arenaFocusTimer_ = 0.0f;
	arenaFocusPhase_ = ArenaFocusPhase::kEnter;
	mode_ = Mode::kArenaFocus;
}

void CameraController::EndArenaFocus(float lerpDuration) {
	if (mode_ != Mode::kArenaFocus) {
		return;
	}
	arenaFocusStartPos_ = camera_->translation_;
	arenaFocusDuration_ = std::max(lerpDuration, 0.0001f);
	arenaFocusTimer_ = 0.0f;
	arenaFocusPhase_ = ArenaFocusPhase::kReturn;
}

void CameraController::UpdateArenaFocusCamera() {
	camera_->rotation_.x = 0.0f;
	camera_->rotation_.y = 0.0f;

	switch (arenaFocusPhase_) {
	case ArenaFocusPhase::kEnter: {
		arenaFocusTimer_ += 1.0f / 60.0f;
		float t = std::clamp(arenaFocusTimer_ / arenaFocusDuration_, 0.0f, 1.0f);
		float te = EaseInOut(0.0f, 1.0f, t);
		camera_->translation_ = Lerp(arenaFocusStartPos_, arenaFocusPoint_, te);
		if (t >= 1.0f)
			arenaFocusPhase_ = ArenaFocusPhase::kHold;
		break;
	}
	case ArenaFocusPhase::kHold:
		camera_->translation_ = arenaFocusPoint_;
		break;
	case ArenaFocusPhase::kReturn: {
		if (!target_) {
			mode_ = Mode::kFollow;
			break;
		}
		const WorldTransform& targetWT = target_->GetWorldTransform();
		Vector3 followTarget = {
		    targetWT.translation_.x + targetOffset_.x + target_->GetVelocity().x * kVelocityBias,
		    targetWT.translation_.y + targetOffset_.y + target_->GetVelocity().y * kVelocityBias,
		    targetWT.translation_.z + targetOffset_.z + target_->GetVelocity().z * kVelocityBias,
		};
		arenaFocusTimer_ += 1.0f / 60.0f;
		float t = std::clamp(arenaFocusTimer_ / arenaFocusDuration_, 0.0f, 1.0f);
		float te = EaseInOut(0.0f, 1.0f, t);
		camera_->translation_ = Lerp(arenaFocusStartPos_, followTarget, te);
		if (t >= 1.0f)
			mode_ = Mode::kFollow;
		break;
	}
	}
}

void CameraController::Reset() {
	const WorldTransform& targetWT = target_->GetWorldTransform();

	switch (mode_) {
	case Mode::kFollow:
		ResetCameraPerspectives();
		break;
	case Mode::kForcedScroll:
		targetOffset_ = {0.0f, 0.0f, -15.0f};
		scrollX_ = movableArea_.left;
		break;
	case Mode::kArenaFocus:
		targetOffset_ = {0.0f, 0.0f, -5.0f};
		break;
	default:
		break;
	}

	camera_->translation_.x = targetWT.translation_.x + targetOffset_.x;
	camera_->translation_.y = targetWT.translation_.y + targetOffset_.y;
	camera_->translation_.z = targetWT.translation_.z + targetOffset_.z;
	camera_->UpdateMatrix();
}

void CameraController::SetWorldBounds(float worldWidth, float worldHeight) {
	constexpr float kHalfScreenWidth = 10.5f;
	constexpr float kHalfScreenHeight = 6.5f;

	movableArea_.left = kHalfScreenWidth;
	movableArea_.right = worldWidth - kHalfScreenWidth;
	movableArea_.bottom = kHalfScreenHeight;
	movableArea_.top = worldHeight - kHalfScreenHeight;
}

#pragma region Additional Camera Perspectives

void CameraController::Perspective() {
#ifdef _DEBUG
	ImGui::Begin("Camera Perspective Customization");
	ImGui::Text("Press Q to enable debug camera");
	int currentType = static_cast<int>(perspectiveType);
	ImGui::Value("Current Perspective", static_cast<int>(perspectiveType));
	ImGui::Combo("Perspective Type", &currentType, "Sidescroll\0Fixed\0TopDown\0TopDownDiagonal\0Isometric\0ThirdPerson\0FirstPerson\0");
	perspectiveType = static_cast<PerspectiveType>(currentType);
	ImGui::DragFloat("Push Boundary Offset", &kPushBoundaryOffset, 0.1f, -20.0f, 0.0f);
	ImGui::BulletText("Camera Parameters");
	ImGui::DragFloat("FOV Angle Y", &camera_->fovAngleY, 0.01f, 0.01f, 3.14f);
	ImGui::DragFloat("Aspect Ratio", &camera_->aspectRatio, 0.01f, 0.01f, 10.0f);
	ImGui::DragFloat("Near Z", &camera_->nearZ, 0.01f, 0.01f, 100.0f);
	ImGui::DragFloat("Far Z", &camera_->farZ, 1.0f, 0.01f, 10000.0f);
	ImGui::BulletText("Camera Tracking Parameters");
	ImGui::DragFloat3("Target Offset", &targetOffset_.x, 0.1f, -100.0f, 100.0f);
	ImGui::DragFloat4("Movable Area", &movableArea_.left, 0.1f, -100.0f, 100.0f);
	ImGui::BulletText("Camera SRT Parameters");
	ImGui::DragFloat2("Rotation", &camera_->rotation_.x, 0.1f, -100.0f, 100.0f);
	ImGui::DragFloat3("Translation", &camera_->translation_.x, 0.1f, -100.0f, 1000.0f);
	ImGui::End();
#endif
}

void CameraController::UpdateCameraPerspectives(const KamataEngine::WorldTransform& targetWorldTransform) {
	switch (perspectiveType) {
	case PerspectiveType::Sidescroll:
		camera_->rotation_.x = 0.0f;
		camera_->rotation_.y = 0.0f;
		camera_->translation_ = Lerp(camera_->translation_, targetPos, kInterpolationRate);
		camera_->translation_.x = std::clamp(camera_->translation_.x, targetWorldTransform.translation_.x + margin.left, targetWorldTransform.translation_.x + margin.right);
		camera_->translation_.y = std::clamp(camera_->translation_.y, targetWorldTransform.translation_.y + margin.bottom, targetWorldTransform.translation_.y + margin.top);
		camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea_.left, movableArea_.right);
		camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea_.bottom, movableArea_.top);
		break;
	case PerspectiveType::Fixed:
		UpdateFixedCamera(targetWorldTransform);
		break;
	case PerspectiveType::TopDown:
		UpdateTopDownCamera(targetWorldTransform);
		break;
	case PerspectiveType::TopDownDiagonal:
		UpdateTopDownDiagonalCamera(targetWorldTransform);
		break;
	case PerspectiveType::Isometric:
		UpdateIsometricCamera(targetWorldTransform);
		break;
	case PerspectiveType::ThirdPerson:
		UpdateThirdPersonCamera(targetWorldTransform);
		break;
	case PerspectiveType::FirstPerson:
		UpdateFirstPersonCamera(targetWorldTransform);
		break;
	default:
		break;
	}
}

void CameraController::ResetCameraPerspectives() {
	switch (perspectiveType) {
	case PerspectiveType::Sidescroll:
		targetOffset_ = {0.0f, 0.0f, -20.0f};
		break;
	default:
		targetOffset_ = {0.0f, 0.0f, 0.0f};
		break;
	}
}
void CameraController::UpdateFixedCamera(const WorldTransform& worldTransformTarget) {
	if (worldTransformTarget.translation_.x >= 10.0f) {
		camera_->rotation_.x = 0.0f;
		camera_->rotation_.y = 0.0f;
		camera_->translation_.x = 10.5f;
		camera_->translation_.y = 6.5f;
		camera_->translation_.z = -15.0f;
	}
	if (worldTransformTarget.translation_.x >= 22.0f) {
		camera_->rotation_.x = 0.6f;
		camera_->rotation_.y = 0.2f;
		camera_->translation_.x = 36.0f;
		camera_->translation_.y = 17.8f;
		camera_->translation_.z = -15.0f;
	}
	if (worldTransformTarget.translation_.x >= 56.0f) {
		camera_->rotation_.x = 0.1f;
		camera_->rotation_.y = -3.14f;
		camera_->translation_.x = 62.0f;
		camera_->translation_.y = 3.8f;
		camera_->translation_.z = 6.8f;
	}
	if (worldTransformTarget.translation_.x >= 67.0f) {
		camera_->rotation_.x = -0.4f;
		camera_->rotation_.y = 0.9f;
		camera_->translation_.x = 69.0f;
		camera_->translation_.y = -0.2f;
		camera_->translation_.z = -10.2f;
	}
	if (worldTransformTarget.translation_.x >= 82.0f) {
		camera_->rotation_.x = 0.0f;
		camera_->rotation_.y = 0.0f;
		camera_->translation_.x = 88.5f;
		camera_->translation_.y = 6.5f;
		camera_->translation_.z = -15.0f;
	}
}
void CameraController::UpdateTopDownCamera(const WorldTransform& worldTransformTarget) {
	camera_->rotation_.x = 1.57f;
	Vector3 desiredPos = {worldTransformTarget.translation_.x, worldTransformTarget.translation_.y + 8.0f, worldTransformTarget.translation_.z};
	camera_->translation_ = Lerp(camera_->translation_, desiredPos, kInterpolationRate);
}
void CameraController::UpdateTopDownDiagonalCamera(const WorldTransform& worldTransformTarget) {
	camera_->rotation_.x = 0.785f;
	camera_->rotation_.y = 0.0f;
	Vector3 desiredPos = {worldTransformTarget.translation_.x, worldTransformTarget.translation_.y + 8.0f, worldTransformTarget.translation_.z - 8.0f};
	camera_->translation_ = Lerp(camera_->translation_, desiredPos, kInterpolationRate);
}
void CameraController::UpdateIsometricCamera(const WorldTransform& worldTransformTarget) {
	camera_->rotation_.x = 0.615f;
	camera_->rotation_.y = 0.785f;
	Vector3 desiredPos = {worldTransformTarget.translation_.x - 10.0f, worldTransformTarget.translation_.y + 10.0f, worldTransformTarget.translation_.z - 10.0f};
	camera_->translation_ = Lerp(camera_->translation_, desiredPos, kInterpolationRate);
}
void CameraController::UpdateThirdPersonCamera(const WorldTransform& worldTransformTarget) {
	camera_->rotation_.x = 0.3f;
	camera_->rotation_.y = worldTransformTarget.rotation_.y;
	float distance = 8.0f;
	Vector3 desiredPos = {
	    worldTransformTarget.translation_.x - std::sin(camera_->rotation_.y) * distance, worldTransformTarget.translation_.y + 3.0f,
	    worldTransformTarget.translation_.z - std::cos(camera_->rotation_.y) * distance};
	camera_->translation_ = Lerp(camera_->translation_, desiredPos, kInterpolationRate);
}
void CameraController::UpdateFirstPersonCamera(const WorldTransform& worldTransformTarget) {
	camera_->rotation_.x = 0.0f;
	camera_->rotation_.y = worldTransformTarget.rotation_.y;
	camera_->translation_.x = worldTransformTarget.translation_.x;
	camera_->translation_.y = worldTransformTarget.translation_.y + 1.5f;
	camera_->translation_.z = worldTransformTarget.translation_.z;
}

#pragma endregion