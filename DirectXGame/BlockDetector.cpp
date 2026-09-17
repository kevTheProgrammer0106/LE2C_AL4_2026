#define NOMINMAX
#include "BlockDetector.h"
#include "GlobalVariables.h"

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
	std::array<Vector3, kNumCorner> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + collisionInfo.move, static_cast<Corner>(i));
	}

	// Current (pre-move) foot corner positions — needed to tell whether we
	// were already above the platform's top surface before this frame's
	// movement. That's the deciding factor for one-way tiles: land on it
	// only if we were coming from above, otherwise pass straight through
	// (e.g. jumping up through it, or already standing inside it due to some
	// other resolution).
	std::array<Vector3, kNumCorner> positionOld;
	for (uint32_t i = 0; i < positionOld.size(); ++i) {
		positionOld[i] = CornerPosition(worldTransform_.translation_, static_cast<Corner>(i));
	}

	MapChipField::IndexSet indexSet;
	MapChipType mapChipTypeNext;

	auto resolveBottomHit = [&](const Vector3& newPos, const Vector3& oldPos) -> bool {
		if (!mapChipField_->IsSolidAtPosition(newPos)) {
			return false;
		}

		if (mapChipField_->IsOneWayAtPosition(newPos)) {
			// [FIX] Core of the one-way fix. Only accept this as a landing
			// if the corner's *previous* Y was at or above the platform's
			// top surface — i.e. we were falling onto it from above, not
			// already below it (which would mean jumping up through it or
			// walking under it, and should pass through unobstructed).
			MapChipField::IndexSet oneWayIndex = mapChipField_->GetMapChipIndexSetByPosition(newPos);
			MapChipField::Rect oneWayRect = mapChipField_->GetRectByIndex(oneWayIndex.xIndex, oneWayIndex.yIndex);
			if (oldPos.y < oneWayRect.top) {
				return false; // was already at/under the platform's surface -> pass through
			}
			return true;
		}

		// Non-platform solid tile: keep the original behavior of ignoring
		// the hit if the tile directly below (in the fall direction) is
		// itself a full kBlock (this preserves the existing "don't double
		// count stacked blocks" logic from before the fix).
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(newPos);
		mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
		return mapChipTypeNext != MapChipType::kBlock;
	};

	bool hitLeft = resolveBottomHit(positionNew[kBottomLeft], positionOld[kBottomLeft]);
	bool hitRight = resolveBottomHit(positionNew[kBottomRight], positionOld[kBottomRight]);

	if (!hitLeft && !hitRight)
		return;

	float clampedMoveY = collisionInfo.move.y;
	if (hitLeft) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kBottomLeft]);
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		clampedMoveY = std::max(clampedMoveY, std::max(0.0f, (rect.top - worldTransform_.translation_.y) + (kHeight / 2.0f) + kBlank));
	}
	if (hitRight) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kBottomRight]);
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		clampedMoveY = std::max(clampedMoveY, std::max(0.0f, (rect.top - worldTransform_.translation_.y) + (kHeight / 2.0f) + kBlank));
	}

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