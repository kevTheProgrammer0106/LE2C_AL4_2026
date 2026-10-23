#include "EnemySapper.h"
#include "GameScene.h"
#include "GlobalVariables.h"
#include "MapChipField.h"
#include "Player.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "EnemySapper";
}

void EnemySapper::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "MaxHP", kSapperHP);
	gv->AddItem(kGroupName, "SmashActiveDuration", kSmashActiveDuration);
	RegisterBodyComboPoseSet(kGroupName, "BodyCombo", kBodyCombo);
	RegisterBodyLungePoseSet(kGroupName, "BodySmash", kBodySmash);
	WeaponAxe::RegisterGlobalVariables();
	SapperBehavior::RegisterGlobalVariables();
}

void EnemySapper::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kSapperHP = gv->GetIntValue(kGroupName, "MaxHP");
	kSmashActiveDuration = gv->GetFloatValue(kGroupName, "SmashActiveDuration");
	ApplyBodyComboPoseSet(kGroupName, "BodyCombo", kBodyCombo);
	ApplyBodyLungePoseSet(kGroupName, "BodySmash", kBodySmash);
	WeaponAxe::ApplyGlobalVariables();
	SapperBehavior::ApplyGlobalVariables();
}

void EnemySapper::Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, Camera* camera, const Vector3& position) {
	BaseEnemy::Initialize(partResources, camera, position);
	SetMaxHP(kSapperHP);
	SetHP(kSapperHP);
}

void EnemySapper::Update() {
	BaseEnemy::Update();

	if (isDead_)
		return;
	TickSmashImpact();

	if (!currentMelee_)
		return;
	if (GetBehaviorModule().IsDeathBehavior())
		return;

	UpdateWeaponEquipState();

	// kMelee / kLeap は Behavior 側が毎フレーム見た目を更新する
	using B = BaseEnemyBehavior::Behavior;
	B current = GetBehaviorModule().GetBehavior();
	if (current == B::kMelee || current == B::kLeap)
		return;
	if (TickObstacleSwing())
		return;
	UpdateMeleeVisual(nullptr);
}

void EnemySapper::Draw() {
	BaseEnemy::Draw();
	if (!isDead_ && !GetBehaviorModule().IsDeathBehavior() && currentMelee_) {
		currentMelee_->Draw();
		DrawAttackEffect(isSwingActive_ || IsSmashActive(), IsSmashActive() ? GetSmashAABB() : GetSwingAABB());
	}
}

void EnemySapper::UpdateMeleeVisual(const BaseMelee::GenericAttackAnimState* attackState) {
	if (!currentMelee_) {
		return;
	}
	currentMelee_->UpdateWeaponState();
	currentMelee_->SetReactionPose(GetReactionPose());
	currentMelee_->SetMeleePositionGeneric(modelWorldTransform_.translation_, modelWorldTransform_.rotation_, isRight(), attackState);

	isSwingActive_ = (attackState != nullptr && attackState->phase == BaseMelee::AttackPhase::kAttack);
	if (attackState) {
		currentComboIndex_ = attackState->comboIndex;
		attackBodyPose_ = EvaluateBodyComboPose(kBodyCombo, static_cast<BodyAttackPhase>(static_cast<int>(attackState->phase)), attackState->t, attackState->comboIndex);
	} else {
		attackBodyPose_ = SRTOffset{};
	}
}

void EnemySapper::SetMeleeModel(Model* model, uint32_t textureHandle) {
	currentMelee_ = std::make_unique<WeaponAxe>();
	currentMelee_->Initialize(model, textureHandle, camera_, worldTransform_.translation_);
	SnapWeaponEquipState(ignoreEquipStates_);
}

void EnemySapper::UpdateSmashVisual(BaseMelee::AttackPhase phase, float t) {
	if (!currentMelee_) {
		return;
	}
	currentMelee_->UpdateWeaponState();
	currentMelee_->SetReactionPose(GetReactionPose());
	currentMelee_->SetSmashPositionGeneric(modelWorldTransform_.translation_, modelWorldTransform_.rotation_, isRight(), phase, t);

	isSwingActive_ = false; // 急降下中は見た目だけ。当たり判定は着地の瞬間(TickSmashImpact)に出る
	attackBodyPose_ = EvaluateBodyLungePose(kBodySmash, static_cast<BodyAttackPhase>(static_cast<int>(phase)), t);
}

AABB EnemySapper::GetSwingAABB() const { return currentMelee_->GetSwingAABBGeneric(worldTransform_.translation_, isRight(), currentComboIndex_); }
AABB EnemySapper::GetSmashAABB() const { return currentMelee_->GetSmashAABBGeneric(worldTransform_.translation_); }

// ------------------------------------------------------------------ ホーミングジャンプ用
void EnemySapper::SetPhasing(bool phasing) {
	if (ignoreMapCollision_ == phasing) {
		return;
	}
	ignoreMapCollision_ = phasing;
	if (!phasing) {
		// 判定を戻す瞬間に地形へ埋まっていたら、安全な目標位置へ戻す(ノックバックや死亡で中断された場合の保険)
		if (IsOverlappingTerrain()) {
			SnapTo(leapTarget_);
		}
		onGround_ = false; // 戻した最初のフレームで接地を取り直す
	}
}

bool EnemySapper::IsOverlappingTerrain() const {
	if (!mapChipField_) {
		return false;
	}
	const Vector3 c = worldTransform_.translation_;
	const float hx = kWidth * 0.5f - 0.05f;
	const float hy = kHeight * 0.5f - 0.05f;
	const float xs[3] = {c.x - hx, c.x, c.x + hx};
	const float ys[3] = {c.y - hy, c.y, c.y + hy};
	for (float x : xs) {
		for (float y : ys) {
			const Vector3 p = {x, y, 0.0f};
			// 一方通行の足場は横/下からは壁ではないので数えない
			if (mapChipField_->IsSolidAtPosition(p, true) && !mapChipField_->IsOneWayAtPosition(p)) {
				return true;
			}
		}
	}
	return false;
}

void EnemySapper::SnapTo(const Vector3& position) {
	worldTransform_.translation_ = position;
	velocity_ = {0.0f, 0.0f, 0.0f};
	WorldTransformUpdate(worldTransform_);
}

void EnemySapper::TickSmashImpact() {
	constexpr float kDt = 1.0f / 60.0f;
	if (smashActiveTimer_ > 0.0f) {
		smashActiveTimer_ = std::max(0.0f, smashActiveTimer_ - kDt);
	}
	if (!awaitingSmashLanding_ || ignoreMapCollision_) {
		return; // 判定が戻るまでは待つ
	}

	smashLandingWait_ += kDt;
	if (onGround_) {
		awaitingSmashLanding_ = false;
		smashActiveTimer_ = kSmashActiveDuration;
		if (currentMelee_) {
			currentMelee_->ResetSwingHits(); // この一撃だけ当たる
		}
		if (gameScene_) {
			Vector3 feet = GetWorldPosition() + Vector3(0.0f, -kHeight * 0.5f, 0.0f);
			gameScene_->CreateBurstEffect(BurstType::kLand, feet, {0.0f, 1.0f});
			gameScene_->CreateBurstEffect(BurstType::kHit, feet, {0.0f, 1.0f});
			// 叩きつけの衝撃波: 地面に沿って左右へ土煙を飛ばす(手前に出して見やすくする)
			Vector3 fx = feet;
			fx.z -= 0.3f;
			gameScene_->CreateBurstEffect(BurstType::kSmash, fx, {-1.0f, 0.15f});
			gameScene_->CreateBurstEffect(BurstType::kSmash, fx, {1.0f, 0.15f});
		}
	} else if (smashLandingWait_ >= kSmashLandingTimeout) {
		awaitingSmashLanding_ = false;
	}
}