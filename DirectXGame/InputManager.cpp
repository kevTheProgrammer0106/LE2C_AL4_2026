#include "InputManager.h"

using namespace KamataEngine;

InputManager* InputManager::GetInstance() {
	static InputManager instance;
	return &instance;
}

void InputManager::Update() {
	Input* input = Input::GetInstance();

	moveLeftHeld_ = input->PushKey(DIK_A);
	moveRightHeld_ = input->PushKey(DIK_D);

	jumpHeld_ = input->PushKey(DIK_SPACE);
	jumpTriggered_ = input->TriggerKey(DIK_SPACE);

	weaponSwitchTriggered_ = input->TriggerKey(DIK_Q);
	pauseTriggered_ = input->TriggerKey(DIK_ESCAPE);
	interactTriggered_ = input->TriggerKey(DIK_F);
	attackUpHeld_ = input->PushKey(DIK_W);
	attackDownHeld_ = input->PushKey(DIK_S);

	mouseMovement_ = input->GetMousePosition();
	mouseX_ = static_cast<int>(mouseMovement_.x);
	mouseY_ = static_cast<int>(mouseMovement_.y);


	primaryMouseTriggered_ = input->IsTriggerMouse(0);
	primaryMouseHeld_ = input->IsPressMouse(0);
	secondaryMouseHeld_ = input->IsPressMouse(1);
	secondaryMouseTriggered_ = input->IsTriggerMouse(1);
}

float InputManager::GetMoveAxisX() const {
	if (moveLeftHeld_ == moveRightHeld_) {
		return 0.0f; // どちらも押していない、または両方押している
	}
	return moveRightHeld_ ? 1.0f : -1.0f;
}