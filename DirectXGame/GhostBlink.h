#pragma once

#include <algorithm>
#include <cmath>
#include <numbers>

/// Shared "about to respawn" visual: eased pulse, then a smooth solidify with a small scale pop.
/// Used by HealthBlock, regenerating explosives and regenerating fragile blocks.
namespace GhostBlink {

struct Visual {
	float alpha = 0.0f; // 0 = don't draw
	float scale = 1.0f; // multiplier on the base scale
};

inline float Smooth(float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	return t * t * (3.0f - 2.0f * t);
}

// remaining: seconds until respawn. The pulse starts at alpha 0 (fades in), eases up and down at `frequency`
// pulses/second, and in the last materializeDuration seconds eases to a fully solid block.
inline Visual Evaluate(float remaining, float warningDuration, float frequency, float materializeDuration, float visibleAlpha = 0.45f) {
	Visual v;
	if (remaining > warningDuration)
		return v;
	remaining = std::max(remaining, 0.0f);

	const float elapsed = warningDuration - remaining;
	const float wave = 0.5f - 0.5f * std::cos(elapsed * frequency * std::numbers::pi_v<float>);
	const float pulse = visibleAlpha * Smooth(wave);

	const float mat = std::clamp(materializeDuration, 0.0001f, warningDuration);
	const float m = (remaining < mat) ? Smooth(1.0f - remaining / mat) : 0.0f;

	v.alpha = pulse + (1.0f - pulse) * m;
	v.scale = 1.0f + 0.12f * std::sin(std::numbers::pi_v<float> * m); // pop up, settle to 1.0 exactly when the real block appears
	return v;
}



// remaining: seconds until respawn. Returns the ghost's alpha this frame (0 = don't draw).
inline float Alpha(float remaining, float warningDuration, float frequency, float visibleAlpha = 0.45f) {
	if (remaining > warningDuration)
		return 0.0f;
	remaining = remaining < 0.0f ? 0.0f : remaining;
	const int phase = static_cast<int>((warningDuration - remaining) * frequency) % 2;
	return (phase == 0) ? visibleAlpha : 0.0f;
}
} // namespace GhostBlink