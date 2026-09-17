#include "BaseEffect.h"

using namespace KamataEngine;

void BaseEffect::BaseInitialize(uint32_t textureHandle, float expandDuration, float fadeDuration) {
	assert(textureHandle);
	textureHandle_ = textureHandle;

	expandDuration_ = expandDuration;
	fadeDuration_ = fadeDuration;

	objectColor_.Initialize();
	color_ = {1.0f, 1.0f, 1.0f, 1.0f};
	objectColor_.SetColor(color_);

	status_ = Status::kExpand;
	duration_ = expandDuration_;
	counter_ = 0.0f;
	isFinished_ = false;
}

	// 拡大フェーズで毎フレーム呼ばれる（0.0〜1.0の進行度tを渡す）。スケール補間など派生固有の処理をここに書く。
void BaseEffect::OnExpandUpdate(float t) { t; }
// フェードアウトフェーズで毎フレーム呼ばれる（0.0〜1.0の進行度t）。基本は色のアルファのみで十分だが、追加演出があれば派生でオーバーライド。
void BaseEffect::OnFadeOutUpdate(float t) { t; }
// 状態に関わらず毎フレーム呼ばれる（回転演出など）
void BaseEffect::OnCommonUpdate() {}

void BaseEffect::Update() {
	if (isFinished_) {
		return;
	}

	switch (status_) {
	case Status::kExpand: {
		counter_ += 1.0f / 60.0f;
		if (counter_ >= duration_) {
			counter_ = duration_;
		}

		float t = std::clamp(counter_ / duration_, 0.0f, 1.0f);
		OnExpandUpdate(t);

		if (counter_ >= duration_) {
			status_ = Status::kFadeOut;
			duration_ = fadeDuration_;
			counter_ = 0.0f;
		}
		break;
	}
	case Status::kFadeOut: {
		counter_ += 1.0f / 60.0f;
		if (counter_ >= duration_) {
			counter_ = duration_;
		}

		float t = std::clamp(counter_ / duration_, 0.0f, 1.0f);
		color_.w = 1.0f - t;
		objectColor_.SetColor(color_);

		OnFadeOutUpdate(t);

		if (counter_ >= duration_) {
			isFinished_ = true;
		}
		break;
	}
	}

	OnCommonUpdate();
}