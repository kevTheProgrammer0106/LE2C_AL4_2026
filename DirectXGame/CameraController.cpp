#include "CameraController.h"

#include "Global.h"
#include "Player.h"

using namespace KamataEngine;

void CameraController::Initialize(Camera* camera) {
	// カメラ受け取り
	assert(camera);
	camera_ = camera;
	camera_->Initialize();
}

void CameraController::Update() {
	// 追従対象とオフセットからカメラの座標を計算
	const WorldTransform& targetWT = target_->GetWorldTransform();

	targetPos.x = targetWT.translation_.x + targetOffset_.x + target_->GetVelocity().x * kVelocityBias;
	targetPos.y = targetWT.translation_.y + targetOffset_.y + target_->GetVelocity().y * kVelocityBias;
	targetPos.z = targetWT.translation_.z + targetOffset_.z + target_->GetVelocity().z * kVelocityBias;

	float speed = 0.25f;
	Vector3 velocity = {speed, 0.0f, 0.0f};
	switch (mode_) {
	case Mode::kFollow:
		UpdateCameraPerspectives(targetWT);
		break;
	case Mode::kForcedScroll:
		camera_->rotation_.x = 0.0f;
		camera_->rotation_.y = 0.0f;

		camera_->translation_ = Lerp(camera_->translation_, targetPos, kInterpolationRate);

		scrollX_ += scrollSpeed_; // always advances, independent of player
		scrollX_ = std::clamp(scrollX_, movableArea_.left, movableArea_.right);
		// 追従
		camera_->translation_.x = scrollX_;
		camera_->translation_.x = std::clamp(camera_->translation_.x, scrollX_ + margin.left, scrollX_ + margin.right);
		camera_->translation_.y = std::clamp(camera_->translation_.y, targetWT.translation_.y + margin.bottom, targetWT.translation_.y + margin.top);

		// 移動範囲の制限
		camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea_.left, movableArea_.right);
		camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea_.bottom, movableArea_.top);
		break;
	}

	// 行列を更新
	camera_->UpdateMatrix();
}

void CameraController::Reset() {
	// 追従対象とオフセットからカメラの座標を計算
	const WorldTransform& targetWT = target_->GetWorldTransform();

	switch (mode_) {
	case Mode::kFollow:
		ResetCameraPerspectives();
		break;
	case Mode::kForcedScroll:
		targetOffset_ = {0.0f, 0.0f, -15.0f};
		scrollX_ = movableArea_.left; // start scroll at the left edge, not the player's raw X
		break;
	}

	camera_->translation_.x = targetWT.translation_.x + targetOffset_.x;
	camera_->translation_.y = targetWT.translation_.y + targetOffset_.y;
	camera_->translation_.z = targetWT.translation_.z + targetOffset_.z;
	// 行列を更新
	camera_->UpdateMatrix();
}

#pragma region Additional Camera Perspectives
void CameraController::Perspective() {
	ImGui::Begin("Camera Perspective Customization");
	ImGui::Text("Press Q to enable debug camera");
	int currentType = static_cast<int>(perspectiveType);
	ImGui::Value("Current Perspective", static_cast<int>(perspectiveType));
	ImGui::Combo("Perspective Type", &currentType, "Sidescroll\0Fixed\0TopDown\0TopDownDiagonal\0Isometric\0ThirdPerson\0FirstPerson\0");
	{
		perspectiveType = static_cast<PerspectiveType>(currentType);
	}
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
	ImGui::DragFloat2("Rotation", &camera_->rotation_.x, 0.1f, -100.0f, 100.0f); // zが操作ないので無視
	ImGui::DragFloat3("Translation", &camera_->translation_.x, 0.1f, -100.0f, 1000.0f);
	ImGui::End();
}
void CameraController::UpdateCameraPerspectives(const KamataEngine::WorldTransform& targetWorldTransform) {
	switch (perspectiveType) {
	case PerspectiveType::Sidescroll:
		camera_->rotation_.x = 0.0f;
		camera_->rotation_.y = 0.0f;
		camera_->translation_ = Lerp(camera_->translation_, targetPos, kInterpolationRate);
		// 追従
		camera_->translation_.x = std::clamp(camera_->translation_.x, targetWorldTransform.translation_.x + margin.left, targetWorldTransform.translation_.x + margin.right);
		camera_->translation_.y = std::clamp(camera_->translation_.y, targetWorldTransform.translation_.y + margin.bottom, targetWorldTransform.translation_.y + margin.top);
		// 移動範囲の制限
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
	}
}
void CameraController::ResetCameraPerspectives() {
	switch (perspectiveType) {
	case PerspectiveType::Sidescroll:
		targetOffset_ = {0.0f, 0.0f, -15.0f};
		break;
	case PerspectiveType::Fixed:
		targetOffset_ = {0.0f, 0.0f, 0.0f};
		break;
	case PerspectiveType::TopDown:
		targetOffset_ = {0.0f, 0.0f, 0.0f};
		break;
	case PerspectiveType::TopDownDiagonal:
		targetOffset_ = {0.0f, 0.0f, 0.0f};
		break;
	case PerspectiveType::Isometric:
		targetOffset_ = {0.0f, 0.0f, 0.0f};
		break;
	case PerspectiveType::ThirdPerson:
		targetOffset_ = {0.0f, 0.0f, 0.0f};
		break;
	case PerspectiveType::FirstPerson:
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
	camera_->rotation_.x = 0.15f;
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