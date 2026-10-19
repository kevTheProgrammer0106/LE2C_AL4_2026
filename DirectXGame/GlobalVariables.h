#pragma once

#include <KamataEngine.h>
#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <variant>

/// <summary>
/// グローバル変数。独立した "Global Variables" ウィンドウを持ち、
/// DebugMenuと同じタブバー形式で、全変数を1つのタブにまとめて表示する。
/// </summary>
class GlobalVariables {
public:
	static GlobalVariables* GetInstance();

	using Value = std::variant<int32_t, float, KamataEngine::Vector3>;

	// 項目
	struct Item {
		Value value;
	};

	// グループ
	struct Group {
		std::map<std::string, Item> items;
	};

	GlobalVariables(const GlobalVariables&) = delete;
	GlobalVariables& operator=(const GlobalVariables&) = delete;
	GlobalVariables(GlobalVariables&&) = delete;
	GlobalVariables& operator=(GlobalVariables&&) = delete;

	void CreateGroup(const std::string& groupName);

	// 値のセット
	void SetValue(const std::string& groupName, const std::string& key, int32_t value) { SetImpl(groupName, key, value); }
	void SetValue(const std::string& groupName, const std::string& key, float value) { SetImpl(groupName, key, value); }
	void SetValue(const std::string& groupName, const std::string& key, KamataEngine::Vector3 value) { SetImpl(groupName, key, value); }

	// 項目の追加(未登録のときのみ)
	void AddItem(const std::string& groupName, const std::string& key, int32_t value) { AddImpl(groupName, key, value); }
	void AddItem(const std::string& groupName, const std::string& key, float value) { AddImpl(groupName, key, value); }
	void AddItem(const std::string& groupName, const std::string& key, KamataEngine::Vector3 value) { AddImpl(groupName, key, value); }

	// 値の取得
	int32_t GetIntValue(const std::string& groupName, const std::string& key) const { return GetImpl<int32_t>(groupName, key); }
	float GetFloatValue(const std::string& groupName, const std::string& key) const { return GetImpl<float>(groupName, key); }
	KamataEngine::Vector3 GetVector3Value(const std::string& groupName, const std::string& key) const { return GetImpl<KamataEngine::Vector3>(groupName, key); }

	/// <summary>
	/// 毎フレーム処理。独立ウィンドウ "Global Variables" を描画する(Debugビルドのみ)。
	/// </summary>
	void Update();

	void SaveFile(const std::string& groupName);
	void SaveAllFiles();
	void LoadFiles();
	void LoadFile(const std::string& groupName);

private:
	const std::string kDirectoryPath = "Resources/GlobalVariables/";

	std::map<std::string, Group> datas_;

	// ---- 型ごとの重複実装をまとめたテンプレート ----
	template<typename T> void SetImpl(const std::string& groupName, const std::string& key, const T& value) { datas_[groupName].items[key] = Item{value}; }

	template<typename T> void AddImpl(const std::string& groupName, const std::string& key, const T& value) {
		Group& group = datas_[groupName];
		if (group.items.find(key) == group.items.end()) {
			group.items[key] = Item{value};
		}
	}

	template<typename T> T GetImpl(const std::string& groupName, const std::string& key) const {
#ifdef _DEBUG
		assert(datas_.find(groupName) != datas_.end());
#endif
		const Group& group = datas_.at(groupName);
#ifdef _DEBUG
		assert(group.items.find(key) != group.items.end());
#endif
		const Item& item = group.items.at(key);
#ifdef _DEBUG
		assert(std::holds_alternative<T>(item.value));
#endif
		return std::get<T>(item.value);
	}

	// 保存結果などの表示用
	std::string statusMessage_;
	float statusTimer_ = 0.0f;

	GlobalVariables() = default;
	~GlobalVariables() = default;
};