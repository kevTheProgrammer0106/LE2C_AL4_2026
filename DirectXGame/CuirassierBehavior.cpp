#include "CuirassierBehavior.h"
#include "EnemyCuirassier.h"
#include "GameScene.h"
#include "Player.h"

#include <cmath>

using namespace KamataEngine;

CuirassierBehavior::CuirassierBehavior(EnemyCuirassier& owner) : BaseEnemyBehavior(owner), cuirassierOwner_(owner) {}

void CuirassierBehavior::InitializeBehaviorState(Behavior behavior) {
	switch (behavior) {
	case Behavior::kMelee:
		BehaviorMeleeInitialize();
		return;
	case Behavior::kBlock:
		BehaviorBlockInitialize();
		return;
	default:
		break;
	}
	BaseEnemyBehavior::InitializeBehaviorState(behavior);
}

void CuirassierBehavior::UpdateBehaviorState() {
	switch (GetBehavior()) {
	case Behavior::kMelee:
		BehaviorMeleeUpdate();
		RunPhysicsAndCollisionUpdate();
		return;
	case Behavior::kBlock:
		BehaviorBlockUpdate();
		RunPhysicsAndCollisionUpdate();
		return;
	default:
		break;
	}
	BaseEnemyBehavior::UpdateBehaviorState();
}

void CuirassierBehavior::BehaviorRootInitialize() {
	BaseEnemyBehavior::BehaviorRootInitialize();
	playerWasAttackingLastFrame_ = false;
}

void CuirassierBehavior::BehaviorRootUpdate() {
	BaseEnemyBehavior::BehaviorRootUpdate(); // 通常のパスファインディング追跡・CheckPlayerDirectionはEnemyCuirassier::Update経由

	GameScene* scene = OwnerGameScene();
	if (!scene || !scene->GetPlayer()) {
		return;
	}
	Player* player = scene->GetPlayer();

	// 通常の見た目(非攻撃時の構え)を追従させる
	cuirassierOwner_.UpdateMeleeVisual(nullptr);

	Vector3 diff = player->GetWorldPosition() - owner_.GetWorldPosition();
	float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

	// --- ランダムガード判定: プレイヤーが「攻撃を開始した瞬間」にのみ抽選する ---
	bool playerIsAttacking = player->isAttack();
	if (playerIsAttacking && !playerWasAttackingLastFrame_ && distance <= kBlockCheckRange) {
		if (sRandom_.GetRandValue(0.0f, 1.0f) < kBlockChance) {
			playerWasAttackingLastFrame_ = playerIsAttacking;
			RequestBehavior(Behavior::kBlock);
			return;
		}
	}
	playerWasAttackingLastFrame_ = playerIsAttacking;

	if (!IsPlayerSpotted()) {
		return;
	}
	if (!IsPlayerOnSameFloor()) {
		return;
	}
	if (distance <= kMeleeEngageRange) {
		RequestBehavior(Behavior::kMelee);
	}
}

void CuirassierBehavior::BehaviorMeleeInitialize() {
	meleeStage_ = MeleeStage::kLunge;
	lungePhase_ = LungePhase::kAnticipate;
	attackParameter_ = 0;
	comboIndex_ = 0;
	OwnerVelocity().x = 0.0f;
	if (WeaponHeavySabre* weapon = cuirassierOwner_.GetCurrentMelee()) {
		weapon->ResetSwingHits();
	}
}

void CuirassierBehavior::BehaviorMeleeUpdate() {
	++attackParameter_;

	GameScene* scene = OwnerGameScene();
	Player* player = scene ? scene->GetPlayer() : nullptr;
	if (player) {
		OwnerFaceDirection(player->GetWorldPosition().x < owner_.GetWorldPosition().x ? 1 /*kLeft*/ : 0 /*kRight*/);
	}
	bool facingRight = owner_.isRight();

	if (meleeStage_ == MeleeStage::kLunge) {
		float t = 0.0f;
		switch (lungePhase_) {
		case LungePhase::kAnticipate: {
			OwnerVelocity().x = 0.0f;
			t = std::clamp(static_cast<float>(attackParameter_) / (kLungeAnticipateDuration * 60.0f), 0.0f, 1.0f);
			if (static_cast<float>(attackParameter_) >= kLungeAnticipateDuration * 60.0f) {
				lungePhase_ = LungePhase::kAttack;
				attackParameter_ = 0;
			}
			break;
		}
		case LungePhase::kAttack: {
			OwnerVelocity().x = facingRight ? kLungeSpeed : -kLungeSpeed;
			t = std::clamp(static_cast<float>(attackParameter_) / (kLungeAttackDuration * 60.0f), 0.0f, 1.0f);
			if (static_cast<float>(attackParameter_) >= kLungeAttackDuration * 60.0f) {
				lungePhase_ = LungePhase::kRecovery;
				attackParameter_ = 0;
			}
			break;
		}
		case LungePhase::kRecovery:
		default: {
			OwnerVelocity().x = 0.0f;
			t = std::clamp(static_cast<float>(attackParameter_) / (kLungeRecoveryDuration * 60.0f), 0.0f, 1.0f);
			if (static_cast<float>(attackParameter_) >= kLungeRecoveryDuration * 60.0f) {
				meleeStage_ = MeleeStage::kCombo;
				comboPhase_ = ComboPhase::kAnticipate;
				comboIndex_ = 0;
				attackParameter_ = 0;
				if (WeaponHeavySabre* weapon = cuirassierOwner_.GetCurrentMelee()) {
					weapon->ResetSwingHits();
				}
			}
			break;
		}
		}

		WeaponHeavySabre::AttackPhase weaponPhase = (lungePhase_ == LungePhase::kAnticipate) ? WeaponHeavySabre::AttackPhase::kAnticipate
		                                            : (lungePhase_ == LungePhase::kAttack)   ? WeaponHeavySabre::AttackPhase::kAttack
		                                                                                     : WeaponHeavySabre::AttackPhase::kRecovery;
		cuirassierOwner_.UpdateLungeVisual(weaponPhase, t);
		return;
	}

	BaseMelee::GenericAttackAnimState animState;
	animState.comboIndex = comboIndex_;

	switch (comboPhase_) {
	case ComboPhase::kAnticipate: {
		OwnerVelocity().x = 0.0f;
		animState.phase = BaseMelee::AttackPhase::kAnticipate;
		float duration = 10.0f;
		animState.t = std::clamp(static_cast<float>(attackParameter_) / duration, 0.0f, 1.0f);
		if (static_cast<float>(attackParameter_) >= duration) {
			comboPhase_ = ComboPhase::kAttack;
			attackParameter_ = 0;
			WeaponHeavySabre::PlayLungeSound();
		}
		break;
	}
	case ComboPhase::kAttack: {
		animState.phase = BaseMelee::AttackPhase::kAttack;
		float duration = 10.0f;
		animState.t = std::clamp(static_cast<float>(attackParameter_) / duration, 0.0f, 1.0f);
		if (static_cast<float>(attackParameter_) >= duration) {
			comboPhase_ = ComboPhase::kRecovery;
			attackParameter_ = 0;
		}
		break;
	}
	case ComboPhase::kRecovery:
	default: {
		animState.phase = BaseMelee::AttackPhase::kRecovery;
		float duration = 18.0f;
		animState.t = std::clamp(static_cast<float>(attackParameter_) / duration, 0.0f, 1.0f);
		if (static_cast<float>(attackParameter_) >= duration) {
			if (comboIndex_ + 1 < 3) {
				comboIndex_++;
				comboPhase_ = ComboPhase::kAnticipate;
				attackParameter_ = 0;
				// [FIX] 次のコンボ段に入る前にヒット済みリストをリセットする。
				if (WeaponHeavySabre* weapon = cuirassierOwner_.GetCurrentMelee()) {
					weapon->ResetSwingHits();
				}
			} else {
				RequestBehavior(Behavior::kRoot);
			}
		}
		break;
	}
	}
	cuirassierOwner_.UpdateMeleeVisual(&animState);
}

void CuirassierBehavior::BehaviorBlockInitialize() {
	blockTimer_ = 0.0f;
	OwnerVelocity().x = 0.0f;
}

void CuirassierBehavior::BehaviorBlockUpdate() {
	blockTimer_ += 1.0f / 60.0f;
	OwnerVelocity().x = 0.0f;

	// ガード中も向きだけはプレイヤーへ向ける(EnemyCuirassier::OnCollisionの正面ガード判定と一致させる)
	if (GameScene* scene = OwnerGameScene()) {
		if (Player* player = scene->GetPlayer()) {
			OwnerFaceDirection(player->GetWorldPosition().x < owner_.GetWorldPosition().x ? 1 : 0);
		}
	}

	cuirassierOwner_.UpdateMeleeVisual(nullptr); // ガード中は武器を構えたまま(専用ポーズが欲しければここを拡張)

	if (blockTimer_ >= kBlockDuration) {
		RequestBehavior(Behavior::kRoot);
	}
}
WeaponHeavySabre& CuirassierBehavior::Weapon() { return *cuirassierOwner_.GetCurrentMelee(); }