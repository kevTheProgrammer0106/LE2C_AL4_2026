#define NOMINMAX

#include "MapChipField.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <map>
#include <queue>
#include <sstream>
#include <unordered_set>

using namespace KamataEngine;

uint32_t MapChipField::GetNumBlockVertical() const { return totalBlockVertical_; }
uint32_t MapChipField::GetNumBlockHorizontal() const { return totalBlockHorizontal_; }

namespace {
std::map<char, MapChipType> mapChipTypeTable = {
    {'B', MapChipType::kBlock         },
    {'P', MapChipType::kPlayer        },
    {'E', MapChipType::kEnemy         },
    {'T', MapChipType::kPlatform      },
    {'N', MapChipType::kPathNode      },
    {'J', MapChipType::kJumpNode      },
    {'L', MapChipType::kJumpLandTarget},
    {'X', MapChipType::kGoal          },
    {'1', MapChipType::kGateUp        },
    {'2', MapChipType::kGateDown      },
    {'3', MapChipType::kGateLeft      },
    {'4', MapChipType::kGateRight     },
    {'5', MapChipType::kArenaGateUp   },
    {'6', MapChipType::kArenaGateDown },
    {'7', MapChipType::kArenaGateLeft },
    {'8', MapChipType::kArenaGateRight},
    {'K', MapChipType::kKillBlock     },
    {'A', MapChipType::kBarrier       },
    {'M', MapChipType::kSlowBlock     },
    {'W', MapChipType::kSwitch        },
    {'R', MapChipType::kArenaTrigger  },
    {'S', MapChipType::kEnemySpawner  },
    {'C', MapChipType::kCheckpoint    },
    {'F', MapChipType::kArenaCamera   },
    {'H', MapChipType::kHealthBlock   },
    {'G', MapChipType::kCameraTrigger },
    {'Y', MapChipType::kGlass         },
    {'V', MapChipType::kBreakable     },
    {'Z', MapChipType::kExplosive     },
    {'I', MapChipType::kSpike         },
    {'9', MapChipType::kFragile       },
    {'Q', MapChipType::kMeleeSpawner  },
    {'O', MapChipType::kFirearmSpawner},
    {'U', MapChipType::kTutorialBlock },
};

constexpr MapChipDataUnit kBlankUnit{MapChipType::kBlank, 0, 0};
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
		info.height = kBlockHeight * 0.25f;
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
		info.width = kBlockWidth * 0.8f;
		info.height = kBlockHeight * 0.3f;
		info.offset = {0.0f, -kBlockHeight * 0.35f, 0.0f};
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

void MapChipField::ResetMapChipData() { mapChipData_.data.assign(totalBlockVertical_, std::vector<MapChipDataUnit>(totalBlockHorizontal_, kBlankUnit)); }

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

			uint8_t rawSubID = 0;
			uint8_t rawLinkID = 0;

			// CSVのsubIDとlinkIDを別々に読み込む。
			if (word.size() > kChipSubID && word[kChipSubID] >= '0' && word[kChipSubID] <= '9') {
				rawSubID = static_cast<uint8_t>(word[kChipSubID] - '0');
			}

			if (word.size() > kChipLinkID && word[kChipLinkID] >= '0' && word[kChipLinkID] <= '9') {
				rawLinkID = static_cast<uint8_t>(word[kChipLinkID] - '0');
			}

			// モデル選択用とテクスチャ選択用のIDを保持する。
			unit.subID = rawSubID;
			unit.linkID = rawLinkID;
		}
	}
}

// Single-file load: the CSV becomes the base chunk (its bottom row lands on world y = 0)
void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	Initialize(1, kMaxVerticalChunks);
	LoadChunkCsv(0, kMainChunkRow, filePath);
}

// Row omitted -> the base chunk row
void MapChipField::LoadChunkCsv(uint32_t chunkIndex, const std::string& filePath) { LoadChunkCsv(chunkIndex, kMainChunkRow, filePath); }

void MapChipField::LoadChunkCsv(uint32_t chunkX, uint32_t chunkY, const std::string& filePath) {
	const uint32_t xOffset = chunkX * kChunkBlockSize;
	const uint32_t yOffset = chunkY * kChunkBlockSize;

#ifdef _DEBUG
	assert(xOffset + kChunkBlockSize <= totalBlockHorizontal_);
	assert(yOffset + kChunkBlockSize <= totalBlockVertical_);
#endif

	LoadCsvIntoRegion(filePath, xOffset, kChunkBlockSize, yOffset, kChunkBlockSize);
}

void MapChipField::UnloadChunk(uint32_t chunkIndex) {
	const uint32_t xStart = chunkIndex * kChunkBlockSize;
	const uint32_t xEnd = std::min(xStart + kChunkBlockSize, totalBlockHorizontal_);
	for (uint32_t y = 0; y < totalBlockVertical_; ++y) {
		for (uint32_t x = xStart; x < xEnd; ++x) {
			ClearMapChipByIndex(x, y);
		}
	}
}

const MapChipDataUnit* MapChipField::GetUnit(uint32_t xIndex, uint32_t yIndex) const {
	if (xIndex >= totalBlockHorizontal_ || yIndex >= totalBlockVertical_) {
		return nullptr;
	}
	return &mapChipData_.data[yIndex][xIndex];
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {
	const MapChipDataUnit* unit = GetUnit(xIndex, yIndex);
	return unit ? unit->type : MapChipType::kBlank;
}
uint8_t MapChipField::GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t yIndex) {
	const MapChipDataUnit* unit = GetUnit(xIndex, yIndex);
	return unit ? unit->subID : 0;
}
uint8_t MapChipField::GetMapChipLinkIDByIndex(uint32_t xIndex, uint32_t yIndex) {
	const MapChipDataUnit* unit = GetUnit(xIndex, yIndex);
	return unit ? unit->linkID : 0;
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

// index -> world. x starts at 0; the base chunk's bottom row is y = 0; rows above are positive, rows below negative.
// Adding or removing vertical chunks never moves the base chunk.
Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) {
	const float worldY = kBlockHeight * (static_cast<float>(kBaseWorldBlockHeight) - 1.0f - static_cast<float>(yIndex));
	return Vector3(kBlockWidth * static_cast<float>(xIndex), worldY, 0.0f);
}

// world -> index (exact inverse of the above). Out of range returns the "total" index, which reads as kBlank.
MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) {
	constexpr float kLimit = 1.0e6f; // keeps the float -> int conversion defined

	// floor, not a cast: a cast truncates toward zero and is undefined for negatives (x < 0, rows below the base chunk)
	const int32_t worldXIndex = static_cast<int32_t>(std::floor(std::clamp((position.x + kBlockWidth * 0.5f) / kBlockWidth, -kLimit, kLimit)));
	const int32_t worldYIndex = static_cast<int32_t>(std::floor(std::clamp((position.y + kBlockHeight * 0.5f) / kBlockHeight, -kLimit, kLimit)));
	const int32_t mapYIndex = static_cast<int32_t>(kBaseWorldBlockHeight) - 1 - worldYIndex;

	IndexSet indexSet = {};
	indexSet.xIndex = (worldXIndex < 0 || worldXIndex >= static_cast<int32_t>(totalBlockHorizontal_)) ? totalBlockHorizontal_ : static_cast<uint32_t>(worldXIndex);
	indexSet.yIndex = (mapYIndex < 0 || mapYIndex >= static_cast<int32_t>(totalBlockVertical_)) ? totalBlockVertical_ : static_cast<uint32_t>(mapYIndex);
	return indexSet;
}

// Shared by the Is...AtPosition checks: is the point inside the tile's real rect?
bool MapChipField::IsInsideCell(const Vector3& position, const IndexSet& index) {
	const Rect rect = GetRectByIndex(index.xIndex, index.yIndex);
	return position.x >= rect.left && position.x <= rect.right && position.y >= rect.bottom && position.y <= rect.top;
}

bool MapChipField::IsSolidAtPosition(const Vector3& position, bool ignoreBarrier) {
	const IndexSet index = GetMapChipIndexSetByPosition(position);
	const MapChipType type = GetMapChipTypeByIndex(index.xIndex, index.yIndex);
	const bool solidType = IsGroundType(type) || (type == MapChipType::kBarrier && !ignoreBarrier);
	return solidType && IsInsideCell(position, index);
}

bool MapChipField::IsOneWayAtPosition(const Vector3& position) {
	const IndexSet index = GetMapChipIndexSetByPosition(position);
	return GetTileInfo(index.xIndex, index.yIndex).isOneWay;
}

bool MapChipField::IsKillAtPosition(const Vector3& position) {
	const IndexSet index = GetMapChipIndexSetByPosition(position);
	return GetMapChipTypeByIndex(index.xIndex, index.yIndex) == MapChipType::kKillBlock && IsInsideCell(position, index);
}

bool MapChipField::IsSlowAtPosition(const Vector3& position) {
	const IndexSet index = GetMapChipIndexSetByPosition(position);
	return GetMapChipTypeByIndex(index.xIndex, index.yIndex) == MapChipType::kSlowBlock && IsInsideCell(position, index);
}

void MapChipField::ClearMapChipByIndex(uint32_t xIndex, uint32_t yIndex) { SetMapChipByIndex(xIndex, yIndex, MapChipType::kBlank, 0, 0); }

void MapChipField::SetMapChipByIndex(uint32_t xIndex, uint32_t yIndex, MapChipType type, uint8_t subID, uint8_t linkID) {
	if (xIndex >= totalBlockHorizontal_ || yIndex >= totalBlockVertical_)
		return;
	MapChipDataUnit& unit = mapChipData_.data[yIndex][xIndex];
	unit.type = type;
	unit.subID = subID;
	unit.linkID = linkID;
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