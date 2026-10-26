#pragma once

#define NOMINMAX

#include "Global.h"
#include <KamataEngine.h>
#include <array>
#include <memory>
#include <cstdint>

class Player;

// Tutorial chip 'U' (3 chars: type, subID, linkID). Index = subID + linkID * 10, because a chip digit only goes 0-9.
//   U0..U9 = index 0..9,  "U01" = 10,  "U11" = 11,  "U21" = 12
// Order: move, jump, platform, checkpoint, attack, block, blockExplain, pickUp, weaponSwitch, attackSpecial, pickUpExplain, gate, objective.
enum class TutorialTextureType : uint8_t {
	kMove = 0,      // U0
	kJump,          // U1
	kPlatform,      // U2
	kCheckpoint,    // U3
	kAttack,        // U4
	kBlock,         // U5
	kBlockExplain,  // U6
	kPickUp,        // U7
	kWeaponSwitch,  // U8
	kAttackSpecial, // U9
	kPickUpExplain, // 10  ("U01")
	kGate,          // 11  ("U11")
	kObjective,     // 12  ("U21")
	kCount
};

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
	TutorialTextureType type_ = TutorialTextureType::kMove;
	float currentAlpha_ = 0.0f;

	// Tunables (GlobalVariables group "SpriteTutorial")
	static inline float kFadeInDistance = 4.0f;  // inside this: fade in
	static inline float kFadeOutDistance = 6.0f; // beyond this: fade out (between the two: keep current alpha)
	static inline float kHoverOffsetY = 1.4f;    // world Y offset above the player
	static inline float kFadeSpeed = 0.05f;      // alpha change per frame
	static inline float kSpriteHeight = 84.6f;
	static inline float kSpriteWidth = 341.3f; // px (source art is 1370x760, 1/10)

	// Shared by every instance
	static inline std::array<uint32_t, static_cast<size_t>(TutorialTextureType::kCount)> sTextures_{};
	static inline bool sTexturesLoaded_ = false;
};