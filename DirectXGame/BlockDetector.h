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

	void SetSize(float width, float height) {
		kWidth = width;
		kHeight = height;
	}

	void SetIgnoreBarrier(bool ignore) { ignoreBarrier_ = ignore; }
	void SetIgnoreOneWay(bool ignore) { ignoreOneWay_ = ignore; }

	// true  = real tile shapes/offsets (detector). false = every solid tile is a full 1.0 cell (root).
	void SetUseDetailedTiles(bool detailed) { useDetailedTiles_ = detailed; }

	bool ProbeGroundTop(const KamataEngine::Vector3& center, float stepUp, float maxDepth, float& outTopY);

private:
	bool IsSolid(const KamataEngine::Vector3& position, bool ignoreBarrier) const;
	// Same as IsSolid, but one-way tiles (platforms) never count. Used for head/side checks.
	bool IsSolidIgnoringOneWay(const KamataEngine::Vector3& position, bool ignoreBarrier) const;
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