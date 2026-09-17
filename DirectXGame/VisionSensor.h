#pragma once

#define NOMINMAX

#include "Global.h"
#include "KamataEngine.h"
#include <vector>

class MapChipField;
class Player;

/// <summary>
/// 索敵（視野コーン）判定コンポーネント
/// 敵クラスが所有し、毎フレームUpdateして使う
/// </summary>
class VisionSensor {
public:
	// 視野三角形
	struct VisionTriangle {
		KamataEngine::Vector3 center; // 目の位置（頂点）
		KamataEngine::Vector3 left;   // 左端点
		KamataEngine::Vector3 right;  // 右端点
	};

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// 調整項目を登録／適用
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	// 視野コーンを更新し、プレイヤーが見えているか判定する
	// eyePos    : 目の位置（敵のワールド座標＋高さオフセットなど、呼び出し側で用意）
	// facingDir : 向いている方向（正規化不要、XY平面のベクトル。例: {1,0,0} / {-1,0,0}）
	// player    : 判定対象のプレイヤー
	bool CheckPlayerSpotted(const KamataEngine::Vector3& eyePos, const KamataEngine::Vector3& facingDir, const Player* player);

	bool IsPlayerSpotted() const { return isPlayerSpotted_; }
	const VisionTriangle& GetVisionTriangle() const { return visionTriangle_; }

	// 調整用パラメータへのアクセス（デバッグ描画等で使う場合）
	static float GetVisionRadius() { return kVisionRadius; }
	static float GetVisionHalfAngle() { return kVisionHalfAngle; }
	static constexpr uint32_t kSamplesPerEdge = 6;
	static constexpr uint32_t kNumConeMarkers = (kSamplesPerEdge + 1) * 3;
	std::array<KamataEngine::Vector3, kNumConeMarkers> GetDebugConePoints() const;

private:


	void UpdateVisionCone(const KamataEngine::Vector3& eyePos, const KamataEngine::Vector3& facingDir);
	bool IsSegmentBlockedByMapChip(const KamataEngine::Vector3& from, const KamataEngine::Vector3& to) const;
	static bool SATCollisionRectTriangle(const std::vector<KamataEngine::Vector3>& rect, const std::vector<KamataEngine::Vector3>& tri);

	MapChipField* mapChipField_ = nullptr;

	VisionTriangle visionTriangle_{};
	bool isPlayerSpotted_ = false;

	// --- 索敵(視野)関連 ---
	static inline float kVisionHalfAngle = 35.0f; // 視野の片側角度(度)
	static inline float kVisionRadius = 30.0f;     // 索敵距離
	static inline float kRaySampleStep = 0.2f;    // 視線判定のサンプリング間隔
};