#include "AnimationManager.h"
#include "GlobalVariables.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "AnimationManager";
}

void AnimationManager::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "IdleBreatheAmplitude", kIdleBreatheAmplitude);
	gv->AddItem(kGroupName, "IdleBreatheSpeed", kIdleBreatheSpeed);
	gv->AddItem(kGroupName, "RunBounceAmplitude", kRunBounceAmplitude);
	gv->AddItem(kGroupName, "RunBounceSpeed", kRunBounceSpeed);
	gv->AddItem(kGroupName, "RunTiltAmplitude", kRunTiltAmplitude);
	gv->AddItem(kGroupName, "JumpStretchY", kJumpStretchY);
	gv->AddItem(kGroupName, "JumpSquashX", kJumpSquashX);
	gv->AddItem(kGroupName, "FallSquashY", kFallSquashY);
	gv->AddItem(kGroupName, "FallStretchX", kFallStretchX);
	gv->AddItem(kGroupName, "LandingDuration", kLandingDuration);
	gv->AddItem(kGroupName, "LandingSquashY", kLandingSquashY);
	gv->AddItem(kGroupName, "LandingSquashX", kLandingSquashX);
	gv->AddItem(kGroupName, "RunSpeedThreshold", kRunSpeedThreshold);
	gv->AddItem(kGroupName, "IdleCycleSpeed", kIdleCycleSpeed);
	gv->AddItem(kGroupName, "RunCycleSpeed", kRunCycleSpeed);
	gv->AddItem(kGroupName, "AirPoseBlendSpeed", kAirPoseBlendSpeed);
}
void AnimationManager::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kIdleBreatheAmplitude = gv->GetFloatValue(kGroupName, "IdleBreatheAmplitude");
	kIdleBreatheSpeed = gv->GetFloatValue(kGroupName, "IdleBreatheSpeed");
	kRunBounceAmplitude = gv->GetFloatValue(kGroupName, "RunBounceAmplitude");
	kRunBounceSpeed = gv->GetFloatValue(kGroupName, "RunBounceSpeed");
	kRunTiltAmplitude = gv->GetFloatValue(kGroupName, "RunTiltAmplitude");
	kJumpStretchY = gv->GetFloatValue(kGroupName, "JumpStretchY");
	kJumpSquashX = gv->GetFloatValue(kGroupName, "JumpSquashX");
	kFallSquashY = gv->GetFloatValue(kGroupName, "FallSquashY");
	kFallStretchX = gv->GetFloatValue(kGroupName, "FallStretchX");
	kLandingDuration = gv->GetFloatValue(kGroupName, "LandingDuration");
	kLandingSquashY = gv->GetFloatValue(kGroupName, "LandingSquashY");
	kLandingSquashX = gv->GetFloatValue(kGroupName, "LandingSquashX");
	kRunSpeedThreshold = gv->GetFloatValue(kGroupName, "RunSpeedThreshold");
	kIdleCycleSpeed = gv->GetFloatValue(kGroupName, "IdleCycleSpeed");
	kRunCycleSpeed = gv->GetFloatValue(kGroupName, "RunCycleSpeed");
	kAirPoseBlendSpeed = gv->GetFloatValue(kGroupName, "AirPoseBlendSpeed");
}
AnimationManager::MovementEvents AnimationManager::UpdateMovementState(const AnimationContext& context) {
	MovementEvents events;
	MovementState previous = movementState_;

	if (!context.onGround) {
		movementState_ = (context.velocity.y > 0.0f) ? MovementState::kJump : MovementState::kFall;
	} else if (movementState_ == MovementState::kFall && context.onGround) {
		movementState_ = MovementState::kLanding;
		movementStateTimer_ = 0.0f;
	} else if (movementState_ == MovementState::kLanding) {
		movementStateTimer_ += 1.0f / 60.0f;
		if (movementStateTimer_ >= kLandingDuration) {
			movementState_ = (std::abs(context.velocity.x) > kRunSpeedThreshold) ? MovementState::kRun : MovementState::kIdle;
		}
	} else {
		movementState_ = (std::abs(context.velocity.x) > kRunSpeedThreshold) ? MovementState::kRun : MovementState::kIdle;
	}

	if (movementState_ != previous) {
		movementStateTimer_ = 0.0f;
		footstepTimer_ = 0.0f; 
	} else {
		movementStateTimer_ += 1.0f / 60.0f;
	}

	events.justJumped = (previous != MovementState::kJump && movementState_ == MovementState::kJump);
	events.justLanded = (previous != MovementState::kLanding && movementState_ == MovementState::kLanding);

	if (movementState_ == MovementState::kRun) {
		footstepTimer_ += 1.0f / 60.0f;
		if (footstepTimer_ >= kFootstepInterval) {
			footstepTimer_ -= kFootstepInterval;
			events.footstepTick = true;
		}
	} else {
		footstepTimer_ = 0.0f;
	}

	return events;
}

SRTOffset AnimationManager::GetMovementAnimOffset() const {
	switch (movementState_) {
	case MovementState::kIdle: {
		float t = TriangleWave(movementStateTimer_, kIdleCycleSpeed);
		return LerpSRT(kIdlePoseLow, kIdlePoseHigh, t);
	}
	case MovementState::kRun: {
		float t = TriangleWave(movementStateTimer_, kRunCycleSpeed);
		return LerpSRT(kRunPoseLow, kRunPoseHigh, t);
	}
	case MovementState::kJump: {
		float t = std::clamp(movementStateTimer_ * kAirPoseBlendSpeed, 0.0f, 1.0f);
		return LerpSRT(kNeutralPose, kJumpPose, t);
	}
	case MovementState::kFall: {
		float t = std::clamp(movementStateTimer_ * kAirPoseBlendSpeed, 0.0f, 1.0f);
		return LerpSRT(kNeutralPose, kFallPose, t);
	}
	case MovementState::kLanding: {
		float t = std::clamp(movementStateTimer_ / kLandingDuration, 0.0f, 1.0f);
		SRTOffset landPose = {
		    {kLandingSquashX, kLandingSquashY, 1.0f},
            {0,               0,               0   },
            {0,               0,               0   }
        };
		return LerpSRT(landPose, kNeutralPose, t);
	}
	default:
		return kNeutralPose;
	}
}

void AnimationManager::UpdateBodyParts(const WorldTransform& worldTransform, const AnimationContext& context, std::array<BodyPart, kPartCount>& parts) {

	SRTOffset anim = context.isDead ? SRTOffset{} : GetMovementAnimOffset();

	if (!context.isDead) {
		// 移動アニメーションと戦闘中の体ポーズを合成する
		anim.translation += context.actionPose.translation;
		anim.rotation += context.actionPose.rotation;

		anim.scale = {anim.scale.x * context.actionPose.scale.x, anim.scale.y * context.actionPose.scale.y, anim.scale.z * context.actionPose.scale.z};
	}

	parts[body].transform.translation_ = worldTransform.translation_ + anim.translation;

	parts[body].transform.rotation_ = worldTransform.rotation_ + anim.rotation;

	parts[body].transform.scale_ = {
	    worldTransform.scale_.x * anim.scale.x,
	    worldTransform.scale_.y * anim.scale.y,
	    worldTransform.scale_.z * anim.scale.z,
	};

	WorldTransformUpdate(parts[body].transform);

	// head は最終的な body transform を親として計算する
	Matrix4x4 rotMatrix =
	    Multiply(MakeRotateXMatrix(parts[body].transform.rotation_.x), Multiply(MakeRotateYMatrix(parts[body].transform.rotation_.y), MakeRotateZMatrix(parts[body].transform.rotation_.z)));

	Vector3 rotatedHeadOffset = TransformNormal(headOffset_.translation, rotMatrix);

	parts[head].transform.translation_ = parts[body].transform.translation_ + rotatedHeadOffset;

	parts[head].transform.rotation_ = parts[body].transform.rotation_ + headOffset_.rotation;

	parts[head].transform.scale_ = {
	    parts[body].transform.scale_.x * headOffset_.scale.x,
	    parts[body].transform.scale_.y * headOffset_.scale.y,
	    parts[body].transform.scale_.z * headOffset_.scale.z,
	};

	WorldTransformUpdate(parts[head].transform);

	// hat は最終的な head transform を親として計算する
	Matrix4x4 headRotMatrix =
	    Multiply(MakeRotateXMatrix(parts[head].transform.rotation_.x), Multiply(MakeRotateYMatrix(parts[head].transform.rotation_.y), MakeRotateZMatrix(parts[head].transform.rotation_.z)));

	Vector3 rotatedHatOffset = TransformNormal(hatOffset_.translation, headRotMatrix);

	parts[hat].transform.translation_ = parts[head].transform.translation_ + rotatedHatOffset;

	parts[hat].transform.rotation_ = parts[head].transform.rotation_ + hatOffset_.rotation;

	parts[hat].transform.scale_ = {
	    parts[head].transform.scale_.x * hatOffset_.scale.x,
	    parts[head].transform.scale_.y * hatOffset_.scale.y,
	    parts[head].transform.scale_.z * hatOffset_.scale.z,
	};

	WorldTransformUpdate(parts[hat].transform);
}