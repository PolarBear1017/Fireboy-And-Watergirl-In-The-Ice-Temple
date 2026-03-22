
#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Element.hpp"
#include "GroundState.hpp"
#include "AtlasSprite.hpp"

class Character : public Util::GameObject {
private:
    glm::vec2 m_Velocity = {0.0f, 0.0f};
    float m_Gravity = 0.5f;
    float m_JumpForce = 12.5f;
    GroundState m_GroundState = GroundState::GROUND;
    Element m_Element;

public:
    Character(const std::shared_ptr<AtlasSprite>& sprite, Element element);
    void Update();

    [[nodiscard]] glm::vec2 GetVelocity() const { return m_Velocity; }
    [[nodiscard]] glm::vec2 GetPosition() const { return m_Transform.translation; }
    [[nodiscard]] glm::vec2 GetCollisionSize() const { return {32.0F, 32.0F}; }
    [[nodiscard]] float GetGravity() const { return m_Gravity; }
    [[nodiscard]] float GetJumpForce() const { return m_JumpForce; }
    [[nodiscard]] GroundState GetGroundState() const {return m_GroundState; }
    [[nodiscard]] Element GetElement() const { return m_Element; }
    [[nodiscard]] bool IsGrounded() const {
        return m_GroundState == GroundState::GROUND
            || m_GroundState == GroundState::SLOPE
            || m_GroundState == GroundState::MOVING_PLATFORM;
    }

    void SetPosition(const glm::vec2& position) { m_Transform.translation = position; }
    void SetVelocity(const glm::vec2& velocity) { m_Velocity = velocity; }
    void SetGroundState(const GroundState groundState) { m_GroundState = groundState; }

    void ProcessInput();
    void ApplyGravity();

};


#endif //CHARACTER_HPP
