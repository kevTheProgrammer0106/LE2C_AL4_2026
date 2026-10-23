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

	enum class PerspectiveType { Sidescroll, Count };
	PerspectiveType perspectiveType = PerspectiveType::Sidescroll;

	void Initialize(KamataEngine::Camera* camera);
	void Update();

	void SetTarget(Player* target) { target_ = target; }

	Rect movableArea_ = {};
	void SetMovableArea(Rect area) { movableArea_ = area; }
	void SetWorldBounds(float worldWidth, float worldHeight);

	void Reset();
	void DrawDebugControls(); // ImGui contents only; the caller owns the window

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

		// ---- Shake (explosions / damage / death) ----
	// amplitude = peak offset in world units, duration = seconds. A weaker shake never cuts a stronger one short.
	void Shake(float amplitude, float duration, float frequency = 35.0f);
	void StopShake() { shakeTimer_ = 0.0f; }
	bool IsShaking() const { return shakeTimer_ > 0.0f; }

	// ---- Zoom pulse (block): zooms in by zoomRatio of the CURRENT camera distance, then eases back. Independent of ZoomTo(). ----
	void ZoomPulse(float zoomRatio, float inDuration, float outDuration);

	// Debug overlay (ImGui foreground): vertical dead-zone lines, return margin, player screen Y, HOLD/TRACK. Call once per frame inside the ImGui frame.
	void DrawDebugLines() const;

	bool IsTrackingVertically() const { return verticalMode_ == VerticalMode::kTrack; }

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
	KamataEngine::Vector3 FocusTargetPosition() const; // shot position with the current zoom applied (scaled to the shot distance)
	void BeginFocusReturn();

	KamataEngine::Vector3 ComputeFollowTarget() const;

	float zoomStart_ = 0.0f;
	float zoomTimer_ = 0.0f;
	float zoomDuration_ = 0.0f;
	Ease zoomEase_ = Ease::kInOut;
	bool zoomActive_ = false;
	void UpdateZoom();

		float zoomDelta_ = 0.0f; // current zoom, in "follow-camera units" (positive = in, negative = out)
	float zoomTarget_ = 0.0f;
	float zoomVel_ = 0.0f;
	float zoomSmoothTime_ = 0.2f;

	static inline float kZoomSettleFactor = 0.42f; // smoothTime = duration * this -> ~95% settled by `duration`
	static inline float kMinCameraDistance = 2.0f; // the camera never zooms closer than this to the player
	static inline float kMaxZoomOutRatio = 1.5f;   // furthest zoom-out, as a fraction of the current distance

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

		void UpdateCore(); // the old Update() body (follow / focus / forced scroll)
	void UpdateFx();   // computes this frame's shake + pulse
	void ApplyFx();    // adds them on top of the camera
	void RemoveFx();   // takes last frame's back off

	float shakeAmplitude_ = 0.0f;
	float shakeDuration_ = 0.0f;
	float shakeTimer_ = 0.0f;
	float shakeFrequency_ = 35.0f;
	float shakeTime_ = 0.0f;

	float pulsePeak_ = 0.0f;
	float pulseIn_ = 0.1f;
	float pulseOut_ = 0.4f;
	float pulseTimer_ = 0.0f;
	bool pulseActive_ = false;
	float pulseDelta_ = 0.0f;

	KamataEngine::Vector3 fxOffset_{}; // wanted this frame
	float fxRoll_ = 0.0f;
	KamataEngine::Vector3 appliedOffset_{}; // actually added to the camera (so it can be removed)
	float appliedRoll_ = 0.0f;

		// ---- Sidescroll damping + vertical dead zone ----
	enum class VerticalMode { kHold, kTrack };
	VerticalMode verticalMode_ = VerticalMode::kHold;
	float holdY_ = 0.0f; // camera Y while holding
	float camVelX_ = 0.0f;
	float camVelY_ = 0.0f;
	KamataEngine::Vector3 smoothedVelocity_{}; // player velocity, low-passed, used for the look-ahead

	void UpdateSidescroll(const KamataEngine::WorldTransform& targetWT);
	void ResetSmoothing();
	float GetPlayerScreenY(float cameraY, float playerY) const; // 0 = top of screen, WinApp::kWindowHeight = bottom
	float CameraYForPlayerScreenY(float playerY, float screenY) const; // inverse of GetPlayerScreenY

	float ClampCameraX(float x, float playerX) const;
	float ClampCameraY(float y) const;

	static float SmoothDamp(float current, float target, float& velocity, float smoothTime, float dt);

	static inline float kSmoothTimeX = 0.30f;       // seconds to catch up horizontally (higher = softer)
	static inline float kSmoothTimeY = 0.35f;       // seconds to catch up vertically
	static inline float kLeadSmoothing = 6.0f;      // how quickly the look-ahead velocity follows the player
	static inline float kLineTopOffset = 150.0f;    // px below screen y 0
	static inline float kLineBottomOffset = 200.0f; // px above screen y 720
	static inline float kReturnMargin = 40.0f;      // px inside the lines the player must be to go back to holding
	static inline float kPredictTime = 0.10f;       // seconds ahead used to detect "about to reach a line"

	// player movement measured from real position change (catches knockback / attack steps / pushes, not just velocity_)
	KamataEngine::Vector3 prevTargetPos_{};
	KamataEngine::Vector3 measuredVelocity_{}; // units per frame
	bool hasPrevTarget_ = false;
	void UpdateMeasuredVelocity(const KamataEngine::Vector3& targetPos);

	static inline float kFastSmoothTimeY = 0.12f;        // vertical smooth time when moving at kFastFallSpeed or faster
	static inline float kFastFallSpeed = 0.3f;           // units/frame at which the fast smooth time is fully used
	static inline float kVerticalLeadScale = 1.0f;       // 1 = look-ahead exactly cancels the spring's lag at constant speed
	static inline float kMaxVerticalLead = 6.0f;         // world units
	static inline float kMaxMeasuredSpeed = 1.0f;        // clamp per axis (units/frame)
	static inline float kTeleportDistance = 3.0f;        // a bigger one-frame jump is a respawn/checkpoint, not movement
	static inline float kShakeReferenceDistance = 18.5f; // camera distance the shake amplitudes are authored for

	// ---- fall detection ----
	float lastGroundY_ = 0.0f; // Y the player last stood at
	bool hasGroundY_ = false;
	bool isFalling_ = false;                   // debug readout
	static inline float kFallTrackDrop = 1.2f; // drop below the last floor that starts tracking
	static inline float kFallMinSpeed = 0.08f; // units/frame downward, so hovering at the drop's start doesn't count

	static inline float kFastPredictTime = 0.22f;  // "about to reach a line" horizon at full speed (kPredictTime applies when slow)
	static inline float kCatchUpPx = 120.0f;       // px beyond a line at which the spring is about twice as tight
	static inline float kHardTopMargin = 40.0f;    // the player never goes closer than this to the top of the screen while tracking
	static inline float kHardBottomMargin = 70.0f; // ...or the bottom
};