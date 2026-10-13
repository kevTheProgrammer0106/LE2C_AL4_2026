#pragma once

#include "FighterBehavior.h"

class EnemyOfficer;

/// Fighter behavior plus a one-time "command" action when the player alerts the officer.
/// The buff lands partway through the animation, so an officer killed mid-gesture never grants it.
/// An officer buffs once per alert: the flag only resets after it loses the player, so a second officer
/// that is alerted later (or the same one after re-alerting) can buff again once the first buff is gone.
class OfficerBehavior final : public FighterBehavior {
public:
	explicit OfficerBehavior(EnemyOfficer& owner);

	bool IsEngaged() const override { return GetBehavior() == Behavior::kCommand || FighterBehavior::IsEngaged(); }

	void InitializeBehaviorState(Behavior behavior) override;
	void UpdateBehaviorState() override;

protected:
	void BehaviorRootUpdate() override;

private:
	void BehaviorCommandInitialize();
	void BehaviorCommandUpdate();

	EnemyOfficer& officerOwner_;

	bool alertHandled_ = false;
	float commandTimer_ = 0.0f;
	bool buffApplied_ = false;
};