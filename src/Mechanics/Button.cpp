#include "Mechanics/Button.hpp"


Button::Button(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& pos, int groupId)
    : Activator(groupId), m_Position(pos) {
    
    // Base: pusher_block0000 (110x86)
    m_BaseSprite = std::make_shared<AtlasSprite>(atlas, "pusher_block0000");
    SetDrawable(m_BaseSprite);
    
    // Pusher: pusher_block_light0000 (64x62)
    m_Sprite = std::make_shared<AtlasSprite>(atlas, "pusher_block_light0000");
    m_PusherObject = std::make_shared<Util::GameObject>(m_Sprite, 0.1f);
    
    // Add pusher as child
    AddChild(m_PusherObject);
    
    // Adjust scale and position
    // Original game's buttons are roughly 2 tiles wide (64px)
    // 110 * 0.6 = 66 px
    m_Transform.scale = {0.6f, 0.6f}; 
    m_Transform.translation = pos;
    // Align bottom of button with bottom of tile
    // Tile size is 32, so center to bottom is 16.
    // Button height is 86 * 0.6 = 51.6. Center to bottom is 25.8.
    // We want the button's bottom to be at the tile's bottom.
    // So current center should be at tileCenter.y - 16 + 25.8 = tileCenter.y + 9.8
    m_Transform.translation.y += 9.8f;

    // Absolute position for pusher (unpressed)
    m_PusherObject->m_Transform.translation = m_Transform.translation;
    m_PusherObject->m_Transform.translation.y += 12.0f;

    SetZIndex(1.0F);
}

std::optional<Collider> Button::GetCollider() const {
    // The button base acts as a solid platform
    // Width: 110 * 0.6 = 66
    // Height: Let's make the collision box roughly 20px high so it's a thin platform
    return Collider{
        m_Transform.translation,
        glm::vec2(66.0f, 20.0f)
    };
}

void Button::Update(const glm::vec2& fireboyPos, const glm::vec2& watergirlPos) {
    // AABB trigger logic:
    // Fireboy/Watergirl collision size is 32x50 (half is 16x25)
    // Their feet are at pos.y - 25.
    // The button base top is at m_Transform.translation.y + something
    
    auto checkPressed = [&](const glm::vec2& charPos) {
        float charBottom = charPos.y - 25.0f;
        float charLeft = charPos.x - 16.0f;
        float charRight = charPos.x + 16.0f;
        
        float btnTop = m_Transform.translation.y + 10.0f; // Roughly top of base
        float btnLeft = m_Transform.translation.x - 33.0f;
        float btnRight = m_Transform.translation.x + 33.0f;
        
        // Trigger if feet are on top and within width
        return (charBottom >= btnTop - 5.0f && charBottom <= btnTop + 15.0f) &&
               (charRight > btnLeft && charLeft < btnRight);
    };

    bool currentlyPressed = checkPressed(fireboyPos) || checkPressed(watergirlPos);
    
    if (currentlyPressed != m_IsPressed) {
        m_IsPressed = currentlyPressed;
        // Sinking animation: move the pusher object
        // m_Transform.translation.y + 12.0f is unpressed. 
        m_PusherObject->m_Transform.translation = m_Transform.translation;
        m_PusherObject->m_Transform.translation.y += m_IsPressed ? 2.0f : 12.0f;
    }
}
