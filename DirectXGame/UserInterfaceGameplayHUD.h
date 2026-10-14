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
	static inline float kScreenAnchorY = 400.0f;
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

// Block gauge: bottom-center, enlarged. Blue normally; red while "broken", with a marker at the recover threshold.
class PlayerBlockUI final {
public:
	void Initialize(uint32_t backgroundTextureHandle, uint32_t fillTextureHandle);
	void Update(float gauge, float maxGauge, bool broken, float recoverThreshold);
	void Draw();

private:
	std::unique_ptr<KamataEngine::Sprite> border_;
	std::unique_ptr<KamataEngine::Sprite> background_;
	std::unique_ptr<KamataEngine::Sprite> fill_;
	std::unique_ptr<KamataEngine::Sprite> marker_;
	bool broken_ = false;
	float barX_ = 0.0f;
	float barY_ = 0.0f;

	static inline float kBarWidth = 520.0f;
	static inline float kBarHeight = 34.0f;
	static inline float kBorderThickness = 4.0f;
	static inline float kBottomMargin = 56.0f;
};

// Melee + firearm icons under the health bar. No ammo bar:
// after firing, the firearm icon goes dark, then brightens from the bottom up as it reloads.
// When ready, it flashes white and settles back to its normal colour.
class WeaponStatusUI final {
public:
	struct Textures {
		uint32_t meleeLight = 0;
		uint32_t meleeHeavy = 0;
		uint32_t firearmLong = 0;
		uint32_t firearmBayonet = 0;
		uint32_t bayonetLunge = 0;
		uint32_t meleeAxe = 0;
		uint32_t meleeSpear = 0;
		 uint32_t firearmBlunderbuss = 0;
		 uint32_t firearmDualPistols = 0;
	};
	struct State {
		bool isFirearmMode = false;
		DropWeaponKind meleeKind = DropWeaponKind::kSabreLight;
		DropWeaponKind firearmKind = DropWeaponKind::kMusketLong;
		bool isMeleeCooling = false; // heavy sabre lunge cooldown
		float meleeCooldownProgress01 = 1.0f;
		bool isReloading = false;
		float reloadProgress01 = 1.0f;
		bool isLungeCooling = false; // bayonet lunge cooldown (own icon)
		float lungeCooldownProgress01 = 1.0f;
	};

	void Initialize(const Textures& textures);
	void Update(const State& state);
	void Draw();

private:
	static inline float kIconSize = 64.0f;
	static inline float kIconSpacing = 12.0f;
	static inline float kTopOffsetFromHealthBar = 14.0f;
	static inline float kDarkFactor = 0.25f;
	static inline float kFlashDuration = 0.45f;
	static inline float kFlashBoost = 2.5f;
	static inline float kPopDuration = 0.25f; // replenish "pop" length (s)
	static inline float kPopScale = 0.25f;    // peak enlargement (+25%)

	static inline KamataEngine::Vector4 kActiveColor = {1.0f, 1.0f, 1.0f, 1.0f};
	static inline KamataEngine::Vector4 kInactiveColor = {0.45f, 0.45f, 0.45f, 0.75f};

	struct CooldownIcon {
		std::unique_ptr<KamataEngine::Sprite> dark;   // full icon, darkened
		std::unique_ptr<KamataEngine::Sprite> bright; // cropped from the bottom by progress
		KamataEngine::Vector2 pos{};
		KamataEngine::Vector2 texSize{1.0f, 1.0f};
		bool wasCooling = false;
		float flashTimer = 0.0f;
		float popTimer = 0.0f;
		bool darkVisible = false;
		bool brightVisible = true;

		void Initialize(uint32_t tex, const KamataEngine::Vector2& position);
		void Update(const KamataEngine::Vector4& baseColor, bool cooling, float progress01);
		void Draw();
	};

	CooldownIcon meleeLight_;
	CooldownIcon meleeHeavy_;
	CooldownIcon meleeAxe_;
	CooldownIcon meleeSpear_;

	CooldownIcon firearmLong_;
	CooldownIcon firearmBayonet_;
	CooldownIcon firearmShotgun_;
	CooldownIcon firearmPistols_;
	
	CooldownIcon bayonetLunge_;
	DropWeaponKind shownMelee_ = DropWeaponKind::kSabreLight;
	DropWeaponKind shownFirearm_ = DropWeaponKind::kMusketLong;
};

class EquipmentStatus final : public BaseSprite {};
class PauseMenu final : public BaseSprite {};
class ScoreStatus final : public BaseSprite {};
class ObjectiveStatus final : BaseSprite {};
class EnemyStatus final : BaseSprite {};