#ifndef MECHANICS_LEVER_HPP
#define MECHANICS_LEVER_HPP

#include "Activator.hpp"
#include "AtlasSprite.hpp"
#include "SpriteAtlas.hpp"
#include "Util/GameObject.hpp"
#include <memory>
#include <glm/vec2.hpp>

class Lever : public Activator {
public:
    Lever(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& position, int groupId);

    void Update(const glm::vec2& fireboyPos, const glm::vec2& watergirlPos) override;
    bool IsActivated() const override { return m_IsOn; }

private:
    std::shared_ptr<AtlasSprite> m_Sprite;
    std::shared_ptr<AtlasSprite> m_BaseSprite;
    std::shared_ptr<Util::GameObject> m_StickObject;
    
    glm::vec2 m_Position;
    bool m_IsOn = false;
    float m_Cooldown = 0.0f;
    float m_CurrentRotation = 0.0f;
    float m_TargetRotation = 0.0f;
};

#endif
