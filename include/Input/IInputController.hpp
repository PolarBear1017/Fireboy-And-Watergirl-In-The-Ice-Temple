#ifndef INPUT_I_INPUT_CONTROLLER_HPP
#define INPUT_I_INPUT_CONTROLLER_HPP

/**
 * @brief Interface for character input control (Abstraction/Interface principle).
 * Allows character to query horizontal movement axis and jump status
 * without knowing whether the input source is keyboard, controller, or AI.
 */
class IInputController {
public:
    virtual ~IInputController() = default;
    
    /**
     * @brief Gets horizontal input axis.
     * @return float Value between -1.0f (left) and 1.0f (right).
     */
    virtual float GetHorizontalAxis() = 0;
    
    /**
     * @brief Checks if the jump key is currently pressed.
     * @return true if jump is pressed, false otherwise.
     */
    virtual bool IsJumpPressed() = 0;
};

#endif // INPUT_I_INPUT_CONTROLLER_HPP
