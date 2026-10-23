#pragma once

#include "BaseProjectile.h"

class ProjectileBall final : public BaseProjectile {
public:
	ProjectileBall() {
		kWidth = 0.4f;
		kHeight = 0.4f;
		kLifeTime = 90;
		deathTimer_ = kLifeTime;
	}

	void InitializeAimed(
	    KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, const KamataEngine::Vector2& aimDirection, Owner owner);

	// 発射直後だけ「体の中心→銃口」の区間を当たり判定に含める(至近距離の敵/薄い壁をすり抜けないため)
	void SetSweepFrom(const KamataEngine::Vector3& from) {
		sweepFrom_ = from;
		hasSweep_ = true;
	}
	void Update() override {
		BaseProjectile::Update();
		if (age_ < 255)
			++age_;
	}
	// 壁判定に使う線分の始点
	KamataEngine::Vector3 GetPathStart() const { return (hasSweep_ && age_ <= 1) ? sweepFrom_ : (GetWorldPosition() - GetVelocity()); }
	AABB GetSweepAABB() const;

	// 毎フレーム1回呼ぶ。trueならトレイルを1つ出す
	bool ConsumeTrailTick() {
		trailTimer_ -= 1.0f / 60.0f;
		if (trailTimer_ > 0.0f)
			return false;
		trailTimer_ = kTrailInterval;
		return true;
	}

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

private:
	static inline float kSpeed = 0.8f;
	static inline float kTrailInterval = 0.05f; // トレイルの発生間隔(秒)

	KamataEngine::Vector3 sweepFrom_{};
	bool hasSweep_ = false;
	uint32_t age_ = 0;
	float trailTimer_ = 0.0f;
};