#pragma once

#define NOMINMAX

#include "Global.h"
#include <KamataEngine.h>
#include <memory>

/// <summary>
/// 画面上部中央に出るバナー文字(2048x512の画像)。上から落ちてきて(イージング)、一定時間表示し、上へ戻る。
/// 同時に出るのは1枚だけ。別のバナーを出すと、今の位置から新しいバナーへ切り替わる。
/// holdSeconds < 0 の時は Hide() が呼ばれるまで出しっぱなし(ポーズ/敗北/勝利用)。
/// </summary>
class BannerUI {
public:
	enum class Kind { kStart, kArena, kPaused, kDefeat, kVictory, kOfficer, kCount };

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	// whiteTexture: 画像が無い(ハンドル0)バナーの代用に使う白テクスチャ
	// textures: Kindの順(kStart, kArena, kPaused, kDefeat, kVictory)に kCount 個
	void Initialize(uint32_t whiteTexture, const uint32_t* textures);

	// startDelay: 出し始めるまでの待ち時間(秒)
	void Show(Kind kind, float holdSeconds, float startDelay = 0.0f);
	void Hide();

	void Update();
	void Draw();

	// 指定のバナーが出ている(または出る予定)か
	bool IsShowing(Kind kind) const { return state_ != State::kIdle && state_ != State::kRising && kind_ == kind; }

private:
	enum class State { kIdle, kWaiting, kDropping, kHolding, kRising };

	float GetBannerWidth() const;
	float GetBannerHeight() const { return GetBannerWidth() * (512.0f / 2048.0f); }
	float GetHiddenY() const { return -GetBannerHeight() - 8.0f; }

	std::unique_ptr<KamataEngine::Sprite> sprite_;
	uint32_t whiteTex_ = 0;
	uint32_t textures_[static_cast<size_t>(Kind::kCount)] = {};

	State state_ = State::kIdle;
	Kind kind_ = Kind::kStart;
	float timer_ = 0.0f;
	float holdSeconds_ = 0.0f;
	float startDelay_ = 0.0f;
	float fromY_ = 0.0f;
	float currentY_ = 0.0f;

	// ---- 調整値(GlobalVariables "BannerUI") ----
	static inline float kWidthRatio = 1.0f / 3.0f; // 画面幅に対する横幅
	static inline float kTopMargin = 20.0f;        // 表示位置の上余白(px)
	static inline float kDropDuration = 0.6f;      // 落ちてくる時間(秒)
	static inline float kRiseDuration = 0.35f;     // 戻る時間(秒)
};