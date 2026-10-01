#pragma once

#include "Global.h"
#include "VisionSensor.h"
#include <KamataEngine.h>

/// <summary>
/// Debug-only wireframe helpers built on PrimitiveDrawer::DrawLine3d.
/// PrimitiveDrawer uses whichever camera GameScene last set via SetCamera()
/// (debug camera or the normal camera), so the boxes follow both.
/// </summary>
namespace DebugDraw {

namespace Color {
inline const KamataEngine::Vector4 kPlayer = {0.2f, 1.0f, 0.2f, 1.0f};        // green
inline const KamataEngine::Vector4 kEnemy = {1.0f, 0.9f, 0.1f, 1.0f};         // yellow
inline const KamataEngine::Vector4 kAttack = {1.0f, 0.15f, 0.15f, 1.0f};      // red (active hitboxes)
inline const KamataEngine::Vector4 kProjectile = {0.2f, 0.9f, 1.0f, 1.0f};    // cyan
inline const KamataEngine::Vector4 kObject = {0.8f, 0.4f, 1.0f, 1.0f};        // purple (items, gates, switches, health blocks)
inline const KamataEngine::Vector4 kVision = {0.3f, 0.6f, 1.0f, 1.0f};        // blue (vision cone, player not seen)
inline const KamataEngine::Vector4 kVisionSpotted = {1.0f, 0.2f, 0.9f, 1.0f}; // magenta (vision cone, player spotted)
} // namespace Color

// F4 toggles this (see GameScene::DrawCollisionDebug)
inline bool gShowVision = true;

inline void Line(const KamataEngine::Vector3& a, const KamataEngine::Vector3& b, const KamataEngine::Vector4& color) { KamataEngine::PrimitiveDrawer::GetInstance()->DrawLine3d(a, b, color); }

inline void Box(const AABB& box, const KamataEngine::Vector4& color) {
	using namespace KamataEngine;
	PrimitiveDrawer* drawer = PrimitiveDrawer::GetInstance();

	// corner index bits: x = bit0, y = bit1, z = bit2
	Vector3 corners[8];
	for (int i = 0; i < 8; ++i) {
		corners[i] = {(i & 1) ? box.max.x : box.min.x, (i & 2) ? box.max.y : box.min.y, (i & 4) ? box.max.z : box.min.z};
	}

	// 12 edges: connect every pair of corners that differ by exactly one bit
	for (int i = 0; i < 8; ++i) {
		for (int bit = 1; bit <= 4; bit <<= 1) {
			int j = i | bit;
			if (j != i) {
				drawer->DrawLine3d(corners[i], corners[j], color);
			}
		}
	}
}

// Draws the enemy's vision triangle (eye -> left -> right). The sensor stores left/right as XY only,
// so z is taken from the eye position to keep the cone in the gameplay plane.
inline void VisionCone(const VisionSensor& sensor, bool spotted) {
	using namespace KamataEngine;
	if (!sensor.HasValidCone()) {
		return;
	}
	const VisionSensor::VisionTriangle& tri = sensor.GetVisionTriangle();
	const Vector4& color = spotted ? Color::kVisionSpotted : Color::kVision;

	Vector3 center = tri.center;
	Vector3 left = {tri.left.x, tri.left.y, center.z};
	Vector3 right = {tri.right.x, tri.right.y, center.z};

	Line(center, left, color);
	Line(center, right, color);
	Line(left, right, color);
}

} // namespace DebugDraw