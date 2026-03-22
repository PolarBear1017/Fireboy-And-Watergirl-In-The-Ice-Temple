
#ifndef REPLACE_WITH_YOUR_PROJECT_NAME_CHARACTER_HPP
#define REPLACE_WITH_YOUR_PROJECT_NAME_CHARACTER_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Element.hpp"
#include "AtlasSprite.hpp"

class Character : public Util::GameObject {
private:
    glm::vec2 m_Velocity = {0.0f, 0.0f};
    float m_Gravity = 0.5f;
    float m_JumpForce = 5.0f;
    bool m_IsGrounded = true;
    Element m_Element;

public:
    Character(const std::shared_ptr<AtlasSprite>& sprite, Element element);
    void Update();

    [[nodiscard]] glm::vec2 GetVelocity() const { return m_Velocity; }
    [[nodiscard]] glm::vec2 GetPosition() const { return m_Transform.translation; }
    [[nodiscard]] glm::vec2 GetCollisionSize() const { return {32.0F, 32.0F}; }
    [[nodiscard]] float GetGravity() const { return m_Gravity; }
    [[nodiscard]] float GetJumpForce() const { return m_JumpForce; }
    [[nodiscard]] bool IsGrounded() const {return m_IsGrounded; }
    [[nodiscard]] Element GetElement() const { return m_Element; }

    void SetPosition(const glm::vec2& position) { m_Transform.translation = position; }
    void SetVelocity(const glm::vec2& velocity) { m_Velocity = velocity; }
    void SetGrounded(bool grounded) { m_IsGrounded = grounded; }

    void ProcessInput();
    void ApplyGravity();

};


#endif //REPLACE_WITH_YOUR_PROJECT_NAME_CHARACTER_HPP
