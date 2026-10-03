#include "BaseFirearm.h"
#include "GlobalVariables.h"

    using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "BaseFirearm";
}

void BaseFirearm::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "ImpactPower", kImpactPower);
	gv->AddItem(kGroupName, "AimLeanAmount", kAimLeanAmount);
	gv->AddItem(kGroupName, "RecoilDuration", static_cast<int32_t>(kRecoilDuration));
	gv->AddItem(kGroupName, "RecoilKick", kRecoilKick);
	gv->AddItem(kGroupName, "EquipLerpSpeed", kEquipLerpSpeed);
}
void BaseFirearm::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kImpactPower = gv->GetFloatValue(kGroupName, "ImpactPower");
	kAimLeanAmount = gv->GetFloatValue(kGroupName, "AimLeanAmount");
	kRecoilDuration = static_cast<uint32_t>(gv->GetIntValue(kGroupName, "RecoilDuration"));
	kRecoilKick = gv->GetFloatValue(kGroupName, "RecoilKick");
	kEquipLerpSpeed = gv->GetFloatValue(kGroupName, "EquipLerpSpeed");
}

void BaseFirearm::Initialize(Model* model, uint32_t textureHandle, Camera* camera) {
	assert(model);
	assert(camera);
	model_ = model;
	textureHandle_ = textureHandle;
	camera_ = camera;
	worldTransform_.Initialize();
}

// Shared by every firearm. Only reads the virtual offset getters -- no stats.
void BaseFirearm::UpdateFollow(const WorldTransform& parentTransform, float localAimAngle, bool facingRight) {
	TickReload();

	equipFactor_ = Lerp(equipFactor_, equipTarget_, kEquipLerpSpeed);

	Matrix4x4 rotMatrix = Multiply(MakeRotateXMatrix(parentTransform.rotation_.x), Multiply(MakeRotateYMatrix(parentTransform.rotation_.y), MakeRotateZMatrix(parentTransform.rotation_.z)));

	float facingSign = facingRight ? 1.0f : -1.0f;
	float correctedAimAngle = localAimAngle * facingSign;

	// Combat pose, blended toward knockback/stun when debuffed
	const SRTOffset combat = ComputeCombatOffset(GetCombatOffset());

	// Stowed pose: reload pose while "showing reload", otherwise holster
	const SRTOffset& stowed = isShowingReloadPose_ ? GetReloadOffset() : GetHolsterOffset();

	Vector3 holsterRotation = stowed.rotation;
	if (!facingRight) {
		holsterRotation.y = -holsterRotation.y;
		holsterRotation.z = -holsterRotation.z;
	}

	float holsterRotY = parentTransform.rotation_.y + holsterRotation.y;
	float holsterRotZ = holsterRotation.z;

	float combatRotY = parentTransform.rotation_.y + GetModelYawOffset() + combat.rotation.y;
	// combat.rotation.z is a world-space roll authored facing right, so mirror it when facing left
	// (otherwise knockback/stun/block would tilt the gun the wrong way on left-facing entities)
	float combatRotZ = correctedAimAngle + combat.rotation.z * facingSign;

	Vector3 blendedLocalOffset = Lerp(stowed.translation, combat.translation, equipFactor_);
	Vector3 blendedScale = Lerp(stowed.scale, combat.scale, equipFactor_);
	float blendedRotX = Lerp(stowed.rotation.x, combat.rotation.x, equipFactor_);
	float blendedRotY = Lerp(holsterRotY, combatRotY, equipFactor_);
	float blendedRotZ = Lerp(holsterRotZ, combatRotZ, equipFactor_);

	Vector3 rotatedOffset = TransformNormal(blendedLocalOffset, rotMatrix);

	worldTransform_.translation_ = parentTransform.translation_ + rotatedOffset;
	worldTransform_.rotation_.x = parentTransform.rotation_.x + blendedRotX;
	worldTransform_.rotation_.y = blendedRotY;
	worldTransform_.rotation_.z = blendedRotZ;
	worldTransform_.scale_ = blendedScale;

	WorldTransformUpdate(worldTransform_);
}

void BaseFirearm::Draw() {
	if (!model_ || !camera_)
		return;
	Model::PreDraw();
	model_->Draw(worldTransform_, *camera_, textureHandle_);
	Model::PostDraw();
}
