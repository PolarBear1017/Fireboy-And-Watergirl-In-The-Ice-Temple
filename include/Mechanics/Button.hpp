#ifndef MECHANICS_BUTTON_HPP
#define MECHANICS_BUTTON_HPP

#include "Activator.hpp"
#include "AtlasSprite.hpp"
#include "SpriteAtlas.hpp"
#include <memory>
#include <glm/vec2.hpp>

class TriggerMediator;

class Button : public Activator {
public:
    Button(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& position, int groupId, const std::shared_ptr<TriggerMediator>& mediator);
    
    void Update(const std::vector<glm::vec2>& interactorPositions) override;
    bool IsActivated() const override { return m_IsPressed; }

    std::optional<Collider> GetCollider() const override;

private:
    std::shared_ptr<Util::GameObject> m_PusherObject;
    std::shared_ptr<AtlasSprite> m_Sprite;
    std::shared_ptr<AtlasSprite> m_BaseSprite;
    bool m_IsPressed = false;
    float m_CurrentYOffset = 0.0f;
    glm::vec2 m_InitialPosition;
};

#endif
