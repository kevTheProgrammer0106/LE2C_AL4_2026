#define NOMINMAX
#include "BannerUI.h"
#include "GlobalVariables.h"

#include <algorithm>

using namespace KamataEngine;

namespace {
constexpr const char* kGroupName = "BannerUI";
constexpr float kDt = 1.0f / 60.0f;

// 画像が無い時の代用色(レイアウト確認用)
constexpr float kPlaceholder[6][4] = {
    {0.3f, 0.8f, 0.4f, 0.8f}, // start
    {1.0f, 0.5f, 0.2f, 0.8f}, // arena
    {0.5f, 0.6f, 1.0f, 0.8f}, // paused
    {0.9f, 0.2f, 0.2f, 0.8f}, // defeat
    {1.0f, 0.9f, 0.2f, 0.8f}, // victory
    {0.3f, 0.9f, 0.5f, 0.8f}, // officer (バフの緑)
};
} // namespace

void BannerUI::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "WidthRatio", kWidthRatio);
	gv->AddItem(kGroupName, "TopMargin", kTopMargin);
	gv->AddItem(kGroupName, "DropDuration", kDropDuration);
	gv->AddItem(kGroupName, "RiseDuration", kRiseDuration);
}

void BannerUI::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kWidthRatio = gv->GetFloatValue(kGroupName, "WidthRatio");
	kTopMargin = gv->GetFloatValue(kGroupName, "TopMargin");
	kDropDuration = std::max(gv->GetFloatValue(kGroupName, "DropDuration"), 0.01f);
	kRiseDuration = std::max(gv->GetFloatValue(kGroupName, "RiseDuration"), 0.01f);
}

float BannerUI::GetBannerWidth() const { return static_cast<float>(WinApp::kWindowWidth) * kWidthRatio; }

void BannerUI::Initialize(uint32_t whiteTexture, const uint32_t* textures) {
	whiteTex_ = whiteTexture;
	for (size_t i = 0; i < static_cast<size_t>(Kind::kCount); ++i) {
		textures_[i] = textures ? textures[i] : 0;
	}
	sprite_.reset(Sprite::Create(whiteTex_, {0.0f, 0.0f}));
	state_ = State::kIdle;
	currentY_ = GetHiddenY();
}

void BannerUI::Show(Kind kind, float holdSeconds, float startDelay) {
	// 同じバナーが既に出ている最中なら、表示時間だけ更新する
	if (IsShowing(kind) && (state_ == State::kDropping || state_ == State::kHolding)) {
		holdSeconds_ = holdSeconds;
		if (state_ == State::kHolding) {
			timer_ = 0.0f;
		}
		return;
	}

	const bool visible = (state_ == State::kDropping || state_ == State::kHolding || state_ == State::kRising);
	kind_ = kind;
	holdSeconds_ = holdSeconds;
	startDelay_ = std::max(startDelay, 0.0f);
	timer_ = 0.0f;
	fromY_ = visible ? currentY_ : GetHiddenY(); // 出ている最中なら今の位置から落とし直す
	state_ = (startDelay_ > 0.0f) ? State::kWaiting : State::kDropping;

	const size_t index = static_cast<size_t>(kind_);
	const uint32_t tex = textures_[index] != 0 ? textures_[index] : whiteTex_;
	sprite_->SetTextureHandle(tex);
	if (textures_[index] != 0) {
		sprite_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	} else {
		sprite_->SetColor({kPlaceholder[index][0], kPlaceholder[index][1], kPlaceholder[index][2], kPlaceholder[index][3]});
	}
}

void BannerUI::Hide() {
	if (state_ == State::kIdle || state_ == State::kRising) {
		return;
	}
	if (state_ == State::kWaiting) {
		state_ = State::kIdle; // まだ出ていない
		return;
	}
	state_ = State::kRising;
	timer_ = 0.0f;
	fromY_ = currentY_;
}

void BannerUI::Update() {
	const float restY = kTopMargin;
	const float hiddenY = GetHiddenY();

	switch (state_) {
	case State::kIdle:
		currentY_ = hiddenY;
		break;
	case State::kWaiting:
		timer_ += kDt;
		if (timer_ >= startDelay_) {
			state_ = State::kDropping;
			timer_ = 0.0f;
		}
		currentY_ = fromY_;
		break;
	case State::kDropping: {
		timer_ += kDt;
		const float t = std::clamp(timer_ / kDropDuration, 0.0f, 1.0f);
		currentY_ = Lerp(fromY_, restY, EaseOut(0.0f, 1.0f, t)); // 減速しながら落ちる
		if (t >= 1.0f) {
			state_ = State::kHolding;
			timer_ = 0.0f;
		}
		break;
	}
	case State::kHolding:
		currentY_ = restY;
		if (holdSeconds_ >= 0.0f) {
			timer_ += kDt;
			if (timer_ >= holdSeconds_) {
				Hide();
			}
		}
		break;
	case State::kRising: {
		timer_ += kDt;
		const float t = std::clamp(timer_ / kRiseDuration, 0.0f, 1.0f);
		currentY_ = Lerp(fromY_, hiddenY, EaseIn(0.0f, 1.0f, t)); // 加速しながら戻る
		if (t >= 1.0f) {
			state_ = State::kIdle;
		}
		break;
	}
	}

	const float w = GetBannerWidth();
	sprite_->SetSize({w, GetBannerHeight()});
	sprite_->SetPosition({(static_cast<float>(WinApp::kWindowWidth) - w) * 0.5f, currentY_});
}

void BannerUI::Draw() {
	if (state_ == State::kIdle || state_ == State::kWaiting) {
		return;
	}
	Sprite::PreDraw();
	sprite_->Draw();
	Sprite::PostDraw();
}