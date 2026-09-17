#pragma once

#include "Global.h"
#include "MapChipField.h"
#include <KamataEngine.h>

/// <summary>
/// 可動マップチップ：スライド式ゲート(扉)。
/// 開く方向はマップチップの「型」(kGateUp/Down/Left/Right)で決まる(旧: subID)。
/// subIDは現在テクスチャバリアント(見た目)のみに使う。
/// linkID(呼び出し元でbuttonID_として渡す)は対応するkSwitchのボタンID。
/// 静的な当たり判定(MapChipField::IsSolidAtPosition)には乗らないため、
/// GameScene::CheckAllCollisionsでAABBを使って明示的に押し出す必要がある。
/// </summary>
class SlidingGate {
public:
	enum class SlideDirection { kUp, kDown, kLeft, kRight };

	void Initialize(
	    KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera, const KamataEngine::Vector3& closedPosition, MapChipType gateType, uint8_t textureVariant, uint8_t buttonID);
	uint8_t GetButtonID() const { return buttonID_; }
	void Update();
	void Draw();

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Open() { isOpenTarget_ = true; }
	void Close() { isOpenTarget_ = false; }
	bool IsFullyOpen() const { return openFactor_ >= 1.0f; }

	bool IsBlocking() const { return openFactor_ < kBlockingThreshold; }
	AABB GetAABB() const;

private:
	uint8_t buttonID_ = 0;
	static SlideDirection DirectionFromType(MapChipType type);

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0;
	uint8_t textureVariant_ = 0; // 見た目のバリエーション(木/鉄扉等)。今のところ描画切り替えは未実装、拡張用に保持。

	KamataEngine::Vector3 closedPosition_{};
	SlideDirection direction_ = SlideDirection::kUp;

	bool isOpenTarget_ = false;
	float openFactor_ = 0.0f;

	static inline float kHalfWidth = 1.0f;
	static inline float kHalfHeight = 1.0f;
	static inline float kHalfDepth = 1.0f;

	static inline float kSlideDistance = 2.0f;
	static inline float kSlideSpeed = 0.02f;
	static inline float kBlockingThreshold = 0.5f;
};