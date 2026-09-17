#include "StageManager.h"

#include <algorithm>
#include <fstream>
#include <sstream>

void StageManager::LoadStageDataFile() {
	const std::string filePath = "Resources/stageDatas.csv";

	std::ifstream file;
	file.open(filePath);
#ifdef _DEBUG
	assert(file.is_open() && "ステージデータファイルが存在しません");
#endif

	std::stringstream stageDataCsv;
	stageDataCsv << file.rdbuf();
	file.close();

	std::string line;
	while (std::getline(stageDataCsv, line)) {
		if (line.empty()) {
			continue;
		}

		StageData stageData;
		std::istringstream lineStream(line);
		std::string word;

		std::getline(lineStream, word, ',');
		stageData.name = word;

		std::getline(lineStream, word, ',');
		stageData.timeLimit = std::stoi(word);

		// 3列目: フィールドチャンク数(省略可)
		if (std::getline(lineStream, word, ',') && !word.empty()) {
			stageData.fieldCount = std::stoi(word);
		} else {
			stageData.fieldCount = 1;
		}
		stageData.fieldCount = std::clamp(stageData.fieldCount, 1, 5);

		stageDatas_.push_back(stageData);
	}
}

void StageManager::SetCurrentStageIndexByName(const std::string& name) {
	for (size_t i = 0; i < stageDatas_.size(); ++i) {
		if (stageDatas_[i].name == name) {
			currentStageIndex_ = static_cast<int32_t>(i);
			return;
		}
	}
#ifdef _DEBUG
	assert(false && "指定されたステージ名はぞんざいしません");
#endif
}

std::string StageManager::GetFieldChunkFileName(int32_t stageIndex, uint32_t chunkIndex) const {
	const StageData& data = GetStageData(stageIndex);
	std::string path = "Resources/fields/" + data.name;
	if (chunkIndex > 0) {
		path += "_" + std::to_string(chunkIndex + 1); // chunk1 -> "_2", chunk2 -> "_3", ...
	}
	path += ".csv";
	return path;
}

std::string StageManager::GetBackgroundFieldChunkFileName(int32_t stageIndex, uint32_t chunkIndex) const {
	std::string path = GetFieldChunkFileName(stageIndex, chunkIndex);
	size_t dot = path.rfind(".csv");
	if (dot == std::string::npos) {
		return path + "_bg";
	}
	return path.insert(dot, "_bg");
}

std::string StageManager::GetVerticalFieldChunkFileName(int32_t stageIndex, uint32_t chunkX, int32_t rowOffset) const {
	std::string path = GetFieldChunkFileName(stageIndex, chunkX);
	if (rowOffset == 0) {
		return path;
	}
	std::string suffix = (rowOffset > 0) ? ("_u" + std::to_string(rowOffset)) : ("_d" + std::to_string(-rowOffset));
	size_t dot = path.rfind(".csv");
	if (dot == std::string::npos) {
		return path + suffix;
	}
	return path.insert(dot, suffix);
}