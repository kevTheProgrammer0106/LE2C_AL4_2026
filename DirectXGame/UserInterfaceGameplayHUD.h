#pragma once

#include "BaseSprite.h"
#include "Global.h"

class UserInterfaceGameplayHUD {
public:
	void Initialize();
	void Update();
	void Draw();
	void MainFrame();

private:
	KamataEngine::WorldTransform mainWorldTransform_;
	uint32_t borderSprite = 0;
	std::unique_ptr<KamataEngine::Sprite> spriteTransformBorder;
};

class InteractionPrompt final : public BaseSprite {
public:
	enum class Kind { kNone, kWeaponPickup, kSwitch };

	void Initialize(uint32_t iconTextureHandle);

	void Show(Kind kind);
	void Hide();

	void Update();
	void Draw();

	bool IsVisible() const { return isVisible_; }

private:
	bool isVisible_ = false;
	Kind kind_ = Kind::kNone;
	float bobTimer_ = 0.0f;

	static inline float kScreenAnchorX = 640.0f;
	static inline float kScreenAnchorY = 620.0f;
	static inline float kBobAmplitude = 3.0f;
	static inline float kBobSpeed = 3.0f;
};

// Player health bar. Two flat-color sprites (background + fill) driven by HP/MaxHP.
class PlayerHealthUI final {
public:
	void Initialize(uint32_t backgroundTextureHandle, uint32_t fillTextureHandle);
	void Update(int hp, int maxHp);
	void Draw();

	static float GetBarBottomY() { return kBarY + kBarHeight; }
	static float GetBarX() { return kBarX; }

private:
	std::unique_ptr<KamataEngine::Sprite> background_;
	std::unique_ptr<KamataEngine::Sprite> border_;
	std::unique_ptr<KamataEngine::Sprite> fill_;

	static inline float kBarX = 90.0f;
	static inline float kBarY = 24.0f;
	static inline float kBarWidth = 260.0f;
	static inline float kBarHeight = 26.0f;
	static inline float kBorderThickness = 3.0f;
};

// [NEW] Melee + firearm equipped-weapon icons, shown under the health bar.
// The active weapon mode's icon is highlighted. The firearm icon has a small
// bar underneath it that behaves like the HP bar: full while ready, drains/
// refills to show reload progress, and is tinted differently while reloading.
class WeaponStatusUI final {
public:
	void Initialize(uint32_t meleeIconTexture, uint32_t firearmIconTexture, uint32_t barBackgroundTexture, uint32_t barFillTexture);

	// isFirearmMode: which weapon is currently equipped (for highlighting).
	// isReloading: whether the firearm is currently reloading.
	// reloadProgress01: 0 = just started reloading, 1 = fully reloaded/ready.
	void Update(bool isFirearmMode, bool isReloading, float reloadProgress01);
	void Draw();

private:
	std::unique_ptr<KamataEngine::Sprite> meleeIcon_;
	std::unique_ptr<KamataEngine::Sprite> firearmIcon_;

	std::unique_ptr<KamataEngine::Sprite> ammoBarBorder_;
	std::unique_ptr<KamataEngine::Sprite> ammoBarBackground_;
	std::unique_ptr<KamataEngine::Sprite> ammoBarFill_;

	static inline float kIconSize = 40.0f;
	static inline float kIconSpacing = 12.0f;
	static inline float kTopOffsetFromHealthBar = 14.0f;

	static inline float kAmmoBarHeight = 8.0f;
	static inline float kAmmoBarBorderThickness = 2.0f;
	static inline float kAmmoBarTopOffset = 6.0f; // gap between firearm icon bottom and ammo bar

	static inline KamataEngine::Vector4 kActiveColor = {1.0f, 1.0f, 1.0f, 1.0f};
	static inline KamataEngine::Vector4 kInactiveColor = {0.45f, 0.45f, 0.45f, 0.75f};
};

class EquipmentStatus final : public BaseSprite {};
class PauseMenu final : public BaseSprite {};
class ScoreStatus final : public BaseSprite {};
class ObjectiveStatus final : BaseSprite {};
class EnemyStatus final : BaseSprite {};