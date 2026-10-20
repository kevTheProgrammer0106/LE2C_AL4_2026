#pragma once

#include "BaseEnemy.h"
#include "SapperBehavior.h"
#include "WeaponAxe.h"

/// <summary>
/// 斧を持つ工兵。HPはCuirassierの半分(175)。
/// - プレイヤーの攻撃ではスタン/ノックバックしない。ガード成功の押し戻しだけは受け(RequestBlockKnockback)、スタンは +6秒。
/// - 攻撃中も歩き続ける。
/// - ホーミングジャンプ中はマップ判定を無視する(SetPhasing)。
/// </summary>
class EnemySapper final : public BaseEnemy {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update() override;
	void Draw() override;

	// 外部要因のスタンも受けない。スタンするのは急降下の着地後(自己スタン)だけ
	void RequestStun(float /*duration*/) override {}


	WeaponAxe* GetCurrentMelee() const { return currentMelee_.get(); }
	void SetMeleeModel(KamataEngine::Model* model, uint32_t textureHandle);

	// 通常コンボの見た目更新(attackState==nullptrなら非攻撃時の構えのみ)
	void UpdateMeleeVisual(const BaseMelee::GenericAttackAnimState* attackState);
	// 地面叩きつけの見た目更新(ホーミングジャンプの振りかぶり/急降下)
	void UpdateSmashVisual(BaseMelee::AttackPhase phase, float t);

	bool IsSwingActive() const { return isSwingActive_; }
	AABB GetSwingAABB() const;
	AABB GetSmashAABB() const;
	int GetCurrentComboIndex() const { return currentComboIndex_; }

	// --- ホーミングジャンプ用 ---
	// true の間はマップ判定・重力・敵同士の分離を行わず、速度をそのまま位置へ反映する
	void SetPhasing(bool phasing);
	bool IsPhasing() const { return ignoreMapCollision_; }
	// 体が地形(ブロック)に埋まっているか。判定を戻して良いかの確認に使う
	bool IsOverlappingTerrain() const;
	void SetLeapTarget(const KamataEngine::Vector3& target) { leapTarget_ = target; }
	void SnapTo(const KamataEngine::Vector3& position);

	// プレイヤーの攻撃によるノックバックは受けない(ガードの押し戻しはRequestBlockKnockback経由で受ける)
	void RequestBulletKnockback(const KamataEngine::Vector3& /*direction*/, float /*power*/) override {}

	// 攻撃中も止まらないので、装備/解除で動きを止めない
	EquipStyle GetEquipStyle() const override { return EquipStyle::kLight; }

	bool IsSmashActive() const { return smashActiveTimer_ > 0.0f; }
	// 判定を戻した後、床に着いた瞬間に叩きつけの判定を出す
	void BeginSmashLanding() {
		awaitingSmashLanding_ = true;
		smashLandingWait_ = 0.0f;
	}
	void CancelSmashLanding() {
		awaitingSmashLanding_ = false;
		smashActiveTimer_ = 0.0f;
	}

	BaseMelee* GetObstacleMelee() override { return currentMelee_.get(); }
	void ApplyObstacleSwingVisual(const BaseMelee::GenericAttackAnimState& state) override { UpdateMeleeVisual(&state); }

protected:
	void RefreshWeaponVisual() override { UpdateMeleeVisual(nullptr); }

	std::unique_ptr<BaseEnemyBehavior> CreateBehaviorModule() override { return std::make_unique<SapperBehavior>(*this); }

	void ApplyWeaponEquipState(bool equipped) override {
		if (currentMelee_)
			currentMelee_->SetWeaponState(equipped ? WeaponState::kEquipped : WeaponState::kHolstered);
	}
	void SnapWeaponEquipState(bool equipped) override {
		if (currentMelee_)
			currentMelee_->SnapWeaponState(equipped ? WeaponState::kEquipped : WeaponState::kHolstered);
	}
	// 倒すと斧を落とす(プレイヤーも使える)
	bool GetDropInfo(DropItemCategory& outCategory, DropWeaponKind& outKind) const override {
		outCategory = DropItemCategory::kMelee;
		outKind = DropWeaponKind::kLongAxe;
		return true;
	}
private:
	std::unique_ptr<WeaponAxe> currentMelee_;

	bool isSwingActive_ = false;
	int currentComboIndex_ = 0;
	KamataEngine::Vector3 leapTarget_{};

	static inline int32_t kSapperHP = 175; // Cuirassier(350)の半分

	// 重い振り: Cuirassierと同じ体の傾き
	static inline BodyComboPoseSet kBodyCombo = {
	    {MakeBodyPose(-0.25f, -1.0f, 0.0f, {1.0f, 1.03f, 1.0f},  {0.0f, 0.03f, 0.0f}),  MakeBodyPose(-0.25f, 1.3f, 0.0f, {1.0f, 1.03f, 1.0f},  {0.0f, 0.03f, 0.0f}),
	     MakeBodyPose(-0.6f, -1.0f, 0.0f, {1.0f, 1.05f, 1.0f},  {0.0f, 0.05f, 0.0f}) },
	    {MakeBodyPose(0.4f,   0.35f, 0.0f, {1.03f, 0.97f, 1.0f}, {0.0f, -0.04f, 0.0f}), MakeBodyPose(0.45f,  0.0f, 0.0f, {1.15f, 0.97f, 1.0f}, {0.0f, -0.04f, 0.0f}),
	     MakeBodyPose(0.7f,  -0.2f, 0.0f, {1.05f, 0.95f, 1.0f}, {0.0f, -0.08f, 0.0f})},
	    {MakeBodyPose(0.4f,   1.0f,  0.0f, {1.0f, 0.98f, 1.0f},  {0.0f, -0.03f, 0.0f}), MakeBodyPose(0.5f,   0.0f, 0.0f, {1.0f, 0.98f, 1.0f},  {0.0f, -0.03f, 0.0f}),
	     MakeBodyPose(0.65f, 0.0f,  0.0f, {1.0f, 0.97f, 1.0f},  {0.0f, -0.05f, 0.0f})},
	};
	// 叩きつけ: 振りかぶりで反り返り、急降下で前のめりに沈む
	static inline BodyLungePoseSet kBodySmash = {
	    MakeBodyPose(-0.5f, 0.0f, 0.0f, {0.97f, 1.05f, 1.0f}, {0.0f, 0.05f, 0.0f}),
	    MakeBodyPose(0.9f, 0.0f, 0.0f, {1.08f, 0.9f, 1.0f}, {0.0f, -0.1f, 0.0f}),
	};

	void TickSmashImpact();

	bool awaitingSmashLanding_ = false;
	float smashLandingWait_ = 0.0f;
	float smashActiveTimer_ = 0.0f;
	static inline float kSmashActiveDuration = 0.12f; // 着地後に叩きつけ判定が出ている時間(秒)
	static inline float kSmashLandingTimeout = 3.0f;  // 着地しないまま待ち続けた時の安全弁
};