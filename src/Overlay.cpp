#include "Overlay.hpp"
#include <sstream>
#include <iomanip>

Overlay::Overlay(const std::shared_ptr<SpriteAtlas>& atlas, Element element, const glm::vec2& startPos, int width, float tileSize)
    : m_Element(element), m_Width(width) {

    SetZIndex((m_Element == Element::ICE) ? 1.0F : 15.0F);

    std::string prefix;
    if (element == Element::FIRE) prefix = "FireBox";
    else if (element == Element::WATER) prefix = "WaterBox";
    else if (element == Element::TOXIC) prefix = "GreenBox";
    else if (element == Element::ICE) prefix = "IceBox";

    for (int i = 0; i < width; ++i) {
        std::string positionName = "";

        if (width > 1) {
            if (i == 0) positionName = "Left";
            else if (i == width - 1) positionName = "Right";
        }

        std::string frameName = prefix + positionName + "0000";
        auto sprite = std::make_shared<AtlasSprite>(atlas, frameName);
        m_Sprites.push_back(sprite);

        // 包裝成子物件，並根據 i 往右推移座標
        auto childObj = std::make_shared<Util::GameObject>(sprite, GetZIndex());
        childObj->m_Transform.translation = {startPos.x + (i * tileSize), startPos.y};
        childObj->m_Transform.scale = glm::vec2(tileSize / 32.0F + 0.033F);
        AddChild(childObj);
    }
}

void Overlay::Update() {
    if (m_Element != Element::ICE) {
        UpdateAnimation();
    }
}

void Overlay::UpdateAnimation() {
    m_AnimationTimer += 0.016f;

    if (m_AnimationTimer >= 0.05f) {
        m_AnimationTimer = 0.0f;
        m_AnimationFrame = (m_AnimationFrame + 1) % m_MaxFrames;

        std::string prefix;
        if (m_Element == Element::FIRE) prefix = "FireBox";
        else if (m_Element == Element::WATER) prefix = "WaterBox";
        else if (m_Element == Element::TOXIC) prefix = "GreenBox";

        // 更新這個池子裡所有的圖片
        for (int i = 0; i < m_Width; ++i) {
            std::string positionName = "";
            if (m_Width > 1) {
                if (i == 0) positionName = "Left";
                else if (i == m_Width - 1) positionName = "Right";
            }

            std::ostringstream ss;
            ss << prefix << positionName << std::setw(4) << std::setfill('0') << m_AnimationFrame;

            m_Sprites[i]->SetFrame(ss.str());
        }
    }
}