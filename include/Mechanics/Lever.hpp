#ifndef MECHANICS_LEVER_HPP
#define MECHANICS_LEVER_HPP

#include "Activator.hpp"
#include "AtlasSprite.hpp"
#include "SpriteAtlas.hpp"
#include "Util/GameObject.hpp"
#include <memory>
#include <glm/vec2.hpp>

class TriggerMediator;

class Lever : public Activator {
public:
    Lever(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& position, int groupId, const std::shared_ptr<TriggerMediator>& mediator);

    void Update(const std::vector<glm::vec2>& interactorPositions) override;
    bool IsActivated() const override { return m_IsOn; }
    std::optional<Collider> GetCollider() const override;
    bool IsLever() const override { return true; }

private:
    std::shared_ptr<AtlasSprite> m_Sprite;
    std::shared_ptr<AtlasSprite> m_BaseSprite;
    std::shared_ptr<Util::GameObject> m_StickObject;

    std::shared_ptr<AtlasSprite> m_BaseLightSprite;
    std::shared_ptr<AtlasSprite> m_StickLightSprite;
    std::shared_ptr<Util::GameObject> m_BaseLightObj;
    std::shared_ptr<Util::GameObject> m_StickLightObj;
    
    glm::vec2 m_Position;
    bool m_IsOn = false;
    float m_Cooldown = 0.0f;
    float m_CurrentRotation = 0.0f;
    float m_TargetRotation = 0.0f;
};

#endif
