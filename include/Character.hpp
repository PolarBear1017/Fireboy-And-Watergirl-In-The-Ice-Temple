
#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Element.hpp"
#include "GroundState.hpp"
#include "AtlasSprite.hpp"
#include "SpriteAtlas.hpp"
#include "Util/Transform.hpp"

class Character : public Util::GameObject{
private:
    glm::vec2 m_Velocity = {0.0f, 0.0f};
    glm::vec2 m_CollisionSize = {25.6f, 40.0f};

    float m_Gravity = 0.5f;
    float m_JumpForce = 12.5f;
    float m_Speed = 5.0f;
    GroundState m_GroundState = GroundState::AIR;
    Element m_Element;

    bool m_InputEnabled = true;
    bool m_Visible = true;

    std::shared_ptr<Util::GameObject> m_HeadObject;
    std::shared_ptr<Util::GameObject> m_LegsObject;

    std::shared_ptr<AtlasSprite> m_HeadSprite;
    std::shared_ptr<AtlasSprite> m_LegsSprite;

    int m_AnimationFrame = 0;
    float m_AnimationTimer = 0.0f;
    glm::vec2 m_VisualOffset = {0.0f, 4.0f};

    // std::shared_ptr<AtlasSprite> m_Sprite;

    void ProcessInput();
    void ApplyGravity();
    void UpdateAnimation();

public:
    Character(const std::shared_ptr<SpriteAtlas>& atlas, Element element);

    void Update();
    void AddChildrenTo(const std::shared_ptr<Util::GameObject>& root);

    [[nodiscard]] glm::vec2 GetPosition() const { return m_Transform.translation; }
    [[nodiscard]] glm::vec2 GetVelocity() const { return m_Velocity; }
    [[nodiscard]] glm::vec2 GetCollisionSize() const { return m_CollisionSize; }

    // [[nodiscard]] float GetGravity() const { return m_Gravity; }
    // [[nodiscard]] float GetJumpForce() const { return m_JumpForce; }

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

    void SetInputEnabled(bool enabled) { m_InputEnabled = enabled; }
    void SetVisible(bool visible) { m_Visible = visible; }
    [[nodiscard]] bool IsVisible() const { return m_Visible; }

};


#endif //CHARACTER_HPP
