#include "AnimationUI.h"

using namespace KamataEngine;

void AnimationUI::Play(Type type, float duration, float intensity, bool loop) {
	type_ = type;
	duration_ = std::max(duration, 0.0001f);
	intensity_ = intensity;
	loop_ = loop;
	timer_ = 0.0f;
	isPlaying_ = true;
	currentOffset_ = SpriteOffsetUI{};
}

void AnimationUI::Stop() {
	isPlaying_ = false;
	currentOffset_ = SpriteOffsetUI{};
}

void AnimationUI::Update(float deltaTime) {
	if (!isPlaying_) {
		return;
	}

	timer_ += deltaTime;
	if (timer_ >= duration_) {
		if (loop_) {
			timer_ = std::fmod(timer_, duration_);
		} else {
			timer_ = duration_;
			isPlaying_ = false;
		}
	}

	RecomputeOffset();

	if (!isPlaying_) {
		// 単発アニメーション終了時はニュートラルへ戻す(kFadeOutだけはアルファ0のまま残す)
		if (type_ != Type::kFadeOut) {
			currentOffset_ = SpriteOffsetUI{};
		} else {
			currentOffset_.alpha = 0.0f;
		}
	}
}

void AnimationUI::RecomputeOffset() {
	float t = std::clamp(timer_ / duration_, 0.0f, 1.0f);

	switch (type_) {
	case Type::kShake: {
		float decay = loop_ ? 1.0f : (1.0f - t);
		currentOffset_.translation.x = std::sin(timer_ * 55.0f) * intensity_ * decay;
		currentOffset_.translation.y = std::cos(timer_ * 47.0f) * intensity_ * decay * 0.6f;
		break;
	}
	case Type::kPulse: {
		float wave = TriangleWave(timer_, 6.0f); // 0.0〜1.0を往復
		float scaleAmount = 1.0f + (wave * (intensity_ - 1.0f));
		currentOffset_.scale = {scaleAmount, scaleAmount};
		break;
	}
	case Type::kFadeIn: {
		currentOffset_.alpha = EaseOut(0.0f, 1.0f, t);
		break;
	}
	case Type::kFadeOut: {
		currentOffset_.alpha = EaseIn(1.0f, 0.0f, t);
		break;
	}
	case Type::kBlink: {
		float wave = TriangleWave(timer_, 8.0f);
		currentOffset_.alpha = (wave > 0.5f) ? 1.0f : 0.25f;
		break;
	}
	case Type::kSlideIn: {
		float te = EaseOut(0.0f, 1.0f, t);
		currentOffset_.translation.x = slideFromDir_.x * intensity_ * (1.0f - te);
		currentOffset_.translation.y = slideFromDir_.y * intensity_ * (1.0f - te);
		currentOffset_.alpha = te;
		break;
	}
	case Type::kWiggle: {
		float wave = std::sin(timer_ * 10.0f);
		currentOffset_.rotation = wave * (intensity_ * (std::numbers::pi_v<float> / 180.0f));
		break;
	}
	case Type::kBounce: {
		float wave = std::sin(t * std::numbers::pi_v<float>);
		currentOffset_.translation.y = -wave * intensity_;
		break;
	}
	case Type::kNone:
	default:
		currentOffset_ = SpriteOffsetUI{};
		break;
	}
}