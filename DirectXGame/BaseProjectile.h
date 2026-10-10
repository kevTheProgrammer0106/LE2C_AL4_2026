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

	// 撃った側(owner_)に応じた衝撃力。プレイヤーの弾=敵に当てる値、敵の弾=プレイヤーに当てる値
	virtual float GetImpactPower() const { return GetImpactPower(owner_); }
	virtual float GetImpactPower(Owner shooter) const { return (shooter == Owner::kPlayer) ? kImpactPowerToEnemy : kImpactPowerToPlayer; }

	virtual int GetDamage(Owner owner) const {
		// 銃から値を渡されていればそれを使う。無ければ従来の共通値
		if (damageOverride_ >= 0)
			return damageOverride_;
		return (owner == Owner::kPlayer) ? kDamageToEnemy : kDamageToPlayer;
	}
	// 発射した銃のダメージを設定する
	void SetDamage(int damage) { damageOverride_ = damage; }
	// 射程を変えたい弾用(フレーム数)
	void SetLifeFrames(int32_t frames) { deathTimer_ = frames; }
	protected:
	int damageOverride_ = -1; // -1 = 未設定(共通値を使う)
	static inline int kDamageToEnemy = 160;
	static inline int kDamageToPlayer = 30;
	static inline int kDamage = 50;
	float damageScale_ = 1.0f;

	static inline float kImpactPower = 1.2f;
	static inline float kImpactPowerToEnemy = 1.2f;  // プレイヤーの弾が敵に当たったとき
	static inline float kImpactPowerToPlayer = 1.2f; // 敵の弾がプレイヤーに当たったとき

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