#pragma once

#include "BaseSprite.h"
#include "Global.h"
#include <memory>
#include <vector>

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

/// Arena "boss" bar, top-right. Shows the combined HP of everything alive in the active arena
/// PLUS the HP of every enemy that is still going to spawn (GameScene computes that number).
/// - Fades in when an arena starts and fades out when it ends (alpha eases, never pops).
/// - The red fill eases down to the new value; a pale "chip" bar trails behind it so damage reads.
/// - When hidden, Update() ignores current/max so the bar empties and fades with its last values.
class BossHealthBar final {
public:
	void Initialize(uint32_t backgroundTextureHandle, uint32_t fillTextureHandle);
	// currentHP / maxHP: whole-arena totals. visible: an arena fight is active right now.
	void Update(float currentHP, float maxHP, bool visible);
	void Draw();

	bool IsVisible() const { return alpha_ > 0.001f; }

private:
	std::unique_ptr<KamataEngine::Sprite> border_;
	std::unique_ptr<KamataEngine::Sprite> background_;
	std::unique_ptr<KamataEngine::Sprite> chip_;
	std::unique_ptr<KamataEngine::Sprite> fill_;

	float barX_ = 0.0f;
	float alpha_ = 0.0f;      // overall fade
	float targetFill_ = 1.0f; // real value 0..1
	float shownFill_ = 1.0f;  // eased toward targetFill_
	float chipFill_ = 1.0f;   // trails behind shownFill_

	static inline float kBarWidth = 420.0f;
	static inline float kBarHeight = 22.0f;
	static inline float kBorderThickness = 3.0f;
	static inline float kMarginRight = 40.0f;
	static inline float kBarY = 24.0f;
	static inline float kFadeRate = 0.12f;          // per-frame ease toward the target alpha
	static inline float kFillRate = 0.15f;          // per-frame ease of the red fill
	static inline float kChipDropPerSecond = 0.35f; // how fast the chip bar catches down (fraction of the bar per second)
};

/// Points at the gate(s) a SwitchBlock just opened.
/// - Gate on screen: an arrow floats above it, pointing down at it.
/// - Gate off screen: an arrow sits at the MIDPOINT between the switch and the gate (clamped inside the
///   screen) and points toward the gate.
/// - Fades in, holds, then fades out and removes itself after kDuration seconds.
class SwitchPointer final {
public:
	// arrowTextureHandle: art that points RIGHT at rotation 0 (same convention as the enemy direction arrow)
	void Initialize(uint32_t arrowTextureHandle);
	void Start(const KamataEngine::Vector3& switchWorldPos, const std::vector<KamataEngine::Vector3>& gateWorldPositions);
	void Update(const KamataEngine::Camera& camera);
	void Draw();

	bool IsActive() const { return active_; }

private:
	struct Target {
		KamataEngine::Vector3 gatePos{};
		std::unique_ptr<KamataEngine::Sprite> sprite;
	};

	uint32_t arrowTexture_ = 0;
	std::vector<Target> targets_;
	KamataEngine::Vector3 switchPos_{};
	float timer_ = 0.0f;
	bool active_ = false;

	static inline float kDuration = 3.0f; // total on-screen seconds, including both fades
	static inline float kFadeIn = 0.25f;
	static inline float kFadeOut = 0.5f;
	static inline float kIconSize = 44.0f;
	static inline float kAboveGatePx = 64.0f; // how far above an on-screen gate the arrow floats
	static inline float kBobAmplitude = 6.0f;
	static inline float kBobSpeed = 8.0f;
	static inline float kEdgeMargin = 48.0f;
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