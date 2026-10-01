#pragma once

#define NOMINMAX

#include "Global.h"

/// <summary>
/// BaseEnemyの見た目・アニメーション（体パーツの追従、移動モーションのブレンド）を担当するクラス。
/// BaseEnemy本体の物理/衝突/振る舞いロジックからは独立しており、
/// 毎フレームBaseEnemy側から現在の物理状態(AnimationContext)を渡してUpdate/ApplyToPartsを呼ぶ想定。
/// </summary>
class AnimationManager {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
	enum class MovementState { kIdle, kRun, kJump, kFall, kLanding }; // kKnockback, kStunned

	// 毎フレームBaseEnemyから渡される、アニメーション判定に必要な最小限の物理状態
	struct AnimationContext {
		KamataEngine::Vector3 velocity = {};
		bool onGround = true;
		bool isDead = false;

		// 移動アニメーションとは別の、攻撃・照準・リロード等による体のポーズ
		SRTOffset actionPose = {};
	};

	struct MovementEvents {
		bool justJumped = false;   // ground -> kJump this frame (a real jump, not walking off a ledge)
		bool justLanded = false;   // kFall -> kLanding this frame
		bool footstepTick = false; // periodic beat while actively running on the ground
	};

	// 敵の移動アニメーション状態を更新する（onGround/velocityから状態遷移を判定）。
	// 派生クラスで移動パターンを変えたい場合はこれをオーバーライドする
	virtual MovementEvents UpdateMovementState(const AnimationContext& context);
	virtual SRTOffset GetMovementAnimOffset() const;

	// 現在の移動アニメ状態を外部(BaseEnemy)から参照できるようにする。
	// 着地の瞬間(kFall->kLanding遷移)を検出して足音SFX等をトリガーするために使う。
	MovementState GetCurrentMovementState() const { return movementState_; }

	// parts_[body]/[head]/[hat]のワールドトランスフォームを、
	// worldTransformと現在のアニメーションオフセットから計算して反映する
	virtual void UpdateBodyParts(const KamataEngine::WorldTransform& worldTransform, const AnimationContext& context, std::array<BodyPart, static_cast<size_t>(PartSlot::kCount)>& parts);

	virtual ~AnimationManager() = default;

protected:
	friend class DebugMenu;
	static constexpr size_t kPartCount = static_cast<size_t>(PartSlot::kCount);
	static constexpr size_t body = static_cast<size_t>(PartSlot::kBody);
	static constexpr size_t head = static_cast<size_t>(PartSlot::kHead);
	static constexpr size_t hat = static_cast<size_t>(PartSlot::kHat);

		float footstepTimer_ = 0.0f;
	static inline float kFootstepInterval = 0.32f;

	SRTOffset headOffset_ = {
	    {1.0f,    1.0f,   1.0f},
        {0.0f,    0.0f,   0.0f},
        {-0.050f, 0.720f, 0.0f}
    };
	SRTOffset hatOffset_ = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

	MovementState movementState_ = MovementState::kIdle;
	float movementStateTimer_ = 0.0f;

	// --- Idle ---
	static inline float kIdleBreatheAmplitude = 0.03f;
	static inline float kIdleBreatheSpeed = 2.0f;

	// --- Run ---
	static inline float kRunBounceAmplitude = 0.08f;
	static inline float kRunBounceSpeed = 10.0f;
	static inline float kRunTiltAmplitude = 0.05f;

	// --- Jump / Fall ---
	static inline float kJumpStretchY = 1.1f;
	static inline float kJumpSquashX = 0.95f;
	static inline float kFallSquashY = 0.95f;
	static inline float kFallStretchX = 1.05f;

	// --- Landing ---
	static inline float kLandingDuration = 0.15f;
	static inline float kLandingSquashY = 0.7f;
	static inline float kLandingSquashX = 1.2f;

	static inline float kRunSpeedThreshold = 0.01f;
	static inline float kIdleCycleSpeed = 2.0f;
	static inline float kRunCycleSpeed = 10.0f;
	static inline float kAirPoseBlendSpeed = 8.0f;

	static inline SRTOffset kIdlePoseLow = {
	    {1.0f, 1.0f, 1.0f},
        {0,    0,    0   },
        {0.0f, 0.0f, 0.0f}
    };
	static inline SRTOffset kIdlePoseHigh = {
	    {1.0f, 1.0f,  1.0f},
        {0,    0,     0   },
        {0.0f, 0.03f, 0.0f}
    };
	static inline SRTOffset kRunPoseLow = {
	    {1.0f, 1.0f, 1.0f  },
        {0,    0,    -0.05f},
        {0.0f, 0.0f, 0.0f  }
    };
	static inline SRTOffset kRunPoseHigh = {
	    {1.0f, 1.0f,  1.0f },
        {0,    0,     0.05f},
        {0.0f, 0.08f, 0.0f }
    };
	static inline SRTOffset kJumpPose = {
	    {0.95f, 1.1f, 1.0f},
        {0,     0,    0   },
        {0.0f,  0.0f, 0.0f}
    };
	static inline SRTOffset kFallPose = {
	    {1.05f, 0.95f, 1.0f},
        {0,     0,     0   },
        {0.0f,  0.0f,  0.0f}
    };
	static inline SRTOffset kNeutralPose = {};
};