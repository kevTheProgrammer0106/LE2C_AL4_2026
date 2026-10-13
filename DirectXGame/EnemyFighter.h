#pragma once

#include "BaseEnemy.h"
#include "BaseMelee.h"
#include "FighterBehavior.h"
#include "WeaponSabre.h"

class EnemyFighter : public BaseEnemy {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update() override;
	void Draw() override;

	WeaponSabre* GetCurrentMelee() const { return currentMelee_.get(); }

	void SetMeleeModel(KamataEngine::Model* model, uint32_t textureHandle);

	// FighterBehaviorが毎フレーム、現在の攻撃フェーズ状態を渡してWeaponSabreの見た目を更新する。
	// attackState==nullptrなら非攻撃時のホルスター/構えのみ更新する。
	void UpdateMeleeVisual(const BaseMelee::GenericAttackAnimState* attackState);

	//  対プレイヤー当たり判定用。攻撃フェーズ(kAttack)の間だけtrueになる。
	bool IsSwingActive() const { return isSwingActive_; }
	AABB GetSwingAABB() const;
	int GetCurrentComboIndex() const { return currentComboIndex_; }

protected:
	void RefreshWeaponVisual() override { UpdateMeleeVisual(nullptr); }

	std::unique_ptr<BaseEnemyBehavior> CreateBehaviorModule() override { return std::make_unique<FighterBehavior>(*this); }
	bool GetDropInfo(DropItemCategory& outCategory, DropWeaponKind& outKind) const override {
		outCategory = DropItemCategory::kMelee;
		outKind = DropWeaponKind::kSabreLight;
		return true;
	}

	void ApplyWeaponEquipState(bool equipped) override {
		if (currentMelee_)
			currentMelee_->SetWeaponState(equipped ? WeaponState::kEquipped : WeaponState::kHolstered);
	}
	void SnapWeaponEquipState(bool equipped) override {
		if (currentMelee_)
			currentMelee_->SnapWeaponState(equipped ? WeaponState::kEquipped : WeaponState::kHolstered);
	}
private:
	std::unique_ptr<WeaponSabre> currentMelee_;

	bool isSwingActive_ = false;
	int currentComboIndex_ = 0;

	// Light, quick slashes: small wind-ups, snappy peaks.
	static inline BodyComboPoseSet kBodyCombo = {
	    {MakeBodyPose(-0.15f, -0.9f, 0.0f), MakeBodyPose(-0.15f, 1.2f, 0.0f), MakeBodyPose(-0.5f, -0.9f, 0.0f)},
	    {MakeBodyPose(0.25f, 0.3f, 0.0f), MakeBodyPose(0.35f, 0.0f, 0.0f, {1.08f, 1.0f, 1.0f}), MakeBodyPose(0.5f, -0.2f, 0.0f)},
	    {MakeBodyPose(0.3f, 0.9f, 0.0f), MakeBodyPose(0.4f, 0.0f, 0.0f), MakeBodyPose(0.55f, 0.0f, 0.0f)},
	};
};