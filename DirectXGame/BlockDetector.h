#pragma once

#include "Global.h"
#include <KamataEngine.h>
#include "MapChipField.h"


class BlockDetector
{
public:
	// マップチップフィールドを設定するセッター
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

	void SetWorldTransform(const KamataEngine::WorldTransform& worldTransform)
	{
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
	
	void SetIgnoreBarrier(bool ignore) { ignoreBarrier_ = ignore; }

private:
	bool ignoreBarrier_ = false; // trueなら境界壁(kBarrier)を素通りする(敵用)

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Vector3 velocity_ = {};

	// 接地状態フラグ
	bool onGround_ = false;
	
	// Collision dimensions / blank space used by the detector
	float kWidth = 0.8f;
	float kHeight = 1.6f;
	float kBlank = 0.01f;

	MapChipField* mapChipField_ = nullptr;

	float kAttenuation = 0.1f;
};

