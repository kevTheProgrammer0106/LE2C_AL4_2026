#pragma once

#include "Global.h"
#include <KamataEngine.h>

/// <summary>
/// 敵が死亡時に落とす武器アイテム。プレイヤーが触れると拾える。
/// </summary>
class DroppableItem {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, DropItemCategory category, DropWeaponKind kind);
	void Update();
	void Draw();

	AABB GetAABB() const;

	bool IsPickedUp() const { return isPickedUp_; }
	void MarkPickedUp() { isPickedUp_ = true; }

	DropItemCategory GetCategory() const { return category_; }
	DropWeaponKind GetWeaponKind() const { return kind_; }

	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }



	bool IsExpired() const { return isExpired_; }

private:
	float lifeTimer_ = 0.0f;
	bool isExpired_ = false;
	static inline float kLifeTime = 15.0f;            // 拾われなければ消えるまでの秒数
	static inline float kBlinkWarningDuration = 3.0f; // 消える何秒前から点滅するか
	static inline float kBlinkFrequency = 6.0f;       // 点滅の速さ(Hz相当)

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0;

	DropItemCategory category_ = DropItemCategory::kMelee;
	DropWeaponKind kind_ = DropWeaponKind::kSabreLight;

	KamataEngine::Vector3 basePosition_{};
	float bobTimer_ = 0.0f;
	bool isPickedUp_ = false;

	static inline float kHalfWidth = 0.4f;
	static inline float kHalfHeight = 0.4f;
	static inline float kHalfDepth = 0.4f;
	static inline float kBobAmplitude = 0.1f;
	static inline float kBobSpeed = 2.0f;
	static inline float kSpinSpeed = 1.5f;
};