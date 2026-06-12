#include "Mechanics/Lever.hpp"
#include "Util/Time.hpp"


#include "Mechanics/TriggerMediator.hpp"

Lever::Lever(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& pos, int groupId, const std::shared_ptr<TriggerMediator>& mediator)
    : Activator(groupId, mediator), m_Position(pos) {
    m_BaseSprite = std::make_shared<AtlasSprite>(atlas, "lever_base0000");
    m_Sprite = std::make_shared<AtlasSprite>(atlas, "lever_stick0000"); 
    
    m_BaseLightSprite = std::make_shared<AtlasSprite>(atlas, "lever_base_light_off0000");
    m_StickLightSprite = std::make_shared<AtlasSprite>(atlas, "lever_stick_light_off0000");
    
    m_BaseLightSprite->SetColorTint(GetGroupColor(groupId));
    m_StickLightSprite->SetColorTint(GetGroupColor(groupId));
    
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

    m_BaseLightObj = std::make_shared<Util::GameObject>(m_BaseLightSprite, 1.05f);
    m_BaseLightObj->m_Transform.translation = m_Transform.translation;
    m_BaseLightObj->m_Transform.scale = m_Transform.scale;
    AddChild(m_BaseLightObj);

    m_StickObject->m_Transform.translation = m_Transform.translation; 
    m_StickObject->m_Transform.translation.y -= 5.0f; // Offset to pivot properly on base
    m_StickObject->m_Transform.scale = {0.8f, 0.8f};
    AddChild(m_StickObject);

    m_StickLightObj = std::make_shared<Util::GameObject>(m_StickLightSprite, 0.55f);
    m_StickLightObj->m_Transform.translation = m_StickObject->m_Transform.translation;
    m_StickLightObj->m_Transform.scale = m_StickObject->m_Transform.scale;
    m_StickObject->AddChild(m_StickLightObj);

    // Initial state setup
    m_CurrentRotation = m_TargetRotation = -0.7f; // Approx -40 degrees (Left)
}

std::optional<Collider> Lever::GetCollider() const {
    float L = 30.0f;
    glm::vec2 pivot = m_StickObject->m_Transform.translation;
    glm::vec2 ballPos = pivot + L * glm::vec2(-std::sin(m_CurrentRotation), std::cos(m_CurrentRotation));
    return Collider{
        ballPos,
        {24.0f, 24.0f}
    };
}

void Lever::Update(const std::vector<glm::vec2>& interactorPositions) {
    float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;
    bool wasOn = m_IsOn;

    if (m_Cooldown > 0.0f) {
        m_Cooldown -= dt;
    }

    bool isBeingPushed = false;
    m_TargetRotation = m_IsOn ? 0.7f : -0.7f;

    if (m_Cooldown <= 0.0f) {
        float L = 30.0f;
        float R = 12.0f;
        glm::vec2 pivot = m_StickObject->m_Transform.translation;
        glm::vec2 ballPos = pivot + L * glm::vec2(-std::sin(m_CurrentRotation), std::cos(m_CurrentRotation));

        struct BoundingBox {
            float left;
            float right;
            float bottom;
            float top;
        };

        for (const auto& pos : interactorPositions) {
            BoundingBox box;
            if (std::abs(pos.y - m_Position.y) < 25.0f) {
                // Character (feet position)
                box.left = pos.x - 10.0f;
                box.right = pos.x + 10.0f;
                box.bottom = pos.y;
                box.top = pos.y + 35.0f;
            } else {
                // Block (center position)
                box.left = pos.x - 22.2f;
                box.right = pos.x + 22.2f;
                box.bottom = pos.y - 22.2f;
                box.top = pos.y + 22.2f;
            }

            // Ball bounding box
            float ball_left = ballPos.x - R;
            float ball_right = ballPos.x + R;
            float ball_bottom = ballPos.y - R;
            float ball_top = ballPos.y + R;

            bool overlap = !(box.left > ball_right || box.right < ball_left ||
                             box.bottom > ball_top || box.top < ball_bottom);

            if (overlap) {
                float pushSpeed = 5.0f;
                if (!m_IsOn && pos.x > ballPos.x) {
                    // Push left (turning on)
                    m_CurrentRotation += pushSpeed * dt;
                    isBeingPushed = true;
                    if (m_CurrentRotation >= 0.0f) {
                        m_CurrentRotation = 0.0f;
                        m_IsOn = true;
                        m_Cooldown = 0.5f;
                    }
                    break;
                } else if (m_IsOn && pos.x < ballPos.x) {
                    // Push right (turning off)
                    m_CurrentRotation -= pushSpeed * dt;
                    isBeingPushed = true;
                    if (m_CurrentRotation <= 0.0f) {
                        m_CurrentRotation = 0.0f;
                        m_IsOn = false;
                        m_Cooldown = 0.5f;
                    }
                    break;
                }
            }
        }
    }

    if (!isBeingPushed) {
        float lerpSpeed = 15.0f; 
        m_CurrentRotation += (m_TargetRotation - m_CurrentRotation) * lerpSpeed * dt;
    }

    m_StickObject->m_Transform.rotation = m_CurrentRotation;

    m_StickLightObj->m_Transform.translation = m_StickObject->m_Transform.translation;
    m_StickLightObj->m_Transform.rotation = m_StickObject->m_Transform.rotation;

    // Update light frames based on state
    if (m_IsOn) {
        m_BaseLightSprite->SetFrame("lever_base_light_on0000");
        m_StickLightSprite->SetFrame("lever_stick_light_on0000");
    } else {
        m_BaseLightSprite->SetFrame("lever_base_light_off0000");
        m_StickLightSprite->SetFrame("lever_stick_light_off0000");
    }

    if (wasOn != m_IsOn && m_Mediator) {
        m_Mediator->OnActivatorStateChanged(m_GroupId, m_IsOn);
    }
}
