#pragma once

#include "FighterBehavior.h"

class EnemyOfficer;

/// Officer: commands from the back.
///  - Buff: only when it actually SEES the player (not from spawn awareness), once per sighting.
///  - Player close + officer has escorts: stays behind the closest escort and oversees (no attacks).
///  - Player close + no escorts left: fights with the spear like a Fighter.
///  - Player far: chases (escorts keep close, see BaseEnemyBehavior::ShouldHoldForEscort).
///  - Dodges grenade blasts, then waits kGrenadeDodgeCooldown before it will dodge again.
class OfficerBehavior final : public FighterBehavior {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	explicit OfficerBehavior(EnemyOfficer& owner);

	bool IsEngaged() const override { return GetBehavior() == Behavior::kCommand || FighterBehavior::IsEngaged(); }

	void InitializeBehaviorState(Behavior behavior) override;
	void UpdateBehaviorState() override;

protected:
	void BehaviorRootUpdate() override;

	float GetMeleeEngageRange() const override { return kSpearEngageRange; }

private:
	void BehaviorCommandInitialize();
	void BehaviorCommandUpdate();

	// Closest living non-officer ally (to the player) within kEscortCountRadius of this officer.
	bool FindFrontEscort(const KamataEngine::Vector3& playerPos, KamataEngine::Vector3& outPos) const;
	// Stand behind the front escort, facing the fight.
	void OverseeFight(const KamataEngine::Vector3& playerPos, const KamataEngine::Vector3& frontEscortPos);

	EnemyOfficer& officerOwner_;

	bool alertHandled_ = false;
	float commandTimer_ = 0.0f;
	bool buffApplied_ = false;

	bool dodgingGrenade_ = false;
	float grenadeDodgeCooldown_ = 0.0f;

	static inline float kSpearEngageRange = 4.0f;   // spear reach is 4.5
	static inline float kCommandRange = 25.0f;      // must see the player within this to buff
	static inline float kOverseeRange = 12.0f;      // player this close: hang back behind escorts
	static inline float kEscortCountRadius = 12.0f; // allies this close count as escorts
	static inline float kOverseeBehindGap = 2.5f;   // how much farther from the player than the front escort
	static inline float kOverseeRetreatSpeed = 0.05f;
	static inline float kGrenadeRetreatSpeed = 0.12f;
	static inline float kGrenadeDodgeCooldown = 10.0f;
};