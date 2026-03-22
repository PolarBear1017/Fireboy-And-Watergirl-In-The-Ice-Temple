
#ifndef REPLACE_WITH_YOUR_PROJECT_NAME_CHARACTER_HPP
#define REPLACE_WITH_YOUR_PROJECT_NAME_CHARACTER_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Element.hpp"

class Character : public Util::GameObject {
private:
    glm::vec2 m_Velocity = {0.0f, 0.0f};
    float m_Gravity = 0.5f;
    float m_JumpForce = 5.0f;
    bool m_IsGrounded = true;
    Element m_Element;

public:
    Character(const std::string& imagePath, Element element);
    void Update();

    [[nodiscard]] glm::vec2 GetVelocity() const { return m_Velocity; }
    [[nodiscard]] float GetGravity() const { return m_Gravity; }
    [[nodiscard]] float GetJumpForce() const { return m_JumpForce; }
    [[nodiscard]] bool IsGrounded() const {return m_IsGrounded; }
    [[nodiscard]] Element GetElement() const { return m_Element; }

    void ProcessInput();
    void ApplyGravity();

};


#endif //REPLACE_WITH_YOUR_PROJECT_NAME_CHARACTER_HPP