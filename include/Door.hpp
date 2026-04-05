#ifndef DOOR_HPP
#define DOOR_HPP

#include "Util/GameObject.hpp"
#include "Element.hpp"
#include "SpriteAtlas.hpp"
#include "AtlasSprite.hpp"
#include <glm/vec2.hpp>
#include <memory>
#include <string>

enum class DoorState {
    CLOSED,
    CLOSING,
    OPEN,
    OPENING
};

class Door : public Util::GameObject{
private:
    Element m_Element;
    DoorState m_State = DoorState::CLOSED;
    std::shared_ptr<AtlasSprite> m_Sprite;
    float m_ActivationRadius = 60.0f;

    int m_AnimationFrame = 0;
    float m_AnimationTimer = 0.0f;
    int m_MaxFrames = 22;

    void UpdateAnimation();

public:
    Door(const std::shared_ptr<SpriteAtlas>& atlas, Element element,const glm::vec2& pos = {0.0f, 0.0f});

    void Update(const glm::vec2& playerPos);

    [[nodiscard]] bool IsFullyOpen() const { return m_State == DoorState::OPEN; }
    [[nodiscard]] Element GetElement() const { return m_Element; }
};


#endif //DOOR_HPP