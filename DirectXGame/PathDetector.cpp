#include "PathDetector.h"

using namespace KamataEngine;

std::vector<PathDetector::Segment> PathDetector::Scan(const Vector3& footPos, bool directionIsRight, const ScanParams& params) const {
	std::vector<Segment> result;
	if (!mapChipField_) {
		return result;
	}
	if (params.stepDistance <= 0.0f || params.maxDistance <= 0.0f) {
		return result;
	}

	float sign = directionIsRight ? 1.0f : -1.0f;
	uint32_t sampleCount = static_cast<uint32_t>(params.maxDistance / params.stepDistance);

	for (uint32_t i = 1; i <= sampleCount; ++i) {
		float distance = params.stepDistance * static_cast<float>(i);
		float x = footPos.x + sign * distance;

		Segment segment;
		segment.distance = distance;

		// --- 地面判定：足元より少し下を覗く ---
		Vector3 groundCheckPos(x, footPos.y - params.groundProbeDown, 0.0f);
		MapChipField::IndexSet groundIndex = mapChipField_->GetMapChipIndexSetByPosition(groundCheckPos);
		MapChipType groundType = mapChipField_->GetMapChipTypeByIndex(groundIndex.xIndex, groundIndex.yIndex);
		segment.hasGroundBelow = MapChipField::IsGroundType(groundType);

		// --- 壁判定：足元から指定した高さまで、下から上へブロックが続くかサンプリング ---
		bool wasBlocked = false;
		float topHeight = 0.0f;
		for (float h = 0.0f; h <= params.bodyHeight + params.heightSampleStep; h += params.heightSampleStep) {
			Vector3 wallCheckPos(x, footPos.y + h, 0.0f);
			MapChipField::IndexSet wallIndex = mapChipField_->GetMapChipIndexSetByPosition(wallCheckPos);
			MapChipType wallType = mapChipField_->GetMapChipTypeByIndex(wallIndex.xIndex, wallIndex.yIndex);

			// Platforms are intentionally excluded here: they are one-way and never
			// block horizontal movement (see BlockDetector::IsSolidIgnoringOneWay),
			// so they must never be treated as a "wall" to jump over.
			bool isWallSolid = (wallType == MapChipType::kBlock) || (wallType == MapChipType::kBlockHalfUp) || (wallType == MapChipType::kBlockHalfDown) || (wallType == MapChipType::kSlowBlock) ||
			                   (wallType == MapChipType::kBarrier);

			if (isWallSolid) {
				topHeight = h;
				wasBlocked = true;
			} else if (wasBlocked) {
				// ブロックの上端を通過した地点＝これが壁の高さ
				topHeight = h;
				break;
			}
		}
		segment.isWall = wasBlocked;
		segment.wallTopHeight = topHeight;

		result.push_back(segment);

		// 壁に当たったら、それより先の走査は打ち切る
		// （壁の向こう側の地形はジャンプ後・別スキャンで扱う）
		if (segment.isWall) {
			break;
		}
	}

	return result;
}

bool PathDetector::FirstGapInfo(const std::vector<Segment>& segments, float& outDistance) {
	for (const Segment& segment : segments) {
		if (!segment.hasGroundBelow && !segment.isWall) {
			outDistance = segment.distance;
			return true;
		}
	}
	return false;
}

bool PathDetector::FirstWallInfo(const std::vector<Segment>& segments, float& outDistance, float& outHeight) {
	for (const Segment& segment : segments) {
		if (segment.isWall) {
			outDistance = segment.distance;
			outHeight = segment.wallTopHeight;
			return true;
		}
	}
	return false;
}