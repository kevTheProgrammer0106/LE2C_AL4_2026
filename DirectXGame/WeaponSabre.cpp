#include "WeaponSabre.h"
#include "GlobalVariables.h"

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "MeleeSabre";
} // namespace

void WeaponSabre::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);

	gv->AddItem(kGroupName, "HalfWidthMin", WeaponSabre::kHalfWidthMin);
	gv->AddItem(kGroupName, "HalfWidthMax", WeaponSabre::kHalfWidthMax);
	gv->AddItem(kGroupName, "HalfHeightMin", WeaponSabre::kHalfHeightMin);
	gv->AddItem(kGroupName, "HalfHeightMax", WeaponSabre::kHalfHeightMax);
	gv->AddItem(kGroupName, "HalfDepthMin", WeaponSabre::kHalfDepthMin);
	gv->AddItem(kGroupName, "HalfDepthMax", WeaponSabre::kHalfDepthMax);

	gv->AddItem(kGroupName, "HolsterOffsetTranslation", holsterOffset_.translation);
	gv->AddItem(kGroupName, "HolsterOffsetRotation", holsterOffset_.rotation);
}

void WeaponSabre::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();

	kHalfWidthMin = gv->GetFloatValue(kGroupName, "HalfWidthMin");
	kHalfWidthMax = gv->GetFloatValue(kGroupName, "HalfWidthMax");
	kHalfHeightMin = gv->GetFloatValue(kGroupName, "HalfHeightMin");
	kHalfHeightMax = gv->GetFloatValue(kGroupName, "HalfHeightMax");
	kHalfDepthMin = gv->GetFloatValue(kGroupName, "HalfDepthMin");
	kHalfDepthMax = gv->GetFloatValue(kGroupName, "HalfDepthMax");

	holsterOffset_.translation = gv->GetVector3Value(kGroupName, "HolsterOffsetTranslation");
	holsterOffset_.rotation = gv->GetVector3Value(kGroupName, "HolsterOffsetRotation");
}

AABB WeaponSabre::GetAABB() const {
	Vector3 pos = worldTransform_.translation_;
	AABB aabb;
	aabb.min = {pos.x - kHalfWidthMin / 2.0f, pos.y - kHalfHeightMin / 2.0f, pos.z - kHalfDepthMin / 2.0f};
	aabb.max = {pos.x + kHalfWidthMax / 2.0f, pos.y + kHalfHeightMax / 2.0f, pos.z + kHalfDepthMax / 2.0f};
	return aabb;
}
