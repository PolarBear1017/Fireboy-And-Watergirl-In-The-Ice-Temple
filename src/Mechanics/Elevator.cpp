#include "Mechanics/Elevator.hpp"
#include "Util/Time.hpp"


Elevator::Elevator(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& startPos, const glm::vec2& endPos, const glm::vec2& size, int groupId)
    : Receiver(groupId), m_StartPos(startPos), m_EndPos(endPos), m_Size(size) {
    m_Sprite = std::make_shared<AtlasSprite>(atlas, "moving_platform_light_off0000"); 
    SetDrawable(m_Sprite);
    SetZIndex(-1.0F);
    m_Transform.translation = startPos;
    m_LastPosition = startPos;

    // Override the collision size to exactly match the sprite's visual dimensions.
    // The visual size (138x26) is different from the default tiled size,
    // which caused characters to fall off at the edges.
    m_Size = m_Sprite->GetSize();
}

void Elevator::SetActivated(bool active) {
    m_IsOn = active;
}

void Elevator::Update() {
    float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;
    if (m_IsOn && m_Progress < 1.0f) {
        m_Progress += dt / m_Speed;
        if (m_Progress > 1.0f) m_Progress = 1.0f;
    } else if (!m_IsOn && m_Progress > 0.0f) {
        m_Progress -= dt / m_Speed;
        if (m_Progress < 0.0f) m_Progress = 0.0f;
    }
    
    // Update light state
    if (m_Progress > 0.0f || m_IsOn) {
        m_Sprite->SetFrame("moving_platform_light_on0000");
    } else {
        m_Sprite->SetFrame("moving_platform_light_off0000");
    }
    
    // Smoothstep or linear interpolation
    m_LastPosition = m_Transform.translation;
    m_Transform.translation = m_StartPos + (m_EndPos - m_StartPos) * m_Progress;
}
