#include "EnemyFighter.h"
#include "GameScene.h"
#include "Global.h"
#include "GlobalVariables.h"
#include "Player.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "EnemyFighter";
}

void EnemyFighter::RegisterGlobalVariables() {
	GlobalVariables::GetInstance()->CreateGroup(kGroupName);
	RegisterBodyComboPoseSet(kGroupName, "BodyCombo", kBodyCombo);
}
void EnemyFighter::ApplyGlobalVariables() { ApplyBodyComboPoseSet(kGroupName, "BodyCombo", kBodyCombo); }

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
		DrawAttackEffect(isSwingActive_, GetSwingAABB()); // 斬撃エフェクト(判定の中心に表示)
	}
}

void EnemyFighter::SetMeleeModel(Model* model, uint32_t textureHandle) {
	currentMelee_ = CreateMeleeWeapon();
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

	isSwingActive_ = (attackState != nullptr && attackState->phase == BaseMelee::AttackPhase::kAttack);
	if (attackState) {
		currentComboIndex_ = attackState->comboIndex;
		attackBodyPose_ = EvaluateBodyComboPose(kBodyCombo, static_cast<BodyAttackPhase>(static_cast<int>(attackState->phase)), attackState->t, attackState->comboIndex);
	} else {
		attackBodyPose_ = SRTOffset{};
	}
}

AABB EnemyFighter::GetSwingAABB() const { return currentMelee_->GetSwingAABBGeneric(worldTransform_.translation_, isRight(), currentComboIndex_); }