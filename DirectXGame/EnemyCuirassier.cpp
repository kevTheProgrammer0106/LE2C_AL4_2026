#include "EnemyCuirassier.h"
#include "GameScene.h"
#include "Player.h"

using namespace KamataEngine;

void EnemyCuirassier::Initialize(const std::array<PartResources, static_cast<size_t>(PartSlot::kCount)>& partResources, Camera* camera, const Vector3& position) {
	BaseEnemy::Initialize(partResources, camera, position);
	SetMaxHP(350);
	SetHP(350);
	if (currentMelee_) {
		currentMelee_->SetWeaponState(WeaponState::kEquipped);
	}
}

void EnemyCuirassier::Update() { BaseEnemy::Update(); }

void EnemyCuirassier::Draw() {
	BaseEnemy::Draw();
	if (!isDead_ && !GetBehaviorModule().IsDeathBehavior() && currentMelee_) {
		currentMelee_->Draw();
	}
}

void EnemyCuirassier::SetMeleeModel(Model* model, uint32_t textureHandle) {
	currentMelee_ = std::make_unique<WeaponHeavySabre>();
	currentMelee_->Initialize(model, textureHandle, camera_, worldTransform_.translation_);
	currentMelee_->SetWeaponState(WeaponState::kEquipped);
}

void EnemyCuirassier::UpdateMeleeVisual(const BaseMelee::GenericAttackAnimState* attackState) {
	if (!currentMelee_) {
		return;
	}
	currentMelee_->UpdateWeaponState();
	bool facingRight = isRight();
	currentMelee_->SetMeleePositionGeneric(worldTransform_.translation_, worldTransform_.rotation_, facingRight, attackState);

	isLungeActive_ = false;
	isSwingActive_ = (attackState != nullptr && attackState->phase == BaseMelee::AttackPhase::kAttack);
	if (attackState) {
		currentComboIndex_ = attackState->comboIndex;
	}
}

void EnemyCuirassier::UpdateLungeVisual(WeaponHeavySabre::AttackPhase lungePhase, float t) {
	if (!currentMelee_) {
		return;
	}
	currentMelee_->UpdateWeaponState();
	bool facingRight = isRight();
	currentMelee_->SetLungePositionGeneric(worldTransform_.translation_, worldTransform_.rotation_, facingRight, lungePhase, t);

	isSwingActive_ = false;
	isLungeActive_ = (lungePhase == WeaponHeavySabre::AttackPhase::kAttack);
}

AABB EnemyCuirassier::GetSwingAABB() const { return currentMelee_->GetSwingAABBGeneric(worldTransform_.translation_, isRight(), currentComboIndex_); }
AABB EnemyCuirassier::GetLungeAABB() const { return currentMelee_->GetLungeAABBGeneric(worldTransform_.translation_, isRight()); }

void EnemyCuirassier::CheckPlayerDirection() {
	constexpr float deltaTime = 1.0f / 60.0f;
	Player* player = gameScene_->GetPlayer();

	if (!isTurning_) {
		if (player->GetWorldPosition().x < GetWorldPosition().x) {
			velocity_.x = -kWalkSpeed;
			if (lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
				isTurning_ = true;
			}
		} else {
			velocity_.x = kWalkSpeed;
			if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
				isTurning_ = true;
			}
		}
	}

	if (isTurning_) {
		turnTimer_ -= deltaTime;
		float destinationRotationY = (lrDirection_ == LRDirection::kRight) ? std::numbers::pi_v<float> / 2.0f : std::numbers::pi_v<float> * 3.0f / 2.0f;
		float t = std::clamp(1.0f - (turnTimer_ / kTimeTurn), 0.0f, 1.0f);
		worldTransform_.rotation_.y = EaseInOut(turnFirstRotationY_, destinationRotationY, t);
		if (turnTimer_ <= 0.0f) {
			worldTransform_.rotation_.y = destinationRotationY;
			turnTimer_ = 0.0f;
			isTurning_ = false;
		}
	} else {
		float baseRotation = (lrDirection_ == LRDirection::kLeft) ? -(std::numbers::pi_v<float> / 2.0f) : -(std::numbers::pi_v<float> * 3.0f / 2.0f);
		float angle = std::sin(walkTimer_ * (2.0f * std::numbers::pi_v<float> / kWalkMotionTime)) * 0.2f;
		if (lrDirection_ == LRDirection::kLeft) {
			angle = -angle;
		}
		worldTransform_.rotation_.y = baseRotation + angle;
	}
}

void EnemyCuirassier::OnCollision(const Player* player) {
	if (GetBehaviorModule().IsDeathBehavior() || isDead_) {
		isCollisionDisabled_ = true;
		return;
	}
	if (player->isAttack()) {
		Vector3 dir = GetWorldPosition() - player->GetWorldPosition();
		float len = std::sqrtf(dir.x * dir.x + dir.y * dir.y);
		if (len > 0.0001f) {
			dir.x /= len;
			dir.y /= len;
		}
		dir.y += 0.4f;

		int damage = 10;
		float power = 1.0f;
		if (const BaseMelee* attackerMelee = player->GetCurrentMelee()) {
			damage = attackerMelee->GetDamage(player->GetComboIndex(), Owner::kPlayer);
			power = attackerMelee->GetImpactPower(player->GetComboIndex());
		}
		TakeDamage(damage, dir, power);

		Vector3 effectPos = (GetWorldPosition() + player->GetWorldPosition()) / 2.0f;
		gameScene_->CreateHitEffect(effectPos);
	}
}