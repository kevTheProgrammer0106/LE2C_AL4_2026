#pragma once

#define NOMINMAX

#include "Global.h"
#include <KamataEngine.h>

/// <summary>
/// 実際の入力を扱う唯一のクラス。KamataEngine::Inputへの直接アクセスはここに集約し、
/// Player/敵/UIMenuなど他のシステムは全てこのクラス経由で入力状態を問い合わせる。
/// ボタンAABB・カーソルスプライト・フェードといったUI要素は一切扱わない(それはUIMenuの役目)。
/// 毎フレーム先頭で1回だけUpdate()を呼び、そのフレーム中の問い合わせは全てキャッシュされた値を返す。
/// </summary>
class InputManager {
public:
	static InputManager* GetInstance();

	void Update();

	// --- 移動 ---
	bool IsMoveLeftHeld() const { return moveLeftHeld_; }
	bool IsMoveRightHeld() const { return moveRightHeld_; }
	// -1.0 / 0.0 / 1.0 (左右同時押しは0)
	float GetMoveAxisX() const;

	// --- ジャンプ ---
	bool IsJumpHeld() const { return jumpHeld_; }           // 接地中に押され続けていればジャンプ(既存仕様)
	bool IsJumpTriggered() const { return jumpTriggered_; } // メニュー等の「決定」に使う単発トリガー

	// --- アクション ---
	bool IsWeaponSwitchTriggered() const { return weaponSwitchTriggered_; }
	bool IsPauseTriggered() const { return pauseTriggered_; }
	bool IsInteractTriggered() const { return interactTriggered_; }
	bool IsAttackUpHeld() const { return attackUpHeld_; }
	bool IsAttackDownHeld() const { return attackDownHeld_; }

	// --- マウス ---
	int GetMouseX() const { return mouseX_; }
	int GetMouseY() const { return mouseY_; }
	bool IsPrimaryTriggered() const { return primaryMouseTriggered_; } // 左クリック(トリガー)
	bool IsPrimaryHeld() const { return primaryMouseHeld_; }           // 左クリック(ホールド)
	bool IsBlockHeld() const { return secondaryMouseHeld_; }           // 右クリック(ホールド) = ガード
	bool IsSecondaryTriggered() const { return secondaryMouseTriggered_; }

private:
	InputManager() = default;
	~InputManager() = default;
	InputManager(const InputManager&) = delete;
	InputManager& operator=(const InputManager&) = delete;

	bool moveLeftHeld_ = false;
	bool moveRightHeld_ = false;

	bool jumpHeld_ = false;
	bool jumpTriggered_ = false;

	bool weaponSwitchTriggered_ = false;
	bool pauseTriggered_ = false;
	bool interactTriggered_ = false;
	bool attackUpHeld_ = false;
	bool attackDownHeld_ = false;

	KamataEngine::Vector2 mouseMovement_;
	int mouseX_ = 0;
	int mouseY_ = 0;

	bool primaryMouseTriggered_ = false;
	bool primaryMouseHeld_ = false;
	bool secondaryMouseHeld_ = false;
	bool secondaryMouseTriggered_ = false;
};