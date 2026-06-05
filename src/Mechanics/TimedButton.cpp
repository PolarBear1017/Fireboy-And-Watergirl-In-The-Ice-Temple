#include "Mechanics/TimedButton.hpp"
#include "Util/Time.hpp"
#include <algorithm>

TimedButton::TimedButton(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& pos, int groupId, float durationMs)
    : Activator(groupId), m_DurationMs(durationMs) {
    
    // Scale factor
    m_Transform.scale = {0.6f, 0.6f};

    // Calculate layout offset to align with ground tiles
    m_InitialPosition = pos + glm::vec2(0.0f, -12.0f);
    m_Transform.translation = m_InitialPosition;
    m_CurrentYOffset = 0.0f;

    // Base position adjusted slightly upwards (from -12.0f to -4.0f)
    glm::vec2 basePos = pos + glm::vec2(0.0f, -4.0f);

    // 1. Stationary Base Panel (timed_pusher_base0000)
    m_BasePanelSprite = std::make_shared<AtlasSprite>(atlas, "timed_pusher_base0000");
    m_BasePanelObject = std::make_shared<Util::GameObject>(m_BasePanelSprite, 0.01f);
    m_BasePanelObject->m_Transform.translation = basePos;
    m_BasePanelObject->m_Transform.scale = m_Transform.scale;
    AddChild(m_BasePanelObject);

    // 2. Stationary Indicator Dial OFF (timed_pusher_light_off0000)
    m_BaseLightOffSprite = std::make_shared<AtlasSprite>(atlas, "timed_pusher_light_off0000");
    m_BaseLightOffObject = std::make_shared<Util::GameObject>(m_BaseLightOffSprite, 0.02f);
    m_BaseLightOffObject->m_Transform.translation = basePos;
    m_BaseLightOffObject->m_Transform.scale = m_Transform.scale;
    AddChild(m_BaseLightOffObject);

    // 3. Stationary Indicator Dial ON (timed_pusher_light_on0000)
    m_BaseLightOnSprite = std::make_shared<AtlasSprite>(atlas, "timed_pusher_light_on0000");
    m_BaseLightOnObject = std::make_shared<Util::GameObject>(m_BaseLightOnSprite, 0.03f);
    m_BaseLightOnObject->m_Transform.translation = basePos;
    m_BaseLightOnObject->m_Transform.scale = m_Transform.scale;
    m_BaseLightOnObject->SetVisible(false);
    AddChild(m_BaseLightOnObject);

    // 4. Moving Block (pusher_block0000)
    m_BaseSprite = std::make_shared<AtlasSprite>(atlas, "pusher_block0000");
    SetDrawable(m_BaseSprite);
    SetZIndex(-0.1f); // Layering button block

    // 5. Moving Glow Overlay (pusher_block_light0000)
    m_Sprite = std::make_shared<AtlasSprite>(atlas, "pusher_block_light0000");
    m_Sprite->SetColorTint(GetGroupColor(groupId));
    m_PusherObject = std::make_shared<Util::GameObject>(m_Sprite, -0.05f);
    m_PusherObject->m_Transform.translation = m_Transform.translation;
    m_PusherObject->m_Transform.scale = m_Transform.scale;
    AddChild(m_PusherObject);
}

std::optional<Collider> TimedButton::GetCollider() const {
    glm::vec2 collPos = m_Transform.translation;
    collPos.y += 10.0f; 
    
    return Collider{
        collPos,
        glm::vec2(66.0f, 15.0f)
    };
}

void TimedButton::Update(const std::vector<glm::vec2>& interactorPositions) {
    float dt = Util::Time::GetDeltaTimeMs();

    // 1. Detection Logic (exactly same box dimensions as standard button)
    auto checkPressed = [&](const glm::vec2& pos) {
        float posBottom = pos.y;
        float posLeft = pos.x - 16.0f;
        float posRight = pos.x + 16.0f;
        
        float btnTop = m_InitialPosition.y + 15.0f; 
        float btnLeft = m_InitialPosition.x - 33.0f;
        float btnRight = m_InitialPosition.x + 33.0f;
        
        return (posBottom >= btnTop - 40.0f && posBottom <= btnTop + 10.0f) &&
               (posRight > btnLeft && posLeft < btnRight);
    };

    m_IsPressed = false;
    for (const auto& pos : interactorPositions) {
        if (checkPressed(pos)) {
            m_IsPressed = true;
            break;
        }
    }

    // 2. Timing State Machine
    if (m_IsPressed) {
        m_IsActive = true;
        m_TimeRemainingMs = m_DurationMs;
    } else {
        if (m_IsActive) {
            m_TimeRemainingMs -= dt;
            if (m_TimeRemainingMs <= 0.0f) {
                m_IsActive = false;
                m_TimeRemainingMs = 0.0f;
            }
        }
    }

    // 3. Dial visual feedback using clock shader mask
    if (m_IsActive) {
        m_BaseLightOnObject->SetVisible(true);
        float ratio = m_TimeRemainingMs / m_DurationMs;
        float quantizedC = std::ceil(12.0f * ratio) / 12.0f;
        m_BaseLightOnSprite->SetClockMask(true, quantizedC);
    } else {
        m_BaseLightOnObject->SetVisible(false);
        m_BaseLightOnSprite->SetClockMask(false, 0.0f);
    }

    // 4. Animation Logic (Sinking of the pusher block)
    float targetOffset = m_IsPressed ? -20.0f : 0.0f;
    float lerpSpeed = 5.0f;
    float dtSeconds = dt / 1000.0f;
    
    m_CurrentYOffset += (targetOffset - m_CurrentYOffset) * std::min(1.0f, lerpSpeed * dtSeconds);
    m_Transform.translation.y = m_InitialPosition.y + m_CurrentYOffset;

    // Update child pusher object to sink along with the button block
    m_PusherObject->m_Transform.translation = m_Transform.translation;
}
