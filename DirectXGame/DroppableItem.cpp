#include "DroppableItem.h"
#include "GlobalVariables.h"
#include "ObjectBounce.h"
#include <random>

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "DroppableItem";

// ドロップごとのばらつき用の乱数(全アイテムで共有)
std::mt19937& Rng() {
	static std::mt19937 rng{std::random_device{}()};
	return rng;
}
float RandRange(float minValue, float maxValue) { return std::uniform_real_distribution<float>(minValue, maxValue)(Rng()); }
int RandRangeInt(int minValue, int maxValue) { return std::uniform_int_distribution<int>(minValue, maxValue)(Rng()); }
} // namespace

void DroppableItem::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "HalfWidth", kHalfWidth);
	gv->AddItem(kGroupName, "HalfHeight", kHalfHeight);
	gv->AddItem(kGroupName, "HalfDepth", kHalfDepth);
	gv->AddItem(kGroupName, "BobAmplitude", kBobAmplitude);
	gv->AddItem(kGroupName, "BobSpeed", kBobSpeed);
	gv->AddItem(kGroupName, "SpinSpeed", kSpinSpeed);
	gv->AddItem(kGroupName, "LifeTime", kLifeTime);
	gv->AddItem(kGroupName, "BlinkWarningDuration", kBlinkWarningDuration);
	gv->AddItem(kGroupName, "BlinkFrequency", kBlinkFrequency);
	gv->AddItem(kGroupName, "BodySize", kBodySize);
	gv->AddItem(kGroupName, "Gravity", kGravity);
	gv->AddItem(kGroupName, "MaxFallSpeed", kMaxFallSpeed);
	gv->AddItem(kGroupName, "LaunchSpeedX", kLaunchSpeedX);
	gv->AddItem(kGroupName, "LaunchSpeedYMin", kLaunchSpeedYMin);
	gv->AddItem(kGroupName, "LaunchSpeedYMax", kLaunchSpeedYMax);
	gv->AddItem(kGroupName, "RestitutionMin", kRestitutionMin);
	gv->AddItem(kGroupName, "RestitutionMax", kRestitutionMax);
	gv->AddItem(kGroupName, "Friction", kFriction);
	gv->AddItem(kGroupName, "BounceCountMin", kBounceCountMin);
	gv->AddItem(kGroupName, "BounceCountMax", kBounceCountMax);
	gv->AddItem(kGroupName, "SettleSpeed", kSettleSpeed);
	gv->AddItem(kGroupName, "ThrowSpeedX", kThrowSpeedX);
	gv->AddItem(kGroupName, "ThrowSpeedY", kThrowSpeedY);
}

void DroppableItem::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kHalfWidth = gv->GetFloatValue(kGroupName, "HalfWidth");
	kHalfHeight = gv->GetFloatValue(kGroupName, "HalfHeight");
	kHalfDepth = gv->GetFloatValue(kGroupName, "HalfDepth");
	kBobAmplitude = gv->GetFloatValue(kGroupName, "BobAmplitude");
	kBobSpeed = gv->GetFloatValue(kGroupName, "BobSpeed");
	kSpinSpeed = gv->GetFloatValue(kGroupName, "SpinSpeed");
	kLifeTime = gv->GetFloatValue(kGroupName, "LifeTime");
	kBlinkWarningDuration = gv->GetFloatValue(kGroupName, "BlinkWarningDuration");
	kBlinkFrequency = gv->GetFloatValue(kGroupName, "BlinkFrequency");
	kBodySize = gv->GetFloatValue(kGroupName, "BodySize");
	kGravity = gv->GetFloatValue(kGroupName, "Gravity");
	kMaxFallSpeed = gv->GetFloatValue(kGroupName, "MaxFallSpeed");
	kLaunchSpeedX = gv->GetFloatValue(kGroupName, "LaunchSpeedX");
	kLaunchSpeedYMin = gv->GetFloatValue(kGroupName, "LaunchSpeedYMin");
	kLaunchSpeedYMax = gv->GetFloatValue(kGroupName, "LaunchSpeedYMax");
	kRestitutionMin = gv->GetFloatValue(kGroupName, "RestitutionMin");
	kRestitutionMax = gv->GetFloatValue(kGroupName, "RestitutionMax");
	kFriction = gv->GetFloatValue(kGroupName, "Friction");
	kBounceCountMin = gv->GetIntValue(kGroupName, "BounceCountMin");
	kBounceCountMax = gv->GetIntValue(kGroupName, "BounceCountMax");
	kSettleSpeed = gv->GetFloatValue(kGroupName, "SettleSpeed");
	kThrowSpeedX = gv->GetFloatValue(kGroupName, "ThrowSpeedX");
	kThrowSpeedY = gv->GetFloatValue(kGroupName, "ThrowSpeedY");
}

void DroppableItem::Initialize(Model* model, uint32_t textureHandle, Camera* camera, const Vector3& position, DropItemCategory category, DropWeaponKind kind, DropItemState state) {
#ifdef _DEBUG
	assert(model);
	assert(camera);
#endif
	model_ = model;
	textureHandle_ = textureHandle;
	camera_ = camera;
	category_ = category;
	kind_ = kind;
	state_ = state;

	// 状態に応じた色(準備完了=白、クールダウン中=暗め)
	objectColor_.Initialize();
	if (state_ == DropItemState::kOnCooldown) {
		objectColor_.SetColor({0.45f, 0.45f, 0.45f, 1.0f});
	} else {
		objectColor_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	}

	basePosition_ = position;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {0.6f, 0.6f, 0.6f};

	bobTimer_ = 0.0f;
	isPickedUp_ = false;
	lifeTimer_ = 0.0f;
	isExpired_ = false;

	// ---- 放り出しの初期化(アイテムごとにランダム) ----
	isSettled_ = false;
	launchChecked_ = false;
	bounceCount_ = 0;
	velocity_ = {RandRange(-kLaunchSpeedX, kLaunchSpeedX), RandRange(kLaunchSpeedYMin, kLaunchSpeedYMax), 0.0f};
	restitution_ = RandRange(kRestitutionMin, kRestitutionMax);
	maxBounces_ = RandRangeInt(std::min(kBounceCountMin, kBounceCountMax), std::max(kBounceCountMin, kBounceCountMax));
}

bool DroppableItem::WillLandSafely(const Vector3& startPos, const Vector3& startVel) {
	if (!mapChipField_) {
		return true;
	}
	// 実際の跳ね方と同じ計算を、コピーした状態で先読みする
	Vector3 pos = startPos;
	Vector3 vel = startVel;
	const float halfSize = kBodySize * 0.5f;

	for (int i = 0; i < kPredictFrames; ++i) {
		vel.y -= kGravity;
		vel.y = std::max(vel.y, -kMaxFallSpeed);

		bool landed = false;
		ObjectBounce::Step(mapChipField_, pos, vel, kBodySize, kBodySize, restitution_, kFriction, false, landed);

		// 空中でキルブロックに入るなら危険
		if (mapChipField_->IsKillAtPosition(pos)) {
			return false;
		}
		if (landed) {
			// 着地した足元がキルブロックなら危険、そうでなければ安全
			Vector3 foot = {pos.x, pos.y - halfSize - 0.1f, 0.0f};
			return !mapChipField_->IsKillAtPosition(foot);
		}
	}
	// 予測の間に床へ着地できなかった(穴へ落ちる)場合も危険とみなす
	return false;
}

void DroppableItem::UpdateBounce() {
	// マップが無いときは跳ねられないので、その場で静止扱いにする
	if (!mapChipField_) {
		isSettled_ = true;
		velocity_ = {};
		return;
	}

	// 放り出し直後: 最初の軌道が危険なら横移動をやめ、その場で跳ねるだけにする
	if (!launchChecked_) {
		launchChecked_ = true;
		if (!WillLandSafely(worldTransform_.translation_, velocity_)) {
			velocity_.x = 0.0f;
		}
	}

	// 重力(落下速度は上限あり)
	velocity_.y -= kGravity;
	velocity_.y = std::max(velocity_.y, -kMaxFallSpeed);

	// 壁・天井・床で跳ね返る移動
	Vector3 pos = worldTransform_.translation_;
	bool landed = false;
	ObjectBounce::Step(mapChipField_, pos, velocity_, kBodySize, kBodySize, restitution_, kFriction, false, landed);
	worldTransform_.translation_ = pos;

	if (landed) {
		++bounceCount_;
		bool settle = (bounceCount_ >= maxBounces_) || (std::abs(velocity_.y) < kSettleSpeed);
		// 次の着地先がキルブロック/穴なら、今回の着地位置を最終位置にする
		if (!settle && !WillLandSafely(worldTransform_.translation_, velocity_)) {
			settle = true;
		}
		if (settle) {
			isSettled_ = true;
			velocity_ = {};
			basePosition_ = worldTransform_.translation_;
		}
	}
}

void DroppableItem::UpdateIdle() {
	// 元の挙動: 静止位置を基準に上下にふわふわ
	bobTimer_ += 1.0f / 60.0f;
	float bob = std::sin(bobTimer_ * kBobSpeed) * kBobAmplitude;
	worldTransform_.translation_ = basePosition_ + Vector3(0.0f, bob, 0.0f);
}

void DroppableItem::Update() {
	if (isPickedUp_ || isExpired_) {
		return;
	}

	lifeTimer_ += 1.0f / 60.0f;
	if (lifeTimer_ >= kLifeTime) {
		isExpired_ = true;
		return;
	}

	if (!isSettled_) {
		UpdateBounce();
	} else {
		UpdateIdle();
	}
	worldTransform_.rotation_.y += kSpinSpeed * (1.0f / 60.0f); // 元の挙動: 常にY軸だけ回転
	WorldTransformUpdate(worldTransform_);
}

void DroppableItem::Draw() {
	if (isPickedUp_ || isExpired_ || !model_) {
		return;
	}

	// 消える直前は点滅で警告する(obj/texは不要、単純に描画をスキップするだけ)
	float remaining = kLifeTime - lifeTimer_;
	if (remaining <= kBlinkWarningDuration) {
		int blinkPhase = static_cast<int>(lifeTimer_ * kBlinkFrequency) % 2;
		if (blinkPhase == 0) {
			return;
		}
	}

	Model::PreDraw();
	model_->Draw(worldTransform_, *camera_, textureHandle_, &objectColor_);
	Model::PostDraw();
}

AABB DroppableItem::GetAABB() const {
	Vector3 pos = worldTransform_.translation_;
	AABB aabb;
	aabb.min = {pos.x - kHalfWidth, pos.y - kHalfHeight, pos.z - kHalfDepth};
	aabb.max = {pos.x + kHalfWidth, pos.y + kHalfHeight, pos.z + kHalfDepth};
	return aabb;
}
void DroppableItem::Launch(const Vector3& velocity) {
	// ランダム放り出しの代わりに、指定した初速でバウンドをやり直す
	velocity_ = velocity;
	isSettled_ = false;
	launchChecked_ = false; // 穴・キルブロックへ向かう場合は横速度を消す安全確認を通す
	bounceCount_ = 0;
}

Vector3 DroppableItem::MakeThrowVelocity(bool facingRight) {
	const float dir = facingRight ? 1.0f : -1.0f;
	// 少しだけ飛距離をばらつかせる
	return {dir * kThrowSpeedX * RandRange(0.85f, 1.15f), kThrowSpeedY, 0.0f};
}