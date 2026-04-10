#include "Mechanics/Button.hpp"


Button::Button(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& pos, int groupId)
    : Activator(groupId), m_Position(pos) {
    m_Sprite = std::make_shared<AtlasSprite>(atlas, "lightpusher_light_off0000"); 
    SetDrawable(m_Sprite);
    SetZIndex(1.0F); 
    m_Transform.translation = pos;
    m_Transform.scale = {0.75f, 0.75f};
}

void Button::Update(const glm::vec2& fireboyPos, const glm::vec2& watergirlPos) {
    float fbDist = glm::length(m_Position - fireboyPos);
    float wgDist = glm::length(m_Position - watergirlPos);
    
    // A simple radius check (bounding sphere approximation)
    bool currentlyPressed = (fbDist < 50.0f || wgDist < 50.0f);
    
    if (currentlyPressed != m_IsPressed) {
        m_IsPressed = currentlyPressed;
        m_Sprite->SetFrame(m_IsPressed ? "lightpusher_light_on0000" : "lightpusher_light_off0000");
    }
}
