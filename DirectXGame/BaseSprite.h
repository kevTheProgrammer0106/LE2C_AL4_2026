#pragma once

#include "Global.h"
#include <KamataEngine.h>
#include <memory>

/// <summary>
/// スプライトの基底クラス
/// KamataEngine::Sprite をラップし、共通のアクセサ群を提供する
/// </summary>
class BaseSprite {
public:
	virtual ~BaseSprite() = default;

	virtual void Initialize(uint32_t textureHandle, const KamataEngine::Vector2& position);
	virtual void Update();
	virtual void Draw();

	// テクスチャハンドル
	uint32_t GetTextureHandle() const { return textureHandle_; }
	void SetTextureHandle(uint32_t textureHandle);

	// 座標
	const KamataEngine::Vector2& GetPosition() const { return position_; }
	void SetPosition(const KamataEngine::Vector2& position);

	// 角度
	float GetRotation() const { return rotation_; }
	void SetRotation(float rotation);

	// サイズ
	const KamataEngine::Vector2& GetSize() const { return size_; }
	void SetSize(const KamataEngine::Vector2& size);

	// 色
	const KamataEngine::Vector4& GetColor() const { return color_; }
	void SetColor(const KamataEngine::Vector4& color);

	//// 左右反転
	//bool GetIsFlipX() const { return isFlipX_; }
	//void SetIsFlipX(bool isFlipX);

	//// 上下反転
	//bool GetIsFlipY() const { return isFlipY_; }
	//void SetIsFlipY(bool isFlipY);

protected:
	std::unique_ptr<KamataEngine::Sprite> sprite_ = nullptr;

	uint32_t textureHandle_ = 0;
	KamataEngine::Vector2 position_ = {0.0f, 0.0f};
	float rotation_ = 0.0f;
	KamataEngine::Vector2 size_ = {1.0f, 1.0f};
	KamataEngine::Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};
	bool isFlipX_ = false;
	bool isFlipY_ = false;
};