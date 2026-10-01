#pragma once

#include <KamataEngine.h>
#include <string>
#include <vector>

struct StageData {
	std::string name;
	int32_t timeLimit;
	int32_t fieldCount;
};

class StageManager {
public:
	// --- Checkpoint data, survives GameScene reload (Retry), wiped on Restart / Main Menu ---
	struct CheckpointData {
		bool hasCheckpoint = false;
		KamataEngine::Vector3 playerPosition{};
		int32_t playerHP = 100;
		int32_t weaponModeIsFirearm = 0;
		int32_t meleeKind = 0;
		int32_t firearmKind = 0;
		uint32_t loadedChunkCount = 1;
		uint32_t oldestLoadedChunk = 0;
		uint32_t loadedTopChunkRow = 1;
		uint32_t loadedBottomChunkRow = 1;
	};

	void SetCheckpoint(const CheckpointData& data) { checkpoint_ = data; }
	const CheckpointData& GetCheckpoint() const { return checkpoint_; }
	bool HasCheckpoint() const { return checkpoint_.hasCheckpoint; }
	void ClearCheckpoint() { checkpoint_ = CheckpointData{}; }

	void LoadStageDataFile();

	const StageData& GetStageData(int32_t index) const {
#ifdef _DEBUG
		assert(index >= 0 && index < static_cast<int32_t>(stageDatas_.size()));
#endif
		return stageDatas_[index];
	}
	void SetCurrentStageIndex(int32_t index) {
#ifdef _DEBUG
		assert(index >= 0 && index < static_cast<int32_t>(stageDatas_.size()));
#endif
		currentStageIndex_ = index;
	}
	int32_t GetCurrentStageIndex() const { return currentStageIndex_; }
	int32_t GetStageCount() const { return static_cast<int32_t>(stageDatas_.size()); }
	void SetCurrentStageIndexByName(const std::string& name);
	const StageData& GetCurrentStageData() const { return GetStageData(currentStageIndex_); }
	int32_t GetFieldCount(int32_t stageIndex) const { return GetStageData(stageIndex).fieldCount; }

	std::string GetFieldChunkFileName(int32_t stageIndex, uint32_t chunkIndex) const;
	std::string GetVerticalFieldChunkFileName(int32_t stageIndex, uint32_t chunkX, int32_t rowOffset) const;
	std::string GetBackgroundFieldChunkFileName(int32_t stageIndex, uint32_t chunkIndex) const;

private:
	friend class DebugMenu;

	int32_t currentStageIndex_ = 0;
	std::vector<StageData> stageDatas_;
	CheckpointData checkpoint_;
};