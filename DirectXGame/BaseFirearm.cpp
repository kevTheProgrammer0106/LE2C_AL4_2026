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

void BaseFirearm::UpdateFollow(const WorldTransform& parentTransform, float localAimAngle, bool facingRight) {
	equipFactor_ = Lerp(equipFactor_, equipTarget_, kEquipLerpSpeed);

	Matrix4x4 parentRotation = Multiply(MakeRotateXMatrix(parentTransform.rotation_.x), Multiply(MakeRotateYMatrix(parentTransform.rotation_.y), MakeRotateZMatrix(parentTransform.rotation_.z)));

	float correctedAimAngle = facingRight ? localAimAngle : -localAimAngle;
	float combatRotZ = correctedAimAngle;
	float combatRotY = parentTransform.rotation_.y + offset_.rotation.y;

	const SRTOffset& stowedOffset = isShowingReloadPose_ ? reloadOffset_ : holsterOffset_;

	float holsterRotZ = stowedOffset.rotation.z;
	float holsterRotY = parentTransform.rotation_.y + stowedOffset.rotation.y;

	Vector3 blendedLocalOffset = Lerp(stowedOffset.translation, offset_.translation, equipFactor_);
	Vector3 blendedScale = Lerp(stowedOffset.scale, offset_.scale, equipFactor_);
	float blendedRotX = Lerp(stowedOffset.rotation.x, offset_.rotation.x, equipFactor_);
	float blendedRotY = Lerp(holsterRotY, combatRotY, equipFactor_);
	float blendedRotZ = Lerp(holsterRotZ, combatRotZ, equipFactor_);

	Vector3 rotatedOffset = TransformNormal(blendedLocalOffset, parentRotation);

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