#pragma once

#include "Global.h"
#include <KamataEngine.h>

/// <summary>
/// Debug-only wireframe helpers built on PrimitiveDrawer::DrawLine3d.
/// PrimitiveDrawer uses whichever camera GameScene last set via SetCamera()
/// (debug camera or the normal camera), so the boxes follow both.
/// </summary>
namespace DebugDraw {

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

namespace Color {
inline const KamataEngine::Vector4 kPlayer = {0.2f, 1.0f, 0.2f, 1.0f};     // green
inline const KamataEngine::Vector4 kEnemy = {1.0f, 0.9f, 0.1f, 1.0f};      // yellow
inline const KamataEngine::Vector4 kAttack = {1.0f, 0.15f, 0.15f, 1.0f};   // red (active hitboxes)
inline const KamataEngine::Vector4 kProjectile = {0.2f, 0.9f, 1.0f, 1.0f}; // cyan
inline const KamataEngine::Vector4 kObject = {0.8f, 0.4f, 1.0f, 1.0f};     // purple (items, gates, switches, health blocks)
} // namespace Color

} // namespace DebugDraw