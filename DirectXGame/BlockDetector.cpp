#define NOMINMAX
#include "BlockDetector.h"
#include "GlobalVariables.h"
#include <cmath>

using namespace KamataEngine;

#pragma region タイルアクセス(詳細 / 簡易)
bool BlockDetector::IsSolid(const Vector3& position, bool ignoreBarrier) const {
	if (useDetailedTiles_) {
		return mapChipField_->IsSolidAtPosition(position, ignoreBarrier);
	}
	// 簡易モード: セル全体(1.0)を実体として扱い、オフセットは無視する
	MapChipField::IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(position);
	MapChipType type = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);
	return MapChipField::IsGroundType(type) || (type == MapChipType::kBarrier && !ignoreBarrier);
}

MapChipField::Rect BlockDetector::GetRect(uint32_t xIndex, uint32_t yIndex) const {
	if (useDetailedTiles_) {
		return mapChipField_->GetRectByIndex(xIndex, yIndex);
	}
	Vector3 c = mapChipField_->GetMapChipPositionByIndex(xIndex, yIndex);
	return {c.x - 0.5f, c.x + 0.5f, c.y - 0.5f, c.y + 0.5f};
}
#pragma endregion

bool BlockDetector::ProbeGroundTop(const Vector3& center, float stepUp, float maxDepth, float& outTopY) {
	if (!mapChipField_) {
		return false;
	}

	const float feetY = center.y - kHeight * 0.5f;
	// 足元の左右2列を調べる
	const float columnsX[2] = {center.x - kWidth * 0.5f + kBlank, center.x + kWidth * 0.5f - kBlank};
	constexpr float kStep = 0.05f;

	bool found = false;
	float bestTop = 0.0f;

	for (float x : columnsX) {
		for (float d = -stepUp; d <= maxDepth; d += kStep) {
			Vector3 sample = {x, feetY - d, 0.0f};
			if (!IsSolid(sample, ignoreBarrier_)) { // [CHANGED] バリア無視設定に従う
				continue;
			}

			MapChipField::IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(sample);
			MapChipField::Rect rect = GetRect(index.xIndex, index.yIndex);

			// 段差が高すぎる壁は「立てる面」ではない
			if (rect.top > feetY + stepUp + 0.001f) {
				break;
			}
			// 自分が下にいる一方通行足場は無視する
			if (mapChipField_->IsOneWayAtPosition(sample) && feetY < rect.top - 0.05f) {
				continue;
			}

			if (!found || rect.top > bestTop) {
				bestTop = rect.top;
			}
			found = true;
			break;
		}
	}

	if (found) {
		outTopY = bestTop;
	}
	return found;
}

void BlockDetector::GroundSwitch(const CollisionInfo& collisionInfo) {
	if (onGround_) {
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			std::array<Vector3, kNumCorner> positionNew;
			for (uint32_t i = 0; i < positionNew.size(); ++i) {
				positionNew[i] = CornerPosition(worldTransform_.translation_, static_cast<Corner>(i));
			}

			bool isHit = false;
			Vector3 checkPosLeft = positionNew[kBottomLeft] + Vector3(0.0f, -0.2f, 0.0f);
			if (IsSolid(checkPosLeft, false)) {
				isHit = true;
			}
			Vector3 checkPosRight = positionNew[kBottomRight] + Vector3(0.0f, -0.2f, 0.0f);
			if (IsSolid(checkPosRight, false)) {
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
	TopCollision(collisionInfo);
	BottomCollision(collisionInfo);
	LeftCollision(collisionInfo);
	RightCollision(collisionInfo);
}

void BlockDetector::TopCollision(CollisionInfo& collisionInfo) {
	if (collisionInfo.move.y <= 0.0f)
		return;
	std::array<Vector3, kNumCorner> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + collisionInfo.move, static_cast<Corner>(i));
	}

	bool hitLeft = IsSolid(positionNew[kTopLeft], ignoreBarrier_);
	bool hitRight = IsSolid(positionNew[kTopRight], ignoreBarrier_);
	if (!hitLeft && !hitRight)
		return;

	float clampedMoveY = collisionInfo.move.y;
	if (hitLeft) {
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kTopLeft]);
		MapChipField::Rect rect = GetRect(indexSet.xIndex, indexSet.yIndex);
		clampedMoveY = std::min(clampedMoveY, std::min(0.0f, (rect.bottom - worldTransform_.translation_.y) - (kHeight / 2.0f) - kBlank));
	}
	if (hitRight) {
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kTopRight]);
		MapChipField::Rect rect = GetRect(indexSet.xIndex, indexSet.yIndex);
		clampedMoveY = std::min(clampedMoveY, std::min(0.0f, (rect.bottom - worldTransform_.translation_.y) - (kHeight / 2.0f) - kBlank));
	}

	collisionInfo.move.y = clampedMoveY;
	collisionInfo.isHitTop = true;
}

void BlockDetector::BottomCollision(CollisionInfo& collisionInfo) {
	if (collisionInfo.move.y >= 0.0f)
		return;

	const Vector3 center = worldTransform_.translation_;

	auto resolveCorner = [&](Corner corner, float& outTop) -> bool {
		Vector3 oldPos = CornerPosition(center, corner);
		Vector3 newPos = CornerPosition(center + collisionInfo.move, corner);

		constexpr float kStep = 0.1f;
		float distance = oldPos.y - newPos.y;
		int steps = std::max(1, static_cast<int>(std::ceil(distance / kStep)));

		for (int i = 0; i <= steps; ++i) {
			float t = static_cast<float>(i) / static_cast<float>(steps);

			Vector3 sample = {oldPos.x + (newPos.x - oldPos.x) * t, oldPos.y + (newPos.y - oldPos.y) * t, 0.0f};

			if (!IsSolid(sample, false)) {
				continue;
			}

			MapChipField::IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(sample);
			MapChipField::Rect rect = GetRect(index.xIndex, index.yIndex);

			const bool isOneWay = mapChipField_->IsOneWayAtPosition(sample);

			if (isOneWay) {
				if (oldPos.y < rect.top - 0.001f) {
					continue;
				}
				if (ignoreOneWay_) {
					continue;
				}
			} else {
				MapChipType above = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex - 1);
				if (above == MapChipType::kBlock) {
					continue;
				}
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

	float footY = worldTransform_.translation_.y - (kHeight * 0.5f);

	float clampedMoveY = collisionInfo.move.y;

	if (hitLeft) {
		clampedMoveY = std::max(clampedMoveY, (topLeft + kBlank) - footY);
	}
	if (hitRight) {
		clampedMoveY = std::max(clampedMoveY, (topRight + kBlank) - footY);
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

	bool hitTop = IsSolid(positionNew[kTopLeft], ignoreBarrier_);
	bool hitBottom = IsSolid(positionNew[kBottomLeft], ignoreBarrier_);
	if (!hitTop && !hitBottom)
		return;

	float clampedMoveX = collisionInfo.move.x;
	if (hitTop) {
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kTopLeft]);
		MapChipField::Rect rect = GetRect(indexSet.xIndex, indexSet.yIndex);
		clampedMoveX = std::max(clampedMoveX, std::max(0.0f, (rect.right - worldTransform_.translation_.x) + (kWidth / 2.0f) + kBlank));
	}
	if (hitBottom) {
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kBottomLeft]);
		MapChipField::Rect rect = GetRect(indexSet.xIndex, indexSet.yIndex);
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

	bool hitTop = IsSolid(positionNew[kTopRight], ignoreBarrier_);
	bool hitBottom = IsSolid(positionNew[kBottomRight], ignoreBarrier_);
	if (!hitTop && !hitBottom)
		return;

	float clampedMoveX = collisionInfo.move.x;
	if (hitTop) {
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kTopRight]);
		MapChipField::Rect rect = GetRect(indexSet.xIndex, indexSet.yIndex);
		clampedMoveX = std::min(clampedMoveX, std::min(0.0f, (rect.left - worldTransform_.translation_.x) - (kWidth / 2.0f) - kBlank));
	}
	if (hitBottom) {
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kBottomRight]);
		MapChipField::Rect rect = GetRect(indexSet.xIndex, indexSet.yIndex);
		clampedMoveX = std::min(clampedMoveX, std::min(0.0f, (rect.left - worldTransform_.translation_.x) - (kWidth / 2.0f) - kBlank));
	}

	collisionInfo.move.x = clampedMoveX;
	collisionInfo.isHitWall = true;
}

Vector3 BlockDetector::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorner] = {
	    {-kWidth / 2.0f, kHeight / 2.0f,  0.0f},
	    {kWidth / 2.0f,  kHeight / 2.0f,  0.0f},
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0.0f},
	    {kWidth / 2.0f,  -kHeight / 2.0f, 0.0f},
	};
	Vector3 result;
	result.x = center.x + offsetTable[static_cast<uint32_t>(corner)].x;
	result.y = center.y + offsetTable[static_cast<uint32_t>(corner)].y;
	result.z = center.z + offsetTable[static_cast<uint32_t>(corner)].z;
	return result;
}