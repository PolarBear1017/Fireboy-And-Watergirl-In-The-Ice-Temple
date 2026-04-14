#ifndef MECHANICS_BUTTON_HPP
#define MECHANICS_BUTTON_HPP

#include "Activator.hpp"
#include "AtlasSprite.hpp"
#include "SpriteAtlas.hpp"
#include <memory>
#include <glm/vec2.hpp>

class Button : public Activator {
public:
    Button(const std::shared_ptr<SpriteAtlas>& atlas, const glm::vec2& position, int groupId);
    
    void Update(const glm::vec2& fireboyPos, const glm::vec2& watergirlPos) override;
    bool IsActivated() const override { return m_IsPressed; }

    std::optional<Collider> GetCollider() const override;

private:
    std::shared_ptr<Util::GameObject> m_PusherObject;
    std::shared_ptr<AtlasSprite> m_Sprite;
    std::shared_ptr<AtlasSprite> m_BaseSprite;
    glm::vec2 m_Position;
    bool m_IsPressed = false;
};

#endif
