#pragma once

#include "Global.h"
#include "MapChipField.h"
#include <KamataEngine.h>

class BlockDetector {
public:
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	void TopCollision(CollisionInfo& collisionInfo);
	void BottomCollision(CollisionInfo& collisionInfo);
	void LeftCollision(CollisionInfo& collisionInfo);
	void RightCollision(CollisionInfo& collisionInfo);

	void GroundSwitch(const CollisionInfo& collisionInfo);
	void MapChipCollision(CollisionInfo& collisionInfo);

	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

	KamataEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }
	KamataEngine::Vector3& GetVelocity() { return velocity_; }

	void SetWorldTransform(const KamataEngine::WorldTransform& worldTransform) {
		worldTransform_.translation_ = worldTransform.translation_;
		worldTransform_.rotation_ = worldTransform.rotation_;
		worldTransform_.scale_ = worldTransform.scale_;
	}
	void SetVelocity(const KamataEngine::Vector3& velocity) { velocity_ = velocity; }

	bool IsOnGround() const { return onGround_; }
	void SetOnGround(bool onGround) { onGround_ = onGround; }

	float GetWidth() { return kWidth; }
	float GetHeight() { return kHeight; }
	float GetBlank() { return kBlank; }

	// 当たり判定ボックスのサイズ(ルートとディテクターで同じ値を使う)
	void SetSize(float width, float height) {
		kWidth = width;
		kHeight = height;
	}

	void SetIgnoreBarrier(bool ignore) { ignoreBarrier_ = ignore; }
	void SetIgnoreOneWay(bool ignore) { ignoreOneWay_ = ignore; }

	// true  = タイルの実形状・オフセットを使う(半ブロック、薄い足場)。デフォルト＝従来の挙動(ディテクター用)
	// false = 全ての実体タイルを1.0の正方形セルとして扱う(ルート用)
	void SetUseDetailedTiles(bool detailed) { useDetailedTiles_ = detailed; }

	// 足元の左右2点から真下を走査し、立っている面の上端Yを返す。何も動かさない。
	// stepUp: 足元よりどれだけ上の面まで「立てる面」と見なすか / maxDepth: 下方向の探索距離
	bool ProbeGroundTop(const KamataEngine::Vector3& center, float stepUp, float maxDepth, float& outTopY);

private:
	bool IsSolid(const KamataEngine::Vector3& position, bool ignoreBarrier) const;
	MapChipField::Rect GetRect(uint32_t xIndex, uint32_t yIndex) const;

	bool ignoreBarrier_ = false;
	bool ignoreOneWay_ = false;
	bool useDetailedTiles_ = true;

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Vector3 velocity_ = {};

	bool onGround_ = false;

	float kWidth = 0.8f;
	float kHeight = 1.6f;
	float kBlank = 0.01f;

	MapChipField* mapChipField_ = nullptr;

	float kAttenuation = 0.1f;
};