#include "Mechanics/Button.hpp"


Button::Button(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& pos, int groupId)
    : Activator(groupId), m_Position(pos) {
    m_BaseSprite = std::make_shared<AtlasSprite>(atlas, "pusher_block0000");
    m_Sprite = std::make_shared<AtlasSprite>(atlas, "pusher_block_light0000"); 
    
    SetDrawable(m_BaseSprite);
    
    auto lightObj = std::make_shared<Util::GameObject>(m_Sprite, 0.1f);
    lightObj->m_Transform.translation = pos;
    lightObj->m_Transform.translation.y += 24.0f + 2.0f; // Lowered to sit on top of base
    lightObj->m_Transform.scale = {0.6f, 0.6f};
    AddChild(lightObj);

    SetZIndex(1.0F); 
    // Adjust button height so it sits on the ground (32px tile, pusher_block is 86px tall)
    m_Transform.translation = pos;
    m_Transform.translation.y += 24.0f; // Lift it up so it sits on the tile bottom
    m_Transform.scale = {0.6f, 0.6f}; // Scale down to fit 32px grid better
}

void Button::Update(const glm::vec2& fireboyPos, const glm::vec2& watergirlPos) {
    float fbDist = glm::length(m_Position - fireboyPos);
    float wgDist = glm::length(m_Position - watergirlPos);
    
    // A simple radius check (bounding sphere approximation)
    bool currentlyPressed = (fbDist < 50.0f || wgDist < 50.0f);
    
    if (currentlyPressed != m_IsPressed) {
        m_IsPressed = currentlyPressed;
        // Sinking effect - deeper to show the base hollow state
        for (auto& child : GetChildren()) {
            // Apply absolute position logic here too
            child->m_Transform.translation.y = m_Position.y + 24.0f + (m_IsPressed ? -2.0f : 2.0f);
        }
    }
}
