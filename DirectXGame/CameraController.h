#pragma once

#define NOMINMAX
#include "Global.h"
#include "KamataEngine.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <vector>

class Player;

class CameraController {
public:
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};

	enum class Mode { kFollow, kForcedScroll, kFocus };
	enum class Ease { kLinear, kIn, kOut, kInOut };

	enum class PerspectiveType { Sidescroll, Fixed, TopDown, TopDownDiagonal, Isometric, ThirdPerson, FirstPerson, Count };
	PerspectiveType perspectiveType = PerspectiveType::Sidescroll;

	void Initialize(KamataEngine::Camera* camera);
	void Update();

	void SetTarget(Player* target) { target_ = target; }

	Rect movableArea_ = {};
	void SetMovableArea(Rect area) { movableArea_ = area; }
	void SetWorldBounds(float worldWidth, float worldHeight);

	void Reset();
	void DrawDebugControls(); // ImGui contents only; the caller owns the window

	void UpdateFixedCamera(const KamataEngine::WorldTransform& worldTransformTarget);
	void UpdateTopDownCamera(const KamataEngine::WorldTransform& worldTransformTarget);
	void UpdateTopDownDiagonalCamera(const KamataEngine::WorldTransform& worldTransformTarget);
	void UpdateIsometricCamera(const KamataEngine::WorldTransform& worldTransformTarget);
	void UpdateThirdPersonCamera(const KamataEngine::WorldTransform& worldTransformTarget);
	void UpdateFirstPersonCamera(const KamataEngine::WorldTransform& worldTransformTarget);

	void SetMode(Mode mode) { mode_ = mode; }
	Mode GetMode() const { return mode_; }

	void UpdateCameraPerspectives(const KamataEngine::WorldTransform& targetWorldTransform);
	void ResetCameraPerspectives();

	float GetScrollLeftBoundary() const {
		float halfScreenWidth = std::tan(camera_->fovAngleY / 2.0f) * std::abs(targetOffset_.z) * camera_->aspectRatio;
		return scrollX_ - halfScreenWidth + kPushBoundaryOffset;
	}
	void SetPushBoundaryOffset(float offset) { kPushBoundaryOffset = offset; }

	// ============================================================
	// Focus shot: fly to a world position, hold, fly back.
	// `position` is the FINAL camera position (mapChip position + offset, offset.z = zoom distance).
	// ============================================================
	struct FocusShot {
		KamataEngine::Vector3 position{};
		float enterDuration = 0.8f;
		float holdDuration = 2.0f; // ignored when holdUntilReleased
		float returnDuration = 0.8f;
		Ease enterEase = Ease::kInOut;
		Ease returnEase = Ease::kInOut;
		bool holdUntilReleased = false; // stay until ReleaseFocus() is called
	};
	void StartFocus(const FocusShot& shot);
	void ReleaseFocus(); // start the return now (no-op if not focusing / already returning)
	bool IsFocusActive() const { return mode_ == Mode::kFocus; }
	bool IsFocusHolding() const { return mode_ == Mode::kFocus && focusPhase_ == FocusPhase::kHold; }
	bool IsFocusReturning() const { return mode_ == Mode::kFocus && focusPhase_ == FocusPhase::kReturn; }

	// ============================================================
	// Zoom: additive Z offset on top of the follow offset. Positive = zoom in, negative = zoom out.
	// Works in follow / forced-scroll modes (focus shots set their own distance).
	// ============================================================
	void ZoomTo(float zoomDelta, float duration, Ease ease = Ease::kInOut);
	void ResetZoom(float duration, Ease ease = Ease::kInOut) { ZoomTo(0.0f, duration, ease); }
	float GetZoomDelta() const { return zoomDelta_; }

	static float ApplyEase(Ease ease, float t);

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	// ============================================================
	// Zones. ONE chip does everything: an 'F' chip is the zoom point AND the centre of its own trigger area.
	// While the player overlaps the area the camera eases to (chip position + offset) and holds there; when the player
	// leaves it eases back. Re-entering reactivates it after `reentryCooldown` seconds of continuous presence
	// (the very first entry is immediate).
	// The 'F' chip's SUB ID (2nd char) picks the preset in GetCameraConfig, e.g. "F10" = preset 1.
	// ============================================================
	struct ZoneConfig {
		KamataEngine::Vector3 offset{0.0f, 0.0f, -30.0f}; // added to the chip position (z = zoom distance)
		float areaWidth = 20.0f;                          // trigger area size in world units (1 tile = 1), centred on the chip
		float areaHeight = 12.0f;
		float enterDuration = 0.8f;
		float returnDuration = 0.8f;
		Ease enterEase = Ease::kInOut;
		Ease returnEase = Ease::kInOut;
		float reentryCooldown = 1.0f;
	};
	struct Zone {
		KamataEngine::Vector3 focusPoint{}; // the 'F' chip position: zoom point and centre of the trigger area
		ZoneConfig config;
		bool active = false;
		bool hasActivated = false;
		float enterTimer = 0.0f;
		bool playerInside = false; // debug readout
	};

	// Camera presets. stageIndex -1 = every stage. A stage-specific entry wins over -1. No entry = ZoneConfig defaults.
	struct ZonePreset {
		int32_t stageIndex = -1;
		uint8_t presetID = 0;
		ZoneConfig config;
	};
	static ZoneConfig GetCameraConfig(int32_t stageIndex, uint8_t presetID);

	// The trigger area of a zone (also used by the debug draw).
	static AABB GetZoneArea(const Zone& zone);

	void AddZone(const KamataEngine::Vector3& focusPoint, const ZoneConfig& config);
	void RemoveZoneDataInRange(float minX, float maxX); // streaming unload
	// Call every frame. enabled=false releases the active zone (death etc.).
	void UpdateZones(const AABB& playerAABB, bool enabled);
	void ReleaseActiveZone();
	// Starts a zone's shot without the player being inside it. Held until ReleaseActiveZone().
	bool ForceZone(uint32_t zoneKey);

	const std::map<uint32_t, Zone>& GetZones() const { return zones_; }
	int32_t GetActiveZoneID() const { return activeZoneID_; }

private:
	std::map<uint32_t, Zone> zones_; // key = creation order, not a map ID
	uint32_t nextZoneKey_ = 0;
	int32_t activeZoneID_ = -1;
	bool zoneForced_ = false;
	void BeginZoneShot(Zone& zone, uint32_t zoneKey);
	static const std::vector<ZonePreset>& GetZonePresets();
	KamataEngine::Vector3 ComputeClampedFollowPosition() const;

	enum class FocusPhase { kEnter, kHold, kReturn };
	FocusPhase focusPhase_ = FocusPhase::kEnter;
	FocusShot shot_{};
	KamataEngine::Vector3 focusStartPos_{};
	float focusTimer_ = 0.0f;
	Mode modeBeforeFocus_ = Mode::kFollow;
	void UpdateFocusCamera();
	void BeginFocusReturn();

	KamataEngine::Vector3 ComputeFollowTarget() const;

	float zoomDelta_ = 0.0f;
	float zoomStart_ = 0.0f;
	float zoomTarget_ = 0.0f;
	float zoomTimer_ = 0.0f;
	float zoomDuration_ = 0.0f;
	Ease zoomEase_ = Ease::kInOut;
	bool zoomActive_ = false;
	void UpdateZoom();

	float scrollX_ = 0.0f;
	float scrollSpeed_ = 0.025f;
	float kPushBoundaryOffset = -6.5f;
	KamataEngine::Vector3 targetOffset_ = {0.0f, 0.0f, -15.0f};
	KamataEngine::Vector3 targetPos = {};
	KamataEngine::Camera* camera_ = nullptr;
	Player* target_ = nullptr;
	static inline float kInterpolationRate = 0.1f;
	static inline float kVelocityBias = 30.0f;
	static inline const Rect margin = {-10, 10, 0, 0};

	Mode mode_ = Mode::kFollow;
};