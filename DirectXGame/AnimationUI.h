#pragma once

#define NOMINMAX

#include "Global.h"
#include <KamataEngine.h>

/// <summary>
/// Sprite(2D UI)向けの汎用アニメーションユーティリティ。
/// AnimationManagerが3Dキャラの見た目(SRTOffset)を扱うのと同じ考え方で、
/// こちらはSprite向けのオフセット(スケール・回転・平行移動・アルファ)を計算する。
/// ボタンのクリック演出、HUDアイコンの被発見シェイク等、任意のSpriteに使い回せる。
/// </summary>
struct SpriteOffsetUI {
	KamataEngine::Vector2 scale{1.0f, 1.0f};
	float rotation = 0.0f;
	KamataEngine::Vector2 translation{0.0f, 0.0f};
	float alpha = 1.0f;
};

class AnimationUI {
public:
	enum class Type {
		kNone,
		kShake,   // 短時間、細かく揺れる(被弾・警告等)
		kPulse,   // 拡大縮小を繰り返す(強調・クリックフィードバック)
		kFadeIn,  // アルファ 0 -> 1
		kFadeOut, // アルファ 1 -> 0
		kBlink,   // アルファを点滅させる(警告アイコン等、ループ向き)
		kSlideIn, // 指定方向から中央へスライドイン
		kWiggle,  // 回転がゆらゆら揺れる(注意喚起)
		kBounce,  // Y方向にバウンドする(出現演出)
	};

	// アニメーションを(再)生成する。durationは秒。intensityは種類ごとに意味が変わる
	// (shake=揺れ幅px, pulse=拡大率, slideIn=開始オフセットpx, bounce=跳ねる高さpx 等)。
	// loop=trueなら経過後も最初からループし続ける(pulse/blink/wiggle向け)
	void Play(Type type, float duration, float intensity = 1.0f, bool loop = false);
	void Stop();

	void Update(float deltaTime);

	SpriteOffsetUI GetOffset() const { return currentOffset_; }
	bool IsPlaying() const { return isPlaying_; }
	bool IsFinished() const { return !isPlaying_; }

	void SetSlideDirection(const KamataEngine::Vector2& dir) { slideFromDir_ = dir; }

private:
	void RecomputeOffset();

	Type type_ = Type::kNone;
	bool isPlaying_ = false;
	bool loop_ = false;
	float duration_ = 0.0f;
	float intensity_ = 1.0f;
	float timer_ = 0.0f;

	SpriteOffsetUI currentOffset_{};
	KamataEngine::Vector2 slideFromDir_{-1.0f, 0.0f};
};