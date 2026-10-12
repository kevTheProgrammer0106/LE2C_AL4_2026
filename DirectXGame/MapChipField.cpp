#define NOMINMAX

#include "MapChipField.h"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <map>
#include <sstream>
#include <queue>
#include <unordered_set>

using namespace KamataEngine;

uint32_t MapChipField::GetNumBlockVertical() const { return totalBlockVertical_; }
uint32_t MapChipField::GetNumBlockHorizontal() const { return totalBlockHorizontal_; }

namespace {
std::map<char, MapChipType> mapChipTypeTable = {
    {'B', MapChipType::kBlock        },
    {'P', MapChipType::kPlayer       },
    {'E', MapChipType::kEnemy        },
    {'T', MapChipType::kPlatform     },
    {'N', MapChipType::kPathNode     },
    {'J', MapChipType::kJumpNode     },
    {'L', MapChipType::kJumpLandTarget},
    {'X', MapChipType::kGoal         },
    {'1', MapChipType::kGateUp       },
    {'2', MapChipType::kGateDown     },
    {'3', MapChipType::kGateLeft     },
    {'4', MapChipType::kGateRight    },
    {'5', MapChipType::kArenaGateUp   }, // Triggers when player enters arena, closes all gates in that arena
    {'6', MapChipType::kArenaGateDown },
    {'7', MapChipType::kArenaGateLeft },
    {'8', MapChipType::kArenaGateRight },
    {'K', MapChipType::kKillBlock    },
    {'A', MapChipType::kBarrier      },
    {'M', MapChipType::kSlowBlock    },
    {'W', MapChipType::kSwitch       },
    {'R', MapChipType::kArenaTrigger },
    {'S', MapChipType::kEnemySpawner },
    {'C', MapChipType::kCheckpoint   },
	{'F', MapChipType::kArenaCamera},
    {'H', MapChipType::kHealthBlock   },
    {'G', MapChipType::kCameraTrigger },
    {'Y', MapChipType::kGlass         },
    {'V', MapChipType::kBreakable     },
    {'Z', MapChipType::kExplosive     },
    {'I', MapChipType::kSpike         },
    {'9', MapChipType::kFragile       },
    {'Q', MapChipType::kMeleeSpawner  },
    {'O', MapChipType::kFirearmSpawner},
};
} // namespace

TileInfo MapChipField::GetTileInfo(uint32_t xIndex, uint32_t yIndex) {
	TileInfo info;
	MapChipType type = GetMapChipTypeByIndex(xIndex, yIndex);

	switch (type) {
	case MapChipType::kBlock:
	case MapChipType::kGlass:
	case MapChipType::kBreakable:
	case MapChipType::kFragile:
		info.width = kBlockWidth;
		info.height = kBlockHeight;
		info.offset = {0.0f, 0.0f, 0.0f};
		info.isSolid = true;
		info.isOneWay = false;
		return info;
	case MapChipType::kExplosive:
	case MapChipType::kSpike:
		info.width = kBlockWidth;
		info.height = kBlockHeight;
		info.offset = {0.0f, 0.0f, 0.0f};
		info.isSolid = false;
		info.isOneWay = false;
		return info;
	case MapChipType::kPlatform:
		info.width = kBlockWidth;
		info.height = kBlockHeight * 0.25f; // was 0.5f at block size 2.0
		info.offset = {0.0f, 0.0f, 0.0f};
		info.isSolid = true;
		info.isOneWay = true;
		return info;
	case MapChipType::kSlowBlock:
		info.width = kBlockWidth;
		info.height = kBlockHeight;
		info.offset = {0.0f, 0.0f, 0.0f};
		info.isSolid = true;
		info.isOneWay = false;
		info.isSlow = true;
		return info;
	case MapChipType::kSwitch:
		info.width = kBlockWidth * 0.8f;                   // was 1.6f at block size 2.0
		info.height = kBlockHeight * 0.3f;                 // was 0.6f at block size 2.0
		info.offset = {0.0f, -kBlockHeight * 0.35f, 0.0f}; // was -0.7f at block size 2.0
		info.isSolid = false;
		return info;
	case MapChipType::kBarrier:
		info.width = kBlockWidth;
		info.height = kBlockHeight;
		info.offset = {0.0f, 0.0f, 0.0f};
		info.isSolid = true;
		info.isOneWay = false;
		return info;
	case MapChipType::kKillBlock:
		info.width = kBlockWidth;
		info.height = kBlockHeight;
		info.offset = {0.0f, 0.0f, 0.0f};
		info.isSolid = false;
		return info;
	default:
		return info;
	}
}

void MapChipField::ResetMapChipData() {
	mapChipData_.data.clear();
	mapChipData_.data.resize(totalBlockVertical_);
	for (std::vector<MapChipDataUnit>& line : mapChipData_.data) {
		line.resize(totalBlockHorizontal_);
		for (MapChipDataUnit& unit : line) {
			unit.type = MapChipType::kBlank;
			unit.subID = 0;
			unit.linkID = 0;
		}
	}
}

void MapChipField::Initialize(uint32_t chunkCountX, uint32_t chunkCountY) {
	chunkCountX = std::clamp(chunkCountX, 1u, kMaxFieldChunks);
	chunkCountY = std::clamp(chunkCountY, 1u, kMaxVerticalChunks);
	totalBlockHorizontal_ = kChunkBlockSize * chunkCountX;
	totalBlockVertical_ = kChunkBlockSize * chunkCountY;
	ResetMapChipData();
}

void MapChipField::LoadCsvIntoRegion(const std::string& filePath, uint32_t xOffset, uint32_t width, uint32_t yOffset, uint32_t height) {
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

	std::stringstream mapChipCsv;
	mapChipCsv << file.rdbuf();
	file.close();

	for (uint32_t i = 0; i < height; ++i) {
		std::string line;
		if (!std::getline(mapChipCsv, line)) {
			break;
		}
		uint32_t y = yOffset + i;
		if (y >= totalBlockVertical_) {
			continue;
		}

		std::istringstream lineStream(line);

		for (uint32_t j = 0; j < width; ++j) {
			std::string word;
			if (!std::getline(lineStream, word, ',')) {
				break;
			}
			if (word.empty()) {
				continue;
			}
			if (!mapChipTypeTable.contains(word[kChipType])) {
				continue;
			}

			uint32_t x = xOffset + j;
			if (x >= totalBlockHorizontal_) {
				continue;
			}

			MapChipDataUnit& unit = mapChipData_.data[y][x];
			unit.type = mapChipTypeTable[word[kChipType]];
			unit.subID = 0;
			unit.linkID = 0;

			if (word.size() > kChipSubID) {
				unit.subID = static_cast<uint8_t>(word[kChipSubID] - '0');
			}
			if (word.size() > kChipLinkID) {
				unit.linkID = static_cast<uint8_t>(word[kChipLinkID] - '0');
			}
		}
	}
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	Initialize(1, kMaxVerticalChunks);
	LoadCsvIntoRegion(filePath, 0, kChunkBlockSize, kGroundChunkRow * kChunkBlockSize, kChunkBlockSize);
}

void MapChipField::LoadChunkCsv(uint32_t chunkIndex, const std::string& filePath) { LoadChunkCsv(chunkIndex, kGroundChunkRow, filePath); }

void MapChipField::LoadChunkCsv(uint32_t chunkX, uint32_t chunkY, const std::string& filePath) {
	uint32_t xOffset = chunkX * kChunkBlockSize;
	uint32_t yOffset = chunkY * kChunkBlockSize;
#ifdef _DEBUG
	assert(xOffset + kChunkBlockSize <= totalBlockHorizontal_);
	assert(yOffset + kChunkBlockSize <= totalBlockVertical_);
#endif
	LoadCsvIntoRegion(filePath, xOffset, kChunkBlockSize, yOffset, kChunkBlockSize);
}

void MapChipField::UnloadChunk(uint32_t chunkIndex) {
	uint32_t xStart = chunkIndex * kChunkBlockSize;
	uint32_t xEnd = std::min(xStart + kChunkBlockSize, totalBlockHorizontal_);
	for (uint32_t i = 0; i < totalBlockVertical_; ++i) {
		for (uint32_t j = xStart; j < xEnd; ++j) {
			mapChipData_.data[i][j].type = MapChipType::kBlank;
			mapChipData_.data[i][j].subID = 0;
			mapChipData_.data[i][j].linkID = 0;
		}
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {
	if (totalBlockHorizontal_ <= xIndex)
		return MapChipType::kBlank;
	if (totalBlockVertical_ <= yIndex)
		return MapChipType::kBlank;
	return mapChipData_.data[yIndex][xIndex].type;
}

uint8_t MapChipField::GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t yIndex) {
	if (totalBlockHorizontal_ <= xIndex)
		return 0;
	if (totalBlockVertical_ <= yIndex)
		return 0;
	return mapChipData_.data[yIndex][xIndex].subID;
}

uint8_t MapChipField::GetMapChipLinkIDByIndex(uint32_t xIndex, uint32_t yIndex) {
	if (totalBlockHorizontal_ <= xIndex)
		return 0;
	if (totalBlockVertical_ <= yIndex)
		return 0;
	return mapChipData_.data[yIndex][xIndex].linkID;
}

MapChipField::Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) {
	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);
	TileInfo info = GetTileInfo(xIndex, yIndex);

	center.x += info.offset.x;
	center.y += info.offset.y;

	Rect rect;
	rect.left = center.x - info.width / 2.0f;
	rect.right = center.x + info.width / 2.0f;
	rect.bottom = center.y - info.height / 2.0f;
	rect.top = center.y + info.height / 2.0f;
	return rect;
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) { return Vector3(kBlockWidth * xIndex, kBlockHeight * (static_cast<float>(totalBlockVertical_) - 1 - yIndex), 0); }

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) {
	IndexSet indexSet = {};
	indexSet.xIndex = static_cast<uint32_t>((position.x + kBlockWidth / 2.0f) / kBlockWidth);
	indexSet.yIndex = static_cast<uint32_t>((position.y + kBlockHeight / 2.0f) / kBlockHeight);
	indexSet.yIndex = totalBlockVertical_ - 1 - indexSet.yIndex;
	return indexSet;
}

bool MapChipField::IsSolidAtPosition(const Vector3& position, bool ignoreBarrier) {
	IndexSet indexSet = GetMapChipIndexSetByPosition(position);
	MapChipType type = GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	bool solidType = IsGroundType(type) || (type == MapChipType::kBarrier && !ignoreBarrier);
	if (!solidType)
		return false;

	Rect rect = GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
	return position.x >= rect.left && position.x <= rect.right && position.y >= rect.bottom && position.y <= rect.top;
}

bool MapChipField::IsOneWayAtPosition(const Vector3& position) {
	IndexSet indexSet = GetMapChipIndexSetByPosition(position);
	TileInfo info = GetTileInfo(indexSet.xIndex, indexSet.yIndex);
	return info.isOneWay;
}

bool MapChipField::IsKillAtPosition(const Vector3& position) {
	IndexSet indexSet = GetMapChipIndexSetByPosition(position);
	MapChipType type = GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (type != MapChipType::kKillBlock)
		return false;
	Rect rect = GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
	return position.x >= rect.left && position.x <= rect.right && position.y >= rect.bottom && position.y <= rect.top;
}

bool MapChipField::IsSlowAtPosition(const Vector3& position) {
	IndexSet indexSet = GetMapChipIndexSetByPosition(position);
	MapChipType type = GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (type != MapChipType::kSlowBlock)
		return false;
	Rect rect = GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
	return position.x >= rect.left && position.x <= rect.right && position.y >= rect.bottom && position.y <= rect.top;
}
void MapChipField::ClearMapChipByIndex(uint32_t xIndex, uint32_t yIndex) {
	if (xIndex >= totalBlockHorizontal_ || yIndex >= totalBlockVertical_)
		return;
	MapChipDataUnit& unit = mapChipData_.data[yIndex][xIndex];
	unit.type = MapChipType::kBlank;
	unit.subID = 0;
	unit.linkID = 0;
}

std::vector<MapChipField::IndexSet> MapChipField::CollectConnectedByType(uint32_t xIndex, uint32_t yIndex, MapChipType type) {
	std::vector<IndexSet> result;
	if (GetMapChipTypeByIndex(xIndex, yIndex) != type)
		return result;

	auto key = [](uint32_t x, uint32_t y) { return (static_cast<uint64_t>(x) << 32) | y; };
	std::unordered_set<uint64_t> visited;
	std::queue<IndexSet> open;
	open.push({xIndex, yIndex});
	visited.insert(key(xIndex, yIndex));

	constexpr int dx[4] = {1, -1, 0, 0};
	constexpr int dy[4] = {0, 0, 1, -1};
	while (!open.empty()) {
		IndexSet cur = open.front();
		open.pop();
		result.push_back(cur);
		for (int d = 0; d < 4; ++d) {
			int nx = static_cast<int>(cur.xIndex) + dx[d];
			int ny = static_cast<int>(cur.yIndex) + dy[d];
			if (nx < 0 || ny < 0)
				continue;
			uint32_t ux = static_cast<uint32_t>(nx), uy = static_cast<uint32_t>(ny);
			if (GetMapChipTypeByIndex(ux, uy) != type || !visited.insert(key(ux, uy)).second)
				continue;
			open.push({ux, uy});
		}
	}
	return result;
}