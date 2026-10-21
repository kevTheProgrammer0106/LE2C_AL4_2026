#pragma once

#define NOMINMAX

#include "Global.h"
#include <KamataEngine.h>
#include <array>
#include <memory>

class Player;

// Tutorial chip 'U': the SUB ID (2nd char) selects the texture, in this order. e.g. "U0" = left, "U3" = attack.
enum class TutorialTextureType : uint8_t { kLeft = 0, kRight, kBottom, kAttack, kThrow, kLeftArrow, kCount };

/// <summary>
/// Tutorial block placed from the map ('U' chip). It has its own 3D model.
/// When the player gets close, a control hint sprite fades in above the player's head; it fades out when they leave.
/// </summary>
class SpriteTutorial {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	// Loads the shared hint textures once (safe to call repeatedly).
	static void LoadSharedTextures();

	void Initialize(KamataEngine::Model* blockModel, uint32_t blockTexture, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, TutorialTextureType type);
	void Update(Player* player);
	void DrawBlock();  // 3D block model (world pass)
	void DrawSprite(); // hint sprite (UI pass)

	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

private:
	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::Model* model_ = nullptr;
	uint32_t blockTexture_ = 0;
	KamataEngine::WorldTransform worldTransform_;

	std::unique_ptr<KamataEngine::Sprite> sprite_;
	TutorialTextureType type_ = TutorialTextureType::kLeft;
	float currentAlpha_ = 0.0f;

	// Tunables (GlobalVariables group "SpriteTutorial")
	static inline float kFadeInDistance = 4.0f;  // inside this: fade in
	static inline float kFadeOutDistance = 6.0f; // beyond this: fade out (between the two: keep current alpha)
	static inline float kHoverOffsetY = 1.4f;    // world Y offset above the player
	static inline float kFadeSpeed = 0.05f;      // alpha change per frame
	static inline float kSpriteWidth = 137.0f;   // px (source art is 1370x760, 1/10)
	static inline float kSpriteHeight = 76.0f;

	// Shared by every instance
	static inline std::array<uint32_t, static_cast<size_t>(TutorialTextureType::kCount)> sTextures_{};
	static inline bool sTexturesLoaded_ = false;
};