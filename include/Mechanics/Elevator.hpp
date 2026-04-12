#ifndef MECHANICS_ELEVATOR_HPP
#define MECHANICS_ELEVATOR_HPP

#include "Receiver.hpp"
#include "AtlasSprite.hpp"
#include "SpriteAtlas.hpp"
#include <memory>
#include <glm/vec2.hpp>

class Elevator : public Receiver {
public:
    Elevator(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& startPos, const glm::vec2& endPos, const glm::vec2& size, int groupId);

    void Update() override;
    void SetActivated(bool active) override;

    std::optional<Collider> GetCollider() const override {
        return Collider{ m_Transform.translation, m_Size };
    }

    glm::vec2 GetDeltaPosition() const { return m_Transform.translation - m_LastPosition; }

private:
    std::shared_ptr<AtlasSprite> m_Sprite;
    glm::vec2 m_StartPos;
    glm::vec2 m_EndPos;
    glm::vec2 m_Size;
    glm::vec2 m_LastPosition;
    bool m_IsOn = false;
    
    float m_Progress = 0.0f; // 0.0 -> StartPos, 1.0 -> EndPos
    float m_Speed = 2.0f;    // Time to complete transition in seconds
};

#endif
