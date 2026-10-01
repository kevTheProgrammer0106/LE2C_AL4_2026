#define NOMINMAX
#include "BlockDetector.h"
#include "GlobalVariables.h"
#include <cmath>

using namespace KamataEngine;

namespace {
bool IsSolidIgnoringOneWay(MapChipField* mapChipField, const Vector3& position, bool ignoreBarrier) {
	if (!mapChipField->IsSolidAtPosition(position, ignoreBarrier)) {
		return false;
	}
	if (mapChipField->IsOneWayAtPosition(position)) {
		return false;
	}
	return true;
}
} // namespace

void BlockDetector::GroundSwitch(const CollisionInfo& collisionInfo) {
	if (onGround_) {
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			std::array<Vector3, kNumCorner> positionNew;
			for (uint32_t i = 0; i < positionNew.size(); ++i) {
				positionNew[i] = CornerPosition(worldTransform_.translation_, static_cast<Corner>(i));
			}

			MapChipType mapChipType;
			// 真下の当たり判定を行う
			bool isHit = false;
			MapChipField::IndexSet indexSet;

			// [FIX] Ground-hold check now also accepts one-way platforms —
			// standing on a platform is exactly the case a one-way tile is
			// meant to support, so IsSolidAtPosition() (which already
			// reports platforms as solid) is correct here. No change needed
			// to the solidity check itself, just confirming this path stays
			// as-is; the bug was only in the directional checks below.
			Vector3 checkPosLeft = positionNew[kBottomLeft] + Vector3(0.0f, -0.2f, 0.0f);
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(checkPosLeft);
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipField_->IsSolidAtPosition(checkPosLeft)) {
				isHit = true;
			}
			// 右上点の判定
			Vector3 checkPosRight = positionNew[kBottomRight] + Vector3(0.0f, -0.2f, 0.0f);
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(checkPosRight);
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipField_->IsSolidAtPosition(checkPosRight)) {
				isHit = true;
			}
			if (!isHit) {
				onGround_ = false;
			}
		}
	} else {

		if (collisionInfo.isHitGround) {
			onGround_ = true;
			velocity_.x *= (1.0f - kAttenuation);
			velocity_.y = 0.0f;
		}
	}
}
void BlockDetector::MapChipCollision(CollisionInfo& collisionInfo) {
	// 上当たり判定
	TopCollision(collisionInfo);
	// 下当たり判定
	BottomCollision(collisionInfo);
	// 左当たり判定
	LeftCollision(collisionInfo);
	// 右当たり判定
	RightCollision(collisionInfo);
}

void BlockDetector::TopCollision(CollisionInfo& collisionInfo) {
	if (collisionInfo.move.y <= 0.0f)
		return;
	std::array<Vector3, kNumCorner> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + collisionInfo.move, static_cast<Corner>(i));
	}

	// [FIX] Jumping upward into the underside of a platform must never stop
	// the jump — one-way platforms only block landings from above. Swapped
	// IsSolidAtPosition() -> IsSolidIgnoringOneWay() so platform tiles are
	// simply invisible to the "hit ceiling" check.
	bool hitLeft = IsSolidIgnoringOneWay(mapChipField_, positionNew[kTopLeft], ignoreBarrier_);
	bool hitRight = IsSolidIgnoringOneWay(mapChipField_, positionNew[kTopRight], ignoreBarrier_);
	if (!hitLeft && !hitRight)
		return;

	float clampedMoveY = collisionInfo.move.y;
	if (hitLeft) {
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kTopLeft]);
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		clampedMoveY = std::min(clampedMoveY, std::min(0.0f, (rect.bottom - worldTransform_.translation_.y) - (kHeight / 2.0f) - kBlank));
	}
	if (hitRight) {
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kTopRight]);
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		clampedMoveY = std::min(clampedMoveY, std::min(0.0f, (rect.bottom - worldTransform_.translation_.y) - (kHeight / 2.0f) - kBlank));
	}

	collisionInfo.move.y = clampedMoveY;
	collisionInfo.isHitTop = true;
}

void BlockDetector::BottomCollision(CollisionInfo& collisionInfo) {
	if (collisionInfo.move.y >= 0.0f)
		return;

	const Vector3 center = worldTransform_.translation_;

	// Sweeps one foot corner from its old to its new position so thin one-way platforms
	// can't be skipped over in a single frame. Returns the landing surface's top Y.
	auto resolveCorner = [&](Corner corner, float& outTop) -> bool {
		Vector3 oldPos = CornerPosition(center, corner);
		Vector3 newPos = CornerPosition(center + collisionInfo.move, corner);

		constexpr float kStep = 0.1f;
		float distance = oldPos.y - newPos.y;
		int steps = std::max(1, static_cast<int>(std::ceil(distance / kStep)));

		for (int i = 0; i <= steps; ++i) {
			float t = static_cast<float>(i) / static_cast<float>(steps);
			Vector3 sample = {oldPos.x + (newPos.x - oldPos.x) * t, oldPos.y + (newPos.y - oldPos.y) * t, 0.0f};

			if (!mapChipField_->IsSolidAtPosition(sample))
				continue;

			MapChipField::IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(sample);
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(index.xIndex, index.yIndex);

			if (mapChipField_->IsOneWayAtPosition(sample)) {
				// Only land if the foot started at/above the platform's top surface.
				if (oldPos.y < rect.top - 0.001f)
					continue;
			} else {
				// Keep the existing "don't double count stacked blocks" rule.
				MapChipType above = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex - 1);
				if (above == MapChipType::kBlock)
					continue;
			}

			outTop = rect.top;
			return true;
		}
		return false;
	};

	float topLeft = 0.0f;
	float topRight = 0.0f;
	bool hitLeft = resolveCorner(kBottomLeft, topLeft);
	bool hitRight = resolveCorner(kBottomRight, topRight);
	if (!hitLeft && !hitRight)
		return;

	// Snap the feet onto the highest surface that was hit (instead of stopping dead above it).
	float footY = worldTransform_.translation_.y - (kHeight / 2.0f);
	float clampedMoveY = collisionInfo.move.y;
	if (hitLeft)
		clampedMoveY = std::max(clampedMoveY, (topLeft + kBlank) - footY);
	if (hitRight)
		clampedMoveY = std::max(clampedMoveY, (topRight + kBlank) - footY);

	collisionInfo.move.y = clampedMoveY;
	collisionInfo.isHitGround = true;
}

void BlockDetector::LeftCollision(CollisionInfo& collisionInfo) {
	if (collisionInfo.move.x >= 0.0f)
		return;
	std::array<Vector3, kNumCorner> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + collisionInfo.move, static_cast<Corner>(i));
	}

	// [FIX] One-way platforms must not block horizontal movement — walking
	// into the side of a platform should pass straight through it.
	bool hitTop = IsSolidIgnoringOneWay(mapChipField_, positionNew[kTopLeft], ignoreBarrier_);
	bool hitBottom = IsSolidIgnoringOneWay(mapChipField_, positionNew[kBottomLeft], ignoreBarrier_);
	if (!hitTop && !hitBottom)
		return;

	float clampedMoveX = collisionInfo.move.x;
	if (hitTop) {
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kTopLeft]);
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		clampedMoveX = std::max(clampedMoveX, std::max(0.0f, (rect.right - worldTransform_.translation_.x) + (kWidth / 2.0f) + kBlank));
	}
	if (hitBottom) {
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kBottomLeft]);
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		clampedMoveX = std::max(clampedMoveX, std::max(0.0f, (rect.right - worldTransform_.translation_.x) + (kWidth / 2.0f) + kBlank));
	}

	collisionInfo.move.x = clampedMoveX;
	collisionInfo.isHitWall = true;
}

void BlockDetector::RightCollision(CollisionInfo& collisionInfo) {
	if (collisionInfo.move.x <= 0.0f)
		return;
	std::array<Vector3, kNumCorner> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + collisionInfo.move, static_cast<Corner>(i));
	}

	// [FIX] Same reasoning as LeftCollision — platforms are pass-through
	// from the side.
	bool hitTop = IsSolidIgnoringOneWay(mapChipField_, positionNew[kTopRight], ignoreBarrier_);
	bool hitBottom = IsSolidIgnoringOneWay(mapChipField_, positionNew[kBottomRight], ignoreBarrier_);
	if (!hitTop && !hitBottom)
		return;

	float clampedMoveX = collisionInfo.move.x;
	if (hitTop) {
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kTopRight]);
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		clampedMoveX = std::min(clampedMoveX, std::min(0.0f, (rect.left - worldTransform_.translation_.x) - (kWidth / 2.0f) - kBlank));
	}
	if (hitBottom) {
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kBottomRight]);
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		clampedMoveX = std::min(clampedMoveX, std::min(0.0f, (rect.left - worldTransform_.translation_.x) - (kWidth / 2.0f) - kBlank));
	}

	collisionInfo.move.x = clampedMoveX;
	collisionInfo.isHitWall = true;
}

Vector3 BlockDetector::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorner] = {
	    {-kWidth / 2.0f, kHeight / 2.0f,  0.0f}, // 左上
	    {kWidth / 2.0f,  kHeight / 2.0f,  0.0f}, // 右上
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0.0f}, // 左下
	    {kWidth / 2.0f,  -kHeight / 2.0f, 0.0f}, // 右下
	};
	Vector3 result;
	result.x = center.x + offsetTable[static_cast<uint32_t>(corner)].x;
	result.y = center.y + offsetTable[static_cast<uint32_t>(corner)].y;
	result.z = center.z + offsetTable[static_cast<uint32_t>(corner)].z;

	return result;
}