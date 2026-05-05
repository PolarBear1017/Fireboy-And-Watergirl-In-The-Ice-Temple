#ifndef OVERLAY_HPP
#define OVERLAY_HPP

#include "Util/GameObject.hpp"
#include "Element.hpp"
#include "AtlasSprite.hpp"
#include "SpriteAtlas.hpp"

class Overlay : public Util::GameObject {
private:
    Element m_Element;
    int m_Width;
    std::vector<std::shared_ptr<AtlasSprite>> m_Sprites;

    float m_AnimationTimer = 0.0f;
    int m_AnimationFrame = 0;
    int m_MaxFrames = 14;

    void UpdateAnimation();

public:
    Overlay(const std::shared_ptr<SpriteAtlas>& atlas, Element element,
            const glm::vec2& startPos, int width, float tileSize);

    void Update();
};


#endif //OVERLAY_HPP