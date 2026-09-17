#pragma once

#include <KamataEngine.h>
#include <cstdint>
#include <map>
#include <string>
#include <variant>

/// <summary>
/// グローバル変数
/// </summary>
class GlobalVariables {
public:
	static GlobalVariables* GetInstance();

	// 項目
	struct Item {
		// 項目の値
		std::variant<int32_t, float, KamataEngine::Vector3> value;
	};

	// グループ
	struct Group {
		std::map<std::string, Item> items;
	};

	// Disable copying
	GlobalVariables(const GlobalVariables&) = delete;
	GlobalVariables& operator=(const GlobalVariables&) = delete;

	// Disable moving
	GlobalVariables(GlobalVariables&&) = delete;
	GlobalVariables& operator=(GlobalVariables&&) = delete;

	/// <summary>
	/// グループの作成
	/// </summary>
	/// <param name="groupName">グループ名</param>
	void CreateGroup(const std::string& groupName);

	// 値のセット(int)
	void SetValue(const std::string& groupName, const std::string& key, int32_t value);
	// 値のセット(float)
	void SetValue(const std::string& groupName, const std::string& key, float value);
	// 値のセット(Vector3)
	void SetValue(const std::string& groupName, const std::string& key, KamataEngine::Vector3 value);

	// 項目の追加(int)
	void AddItem(const std::string& groupName, const std::string& key, int32_t value);
	// 項目の追加(float)
	void AddItem(const std::string& groupName, const std::string& key, float value);
	// 項目の追加(Vector3)
	void AddItem(const std::string& groupName, const std::string& key, KamataEngine::Vector3 value);

	// 値の取得
	int32_t GetIntValue(const std::string& groupName, const std::string& key) const;
	float GetFloatValue(const std::string& groupName, const std::string& key) const;
	KamataEngine::Vector3 GetVector3Value(const std::string& groupName, const std::string& key) const;

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	/// <summary>
	/// ファイルに書き出し
	/// </summary>
	/// <param name="groupName">グループ</param>
	void SaveFile(const std::string& groupName);

	/// <summary>
	/// ディレクトリの全ファイル読み込み
	/// </summary>
	void LoadFiles();

	/// <summary>
	/// ファイルから読み込む
	/// </summary>
	/// <param name="groupName">グループ</param>
	void LoadFile(const std::string& groupName);

private:
	// グローバル変数の保存先ファイルパス
	const std::string kDirectoryPath = "Resources/GlobalVariables/";

	// 全データ
	std::map<std::string, Group> datas_;

	GlobalVariables() = default;
	~GlobalVariables() = default;
};