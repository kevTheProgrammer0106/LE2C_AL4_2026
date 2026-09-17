#pragma once

#include <KamataEngine.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <cassert>
#include <numbers>


class Global {};

enum class Owner { kEmpty, kPlayer, kEnemy };

enum class DropItemCategory { kMelee, kFirearm, kUnknown};
enum class DropWeaponKind { kSabreLight, kSabreHeavy, kMusketLong, kMusketBayonet, kUnknown };


struct AABB {
	KamataEngine::Vector3 min; //!< 最小点
	KamataEngine::Vector3 max; //!< 最大点
};
enum Corner {
	kTopLeft,     // 左上
	kTopRight,    // 右上
	kBottomLeft,  // 左下
	kBottomRight, // 右下

	kNumCorner // 要素数
};
// 当たり判定構造体
struct CollisionInfo {
	bool isHitTop = false;
	bool isHitGround = false;
	bool isHitWall = false;
	KamataEngine::Vector3 move = {};
};

enum class PartSlot { kBody, kHead, kHat, kCount };

struct SRTOffset {
	KamataEngine::Vector3 scale{1.0f, 1.0f, 1.0f};
	KamataEngine::Vector3 rotation{0.0f, 0.0f, 0.0f};
	KamataEngine::Vector3 translation{0.0f, 0.0f, 0.0f};
};

struct ComboAttackPattern {
	SRTOffset anticipateStart; // 予備動作開始時の姿勢（通常はidle姿勢と同じでよい）
	SRTOffset anticipateEnd;   // 予備動作完了時の姿勢
	SRTOffset attackEnd;       // 攻撃振り切り時の姿勢
	SRTOffset recoveryEnd;     // 硬直終了時の姿勢（通常はidle姿勢に戻す）
};
struct BodyComboPattern {
	SRTOffset anticipateStart; // 予備動作開始時の姿勢（通常はidle姿勢と同じでよい）
	SRTOffset anticipateEnd;   // 予備動作完了時の姿勢
	SRTOffset attackEnd;       // 攻撃振り切り時の姿勢
	SRTOffset recoveryEnd;     // 硬直終了時の姿勢（通常はidle姿勢に戻す）
};


struct PartResources {
	KamataEngine::Model* model = nullptr;
	uint32_t textureHandle = 0;
};
struct BodyPart {
	KamataEngine::WorldTransform transform;
	KamataEngine::Model* model = nullptr;
	uint32_t textureHandle = 0;
};

struct AimFacing {
	bool facingRight;
	float localAngle; // rotation_.z to feed the gun, already mirrored for the flip side
};

AimFacing ComputeAimFacing(const KamataEngine::Vector2& aimDir, bool currentFacingRight);

float TriangleWave(float timer, float speed);
SRTOffset LerpSRT(const SRTOffset& a, const SRTOffset& b, float t);

bool IsCollisionAABB(const AABB& a, const AABB& b);

/// <summary>
///	行列計算・転送する
/// </summary>
void WorldTransformUpdate(KamataEngine::WorldTransform& worldTransform);

KamataEngine::Matrix4x4 Multiply(KamataEngine::Matrix4x4 matrix1, KamataEngine::Matrix4x4 matrix2);

KamataEngine::Matrix4x4 Inverse(KamataEngine::Matrix4x4 matrix);

// 1. X軸回転行列
KamataEngine::Matrix4x4 MakeRotateXMatrix(float radian);
// 2. Y軸回転行列
KamataEngine::Matrix4x4 MakeRotateYMatrix(float radian);
// 3. Z軸回転行列
KamataEngine::Matrix4x4 MakeRotateZMatrix(float radian);

KamataEngine::Matrix4x4 MakeScaleMatrix(KamataEngine::Vector3 scale);
KamataEngine::Matrix4x4 MakeTranslateMatrix(KamataEngine::Vector3 translate);

KamataEngine::Matrix4x4 MakeAffineMatrix(KamataEngine::Vector3 scale, KamataEngine::Vector3 rotation, KamataEngine::Vector3 translation);

KamataEngine::Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

KamataEngine::Vector3 TransformCoordinate(const KamataEngine::Vector3& vector, const KamataEngine::Matrix4x4& m);
KamataEngine::Vector3 TransformNormal(const KamataEngine::Vector3& vector, const KamataEngine::Matrix4x4& m);

float Lerp(float start, float end, float t);

KamataEngine::Vector3 Lerp(KamataEngine::Vector3 start, KamataEngine::Vector3 end, float t);

float EaseIn(float start, float end, float t);
float EaseOut(float start, float end, float t);
float EaseInOut(float start, float end, float t);