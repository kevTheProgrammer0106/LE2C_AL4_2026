#pragma once

#define NOMINMAX

#include "Global.h"
#include "KamataEngine.h"
#include <algorithm>
#include <cassert>

/// <summary>
/// エフェクトの基底クラス（拡大→フェードアウト等の共通処理をまとめる）
/// </summary>
class BaseEffect {
public:
	virtual ~BaseEffect() = default;

	virtual void Update();
	virtual void Draw() = 0;

	bool IsFinished() const { return isFinished_; }

	// エフェクトの状態(型)
	enum class Status {
		kExpand,  // 拡大
		kFadeOut, // フェードアウト
	};

protected:
	// 派生クラスから呼ぶ共通初期化
	void BaseInitialize(uint32_t textureHandle, float expandDuration, float fadeDuration);

	// 拡大フェーズで毎フレーム呼ばれる（0.0〜1.0の進行度tを渡す）。スケール補間など派生固有の処理をここに書く。
	virtual void OnExpandUpdate(float t);
	// フェードアウトフェーズで毎フレーム呼ばれる（0.0〜1.0の進行度t）。基本は色のアルファのみで十分だが、追加演出があれば派生でオーバーライド。
	virtual void OnFadeOutUpdate(float t);
	// 状態に関わらず毎フレーム呼ばれる（回転演出など）
	virtual void OnCommonUpdate();

	uint32_t textureHandle_ = 0;

	bool isFinished_ = false;

	Status status_ = Status::kExpand;
	// 現在の状態の持続時間<秒>
	float duration_ = 0.0f;
	// 経過時間カウント
	float counter_ = 0.0f;

	// 拡大フェーズ / フェードアウトフェーズそれぞれの持続時間
	float expandDuration_ = 0.15f;
	float fadeDuration_ = 0.25f;

	// 色変更オブジェクト
	KamataEngine::ObjectColor objectColor_;
	// 色の数値
	KamataEngine::Vector4 color_{1.0f, 1.0f, 1.0f, 1.0f};
};