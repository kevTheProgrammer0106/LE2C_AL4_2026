#pragma once

#define NOMINMAX
#include "KamataEngine.h"
#include <algorithm>
#include <cmath>

// 前方宣言
class Player;

class CameraController {
public:
	// 矩形
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};

	// カメラモード
	enum class Mode {
		kFollow,      // プレイヤー追従
		kForcedScroll // 強制スクロール
	};

	enum class PerspectiveType { Sidescroll, Fixed, TopDown, TopDownDiagonal, Isometric, ThirdPerson, FirstPerson, Count };
	PerspectiveType perspectiveType = PerspectiveType::ThirdPerson;

	void Initialize(KamataEngine::Camera* camera);

	void Update();

	// 追従ターゲットを設定するセッター
	void SetTarget(Player* target) { target_ = target; }

	// カメラ移動範囲
	Rect movableArea_ = {10.5f, 88.5f, 6.5f, 100.0f};

	void SetMovableArea(Rect area) { movableArea_ = area; }

	// 追従ターゲットをリセットする
	void Reset();

	// ImGuiを使用してカメラのパラメータを調整する
	void Perspective();

public:
	// 固定カメラ関数
	void UpdateFixedCamera(const KamataEngine::WorldTransform& worldTransformTarget);
	// トップダウンカメラ関数
	void UpdateTopDownCamera(const KamataEngine::WorldTransform& worldTransformTarget);
	// トップダウンカメラ斜め関数
	void UpdateTopDownDiagonalCamera(const KamataEngine::WorldTransform& worldTransformTarget);
	// アイソメトリックカメラ関数
	void UpdateIsometricCamera(const KamataEngine::WorldTransform& worldTransformTarget);
	// TPSカメラ関数
	void UpdateThirdPersonCamera(const KamataEngine::WorldTransform& worldTransformTarget);
	// FPSカメラ関数
	void UpdateFirstPersonCamera(const KamataEngine::WorldTransform& worldTransformTarget);

	void SetMode(Mode mode) { mode_ = mode; }
	Mode GetMode() const { return mode_; }

	void UpdateCameraPerspectives(const KamataEngine::WorldTransform& targetWorldTransform);
	void ResetCameraPerspectives();

	// 強制スクロール中、画面左端(押し出し境界線)のワールドX座標を取得
	// 強制スクロール中、画面左端(押し出し境界線)のワールドX座標を取得
	float GetScrollLeftBoundary() const {
		// カメラのFOV・距離から実際の画面左端を算出
		float halfScreenWidth = std::tan(camera_->fovAngleY / 2.0f) * std::abs(targetOffset_.z) * camera_->aspectRatio;
		return scrollX_ - halfScreenWidth + kPushBoundaryOffset;
	}

	// デバッグ調整用
	void SetPushBoundaryOffset(float offset) { kPushBoundaryOffset = offset; }

private:
	float scrollX_ = 0.0f;
	float scrollSpeed_ = 0.025f;
	// カメラ中心から見た押し出し境界のオフセット(負の値 = 画面左寄り)
	float kPushBoundaryOffset = -6.5f; // ← FOV/距離に応じて要調整
	// 追従対象とカメラの座標の差（オフセット)
	KamataEngine::Vector3 targetOffset_ = {0.0f, 0.0f, -15.0f};
	KamataEngine::Vector3 targetPos = {};
	KamataEngine::Camera* camera_ = nullptr;
	// 追従対象
	Player* target_ = nullptr;
	static inline const float kInterpolationRate = 0.1f;
	// 速度掛け率
	static inline const float kVelocityBias = 30.0f;
	static inline const Rect margin = {-10, 10, 0, 0};

	Mode mode_ = Mode::kFollow;
};