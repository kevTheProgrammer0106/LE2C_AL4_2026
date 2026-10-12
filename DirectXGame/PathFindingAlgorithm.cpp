#define NOMINMAX
#include "PathFindingAlgorithm.h"
#include "BaseEnemy.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <unordered_map>
#include <vector>

namespace {

uint64_t MakeKey(uint32_t x, uint32_t y) { return (static_cast<uint64_t>(x) << 32) | static_cast<uint64_t>(y); }

/// <summary>
/// A*のオープンリストに格納するノード。
/// </summary>
struct OpenNode {
	PathFindingAlgorithm::IndexSet index{};
	float gCost = 0.0f;     // 開始地点からこのノードまでの実コスト
	float fCost = 0.0f;     // gCost + ヒューリスティック
	float heuristic = 0.0f; // 同じfCostだった場合の比較用
};

struct OpenNodeCompare {
	bool operator()(const OpenNode& left, const OpenNode& right) const {
		if (std::abs(left.fCost - right.fCost) > 0.0001f) {
			return left.fCost > right.fCost;
		}
		return left.heuristic > right.heuristic;
	}
};
// 上方向(=ジャンプ)への1マス移動に加算するコスト。歩ける迂回路があればそちらを優先させる
constexpr float kUpwardStepPenalty = 0.75f;
} // namespace

const PathFindingAlgorithm::PathCostProfile& PathFindingAlgorithm::GetDefaultPathCostProfile() {
	static const PathCostProfile kDefaultProfile{};
	return kDefaultProfile;
}

bool PathFindingAlgorithm::IsInsideMap(MapChipField* mapChipField, uint32_t xIndex, uint32_t yIndex) {
	if (!mapChipField) {
		return false;
	}
	if (xIndex >= mapChipField->GetNumBlockHorizontal()) {
		return false;
	}
	if (yIndex >= mapChipField->GetNumBlockVertical()) {
		return false;
	}
	return true;
}

bool PathFindingAlgorithm::IsWalkable(MapChipField* mapChipField, uint32_t xIndex, uint32_t yIndex) {
	if (!IsInsideMap(mapChipField, xIndex, yIndex)) {
		return false;
	}
	MapChipType type = mapChipField->GetMapChipTypeByIndex(xIndex, yIndex);
	return type != MapChipType::kBlock && type != MapChipType::kSlowBlock && type != MapChipType::kGlass && type != MapChipType::kBreakable && type != MapChipType::kFragile;
}

uint32_t PathFindingAlgorithm::GetAgentHeightCells(MapChipField* mapChipField) {
	uint32_t numRows = mapChipField->GetNumBlockVertical();
	if (numRows == 0) {
		return 1;
	}
	float blockHeight = mapChipField->GetWorldHeight() / static_cast<float>(numRows);
	if (blockHeight <= 0.0001f) {
		return 1;
	}
	// 足元はセル下端付近に立つので、体高さ/ブロック高さを切り上げたセル数を占める。
	// 0.05fの余裕は、ちょうどセル数ぴったりの体高さ(例: 2.0/1.0)で1セル余分に取らないため。
	float cells = std::ceil(BaseEnemy::GetBodyHeight() / blockHeight - 0.05f);
	return static_cast<uint32_t>(std::max(cells, 1.0f));
}

bool PathFindingAlgorithm::HasHeadroom(MapChipField* mapChipField, uint32_t xIndex, uint32_t yIndex) {
	uint32_t cells = GetAgentHeightCells(mapChipField);
	// yIndexは上が0。体の上側のセルはyIndex-1, yIndex-2, ...
	for (uint32_t k = 1; k < cells; ++k) {
		if (yIndex < k) {
			return false; // マップ上端を突き抜ける
		}
		if (!IsWalkable(mapChipField, xIndex, yIndex - k)) {
			return false;
		}
	}
	return true;
}

bool PathFindingAlgorithm::CanMoveBetween(MapChipField* mapChipField, const IndexSet& current, const IndexSet& next) {
	if (!mapChipField) {
		return false;
	}
	if (!IsWalkable(mapChipField, next.xIndex, next.yIndex)) {
		return false;
	}

	int deltaX = static_cast<int>(next.xIndex) - static_cast<int>(current.xIndex);
	int deltaY = static_cast<int>(next.yIndex) - static_cast<int>(current.yIndex);

	// 上下左右の隣接セル以外への移動は禁止
	if (std::abs(deltaX) + std::abs(deltaY) != 1) {
		return false;
	}

	// [FIX] 体が2セル分の高さを持つので、移動先の頭上にも空きが必要
	if (!HasHeadroom(mapChipField, next.xIndex, next.yIndex)) {
		return false;
	}

	return true;
}

float PathFindingAlgorithm::GetTraversalCost(MapChipField* mapChipField, uint32_t xIndex, uint32_t yIndex, const PathCostProfile& profile) {
	if (!IsInsideMap(mapChipField, xIndex, yIndex)) {
		return std::numeric_limits<float>::infinity();
	}

	MapChipType type = mapChipField->GetMapChipTypeByIndex(xIndex, yIndex);

	switch (type) {
	case MapChipType::kBlock:
	case MapChipType::kSlowBlock:
	case MapChipType::kGlass:
	case MapChipType::kBreakable:
	case MapChipType::kFragile:
		// IsWalkableで除外されるため通常ここには来ない
		return std::numeric_limits<float>::infinity();
	case MapChipType::kSpike:
		return 4.0f;
	case MapChipType::kPlatform:
		return 1.0f;
	case MapChipType::kPathNode: {
		// linkIDがルーティング優先度tier(旧subID)。テクスチャ変更はもはやコストに影響しない。
		uint8_t tier = mapChipField->GetMapChipLinkIDByIndex(xIndex, yIndex);
		size_t tableIndex = std::min<size_t>(tier, profile.pathNodeCosts.size() - 1);
		return profile.pathNodeCosts[tableIndex];
	}
	case MapChipType::kJumpNode: {
		uint8_t tier = mapChipField->GetMapChipLinkIDByIndex(xIndex, yIndex);
		size_t tableIndex = std::min<size_t>(tier, profile.jumpNodeCosts.size() - 1);
		return profile.jumpNodeCosts[tableIndex];
	}
	default:
		return 1.0f;
	}
}

float PathFindingAlgorithm::CalculateHeuristic(const IndexSet& current, const IndexSet& goal) {
	float distanceX = static_cast<float>(std::abs(static_cast<int>(goal.xIndex) - static_cast<int>(current.xIndex)));
	float distanceY = static_cast<float>(std::abs(static_cast<int>(goal.yIndex) - static_cast<int>(current.yIndex)));
	return distanceX + distanceY;
}

std::vector<PathFindingAlgorithm::IndexSet>
    PathFindingAlgorithm::FindPath(MapChipField* mapChipField, const IndexSet& start, const IndexSet& goal, uint32_t maxSearchNodes, const PathCostProfile* costProfile) {
	std::vector<IndexSet> path;

	if (!mapChipField || maxSearchNodes == 0) {
		return path;
	}
	if (!IsInsideMap(mapChipField, start.xIndex, start.yIndex) || !IsInsideMap(mapChipField, goal.xIndex, goal.yIndex)) {
		return path;
	}
	if (!IsWalkable(mapChipField, start.xIndex, start.yIndex) || !IsWalkable(mapChipField, goal.xIndex, goal.yIndex)) {
		return path; // 開始/ゴールが壁の中なら探索不可
	}

	const PathCostProfile& profile = costProfile ? *costProfile : GetDefaultPathCostProfile();

	uint64_t startKey = MakeKey(start.xIndex, start.yIndex);
	uint64_t goalKey = MakeKey(goal.xIndex, goal.yIndex);

	if (startKey == goalKey) {
		return path; // 既に同じセル
	}

	std::priority_queue<OpenNode, std::vector<OpenNode>, OpenNodeCompare> openList;
	std::unordered_map<uint64_t, uint64_t> cameFrom;   // 子キー -> 親キー
	std::unordered_map<uint64_t, IndexSet> nodeLookup; // キー -> 実際のIndexSet
	std::unordered_map<uint64_t, float> gScore;        // キー -> 開始地点からの最小コスト

	float startHeuristic = CalculateHeuristic(start, goal);
	openList.push(OpenNode{start, 0.0f, startHeuristic, startHeuristic});
	cameFrom[startKey] = startKey;
	nodeLookup[startKey] = start;
	gScore[startKey] = 0.0f;

	bool found = false;
	uint32_t searchedNodeCount = 0;

	constexpr int kDirectionCount = 4;
	const int deltaX[kDirectionCount] = {1, -1, 0, 0};
	const int deltaY[kDirectionCount] = {0, 0, 1, -1};

	while (!openList.empty() && searchedNodeCount < maxSearchNodes) {
		OpenNode currentNode = openList.top();
		openList.pop();

		uint64_t currentKey = MakeKey(currentNode.index.xIndex, currentNode.index.yIndex);

		auto currentGScoreIterator = gScore.find(currentKey);
		if (currentGScoreIterator == gScore.end()) {
			continue;
		}
		// priority_queue内に残っている古い候補は無視する
		if (currentNode.gCost > currentGScoreIterator->second + 0.0001f) {
			continue;
		}

		++searchedNodeCount;

		if (currentKey == goalKey) {
			found = true;
			break;
		}

		for (int direction = 0; direction < kDirectionCount; ++direction) {
			int nextX = static_cast<int>(currentNode.index.xIndex) + deltaX[direction];
			int nextY = static_cast<int>(currentNode.index.yIndex) + deltaY[direction];
			if (nextX < 0 || nextY < 0) {
				continue;
			}

			uint32_t nextXIndex = static_cast<uint32_t>(nextX);
			uint32_t nextYIndex = static_cast<uint32_t>(nextY);
			if (!IsInsideMap(mapChipField, nextXIndex, nextYIndex)) {
				continue;
			}

			IndexSet nextIndex{nextXIndex, nextYIndex};
			if (!CanMoveBetween(mapChipField, currentNode.index, nextIndex)) {
				continue;
			}

			float traversalCost = GetTraversalCost(mapChipField, nextXIndex, nextYIndex, profile);
			if (!std::isfinite(traversalCost) || traversalCost <= 0.0f) {
				continue;
			}

			if (deltaY[direction] == -1) {
				traversalCost += kUpwardStepPenalty;
			}

			float tentativeGCost = currentNode.gCost + traversalCost;
			uint64_t nextKey = MakeKey(nextXIndex, nextYIndex);

			auto knownGScoreIterator = gScore.find(nextKey);
			bool isBetterPath = knownGScoreIterator == gScore.end() || tentativeGCost < knownGScoreIterator->second;
			if (!isBetterPath) {
				continue;
			}

			cameFrom[nextKey] = currentKey;
			nodeLookup[nextKey] = nextIndex;
			gScore[nextKey] = tentativeGCost;

			float heuristic = CalculateHeuristic(nextIndex, goal);
			openList.push(OpenNode{nextIndex, tentativeGCost, tentativeGCost + heuristic, heuristic});
		}
	}

	if (!found) {
		return path; // 経路なし、または探索上限到達
	}

	std::vector<IndexSet> reversedPath;
	uint64_t traceKey = goalKey;
	while (traceKey != startKey) {
		auto nodeIterator = nodeLookup.find(traceKey);
		auto parentIterator = cameFrom.find(traceKey);
		if (nodeIterator == nodeLookup.end() || parentIterator == cameFrom.end()) {
			path.clear();
			return path; // 経路情報破損時の安全策
		}
		reversedPath.push_back(nodeIterator->second);
		uint64_t parentKey = parentIterator->second;
		if (parentKey == traceKey) {
			path.clear();
			return path; // 無限ループ防止
		}
		traceKey = parentKey;
	}

	std::reverse(reversedPath.begin(), reversedPath.end());
	return reversedPath;
}