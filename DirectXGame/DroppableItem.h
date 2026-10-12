#pragma once

#include "Global.h"
#include <KamataEngine.h>

class MapChipField;

/// <summary>
/// 敵が死亡時に落とす武器アイテム。プレイヤーが触れると拾える。
/// 出現時にランダムな向きへ放り出され、ObjectBounceで跳ねてから静止し、その場でふわふわ浮く。
/// </summary>
class DroppableItem {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(
	    KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, DropItemCategory category, DropWeaponKind kind,
	    DropItemState state = DropItemState::kReady);
	void Update();
	void Draw();

	// 跳ねる判定に使うマップ。Initialize後に必ず設定する(未設定なら跳ねずにその場で静止)
	void SetMapChipField(MapChipField* field) { mapChipField_ = field; }

	AABB GetAABB() const;

	bool IsPickedUp() const { return isPickedUp_; }
	void MarkPickedUp() { isPickedUp_ = true; }

	DropItemCategory GetCategory() const { return category_; }
	DropWeaponKind GetWeaponKind() const { return kind_; }
	DropItemState GetState() const { return state_; }

	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

	bool IsExpired() const { return isExpired_; }
	// 跳ね終わって静止したか
	bool IsSettled() const { return isSettled_; }

	// 指定した速度で放り出し直す(拾い替えで手放した武器用)
	void Launch(const KamataEngine::Vector3& velocity);
	// プレイヤーの向きへ放る初速
	static KamataEngine::Vector3 MakeThrowVelocity(bool facingRight);

	void SetSpawnerID(int32_t id) { spawnerID_ = id; }
	int32_t GetSpawnerID() const { return spawnerID_; }
	void PlaceStatic(); // sits on its tile, never expires (call right after Initialize)

private:
	// 放り出し〜バウンド中の更新
	void UpdateBounce();
	// 静止後のふわふわ更新
	void UpdateIdle();

	int32_t spawnerID_ = -1;
	bool persistent_ = false;

	float lifeTimer_ = 0.0f;
	bool isExpired_ = false;
	static inline float kLifeTime = 15.0f;            // 拾われなければ消えるまでの秒数
	static inline float kBlinkWarningDuration = 3.0f; // 消える何秒前から点滅するか
	static inline float kBlinkFrequency = 6.0f;       // 点滅の速さ(Hz相当)

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::ObjectColor objectColor_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0;

	DropItemCategory category_ = DropItemCategory::kMelee;
	DropWeaponKind kind_ = DropWeaponKind::kSabreLight;
	DropItemState state_ = DropItemState::kReady;

	KamataEngine::Vector3 basePosition_{}; // 静止位置(ふわふわの基準)
	float bobTimer_ = 0.0f;
	bool isPickedUp_ = false;

	// ---- バウンド状態 ----
	MapChipField* mapChipField_ = nullptr;
	bool isSettled_ = false;
	KamataEngine::Vector3 velocity_{};

	float restitution_ = 0.5f;            // このアイテム固有の反発係数
	int bounceCount_ = 0;
	int maxBounces_ = 3; // このアイテム固有の最大バウンド回数

	static inline float kHalfWidth = 0.4f;
	static inline float kHalfHeight = 0.4f;
	static inline float kHalfDepth = 0.4f;
	static inline float kBobAmplitude = 0.1f;
	static inline float kBobSpeed = 2.0f;
	static inline float kSpinSpeed = 1.5f;

	// ---- バウンドの調整値 ----
	static inline float kBodySize = 0.6f;         // 地形との衝突に使う大きさ
	static inline float kGravity = 0.03f;         // 毎フレームの重力
	static inline float kMaxFallSpeed = 0.5f;     // 落下速度の上限
	static inline float kLaunchSpeedX = 0.18f;    // 横方向の初速(±)
	static inline float kLaunchSpeedYMin = 0.25f; // 縦方向の初速(最小)
	static inline float kLaunchSpeedYMax = 0.45f; // 縦方向の初速(最大)
	static inline float kRestitutionMin = 0.4f;   // 反発係数(最小)
	static inline float kRestitutionMax = 0.65f;  // 反発係数(最大)
	static inline float kFriction = 0.7f;         // 着地時に残る横速度の割合
	static inline int kBounceCountMin = 2;        // 静止するまでの床バウンド回数(最小)
	static inline int kBounceCountMax = 4;        // 静止するまでの床バウンド回数(最大)
	static inline float kSettleSpeed = 0.04f;     // これ以下の跳ね返り速度なら静止とみなす

		// 現在の位置・速度からこのまま跳ねた場合に、安全な床へ着地できるかを予測する(キルブロック/穴ならfalse)
	bool WillLandSafely(const KamataEngine::Vector3& pos, const KamataEngine::Vector3& vel);

	bool launchChecked_ = false;            // 放り出し直後の安全確認が済んだか
	static inline int kPredictFrames = 180; // 着地先の予測に使う最大フレーム数

	static inline float kThrowSpeedX = 0.28f; // 向いている方向への初速
	static inline float kThrowSpeedY = 0.3f;  // 上方向の初速
};