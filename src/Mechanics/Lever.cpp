#include "Mechanics/Lever.hpp"
#include "Util/Time.hpp"


Lever::Lever(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& pos, int groupId)
    : Activator(groupId), m_Position(pos) {
    m_BaseSprite = std::make_shared<AtlasSprite>(atlas, "lever_base0000");
    m_Sprite = std::make_shared<AtlasSprite>(atlas, "lever_stick0000"); 
    
    SetDrawable(m_BaseSprite);
    
    auto stickObj = std::make_shared<Util::GameObject>(m_Sprite, 0.1f);
    stickObj->m_Transform.translation = pos; // Manually apply parent position
    stickObj->m_Transform.translation.y -= 10.0f; 
    stickObj->m_Transform.scale = {0.6f, 0.6f}; // Manually apply parent scale
    AddChild(stickObj);

    SetZIndex(1.0F); 
    m_Transform.translation = pos;
    m_Transform.scale = {0.8f, 0.8f};
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
        // Flip the stick visually
        for (auto& child : GetChildren()) {
            child->m_Transform.scale.x = m_IsOn ? -1.0f : 1.0f;
        }
        m_Cooldown = 0.5f; 
    }
}
