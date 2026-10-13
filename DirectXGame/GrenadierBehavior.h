#pragma once

#include "FighterBehavior.h"

class EnemyGrenadier;

/// Fighter's melee kit, but only used at close range. Otherwise it holds the throwing band and lobs grenades.
///   within kMeleeEngageRange        -> Fighter melee
///   closer than kThrowRangeMin      -> backs away to the throwing band (also right after a melee)
///   kThrowRangeMin..kThrowRangeMax  -> stands and throws every kThrowInterval
///   beyond kThrowRangeMax           -> normal pathfinding chase
class GrenadierBehavior final : public FighterBehavior {
public:
	explicit GrenadierBehavior(EnemyGrenadier& owner);

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

protected:
	void BehaviorRootUpdate() override;

private:
	EnemyGrenadier& grenadierOwner_;

	float throwTimer_ = 0.8f; // short wind-up before the first throw

	static inline float kThrowInterval = 3.5f;
	static inline float kThrowRangeMin = 4.0f;
	static inline float kThrowRangeMax = 12.0f;
	static inline float kThrowHeightTolerance = 3.0f; // the arc is flat, so a different floor means chase instead
	static inline float kRetreatSpeed = 0.07f;
};