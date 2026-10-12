#include "VisionSensor.h"
#include "GlobalVariables.h"
#include "MapChipField.h"
#include "Player.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "VisionSensor";

// XY平面上でベクトルを回転させる
Vector3 RotateXY(const Vector3& v, float angleRad) {
	float c = std::cos(angleRad);
	float s = std::sin(angleRad);
	return Vector3(v.x * c - v.y * s, v.x * s + v.y * c, v.z);
}

float DotXY(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y; }
Vector3 SubXY(const Vector3& a, const Vector3& b) { return Vector3(a.x - b.x, a.y - b.y, 0.0f); }
Vector3 NormalXY(const Vector3& v) { return Vector3(v.y, -v.x, 0.0f); }

struct ProjectionRangeXY {
	float min;
	float max;
};

ProjectionRangeXY ProjectXY(const Vector3& axis, const std::vector<Vector3>& verts) {
	float mn = DotXY(axis, verts[0]);
	float mx = mn;
	for (size_t i = 1; i < verts.size(); ++i) {
		float p = DotXY(axis, verts[i]);
		mn = std::min(mn, p);
		mx = std::max(mx, p);
	}
	return {mn, mx};
}

std::vector<Vector3> GetAxesXY(const std::vector<Vector3>& rect, const std::vector<Vector3>& tri) {
	std::vector<Vector3> axes;
	for (size_t i = 0; i < rect.size(); ++i) {
		axes.push_back(NormalXY(SubXY(rect[(i + 1) % rect.size()], rect[i])));
	}
	for (size_t i = 0; i < tri.size(); ++i) {
		axes.push_back(NormalXY(SubXY(tri[(i + 1) % tri.size()], tri[i])));
	}
	return axes;
}

// 以前はkBlockだけだったので、kSlowBlock/半ブロック越しにプレイヤーが見えてしまっていた。
// プラットフォーム(一方通行)とバリア(カメラ用の見えない壁)は遮らない。
bool BlocksLineOfSight(MapChipType type) { return type == MapChipType::kBlock || type == MapChipType::kBreakable || type == MapChipType::kFragile || type == MapChipType::kSlowBlock; }
} // namespace

void VisionSensor::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "VisionHalfAngle", kVisionHalfAngle);
	gv->AddItem(kGroupName, "VisionRadius", kVisionRadius);
	gv->AddItem(kGroupName, "RaySampleStep", kRaySampleStep);
}

void VisionSensor::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kVisionHalfAngle = gv->GetFloatValue(kGroupName, "VisionHalfAngle");
	kVisionRadius = gv->GetFloatValue(kGroupName, "VisionRadius");
	kRaySampleStep = gv->GetFloatValue(kGroupName, "RaySampleStep");
}

void VisionSensor::UpdateVisionCone(const Vector3& eyePos, const Vector3& facingDir) {
	float halfAngleRad = kVisionHalfAngle * (std::numbers::pi_v<float> / 180.0f);
	Vector3 leftDir = RotateXY(facingDir, halfAngleRad);
	Vector3 rightDir = RotateXY(facingDir, -halfAngleRad);

	visionTriangle_.center = eyePos;
	visionTriangle_.left = {eyePos.x + leftDir.x * kVisionRadius, eyePos.y + leftDir.y * kVisionRadius, eyePos.z};
	visionTriangle_.right = {eyePos.x + rightDir.x * kVisionRadius, eyePos.y + rightDir.y * kVisionRadius, eyePos.z};
	hasValidCone_ = true;
}

bool VisionSensor::SATCollisionRectTriangle(const std::vector<Vector3>& rect, const std::vector<Vector3>& tri) {
	std::vector<Vector3> axes = GetAxesXY(rect, tri);

	for (const Vector3& axis : axes) {
		ProjectionRangeXY pr = ProjectXY(axis, rect);
		ProjectionRangeXY pt = ProjectXY(axis, tri);
		if (pr.max < pt.min || pt.max < pr.min) {
			return false; // 分離軸が見つかった＝当たっていない
		}
	}
	return true;
}

bool VisionSensor::IsSegmentBlockedByMapChip(const Vector3& from, const Vector3& to) const {
	if (!mapChipField_) {
		return false;
	}

	Vector3 diff = SubXY(to, from);
	float distance = std::sqrtf(diff.x * diff.x + diff.y * diff.y);
	if (distance <= 0.0001f) {
		return false;
	}

	uint32_t sampleCount = static_cast<uint32_t>(distance / kRaySampleStep) + 1;

	for (uint32_t i = 0; i <= sampleCount; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(sampleCount);
		Vector3 samplePos = {from.x + diff.x * t, from.y + diff.y * t};

		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(samplePos);
		MapChipType type = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
		if (BlocksLineOfSight(type)) {
			return true; // 遮蔽物あり
		}
	}
	return false;
}
std::array<Vector3, VisionSensor::kNumConeMarkers> VisionSensor::GetDebugConePoints() const {
	std::array<Vector3, kNumConeMarkers> points{};
	size_t idx = 0;
	auto SampleEdge = [&](const Vector3& from, const Vector3& to) {
		for (uint32_t i = 0; i <= kSamplesPerEdge; ++i) {
			float t = static_cast<float>(i) / static_cast<float>(kSamplesPerEdge);
			points[idx++] = {from.x + (to.x - from.x) * t, from.y + (to.y - from.y) * t};
		}
	};
	SampleEdge(visionTriangle_.center, visionTriangle_.left);
	SampleEdge(visionTriangle_.center, visionTriangle_.right);
	SampleEdge(visionTriangle_.left, visionTriangle_.right);
	return points;
}
bool VisionSensor::CheckPlayerSpotted(const Vector3& eyePos, const Vector3& facingDir, const Player* player) {
	if (!player) {
		isPlayerSpotted_ = false;
		return false;
	}

	UpdateVisionCone(eyePos, facingDir);

	AABB playerAABB = const_cast<Player*>(player)->GetAABB();
	std::vector<Vector3> rectCorners = {
	    Vector3(playerAABB.min.x, playerAABB.max.y, 0.0f), // 左上
	    Vector3(playerAABB.max.x, playerAABB.max.y, 0.0f), // 右上
	    Vector3(playerAABB.max.x, playerAABB.min.y, 0.0f), // 右下
	    Vector3(playerAABB.min.x, playerAABB.min.y, 0.0f), // 左下
	};
	std::vector<Vector3> triVerts = {visionTriangle_.center, visionTriangle_.left, visionTriangle_.right};

	// 視野コーンとプレイヤーのAABBが重なっているか(SAT)
	if (!SATCollisionRectTriangle(rectCorners, triVerts)) {
		isPlayerSpotted_ = false;
		return false;
	}

	// 重なっていても壁越しなら見えない → 4隅のうち1つでも視線が通っていればOK
	bool anyCornerVisible = false;
	for (const Vector3& corner : rectCorners) {
		if (!IsSegmentBlockedByMapChip(visionTriangle_.center, corner)) {
			anyCornerVisible = true;
			break;
		}
	}

	isPlayerSpotted_ = anyCornerVisible;
	return isPlayerSpotted_;
}