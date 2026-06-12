#ifndef INPUT_KEYBOARD_INPUT_CONTROLLER_HPP
#define INPUT_KEYBOARD_INPUT_CONTROLLER_HPP

#include "IInputController.hpp"
#include "Util/Keycode.hpp"

/**
 * @brief Concrete keyboard controller implementing IInputController (Polymorphism & Dependency Injection).
 * Maps specific keys to character controls.
 */
class KeyboardInputController : public IInputController {
public:
    KeyboardInputController(Util::Keycode leftKey, Util::Keycode rightKey, Util::Keycode jumpKey);
    
    float GetHorizontalAxis() override;
    bool IsJumpPressed() override;

private:
    Util::Keycode m_LeftKey;
    Util::Keycode m_RightKey;
    Util::Keycode m_JumpKey;
};

#endif // INPUT_KEYBOARD_INPUT_CONTROLLER_HPP
