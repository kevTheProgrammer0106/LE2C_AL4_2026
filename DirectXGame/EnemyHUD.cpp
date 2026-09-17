#include "EnemyHUD.h"
#include "BaseEnemy.h"
#include "GlobalVariables.h"

#include <algorithm>
#include <cmath>
#include <limits>

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "EnemyHUD";
}

void EnemyHUD::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "EnemyCountThreshold", kEnemyCountThreshold);
	gv->AddItem(kGroupName, "IconSize", kIconSize);
	gv->AddItem(kGroupName, "ArrowSize", kArrowSize);
	gv->AddItem(kGroupName, "HeadOffsetY", kHeadOffsetY);
	gv->AddItem(kGroupName, "ScreenEdgeMargin", kScreenEdgeMargin);
}

void EnemyHUD::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kEnemyCountThreshold = gv->GetIntValue(kGroupName, "EnemyCountThreshold");
	kIconSize = gv->GetIntValue(kGroupName, "IconSize");
	kArrowSize = gv->GetIntValue(kGroupName, "ArrowSize");
	kHeadOffsetY = gv->GetFloatValue(kGroupName, "HeadOffsetY");
	kScreenEdgeMargin = gv->GetFloatValue(kGroupName, "ScreenEdgeMargin");
}

void EnemyHUD::Initialize(uint32_t arrowTextureHandle, uint32_t fallbackIconTextureHandle) {
	arrowTextureHandle_ = arrowTextureHandle;
	fallbackIconTextureHandle_ = fallbackIconTextureHandle;
}

EnemyHUD::IndicatorVisual& EnemyHUD::AcquireVisual(size_t index, uint32_t iconTexture) {
	if (index >= visuals_.size()) {
		visuals_.resize(index + 1);
	}
	IndicatorVisual& visual = visuals_[index];
	if (!visual.icon) {
		visual.icon.reset(Sprite::Create(iconTexture, {0.0f, 0.0f}));
		visual.icon->SetSize({static_cast<float>(kIconSize), static_cast<float>(kIconSize)});
		visual.icon->SetAnchorPoint({0.5f, 0.5f});
	}
	if (!visual.arrow) {
		visual.arrow.reset(Sprite::Create(arrowTextureHandle_, {0.0f, 0.0f}));
		visual.arrow->SetSize({static_cast<float>(kArrowSize), static_cast<float>(kArrowSize)});
		visual.arrow->SetAnchorPoint({0.5f, 0.5f});
	}
	visual.inUse = true;
	return visual;
}

void EnemyHUD::ReleaseUnusedVisuals(size_t activeCount) {
	for (size_t i = activeCount; i < visuals_.size(); ++i) {
		visuals_[i].inUse = false;
	}
}

void EnemyHUD::Update(const std::vector<BaseEnemy*>& arenaEnemies, const Camera& camera, int screenWidth, int screenHeight) {
	std::vector<BaseEnemy*> alive;
	alive.reserve(arenaEnemies.size());
	for (BaseEnemy* enemy : arenaEnemies) {
		if (enemy && !enemy->IsDead()) {
			alive.push_back(enemy);
		}
	}

	bool showIndicators = static_cast<int>(alive.size()) < kEnemyCountThreshold && !alive.empty();
	isActive_ = showIndicators;

	if (!showIndicators) {
		ReleaseUnusedVisuals(0);
		return;
	}

	Matrix4x4 matViewport = MakeViewportMatrix(0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0.0f, 1.0f);
	Matrix4x4 matVPV = Multiply(camera.matView, Multiply(camera.matProjection, matViewport));

	float halfW = static_cast<float>(screenWidth) * 0.5f;
	float halfH = static_cast<float>(screenHeight) * 0.5f;
	float edge = kScreenEdgeMargin;

	for (size_t i = 0; i < alive.size(); ++i) {
		BaseEnemy* enemy = alive[i];
		uint32_t iconTexture = enemy->GetHUDIconTexture() != 0 ? enemy->GetHUDIconTexture() : fallbackIconTextureHandle_;

		IndicatorVisual& visual = AcquireVisual(i, iconTexture);
		visual.icon->SetTextureHandle(iconTexture);

		Vector3 headWorldPos = enemy->GetWorldPosition() + Vector3(0.0f, kHeadOffsetY, 0.0f);
		Vector3 screenPos = TransformCoordinate(headWorldPos, matVPV);

		bool onScreen = screenPos.x >= 0.0f && screenPos.x <= static_cast<float>(screenWidth) && screenPos.y >= 0.0f && screenPos.y <= static_cast<float>(screenHeight);

		bool spotted = enemy->IsPlayerSpotted();
		Vector4 iconColor = spotted ? Vector4(1.0f, 0.35f, 0.35f, 1.0f) : Vector4(1.0f, 1.0f, 1.0f, 1.0f);

		if (onScreen) {
			visual.arrow->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
			visual.icon->SetPosition({screenPos.x, screenPos.y});
			visual.icon->SetRotation(0.0f);
			visual.icon->SetColor(iconColor);
		} else {
			Vector2 dir = {screenPos.x - halfW, screenPos.y - halfH};
			float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
			if (len < 0.0001f) {
				dir = {1.0f, 0.0f};
				len = 1.0f;
			}
			dir.x /= len;
			dir.y /= len;

			float scaleX = (dir.x != 0.0f) ? (halfW - edge) / std::abs(dir.x) : std::numeric_limits<float>::max();
			float scaleY = (dir.y != 0.0f) ? (halfH - edge) / std::abs(dir.y) : std::numeric_limits<float>::max();
			float scale = std::min(scaleX, scaleY);

			Vector2 clampedPos = {halfW + dir.x * scale, halfH + dir.y * scale};
			float angle = std::atan2(dir.y, dir.x);

			visual.arrow->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
			visual.arrow->SetPosition(clampedPos);
			visual.arrow->SetRotation(angle);

			visual.icon->SetPosition({clampedPos.x, clampedPos.y});
			visual.icon->SetRotation(0.0f);
			visual.icon->SetColor(iconColor);
		}
	}

	ReleaseUnusedVisuals(alive.size());
}

void EnemyHUD::Draw() {
	if (!isActive_) {
		return;
	}
	Sprite::PreDraw();
	for (IndicatorVisual& visual : visuals_) {
		if (!visual.inUse) {
			continue;
		}
		if (visual.arrow) {
			visual.arrow->Draw();
		}
		if (visual.icon) {
			visual.icon->Draw();
		}
	}
	Sprite::PostDraw();
}