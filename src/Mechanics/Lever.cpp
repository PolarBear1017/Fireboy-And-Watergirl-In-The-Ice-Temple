#include "Mechanics/Lever.hpp"
#include "Util/Time.hpp"


Lever::Lever(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& pos, int groupId)
    : Activator(groupId), m_Position(pos) {
    m_BaseSprite = std::make_shared<AtlasSprite>(atlas, "lever_base0000");
    m_Sprite = std::make_shared<AtlasSprite>(atlas, "lever_stick0000"); 
    
    SetDrawable(m_BaseSprite);
    
    m_StickObject = std::make_shared<Util::GameObject>(m_Sprite, 0.5f);
    
    SetZIndex(1.0F); 
    m_Transform.translation = pos;
    // Lever base is 110x110. At 0.8 scale it's 88x88.
    // Center to bottom is 44. Tile center to floor is 16.
    // Offset = 44 - 16 = 28.
    m_Transform.translation.y -= 15.0f; 
    m_Position.y = m_Transform.translation.y; // 讓互動範圍跟著圖片走
    m_Transform.scale = {0.8f, 0.8f};

    m_StickObject->m_Transform.translation = m_Transform.translation; 
    m_StickObject->m_Transform.translation.y -= 5.0f; // Offset to pivot properly on base
    m_StickObject->m_Transform.scale = {0.8f, 0.8f};
    AddChild(m_StickObject);

    // Initial state setup
    m_CurrentRotation = m_TargetRotation = -0.7f; // Approx -40 degrees (Left)
}

void Lever::Update(const glm::vec2& fireboyPos, const glm::vec2& watergirlPos) {
    float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;

    if (m_Cooldown > 0.0f) {
        m_Cooldown -= dt;
    }

    // Smooth rotation animation
    m_TargetRotation = m_IsOn ? 0.7f : -0.7f;
    float lerpSpeed = 15.0f; 
    m_CurrentRotation += (m_TargetRotation - m_CurrentRotation) * lerpSpeed * dt;
    m_StickObject->m_Transform.rotation = m_CurrentRotation;

    if (m_Cooldown > 0.0f) return;

    auto checkPush = [&](const glm::vec2& playerPos) {
        float distX = std::abs(m_Position.x - playerPos.x);
        float distY = std::abs(m_Position.y - playerPos.y);
        
        // Interaction box: +/- 35px horizontally, +/- 40px vertically
        if (distX < 35.0f && distY < 40.0f) {
            // Directional switch:
            // If lever points Left (off) and player is on the Left, push to Right
            if (!m_IsOn && playerPos.x < m_Position.x - 5.0f) {
                m_IsOn = true;
                m_Cooldown = 0.5f;
            }
            // If lever points Right (on) and player is on the Right, push to Left
            else if (m_IsOn && playerPos.x > m_Position.x + 5.0f) {
                m_IsOn = false;
                m_Cooldown = 0.5f;
            }
        }
    };

    checkPush(fireboyPos);
    checkPush(watergirlPos);
}
