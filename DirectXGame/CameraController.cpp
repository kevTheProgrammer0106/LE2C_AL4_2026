#define NOMINMAX
#include "CameraController.h"

#include "Global.h"
#include "GlobalVariables.h"
#include "Player.h"
#include <cstdio>

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "CameraController";
}

void CameraController::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "InterpolationRate", kInterpolationRate);
	gv->AddItem(kGroupName, "VelocityBias", kVelocityBias);
	gv->AddItem(kGroupName, "SmoothTimeX", kSmoothTimeX);
	gv->AddItem(kGroupName, "SmoothTimeY", kSmoothTimeY);
	gv->AddItem(kGroupName, "LeadSmoothing", kLeadSmoothing);
	gv->AddItem(kGroupName, "LineTopOffset", kLineTopOffset);
	gv->AddItem(kGroupName, "LineBottomOffset", kLineBottomOffset);
	gv->AddItem(kGroupName, "ReturnMargin", kReturnMargin);
	gv->AddItem(kGroupName, "PredictTime", kPredictTime);
	gv->AddItem(kGroupName, "FastSmoothTimeY", kFastSmoothTimeY);
	gv->AddItem(kGroupName, "FastFallSpeed", kFastFallSpeed);
	gv->AddItem(kGroupName, "VerticalLeadScale", kVerticalLeadScale);
	gv->AddItem(kGroupName, "MaxVerticalLead", kMaxVerticalLead);
	gv->AddItem(kGroupName, "FallTrackDrop", kFallTrackDrop);
	gv->AddItem(kGroupName, "FallMinSpeed", kFallMinSpeed);
	gv->AddItem(kGroupName, "FastPredictTime", kFastPredictTime);
	gv->AddItem(kGroupName, "CatchUpPx", kCatchUpPx);
	gv->AddItem(kGroupName, "HardTopMargin", kHardTopMargin);
	gv->AddItem(kGroupName, "HardBottomMargin", kHardBottomMargin);
}
void CameraController::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kInterpolationRate = gv->GetFloatValue(kGroupName, "InterpolationRate");
	kVelocityBias = gv->GetFloatValue(kGroupName, "VelocityBias");
	kSmoothTimeX = gv->GetFloatValue(kGroupName, "SmoothTimeX");
	kSmoothTimeY = gv->GetFloatValue(kGroupName, "SmoothTimeY");
	kLeadSmoothing = gv->GetFloatValue(kGroupName, "LeadSmoothing");
	kLineTopOffset = gv->GetFloatValue(kGroupName, "LineTopOffset");
	kLineBottomOffset = gv->GetFloatValue(kGroupName, "LineBottomOffset");
	kReturnMargin = gv->GetFloatValue(kGroupName, "ReturnMargin");
	kPredictTime = gv->GetFloatValue(kGroupName, "PredictTime");
	kFastSmoothTimeY = gv->GetFloatValue(kGroupName, "FastSmoothTimeY");
	kFastFallSpeed = gv->GetFloatValue(kGroupName, "FastFallSpeed");
	kVerticalLeadScale = gv->GetFloatValue(kGroupName, "VerticalLeadScale");
	kMaxVerticalLead = gv->GetFloatValue(kGroupName, "MaxVerticalLead");
	kFallTrackDrop = gv->GetFloatValue(kGroupName, "FallTrackDrop");
	kFallMinSpeed = gv->GetFloatValue(kGroupName, "FallMinSpeed");
	kFastPredictTime = gv->GetFloatValue(kGroupName, "FastPredictTime");
	kCatchUpPx = gv->GetFloatValue(kGroupName, "CatchUpPx");
	kHardTopMargin = gv->GetFloatValue(kGroupName, "HardTopMargin");
	kHardBottomMargin = gv->GetFloatValue(kGroupName, "HardBottomMargin");
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
	    wt.translation_.x + targetOffset_.x + smoothedVelocity_.x * kVelocityBias,
	    wt.translation_.y + targetOffset_.y + smoothedVelocity_.y * kVelocityBias,
	    wt.translation_.z + targetOffset_.z + smoothedVelocity_.z * kVelocityBias,
	};
}

void CameraController::UpdateCore() {
	UpdateZoom();
	if (target_)
		UpdateMeasuredVelocity(target_->GetWorldTransform().translation_);
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
	smoothedVelocity_ = Lerp(smoothedVelocity_, measuredVelocity_, 1.0f - std::exp(-kLeadSmoothing * (1.0f / 60.0f)));
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

void CameraController::Update() {
	RemoveFx(); // follow logic must see the clean camera, not last frame's shaken one
	UpdateCore();
	UpdateFx();
	ApplyFx();
	camera_->UpdateMatrix();
}

// ---------------- Shake / zoom pulse ----------------
void CameraController::Shake(float amplitude, float duration, float frequency) {
	if (amplitude <= 0.0f || duration <= 0.0f)
		return;
	float current = 0.0f;
	if (shakeTimer_ > 0.0f) {
		const float f = shakeTimer_ / shakeDuration_;
		current = shakeAmplitude_ * f * f;
	}
	if (amplitude < current)
		return; // a stronger shake is still running
	shakeAmplitude_ = amplitude;
	shakeDuration_ = duration;
	shakeTimer_ = duration;
	shakeFrequency_ = frequency;
}

void CameraController::ZoomPulse(float zoomRatio, float inDuration, float outDuration) {
	pulsePeak_ = zoomRatio; // fraction of the current camera distance
	pulseIn_ = std::max(inDuration, 0.0001f);
	pulseOut_ = std::max(outDuration, 0.0001f);
	pulseTimer_ = 0.0f;
	pulseActive_ = true;
}

void CameraController::UpdateFx() {
	constexpr float kDt = 1.0f / 60.0f;
	Vector3 offset{};
	float roll = 0.0f;

	// Camera distance: zone/arena cameras sit much farther back, so shake and zoom scale with it to look the same on screen
	const float targetZ = target_ ? target_->GetWorldTransform().translation_.z : 0.0f;
	const float distance = std::max(std::abs(camera_->translation_.z - targetZ), 1.0f);
	const float distScale = distance / kShakeReferenceDistance;

		// ---- zoom: a fraction of the CURRENT camera distance, added after every camera mode (follow / arena focus / forced scroll) ----
	const float followDist = std::max(std::abs(targetOffset_.z), 1.0f);
	const float zoomRatio = std::clamp(zoomDelta_ / followDist, -kMaxZoomOutRatio, 0.95f);
	float zoomOffset = zoomRatio * distance;
	zoomOffset = std::min(zoomOffset, std::max(distance - kMinCameraDistance, 0.0f)); // never zoom through the player

	if (shakeTimer_ > 0.0f) {
		shakeTimer_ = std::max(0.0f, shakeTimer_ - kDt);
		shakeTime_ += kDt;
		const float f = shakeTimer_ / shakeDuration_;
		const float amp = shakeAmplitude_ * f * f; // quadratic falloff: violent first, then settles
		const float p = shakeTime_ * shakeFrequency_;
		offset.x = amp * (std::sin(p) * 0.6f + std::sin(p * 2.3f + 1.7f) * 0.4f) * distScale;
		offset.y = amp * (std::sin(p * 1.3f + 0.9f) * 0.6f + std::sin(p * 2.9f + 0.4f) * 0.4f) * 0.8f * distScale;
		roll = amp * 0.03f * std::sin(p * 1.1f + 2.1f);
	}

	if (pulseActive_) {
		pulseTimer_ += kDt;
		float ratio = 0.0f;
		if (pulseTimer_ >= pulseIn_ + pulseOut_) {
			pulseActive_ = false;
		} else if (pulseTimer_ < pulseIn_) {
			ratio = pulsePeak_ * ApplyEase(Ease::kOut, pulseTimer_ / pulseIn_);
		} else {
			ratio = pulsePeak_ * (1.0f - ApplyEase(Ease::kInOut, (pulseTimer_ - pulseIn_) / pulseOut_));
		}
		pulseDelta_ = ratio * distance;
	} else {
		pulseDelta_ = 0.0f;
	}
	offset.z = zoomOffset + pulseDelta_; // +z = toward the scene

	fxOffset_ = offset;
	fxRoll_ = roll;
}

void CameraController::ApplyFx() {
	camera_->translation_ += fxOffset_;
	camera_->rotation_.z += fxRoll_;
	appliedOffset_ = fxOffset_;
	appliedRoll_ = fxRoll_;
}

void CameraController::RemoveFx() {
	if (!camera_)
		return;
	camera_->translation_ -= appliedOffset_;
	camera_->rotation_.z -= appliedRoll_;
	appliedOffset_ = {};
	appliedRoll_ = 0.0f;
}

// ---------------- Zoom ----------------
void CameraController::ZoomTo(float zoomDelta, float duration, Ease /*ease: the spring has its own curve*/) {
	zoomTarget_ = zoomDelta;
	zoomSmoothTime_ = std::max(duration * kZoomSettleFactor, 0.02f); // keeps zoomVel_, so interrupting a zoom never snaps
}

void CameraController::UpdateZoom() {
	zoomDelta_ = SmoothDamp(zoomDelta_, zoomTarget_, zoomVel_, zoomSmoothTime_, 1.0f / 60.0f);
	if (std::abs(zoomDelta_ - zoomTarget_) < 0.001f && std::abs(zoomVel_) < 0.001f) {
		zoomDelta_ = zoomTarget_;
		zoomVel_ = 0.0f;
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
			camVelX_ = camVelY_ = 0.0f; // follow takes over from rest, no leftover spring velocity
		}
		break;
	}
	}
}

void CameraController::Reset() {
	if (mode_ == Mode::kFocus) {
		mode_ = modeBeforeFocus_;
	}
	zoomDelta_ = zoomTarget_ = zoomVel_ = 0.0f;

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
	ResetSmoothing();

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



void CameraController::DrawDebugControls() {
#ifdef _DEBUG
	{
		ImGui::Text("Vertical: %s", verticalMode_ == VerticalMode::kTrack ? "TRACK" : "HOLD");
	}
	ImGui::Text("Press P to toggle the debug camera");
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
		UpdateSidescroll(targetWorldTransform);
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

#pragma region Camera damping & smoothing
// Same clamps the follow camera applies, so returning from a focus shot lands exactly where follow mode will be.
Vector3 CameraController::ComputeClampedFollowPosition() const {
	Vector3 p = ComputeFollowTarget();
	if (perspectiveType != PerspectiveType::Sidescroll)
		return p;
	const WorldTransform& wt = target_->GetWorldTransform();
	p.x = ClampCameraX(p.x, wt.translation_.x);
	p.y = ClampCameraY((verticalMode_ == VerticalMode::kTrack) ? wt.translation_.y + targetOffset_.y : holdY_);
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


// Critically damped spring (frame-rate independent, keeps velocity so direction changes stay smooth).
float CameraController::SmoothDamp(float current, float target, float& velocity, float smoothTime, float dt) {
	smoothTime = std::max(smoothTime, 0.0001f);
	const float omega = 2.0f / smoothTime;
	const float x = omega * dt;
	const float expo = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
	const float change = current - target;
	const float temp = (velocity + omega * change) * dt;
	velocity = (velocity - omega * temp) * expo;
	float output = target + (change + temp) * expo;
	if ((target - current > 0.0f) == (output > target)) { // overshoot guard
		output = target;
		velocity = 0.0f;
	}
	return output;
}

float CameraController::GetPlayerScreenY(float cameraY, float playerY) const {
	const float depth = std::abs(camera_->translation_.z - target_->GetWorldTransform().translation_.z);
	const float halfH = std::max(std::tan(camera_->fovAngleY * 0.5f) * depth, 0.01f);
	const float n = (playerY - cameraY) / halfH; // +1 = top edge, -1 = bottom edge
	return (1.0f - n) * 0.5f * static_cast<float>(WinApp::kWindowHeight);
}
float CameraController::CameraYForPlayerScreenY(float playerY, float screenY) const {
	const float depth = std::abs(camera_->translation_.z - target_->GetWorldTransform().translation_.z);
	const float halfH = std::max(std::tan(camera_->fovAngleY * 0.5f) * depth, 0.01f);
	const float n = 1.0f - 2.0f * screenY / static_cast<float>(WinApp::kWindowHeight); // +1 top edge, -1 bottom edge
	return playerY - n * halfH;
}

float CameraController::ClampCameraX(float x, float playerX) const {
	x = std::clamp(x, playerX + margin.left, playerX + margin.right);
	return std::clamp(x, movableArea_.left, movableArea_.right);
}
float CameraController::ClampCameraY(float y) const { return std::clamp(y, movableArea_.bottom, movableArea_.top); }

void CameraController::ResetSmoothing() {
	camVelX_ = camVelY_ = 0.0f;
	smoothedVelocity_ = {};
	measuredVelocity_ = {};
	hasPrevTarget_ = false;
	verticalMode_ = VerticalMode::kHold;
	holdY_ = camera_->translation_.y;
	hasGroundY_ = false;
	isFalling_ = false;
}

void CameraController::UpdateMeasuredVelocity(const Vector3& pos) {
	if (!hasPrevTarget_) {
		prevTargetPos_ = pos;
		hasPrevTarget_ = true;
		measuredVelocity_ = {};
		return;
	}
	const Vector3 d = {pos.x - prevTargetPos_.x, pos.y - prevTargetPos_.y, 0.0f};
	prevTargetPos_ = pos;
	if (std::hypot(d.x, d.y) > kTeleportDistance) { // checkpoint / respawn
		measuredVelocity_ = {};
		return;
	}
	measuredVelocity_ = {std::clamp(d.x, -kMaxMeasuredSpeed, kMaxMeasuredSpeed), std::clamp(d.y, -kMaxMeasuredSpeed, kMaxMeasuredSpeed), 0.0f};
}

void CameraController::UpdateSidescroll(const WorldTransform& targetWT) {
	constexpr float kDt = 1.0f / 60.0f;
	camera_->rotation_.x = 0.0f;
	camera_->rotation_.y = 0.0f;

	const Vector3 playerPos = targetWT.translation_;
	const bool onGround = target_->IsOnGround();
	const float vy = measuredVelocity_.y;
	const float speedT = std::clamp(std::abs(vy) / std::max(kFastFallSpeed, 0.001f), 0.0f, 1.0f); // 0 = slow, 1 = fast flier/faller

	// ---- fall detection: a real drop below the last floor. Jumps land on the floor they left, so they never trigger it ----
	if (!hasGroundY_ || onGround) {
		lastGroundY_ = playerPos.y;
		hasGroundY_ = true;
	}
	const bool falling = !onGround && vy < -kFallMinSpeed && (lastGroundY_ - playerPos.y) > kFallTrackDrop;
	isFalling_ = falling;

	// ---- the two lines ----
	const float screenH = static_cast<float>(WinApp::kWindowHeight);
	const float topLine = kLineTopOffset;
	const float bottomLine = screenH - kLineBottomOffset;
	const float syNow = GetPlayerScreenY(camera_->translation_.y, playerPos.y);

	// "about to reach": the faster the player moves, the farther ahead we look
	const float horizon = Lerp(kPredictTime, kFastPredictTime, speedT);
	const float syAhead = GetPlayerScreenY(camera_->translation_.y, playerPos.y + vy * (horizon / kDt));

	if (verticalMode_ == VerticalMode::kHold) {
		const bool reached = syNow < topLine || syNow > bottomLine || syAhead < topLine || syAhead > bottomLine;
		if (reached || falling) {
			verticalMode_ = VerticalMode::kTrack;
			camVelY_ = vy * 60.0f; // start already moving with the player (SmoothDamp velocity is units/second), not from rest
		}
	} else {
		// keep following while airborne; once standing and back inside the band, hold again
		const bool insideBand = syNow >= topLine + kReturnMargin && syNow <= bottomLine - kReturnMargin;
		if (onGround && insideBand) {
			verticalMode_ = VerticalMode::kHold;
			holdY_ = ClampCameraY(playerPos.y + targetOffset_.y); // recentre on the floor we landed on (matches the spawn framing)
		}
	}

	float smoothTimeY = kSmoothTimeY;
	float desiredY = holdY_;

	if (verticalMode_ == VerticalMode::kTrack) {
		// 1) speed: tighter spring the faster we move, always tight for a real fall
		smoothTimeY = Lerp(kSmoothTimeY, kFastSmoothTimeY, std::clamp(std::abs(smoothedVelocity_.y) / std::max(kFastFallSpeed, 0.001f), 0.0f, 1.0f));
		if (falling)
			smoothTimeY = std::min(smoothTimeY, kFastSmoothTimeY);

		// 2) catch-up: the farther the player is beyond a line, the tighter the spring
		const float outsidePx = std::max(0.0f, std::max(topLine - syNow, syNow - bottomLine));
		smoothTimeY /= 1.0f + outsidePx / std::max(kCatchUpPx, 1.0f);
		smoothTimeY = std::max(smoothTimeY, 0.04f);

		// 3) lead: velocity * smoothTime cancels the spring's steady-state lag
		const float lead = std::clamp(smoothedVelocity_.y * 60.0f * smoothTimeY * kVerticalLeadScale, -kMaxVerticalLead, kMaxVerticalLead);
		desiredY = playerPos.y + targetOffset_.y + lead;
	}
	desiredY = ClampCameraY(desiredY);

	// ---- damped follow ----
	camera_->translation_.x = SmoothDamp(camera_->translation_.x, targetPos.x, camVelX_, kSmoothTimeX, kDt);
	camera_->translation_.y = SmoothDamp(camera_->translation_.y, desiredY, camVelY_, smoothTimeY, kDt);
	camera_->translation_.z = Lerp(camera_->translation_.z, targetPos.z, kInterpolationRate); // zoom keeps its existing feel

	//// ---- hard limit while tracking: the player can never leave the screen, even at terminal speed ----
	//if (verticalMode_ == VerticalMode::kTrack) {
	//	const float camMaxY = CameraYForPlayerScreenY(playerPos.y, kHardTopMargin);              // camera high enough that the player stays below the top margin
	//	const float camMinY = CameraYForPlayerScreenY(playerPos.y, screenH - kHardBottomMargin); // ...and above the bottom margin
	//	if (camera_->translation_.y > camMaxY) {
	//		camera_->translation_.y = camMaxY;
	//		camVelY_ = std::max(camVelY_, vy * 60.0f);
	//	} else if (camera_->translation_.y < camMinY) {
	//		camera_->translation_.y = camMinY;
	//		camVelY_ = std::min(camVelY_, vy * 60.0f);
	//	}
	//}

	camera_->translation_.x = ClampCameraX(camera_->translation_.x, playerPos.x);
	camera_->translation_.y = ClampCameraY(camera_->translation_.y);
}

#pragma endregion

void CameraController::DrawDebugLines() const {
#ifdef _DEBUG
	if (!camera_)
		return;
	ImDrawList* dl = ImGui::GetForegroundDrawList();
	const float w = static_cast<float>(WinApp::kWindowWidth);
	const float h = static_cast<float>(WinApp::kWindowHeight);
	const float top = kLineTopOffset;
	const float bottom = h - kLineBottomOffset;
	const bool tracking = (verticalMode_ == VerticalMode::kTrack);

	const ImU32 lineCol = tracking ? IM_COL32(80, 255, 140, 230) : IM_COL32(255, 200, 0, 230);
	const ImU32 returnCol = IM_COL32(255, 255, 255, 70);

	dl->AddLine({0.0f, top}, {w, top}, lineCol, 2.0f);
	dl->AddLine({0.0f, bottom}, {w, bottom}, lineCol, 2.0f);
	// the player must get back inside these before the camera returns to HOLD
	dl->AddLine({0.0f, top + kReturnMargin}, {w, top + kReturnMargin}, returnCol, 1.0f);
	dl->AddLine({0.0f, bottom - kReturnMargin}, {w, bottom - kReturnMargin}, returnCol, 1.0f);

	char buf[96];
	std::snprintf(buf, sizeof(buf), "TOP LINE  %.0fpx   (%s)", top, tracking ? "TRACK" : "HOLD");
	dl->AddText({8.0f, top + 4.0f}, lineCol, buf);
	std::snprintf(buf, sizeof(buf), "BOTTOM LINE  %.0fpx from bottom (y=%.0f)", kLineBottomOffset, bottom);
	dl->AddText({8.0f, bottom - 18.0f}, lineCol, buf);

	if (target_) {
		const float sy = GetPlayerScreenY(camera_->translation_.y, target_->GetWorldTransform().translation_.y);
		dl->AddCircleFilled({w - 24.0f, sy}, 6.0f, IM_COL32(255, 80, 80, 255));
		std::snprintf(buf, sizeof(buf), "player y=%.0f  vy=%.2f", sy, measuredVelocity_.y);
		dl->AddText({w - 190.0f, sy - 8.0f}, IM_COL32(255, 120, 120, 255), buf);
	}
#endif
}