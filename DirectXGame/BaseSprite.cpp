#include "BaseSprite.h"

using namespace KamataEngine;

void BaseSprite::Initialize(uint32_t textureHandle, const Vector2& position) {
	textureHandle_ = textureHandle;
	position_ = position;

	sprite_.reset(Sprite::Create(textureHandle_, position_));

	size_ = sprite_->GetSize();
	sprite_->SetRotation(rotation_);
	sprite_->SetColor(color_);
	//sprite_->SetFlipX(false);
	//sprite_->SetFlipY(true);
}

void BaseSprite::Update() {
	if (!sprite_) {
		return;
	}
	sprite_->SetPosition(position_);
	sprite_->SetRotation(rotation_);
	sprite_->SetSize(size_);
	sprite_->SetColor(color_);
	//sprite_->SetFlipX(isFlipX_);
	//sprite_->SetFlipY(isFlipY_);
}

void BaseSprite::Draw() {
	if (!sprite_) {
		return;
	}
	Sprite::PreDraw();
	sprite_->Draw();
	Sprite::PostDraw();
}

void BaseSprite::SetTextureHandle(uint32_t textureHandle) {
	textureHandle_ = textureHandle;
	if (sprite_) {
		sprite_->SetTextureHandle(textureHandle_);
	}
}

void BaseSprite::SetPosition(const Vector2& position) {
	position_ = position;
	if (sprite_) {
		sprite_->SetPosition(position_);
	}
}

void BaseSprite::SetRotation(float rotation) {
	rotation_ = rotation;
	if (sprite_) {
		sprite_->SetRotation(rotation_);
	}
}

void BaseSprite::SetSize(const Vector2& size) {
	size_ = size;
	if (sprite_) {
		sprite_->SetSize(size_);
	}
}

void BaseSprite::SetColor(const Vector4& color) {
	color_ = color;
	if (sprite_) {
		sprite_->SetColor(color_);
	}
}

// 
//void BaseSprite::SetIsFlipX(bool isFlipX) {
//	isFlipX_ = isFlipX;
//	if (sprite_) {
//		sprite_->SetFlipX(isFlipX_);
//	}
//}
//
//void BaseSprite::SetIsFlipY(bool isFlipY) {
//	isFlipY_ = isFlipY;
//	if (sprite_) {
//		sprite_->SetFlipY(isFlipY_);
//	}
//}