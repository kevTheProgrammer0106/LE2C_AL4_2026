#include "GlobalVariables.h"


#include <json.hpp>
using json = nlohmann::json;

#include <fstream>
#include <sstream>

using namespace KamataEngine;


void GlobalVariables::Update() 
{ 
	#ifdef _DEBUG
	if (!ImGui::Begin("Global Variables", nullptr, ImGuiWindowFlags_MenuBar))
	{
		ImGui::End();
		return;
	}
	if (!ImGui::BeginMenuBar())
	{
		return;
	}
		

	//各グループについて
	for (std::map<std::string, Group>::iterator itGroup = datas_.begin(); itGroup != datas_.end(); ++itGroup)
	{
		//グループ名を取得
		const std::string& groupName = itGroup->first;
		// グループの参照を取得
		Group& group = itGroup->second;

		if (!ImGui::BeginMenu(groupName.c_str()))
			continue;
		// 各項目について
		for (std::map<std::string, Item>::iterator itItem = group.items.begin(); itItem != group.items.end(); ++itItem)
		{
			// 項目名を取得
			const std::string& itemName = itItem->first;
			// 項目の参照を取得
			Item& item = itItem->second;
			// int32_t型の値を保持していれば
			if (std::holds_alternative<int32_t>(item.value))
			{
				int32_t* ptr = std::get_if<int32_t>(&item.value);
				ImGui::DragInt(itemName.c_str(), ptr, 0.1f, 0, 100);
			}
			// float型の値を保持していれば
			else if (std::holds_alternative<float>(item.value)) {
				float* ptr = std::get_if<float>(&item.value);
				ImGui::DragFloat(itemName.c_str(), ptr, 0.1f, 0.0f, 100.0f);
			}
			// Vector3型の値を保持していれば
			else if (std::holds_alternative<Vector3>(item.value)) {
				Vector3* ptr = std::get_if<Vector3>(&item.value);
				ImGui::DragFloat3(itemName.c_str(), reinterpret_cast<float*>(ptr), 0.1f, -10.0f, 10.0f);
			}
		}

		//改行 (kaigyou)
		ImGui::Text("\n");
		if (ImGui::Button("Save")) 
		{
			SaveFile(groupName);
			std::string message = std::format("{}.json saved", groupName);
			MessageBoxA(nullptr, message.c_str(), "GlobalVariables", 0);
		}

		ImGui::EndMenu();
	}

	ImGui::EndMenuBar();
	ImGui::End();
#endif
}

void GlobalVariables::SaveFile(const std::string& groupName) {
	// グループを検索
	std::map<std::string, Group>::iterator itGroup = datas_.find(groupName);
#ifdef _DEBUG
	// 未登録チェック
	assert(itGroup != datas_.end());
	#endif
	json root;

	root = json::object();

	// jsonオブジェクト登録
	root[groupName] = json::object();

	// 各項目について
	for (std::map<std::string, Item>::iterator itItem = itGroup->second.items.begin(); itItem != itGroup->second.items.end(); ++itItem) 
	{
		// 項目名を取得
		const std::string& itemName = itItem->first;
		// 項目の参照を取得
		Item& item = itItem->second;

		// int32_t型の値を保持していれば
		if (std::holds_alternative<int32_t>(item.value))
		{
			// int32_t型の値を登録
			root[groupName][itemName] = std::get<int32_t>(item.value);
		}
		// float型の値を保持していれば
		else if (std::holds_alternative<float>(item.value)) {
			// float型の値を登録
			root[groupName][itemName] = std::get<float>(item.value);
		}
		// Vector3型の値を保持していれば
		else if (std::holds_alternative<Vector3>(item.value)) {
			// float型のjson配列行列
			Vector3 value = std::get<Vector3>(item.value);
			root[groupName][itemName] = json::array({ value.x, value.y, value.z });
		}
	}
	// ディレクトリがなければ作成する
	std::filesystem::path dir(kDirectoryPath);
	if (!std::filesystem::exists(kDirectoryPath)) {
		std::filesystem::create_directory(kDirectoryPath);
	}
	// 書き込みJSONファイルのフルパスを合成する
	std::string filePath = kDirectoryPath + groupName + ".json";
	// 書き込み用ファイルストリーム
	std::ofstream ofs;
	// ファイルを書き込み用に開く
	ofs.open(filePath);

	// ファイルオープン失敗
	if (ofs.fail()) {
		std::string message = "Failed open data file for write.";
		MessageBoxA(nullptr, message.c_str(), "GlobalVariables", 0);
#ifdef _DEBUG
		assert(0);
		#endif
		return;
	}

	// ファイルにjson文字列を書き込む(インデント幅4)
	ofs << std::setw(4) << root << std::endl;
	// ファイルを閉じる
	ofs.close();

}

void GlobalVariables::LoadFiles() 
{
	std::filesystem::path dir(kDirectoryPath);
	// ディレクトリがなければスキップする
	if (!std::filesystem::exists(kDirectoryPath)) {
		return;
	}

	std::filesystem::directory_iterator dir_it(kDirectoryPath);
	for (const std::filesystem::directory_entry& entry : dir_it)
	{
		// ファイルパスを取得
		const std::filesystem::path& filePath = entry.path();

		//ファイル拡張子を取得
		std::string extension = filePath.extension().string();
		// .jsonファイル以外はスキップ
		if (extension.compare(".json") != 0)
			continue;

		// ファイル読み込み
		LoadFile(filePath.stem().string());
	}
}

void GlobalVariables::LoadFile(const std::string& groupName) {
	// 読み込むJSONファイルのフルパスを合成する
	std::string filePath = kDirectoryPath + groupName + ".json";
	//  読み込み用ファイルストリーム
	std::ifstream ifs;
	// ファイルを読み込み用に開く
	ifs.open(filePath);

	//ファイルオープン失敗
	if (ifs.fail()) {
		std::string message = "Failed open data file for load";
		MessageBoxA(nullptr, message.c_str(), "GlobalVariables", 0);
#ifdef _DEBUG
		assert(0);
		#endif
		return;
	}

	json root;
	// json文字列からjsonのデータ構造に展開
	ifs >> root;
	// ファイルを閉じる
	ifs.close();

	// グループを検索
	json::iterator itGroup = root.find(groupName);

	#ifdef _DEBUG
	//未登録チェック
	assert(itGroup != root.end());
	#endif
	//各アイテムについて
	for (json::iterator itItem = itGroup->begin(); itItem != itGroup->end(); ++itItem)
	{
		// アイテム名を取得
		const std::string& itemName = itItem.key();

		// int32_t型の値を保持していれば
		if (itItem->is_number_integer()) 
		{
			//int型の値を登録
			int32_t value = itItem->get<int32_t>();
			SetValue(groupName, itemName, value);
		}
		// float型の値を保持していれば
		else if (itItem->is_number_float()) 
		{
			// int型の値を登録
			float value = itItem->get<float>();
			SetValue(groupName, itemName, value);
		}
		// 要素数3の配列であれば
		else if (itItem->is_array() && itItem->size() == 3)
		{
			// float型のjson配列登録
			Vector3 value = {itItem->at(0), itItem->at(1), itItem->at(2)};
			SetValue(groupName, itemName, value);
		}

	}

}

GlobalVariables* GlobalVariables::GetInstance() {
	static GlobalVariables instance;
	return &instance;
}

void GlobalVariables::CreateGroup(const std::string& groupName)
{
	// 指定名のオブジェクトがなけらば追加する
	datas_[groupName];
}

void GlobalVariables::SetValue(const std::string& groupName, const std::string& key, int32_t value) {
	//グループの参照を取得
	Group& group = datas_[groupName];
	//新しい科目のデータを設定
	Item newItem{};
	newItem.value = value;
	// 設定した科目をstd::mapに追加
	group.items[key] = newItem;
}
void GlobalVariables::SetValue(const std::string& groupName, const std::string& key, float value) {
	// グループの参照を取得
	Group& group = datas_[groupName];
	// 新しい科目のデータを設定
	Item newItem{};
	newItem.value = value;
	// 設定した科目をstd::mapに追加
	group.items[key] = newItem;
}
void GlobalVariables::SetValue(const std::string& groupName, const std::string& key, const Vector3 value) {
	// グループの参照を取得
	Group& group = datas_[groupName];
	// 新しい科目のデータを設定
	Item newItem{};
	newItem.value = value;
	// 設定した科目をstd::mapに追加
	group.items[key] = newItem;
}

void GlobalVariables::AddItem(const std::string& groupName, const std::string& key, int32_t value) {
	Group& group = datas_[groupName];

	//項目が未登録なら
	if (group.items.find(key) == group.items.end())
	{
		SetValue(groupName, key, value);
	}
}
void GlobalVariables::AddItem(const std::string& groupName, const std::string& key, float value) {
	Group& group = datas_[groupName];

	if (group.items.find(key) == group.items.end()) {
		SetValue(groupName, key, value);
	}
}

void GlobalVariables::AddItem(const std::string& groupName, const std::string& key, const Vector3 value) {
	Group& group = datas_[groupName];

	if (group.items.find(key) == group.items.end()) {
		SetValue(groupName, key, value);
	}
}

int32_t GlobalVariables::GetIntValue(const std::string& groupName, const std::string& key) const {
#ifdef _DEBUG
	assert(datas_.find(groupName) != datas_.end());
	#endif
	const Group& group = datas_.at(groupName);

	#ifdef _DEBUG
	assert(group.items.find(key) != group.items.end());
#endif
	
	const Item& item = group.items.at(key);

	#ifdef _DEBUG
	assert(std::holds_alternative<int32_t>(item.value));
	#endif
	
	return std::get<int32_t>(item.value);
}

float GlobalVariables::GetFloatValue(const std::string& groupName, const std::string& key) const 
{
#ifdef _DEBUG
	assert(datas_.find(groupName) != datas_.end());
	#endif
	const Group& group = datas_.at(groupName);
#ifdef _DEBUG
	assert(group.items.find(key) != group.items.end());
	#endif
	const Item& item = group.items.at(key);
#ifdef _DEBUG
	assert(std::holds_alternative<float>(item.value));
	#endif
	return std::get<float>(item.value);
}
Vector3 GlobalVariables::GetVector3Value(const std::string& groupName, const std::string& key) const 
{
#ifdef _DEBUG
	assert(datas_.find(groupName) != datas_.end());
	#endif
	const Group& group = datas_.at(groupName);

	#ifdef _DEBUG
	assert(group.items.find(key) != group.items.end());
	#endif
	const Item& item = group.items.at(key);

	#ifdef _DEBUG
	assert(std::holds_alternative<Vector3>(item.value));
	#endif
	return std::get<Vector3>(item.value);
}
