#pragma once

#include "Global.h"

class BaseProjectile {
public:
	virtual ~BaseProjectile() = default;

	virtual void
	    Initialize(KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity, Owner owner);
	virtual void Update();
	virtual void Draw();

	bool IsDead() const { return isDead_; }
	AABB GetAABB() const;
	Owner GetOwner() const { return owner_; }

	virtual void OnCollision() { isDead_ = true; }

	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }
	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

	virtual float GetImpactPower() const { return kImpactPower; }

virtual int GetDamage() const { return kDamage; }
	virtual int GetDamage(Owner owner) const { return (owner == Owner::kPlayer) ? kDamageToEnemy : kDamageToPlayer; }
	protected:
	static inline int kDamageToEnemy = 160;
	static inline int kDamageToPlayer = 30;
	static inline int kDamage = 50;
	static inline float kImpactPower = 1.2f;

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0;

	KamataEngine::Vector3 velocity_{};
	Owner owner_ = Owner::kEnemy;

	static inline int32_t kLifeTime = 60;
	int32_t deathTimer_ = kLifeTime;

	bool isDead_ = false;

	static inline float kWidth = 0.4f;
	static inline float kHeight = 0.4f;
};