#include "Mechanics/Lever.hpp"
#include "Util/Time.hpp"


Lever::Lever(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& pos, int groupId)
    : Activator(groupId), m_Position(pos) {
    m_Sprite = std::make_shared<AtlasSprite>(atlas, "lever_stick_light_off0000"); 
    SetDrawable(m_Sprite);
    SetZIndex(1.0F); 
    m_Transform.translation = pos;
    m_Transform.scale = {0.75f, 0.75f};
}

void Lever::Update(const glm::vec2& fireboyPos, const glm::vec2& watergirlPos) {
    if (m_Cooldown > 0.0f) {
        m_Cooldown -= static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;
        return;
    }

    float fbDist = glm::length(m_Position - fireboyPos);
    float wgDist = glm::length(m_Position - watergirlPos);
    
    if (fbDist < 40.0f || wgDist < 40.0f) {
        m_IsOn = !m_IsOn;
        m_Sprite->SetFrame(m_IsOn ? "lever_stick_light_on0000" : "lever_stick_light_off0000");
        m_Cooldown = 0.5f; 
    }
}
