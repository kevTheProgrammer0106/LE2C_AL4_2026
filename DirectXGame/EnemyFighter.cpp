#include "EnemyFighter.h"
#include "GameScene.h"
#include "Global.h"
#include "GlobalVariables.h"
#include "Player.h"

using namespace KamataEngine;

void EnemyFighter::Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	BaseEnemy::Initialize(partResources, camera, position);
	SetMaxHP(125);
	SetHP(125);

	velocity_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}


void EnemyFighter::Update() {
	BaseEnemy::Update();

	if (isDead_ || !currentMelee_)
		return;
	if (GetBehaviorModule().IsDeathBehavior())
		return;

	UpdateWeaponEquipState();

	// kMelee drives the attack pose itself.
	if (GetBehaviorModule().GetBehavior() == BaseEnemyBehavior::Behavior::kMelee)
		return;
	UpdateMeleeVisual(nullptr);
}


void EnemyFighter::Draw() {
	BaseEnemy::Draw();
	if (!isDead_ && !GetBehaviorModule().IsDeathBehavior() && currentMelee_) {
		currentMelee_->Draw();
	}
}

void EnemyFighter::SetMeleeModel(Model* model, uint32_t textureHandle) {
	currentMelee_ = std::make_unique<WeaponSabre>();
	currentMelee_->Initialize(model, textureHandle, camera_, worldTransform_.translation_);
	SnapWeaponEquipState(ignoreEquipStates_); // starts holstered unless arena-spawned
}

void EnemyFighter::UpdateMeleeVisual(const BaseMelee::GenericAttackAnimState* attackState) {
	if (!currentMelee_) {
		return;
	}
	currentMelee_->UpdateWeaponState();
	bool facingRight = isRight();
	currentMelee_->SetReactionPose(GetReactionPose());
	currentMelee_->SetMeleePositionGeneric(modelWorldTransform_.translation_, modelWorldTransform_.rotation_, facingRight, attackState);

	// 攻撃判定は「振っている(kAttack)」フェーズの間だけ有効
	isSwingActive_ = (attackState != nullptr && attackState->phase == BaseMelee::AttackPhase::kAttack);
	if (attackState) {
		currentComboIndex_ = attackState->comboIndex;
	}
}

AABB EnemyFighter::GetSwingAABB() const { return currentMelee_->GetSwingAABBGeneric(worldTransform_.translation_, isRight(), currentComboIndex_); }