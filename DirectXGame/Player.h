#pragma once

#include "Global.h"

class GameScene;

class Player {
public:
	~Player();
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(KamataEngine::Model* modelPlayer, uint32_t texturePlayer, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();

	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

	const KamataEngine::WorldTransform& GetWorldTransform() const;
	void SetWorldTransform(const KamataEngine::WorldTransform& worldTransform);
	KamataEngine::Vector3 GetWorldPosition() const;
	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }
	void ResetVelocity() { velocity_ = {}; }

	AABB GetAABB();

	void GroundMovement();
	void JumpMovement();
	void ApplyScrollPush();

	void OnCollision();

	bool isDead() const { return isDead_; }
private:
	int hp_ = kMaxHP;
	static inline int kMaxHP = 100;

	GameScene* gameScene_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	static inline SRTOffset currentSRT = {(1.0f, 1.0f, 1.0f), (0.0f, 0.0f, 0.0f), (0.0f, 0.0f, 0.0f)};

	KamataEngine::Camera* camera_ = nullptr;

	KamataEngine::Model* modelPlayer_ = nullptr;
	uint32_t playerTex_ = 0;

	KamataEngine::Vector3 velocity_ = {0, 0, 0};

	static inline float kAcceleration = 0.05f;
	static inline float kAttenuation = 0.1f;
	static inline float kLimitRunSpeed = 0.25f;
	static inline float kGravityAcceleration = 0.05f;

	static inline float kWidth = 0.8f;
	static inline float kHeight = 1.6f;
	static inline float kBlank = 0.01f;

	bool isDead_ = false;
};