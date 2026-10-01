#pragma once

#include "MapChipField.h"
#include <array>
#include <cstdint>
#include <vector>

/// <summary>
/// マップチップフィールド上のグリッドパスファインダー。
/// A*アルゴリズムを使用して、移動コストが最も低い経路を探索する。
///
/// 現在は上下左右の4方向移動のみ対応。
/// GetTraversalCostは今後ブロック種別（泥・水・氷など）を追加した際に
/// ここへコスト差分を書き足していくためのプレースホルダーで、
/// 現状はkBlock以外すべて一律コスト1.0として扱う。
///
/// [FIX] ブロックサイズが1.0になり、敵(高さ1.6)が縦2セル分を占めるようになったため、
/// 移動先セルの真上にも頭上の空き(HasHeadroom)を要求する。
/// </summary>
class PathFindingAlgorithm {
public:
	using IndexSet = MapChipField::IndexSet;

	// パスノード(kPathNode)のsubID別コストテーブル。
	// subIDが小さいほど(=デフォルトでは)低コスト＝A*に優先して選ばれやすい。
	// 「メインルート(subID 0)」「フランクルート1(subID 1)」「フランクルート2(subID 2)」…と
	// デザイナーがsubIDを振り分けることで、敵に優先順位付きの複数ルートを提示できる。
	// BaseEnemy::FlankPreferenceで敵ごとにこの優先順位を反転・無効化できる。
	//
	// ジャンプノード(kJumpNode)のsubID別コストテーブル。
	// subID 0(J0) = 「任意ジャンプ」。他に安いルートがあれば避けてほしいので高コストにしておく。
	// 　　　　　　　実際にジャンプが発生するのは、通過が必要でA*がこのノードを選んだ場合のみ
	// 　　　　　　　＝ChaseMovement側でも「必要な時だけ」ジャンプする(kJumpNodeIgnoreDistanceの対象)。
	// subID 1(J1) = 「常時ジャンプ」。A*が経路として選びやすいよう低コストにしておき、
	// 　　　　　　　通過時は必ずジャンプを実行する(プレイヤーが近くにいても無視しない)。
	struct PathCostProfile {
		std::array<float, 10> pathNodeCosts = {0.8f, 1.0f, 1.2f, 1.4f, 1.6f, 1.8f, 2.0f, 2.2f, 2.4f, 2.6f};
		std::array<float, 10> jumpNodeCosts = {2.0f, 1.0f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f};
	};

	// デフォルトのコストプロファイル(subIDが小さいほど低コスト)を取得する
	static const PathCostProfile& GetDefaultPathCostProfile();

	// startからgoalまでをA*で探索する。
	// costProfileを渡すと、kPathNode/kJumpNodeのコストをsubIDに応じて変化させられる(nullptrならデフォルトを使用)。
	// 戻り値には開始地点を含まず、開始地点の次のセルからゴールまでが順番に格納される。
	// 経路が存在しない、または探索上限に達した場合は空のvectorを返す。
	static std::vector<IndexSet> FindPath(MapChipField* mapChipField, const IndexSet& start, const IndexSet& goal, uint32_t maxSearchNodes = 4000, const PathCostProfile* costProfile = nullptr);

	// ジャンプノードのlinkID(ルーティング優先度tier)が「常時ジャンプ(tier1以上)」かどうかを判定する。
	// tier0だけが「任意ジャンプ(必要な時のみ)」で、それ以外は全て常時ジャンプ扱いにする。
	static bool IsAlwaysJumpSubID(uint8_t tier) { return tier != 0; }

private:
	static bool IsInsideMap(MapChipField* mapChipField, uint32_t xIndex, uint32_t yIndex);
	static bool IsWalkable(MapChipField* mapChipField, uint32_t xIndex, uint32_t yIndex);
	static bool CanMoveBetween(MapChipField* mapChipField, const IndexSet& current, const IndexSet& next);

	// [NEW] 敵の体が占める縦方向のセル数(1セル=現在のブロック高さ)。BaseEnemyの体高さとマップのブロック高さから算出する。
	static uint32_t GetAgentHeightCells(MapChipField* mapChipField);
	// [NEW] (xIndex, yIndex)のセルに立った時、頭上(体が占める上側のセル)が全て歩行可能か。
	static bool HasHeadroom(MapChipField* mapChipField, uint32_t xIndex, uint32_t yIndex);

	// 移動コストのプレースホルダー。ブロック種別が増えたらswitchへケースを足していく。
	// kPathNode/kJumpNodeについてはprofileのsubID別テーブルを参照する。
	static float GetTraversalCost(MapChipField* mapChipField, uint32_t xIndex, uint32_t yIndex, const PathCostProfile& profile);
	// 4方向移動なのでマンハッタン距離を使用する。
	static float CalculateHeuristic(const IndexSet& current, const IndexSet& goal);
};