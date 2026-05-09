#include "Mechanics/Block.hpp"
#include "AtlasSprite.hpp"
#include "config.hpp"
#include "Util/Time.hpp"

Block::Block(std::shared_ptr<SpriteAtlas> atlas, const glm::vec2& pos) 
    : BaseMechanism(-1) {
    
    auto sprite = std::make_shared<AtlasSprite>(atlas, "movingbox0000");
    SetDrawable(sprite);
    
    // Scale down a bit to fit inside tile and look reasonable
    m_Transform.scale = {0.6f, 0.6f};
    
    m_Size = glm::vec2(74.0f, 76.0f) * m_Transform.scale;
    m_Transform.translation = pos;
    m_ZIndex = 3.0f;
}

std::optional<Collider> Block::GetCollider() const {
    return Collider{
        m_Transform.translation,
        m_Size
    };
}

void Block::Update() {
    // Apply gravity (per-frame, like Character)
    m_Velocity.y -= 0.5f;
    
    // Max fall speed
    if (m_Velocity.y < -15.0f) {
        m_Velocity.y = -15.0f;
    }
    
    // Horizontal friction 水平摩擦力
    if (m_Velocity.x > 0.0f) {
        m_Velocity.x -= 0.5f;
        if (m_Velocity.x < 0.0f) m_Velocity.x = 0.0f;
    } else if (m_Velocity.x < 0.0f) {
        m_Velocity.x += 0.5f;
        if (m_Velocity.x > 0.0f) m_Velocity.x = 0.0f;
    }

    m_Transform.translation += m_Velocity;
}
