#pragma once

#define NOMINMAX
#include "KamataEngine.h"
#include <algorithm>
#include <cmath>

class Player;

class CameraController {
public:
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};

	enum class Mode { kFollow, kForcedScroll, kArenaFocus };

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

	// --- Arena focus mode ---
	void StartArenaFocus(const KamataEngine::Vector3& focusPoint, float lerpDuration = 0.8f);
	void EndArenaFocus(float lerpDuration = 0.8f);
	bool IsArenaFocusHolding() const { return mode_ == Mode::kArenaFocus && arenaFocusPhase_ == ArenaFocusPhase::kHold; }
	bool IsArenaFocusReturning() const { return mode_ == Mode::kArenaFocus && arenaFocusPhase_ == ArenaFocusPhase::kReturn; }

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

private:
	enum class ArenaFocusPhase { kEnter, kHold, kReturn };
	ArenaFocusPhase arenaFocusPhase_ = ArenaFocusPhase::kEnter;
	KamataEngine::Vector3 arenaFocusPoint_{};
	KamataEngine::Vector3 arenaFocusStartPos_{};
	float arenaFocusTimer_ = 0.0f;
	float arenaFocusDuration_ = 0.8f;
	void UpdateArenaFocusCamera();

	float scrollX_ = 0.0f;
	float scrollSpeed_ = 0.025f;
	float kPushBoundaryOffset = -6.5f;
	KamataEngine::Vector3 targetOffset_ = {0.0f, 0.0f, -5.0f};
	KamataEngine::Vector3 targetPos = {};
	KamataEngine::Camera* camera_ = nullptr;
	Player* target_ = nullptr;
	static inline float kInterpolationRate = 0.1f;
	static inline float kVelocityBias = 30.0f;
	static inline const Rect margin = {-10, 10, 0, 0};

	Mode mode_ = Mode::kFollow;
};