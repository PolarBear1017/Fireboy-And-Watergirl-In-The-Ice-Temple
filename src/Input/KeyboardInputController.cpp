#include "Input/KeyboardInputController.hpp"
#include "Util/Input.hpp"

KeyboardInputController::KeyboardInputController(Util::Keycode leftKey, Util::Keycode rightKey, Util::Keycode jumpKey)
    : m_LeftKey(leftKey), m_RightKey(rightKey), m_JumpKey(jumpKey) {}

float KeyboardInputController::GetHorizontalAxis() {
    float axis = 0.0f;
    if (Util::Input::IsKeyPressed(m_LeftKey)) {
        axis -= 1.0f;
    }
    if (Util::Input::IsKeyPressed(m_RightKey)) {
        axis += 1.0f;
    }
    return axis;
}

bool KeyboardInputController::IsJumpPressed() {
    return Util::Input::IsKeyPressed(m_JumpKey);
}
