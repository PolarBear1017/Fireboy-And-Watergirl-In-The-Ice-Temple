#ifndef MECHANICS_TIMED_BUTTON_HPP
#define MECHANICS_TIMED_BUTTON_HPP

#include "Activator.hpp"
#include "AtlasSprite.hpp"
#include "SpriteAtlas.hpp"
#include <memory>
#include <glm/vec2.hpp>

class TriggerMediator;

class TimedButton : public Activator {
public:
    TimedButton(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& position, int groupId, float durationMs, const std::shared_ptr<TriggerMediator>& mediator);
    
    void Update(const std::vector<glm::vec2>& interactorPositions) override;
    bool IsActivated() const override { return m_IsActive; }

    std::optional<Collider> GetCollider() const override;

private:
    // Moving button block (standard button graphics, gold)
    std::shared_ptr<Util::GameObject> m_PusherObject;
    std::shared_ptr<AtlasSprite> m_Sprite;
    std::shared_ptr<AtlasSprite> m_BaseSprite; // pusher_block0000

    // Static background clock dial
    std::shared_ptr<Util::GameObject> m_BasePanelObject;
    std::shared_ptr<AtlasSprite> m_BasePanelSprite; // timed_pusher_base0000

    std::shared_ptr<Util::GameObject> m_BaseLightOffObject;
    std::shared_ptr<AtlasSprite> m_BaseLightOffSprite; // timed_pusher_light_off0000

    std::shared_ptr<Util::GameObject> m_BaseLightOnObject;
    std::shared_ptr<AtlasSprite> m_BaseLightOnSprite; // timed_pusher_light_on0000

    bool m_IsPressed = false;      // Is currently physically pressed
    bool m_IsActive = false;       // Is the mechanism currently active
    float m_DurationMs = 5000.0f;
    float m_TimeRemainingMs = 0.0f;

    float m_CurrentYOffset = 0.0f;
    glm::vec2 m_InitialPosition;
};

#endif
