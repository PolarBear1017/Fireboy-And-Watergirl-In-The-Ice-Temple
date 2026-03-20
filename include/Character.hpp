
#ifndef REPLACE_WITH_YOUR_PROJECT_NAME_CHARACTER_HPP
#define REPLACE_WITH_YOUR_PROJECT_NAME_CHARACTER_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Element.hpp"

class Character : public Util::GameObject {
private:
    glm::vec2 m_Velocity = {0.0f, 0.0f};
    float m_Gravity;
    float m_JumpForce;
    bool m_IsGrounded = false;
    Element m_Element;

public:
    Character(const std::string& imagePath, Element element);
    void Update();

    glm::vec2 GetVelocity() const { return m_Velocity; }
    float GetGravity() const { return m_Gravity; }
    float GetJumpForce() const { return m_JumpForce; }
    bool IsGrounded() const {return m_IsGrounded; }
    Element GetElement() const { return m_Element; }
};


#endif //REPLACE_WITH_YOUR_PROJECT_NAME_CHARACTER_HPP