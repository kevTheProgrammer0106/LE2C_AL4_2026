#pragma once

/// Global officer buff. GameScene flips gActive; everything else only reads.
namespace OfficerBuff {
inline bool gActive = false;
inline float kDamageReduction = 0.2f; // 20% less damage taken
inline float kMoveBonus = 0.2f;       // 20% faster horizontal movement
inline float kChaseTimeBonus = 0.5f;  // chase timeout lasts +50% longer

// 2つの整数のうち大きい値を返す
inline int MaxInt(int a, int b) { return (a > b) ? a : b; }

// 浮動小数点数を四捨五入して整数に変換する
inline int RoundToInt(float value) { return (value >= 0.0f) ? static_cast<int>(value + 0.5f) : static_cast<int>(value - 0.5f); }

inline int ReduceDamage(int damage) {
	if (!gActive || damage <= 0)
		return damage;

	float reducedDamage = static_cast<float>(damage) * (1.0f - kDamageReduction);
	return MaxInt(1, RoundToInt(reducedDamage));
}
// suppressed = knocked back / stunned: launches are not sped up
inline float MoveScale(bool suppressed) { return (gActive && !suppressed) ? 1.0f + kMoveBonus : 1.0f; }
inline float ChaseTimeScale() { return gActive ? 1.0f + kChaseTimeBonus : 1.0f; }
} // namespace OfficerBuff