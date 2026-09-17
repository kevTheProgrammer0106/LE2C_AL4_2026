#pragma once

#include "KamataEngine.h"
#include "MapChipField.h"
#include <vector>

/// <summary>
/// 敵の進行方向前方を複数区間(セグメント)に分けて走査し、
/// 「崖(足場切れ)」と「壁(高さ判定込み)」を先読みで検出するセンサー。
///
/// BaseEnemyのIsGroundAheadClear / CanJumpOverWallが行っていた
/// 単発の点チェックを、連続したセグメント列としてまとめて評価できるようにし、
/// ChaseMovement / JumpMovement からも同じロジックを使い回せるようにする。
/// </summary>
class PathDetector {
public:
	// 1区間分の走査結果
	struct Segment {
		float distance = 0.0f;       // 原点(足元)からの水平距離（符号なし。方向はScan呼び出し側で指定）
		bool hasGroundBelow = false; // このX位置の足元付近に地面(ブロック)があるか
		bool isWall = false;         // このX位置に、指定した高さ範囲内で壁(ブロック)があるか
		float wallTopHeight = 0.0f;  // 壁の場合、足元からの高さ（連続するブロックの上端）
	};

	// 走査パラメータ
	struct ScanParams {
		float stepDistance = 0.25f;    // サンプリング間隔（水平方向）
		float maxDistance = 2.0f;      // 走査する最大距離
		float bodyHeight = 0.8f;       // 壁の高さ探索範囲（この高さまでブロックが続くか調べる）
		float groundProbeDown = 0.3f;  // 足元からどれだけ下を覗いて地面判定するか
		float heightSampleStep = 0.1f; // 壁の高さ方向のサンプリング間隔
	};

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// footPos: 足元（当たり判定下端）のワールド座標
	// directionIsRight: true=右方向, false=左方向へ走査する
	// 壁にぶつかったセグメントで走査を打ち切る（壁の向こう側は別スキャン・ジャンプ後の再スキャンで扱う想定）
	std::vector<Segment> Scan(const KamataEngine::Vector3& footPos, bool directionIsRight, const ScanParams& params) const;

	// 走査結果から最初の「崖(地面が途切れている区間)」を探す。見つかればtrue
	static bool FirstGapInfo(const std::vector<Segment>& segments, float& outDistance);

	// 走査結果から最初の「壁」情報を取得する。見つかればtrue
	static bool FirstWallInfo(const std::vector<Segment>& segments, float& outDistance, float& outHeight);

private:
	MapChipField* mapChipField_ = nullptr;
};