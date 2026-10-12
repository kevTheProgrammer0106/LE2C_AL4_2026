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

float CameraController::ApplyEase(Ease ease, float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	switch (ease) {
	case Ease::kIn:
		return EaseIn(0.0f, 1.0f, t);
	case Ease::kOut:
		return EaseOut(0.0f, 1.0f, t);
	case Ease::kInOut:
		return EaseInOut(0.0f, 1.0f, t);
	case Ease::kLinear:
	default:
		return t;
	}
}

Vector3 CameraController::ComputeFollowTarget() const {
	const WorldTransform& wt = target_->GetWorldTransform();
	return {
	    wt.translation_.x + targetOffset_.x + target_->GetVelocity().x * kVelocityBias,
	    wt.translation_.y + targetOffset_.y + target_->GetVelocity().y * kVelocityBias,
	    wt.translation_.z + targetOffset_.z + target_->GetVelocity().z * kVelocityBias + zoomDelta_,
	};
}

void CameraController::Update() {
	UpdateZoom();

	if (mode_ == Mode::kFocus) {
		UpdateFocusCamera();
		camera_->UpdateMatrix();
		return;
	}
	if (!target_) {
		camera_->UpdateMatrix();
		return;
	}

	const WorldTransform& targetWT = target_->GetWorldTransform();
	targetPos = ComputeFollowTarget();

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

// ---------------- Zoom ----------------
void CameraController::ZoomTo(float zoomDelta, float duration, Ease ease) {
	zoomStart_ = zoomDelta_;
	zoomTarget_ = zoomDelta;
	zoomDuration_ = std::max(duration, 0.0001f);
	zoomTimer_ = 0.0f;
	zoomEase_ = ease;
	zoomActive_ = true;
}

void CameraController::UpdateZoom() {
	if (!zoomActive_) {
		return;
	}
	zoomTimer_ += 1.0f / 60.0f;
	float t = std::clamp(zoomTimer_ / zoomDuration_, 0.0f, 1.0f);
	zoomDelta_ = Lerp(zoomStart_, zoomTarget_, ApplyEase(zoomEase_, t));
	if (t >= 1.0f) {
		zoomDelta_ = zoomTarget_;
		zoomActive_ = false;
	}
}

// ---------------- Focus shot ----------------
void CameraController::StartFocus(const FocusShot& shot) {
	if (mode_ != Mode::kFocus) {
		modeBeforeFocus_ = mode_; // restored when the shot ends (e.g. forced-scroll)
	}
	shot_ = shot;
	shot_.enterDuration = std::max(shot.enterDuration, 0.0001f);
	shot_.returnDuration = std::max(shot.returnDuration, 0.0001f);
	focusStartPos_ = camera_->translation_;
	focusTimer_ = 0.0f;
	focusPhase_ = FocusPhase::kEnter;
	mode_ = Mode::kFocus;
}

void CameraController::BeginFocusReturn() {
	focusStartPos_ = camera_->translation_;
	focusTimer_ = 0.0f;
	focusPhase_ = FocusPhase::kReturn;
}

void CameraController::ReleaseFocus() {
	if (mode_ == Mode::kFocus && focusPhase_ != FocusPhase::kReturn) {
		BeginFocusReturn();
	}
}

void CameraController::UpdateFocusCamera() {
	camera_->rotation_.x = 0.0f;
	camera_->rotation_.y = 0.0f;
	constexpr float kDt = 1.0f / 60.0f;

	switch (focusPhase_) {
	case FocusPhase::kEnter: {
		focusTimer_ += kDt;
		float t = std::clamp(focusTimer_ / shot_.enterDuration, 0.0f, 1.0f);
		camera_->translation_ = Lerp(focusStartPos_, shot_.position, ApplyEase(shot_.enterEase, t));
		if (t >= 1.0f) {
			focusPhase_ = FocusPhase::kHold;
			focusTimer_ = 0.0f;
		}
		break;
	}
	case FocusPhase::kHold:
		camera_->translation_ = shot_.position;
		if (!shot_.holdUntilReleased) {
			focusTimer_ += kDt;
			if (focusTimer_ >= shot_.holdDuration) {
				BeginFocusReturn();
			}
		}
		break;
	case FocusPhase::kReturn: {
		if (!target_) {
			mode_ = modeBeforeFocus_;
			break;
		}
		focusTimer_ += kDt;
		float t = std::clamp(focusTimer_ / shot_.returnDuration, 0.0f, 1.0f);
		// target is recomputed each frame (with the same clamps follow mode uses) so the camera lands exactly where follow takes over
		camera_->translation_ = Lerp(focusStartPos_, ComputeClampedFollowPosition(), ApplyEase(shot_.returnEase, t));
		if (t >= 1.0f) {
			mode_ = modeBeforeFocus_;
		}
		break;
	}
	}
}

void CameraController::Reset() {
	if (mode_ == Mode::kFocus) {
		mode_ = modeBeforeFocus_;
	}
	zoomDelta_ = 0.0f;
	zoomActive_ = false;

	const WorldTransform& targetWT = target_->GetWorldTransform();

	switch (mode_) {
	case Mode::kFollow:
		ResetCameraPerspectives();
		break;
	case Mode::kForcedScroll:
		targetOffset_ = {0.0f, 0.0f, -15.0f};
		scrollX_ = movableArea_.left;
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

void CameraController::DrawDebugControls() {
#ifdef _DEBUG
	ImGui::Text("Press P to toggle the debug camera");
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
	ImGui::BulletText("Zoom (+in / -out)");
	static float zoomDelta = 5.0f;
	ImGui::DragFloat("Zoom Delta", &zoomDelta, 0.1f, -30.0f, 30.0f);
	if (ImGui::Button("Zoom"))
		ZoomTo(zoomDelta, 0.6f);
	ImGui::SameLine();
	if (ImGui::Button("Reset Zoom"))
		ResetZoom(0.6f);
	ImGui::Text("Focus active: %s", IsFocusActive() ? "yes" : "no");
	if (ImGui::Button("Release Focus"))
		ReleaseFocus();

	ImGui::Separator();
	ImGui::BulletText("Camera Zones");
	ImGui::Text("Follow z offset: %.1f   Camera z: %.1f", targetOffset_.z, camera_->translation_.z);
	if (zones_.empty())
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f), "No zones loaded (no F chips generated)");
	for (auto& [key, z] : zones_) {
		ImGui::PushID(static_cast<int>(key));
		ImGui::Text(
		    "Zone #%d at (%.0f, %.0f)  area %.0f x %.0f  inside=%s  entered=%.2fs  active=%s", static_cast<int>(key), z.focusPoint.x, z.focusPoint.y, z.config.areaWidth, z.config.areaHeight,
		    z.playerInside ? "yes" : "no", z.enterTimer, z.active ? "yes" : "no");
		ImGui::Text("  shot camera z: %.1f", z.focusPoint.z + z.config.offset.z);
		if (ImGui::Button("Force zoom"))
			ForceZone(key);
		ImGui::SameLine();
		if (ImGui::Button("Release"))
			ReleaseActiveZone();
		ImGui::PopID();
	}
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
		targetOffset_ = {0.0f, 0.0f, -18.5f}; 
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
// Same clamps the follow camera applies, so returning from a focus shot lands exactly where follow mode will be.
Vector3 CameraController::ComputeClampedFollowPosition() const {
	Vector3 p = ComputeFollowTarget();
	if (perspectiveType != PerspectiveType::Sidescroll)
		return p;
	const WorldTransform& wt = target_->GetWorldTransform();
	p.x = std::clamp(p.x, wt.translation_.x + margin.left, wt.translation_.x + margin.right);
	p.y = std::clamp(p.y, wt.translation_.y + margin.bottom, wt.translation_.y + margin.top);
	p.x = std::clamp(p.x, movableArea_.left, movableArea_.right);
	p.y = std::clamp(p.y, movableArea_.bottom, movableArea_.top);
	return p;
}

// ---------------- Zone presets ----------------
const std::vector<CameraController::ZonePreset>& CameraController::GetZonePresets() {
	// areaWidth / areaHeight = the trigger area centred on the 'F' chip (world units, 1 tile = 1). Placeholder sizes: set them per room.
	static const std::vector<ZonePreset> presets = {
	    {.stageIndex = -1, .presetID = 0, .config = {.offset = {0.0f, 0.0f, -35.0f}, .areaWidth = 24.0f, .areaHeight = 14.0f, .enterDuration = 1.0f, .returnDuration = 1.0f, .reentryCooldown = 1.0f}},
	    {.stageIndex = -1, .presetID = 1, .config = {.offset = {0.0f, 0.0f, -35.0f}, .areaWidth = 20.0f, .areaHeight = 12.0f}                                                                        },
	    {.stageIndex = -1, .presetID = 2, .config = {.offset = {0.0f, 0.0f, -35.0f}, .areaWidth = 20.0f, .areaHeight = 12.0f, .enterEase = Ease::kOut}                                               },
	    // stage-specific override example:
	    // {.stageIndex = 2, .presetID = 1, .config = {.offset = {0.0f, 2.0f, -40.0f}, .areaWidth = 30.0f, .areaHeight = 16.0f}},
	};
	return presets;
}

CameraController::ZoneConfig CameraController::GetCameraConfig(int32_t stageIndex, uint8_t presetID) {
	const ZonePreset* anyStage = nullptr;
	for (const ZonePreset& p : GetZonePresets()) {
		if (p.presetID != presetID)
			continue;
		if (p.stageIndex == stageIndex)
			return p.config; // exact stage wins
		if (p.stageIndex < 0)
			anyStage = &p;
	}
	return anyStage ? anyStage->config : ZoneConfig{}; // unknown preset: -30 offset, 20 x 12 area, 0.8s, 1.0s cooldown
}

// ---------------- Zones ----------------
AABB CameraController::GetZoneArea(const Zone& zone) {
	const float halfW = zone.config.areaWidth * 0.5f;
	const float halfH = zone.config.areaHeight * 0.5f;
	AABB area;
	area.min = {zone.focusPoint.x - halfW, zone.focusPoint.y - halfH, -1.0f};
	area.max = {zone.focusPoint.x + halfW, zone.focusPoint.y + halfH, 1.0f};
	return area;
}

void CameraController::AddZone(const Vector3& focusPoint, const ZoneConfig& config) {
	for (auto& [key, z] : zones_) {
		if (z.focusPoint.x == focusPoint.x && z.focusPoint.y == focusPoint.y) {
			z.config = config; // already registered
			return;
		}
	}
	Zone zone;
	zone.focusPoint = focusPoint;
	zone.config = config;
	zones_.emplace(nextZoneKey_++, zone);
}

void CameraController::RemoveZoneDataInRange(float minX, float maxX) {
	for (auto it = zones_.begin(); it != zones_.end();) {
		float x = it->second.focusPoint.x;
		if (x >= minX && x < maxX) {
			if (static_cast<int32_t>(it->first) == activeZoneID_) {
				activeZoneID_ = -1;
				zoneForced_ = false;
				ReleaseFocus(); // its chip is being unloaded: ease back
			}
			it = zones_.erase(it);
		} else {
			++it;
		}
	}
}

void CameraController::BeginZoneShot(Zone& zone, uint32_t zoneKey) {
	FocusShot shot;
	shot.position = zone.focusPoint + zone.config.offset;
	shot.enterDuration = zone.config.enterDuration;
	shot.returnDuration = zone.config.returnDuration;
	shot.enterEase = zone.config.enterEase;
	shot.returnEase = zone.config.returnEase;
	shot.holdUntilReleased = true; // held until the player leaves the area
	StartFocus(shot);

	zone.active = true;
	zone.hasActivated = true;
	activeZoneID_ = static_cast<int32_t>(zoneKey);
}

bool CameraController::ForceZone(uint32_t zoneKey) {
	auto it = zones_.find(zoneKey);
	if (it == zones_.end())
		return false;
	ReleaseActiveZone();
	BeginZoneShot(it->second, zoneKey);
	zoneForced_ = true; // UpdateZones won't release it just because the player isn't inside
	return true;
}

void CameraController::ReleaseActiveZone() {
	zoneForced_ = false;
	if (activeZoneID_ < 0)
		return;
	auto it = zones_.find(static_cast<uint32_t>(activeZoneID_));
	if (it != zones_.end())
		it->second.active = false;
	activeZoneID_ = -1;
	ReleaseFocus(); // eases back to the follow camera
}

void CameraController::UpdateZones(const AABB& playerAABB, bool enabled) {
	constexpr float kDt = 1.0f / 60.0f;

	if (!enabled) {
		ReleaseActiveZone();
		return;
	}
	// Focus was cancelled from elsewhere (Reset, debug camera): drop the stale ownership.
	if (activeZoneID_ >= 0 && !IsFocusActive()) {
		auto it = zones_.find(static_cast<uint32_t>(activeZoneID_));
		if (it != zones_.end())
			it->second.active = false;
		activeZoneID_ = -1;
		zoneForced_ = false;
	}

	for (auto& [key, zone] : zones_) {
		const bool inside = IsCollisionAABB(playerAABB, GetZoneArea(zone));
		zone.playerInside = inside;

		if (!inside) {
			zone.enterTimer = 0.0f; // leaving resets the re-entry timer
			if (static_cast<int32_t>(key) == activeZoneID_ && !zoneForced_)
				ReleaseActiveZone();
			continue;
		}

		if (activeZoneID_ >= 0)
			continue; // another zone owns the camera

		zone.enterTimer += kDt;
		if (zone.hasActivated && zone.enterTimer < zone.config.reentryCooldown)
			continue; // re-entry cooldown

		BeginZoneShot(zone, key);
	}
}
#pragma endregion