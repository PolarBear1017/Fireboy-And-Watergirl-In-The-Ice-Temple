#ifndef OVERLAY_HPP
#define OVERLAY_HPP

#include "Util/GameObject.hpp"
#include "Element.hpp"
#include "AtlasSprite.hpp"
#include "SpriteAtlas.hpp"

class Overlay : public Util::GameObject {
private:
    int m_Row = 0;
    int m_StartCol = 0;
    int m_Width;
    // Element m_Element;
    std::vector<Element> m_Elements;
    std::vector<std::shared_ptr<AtlasSprite>> m_Sprites;
    std::vector<std::shared_ptr<Util::GameObject>> m_Puddles;


    float m_AnimTimer = 0.0f;
    int m_AnimFrame = 0;
    int m_MaxFrames = 14;

    bool m_IsConverting = false;
    float m_ConvTimer = 0.0f;
    int m_ConvStartIndex = 0;
    int m_ConvRadius = 0;
    std::vector<int> m_NewlyConvertedIndex;

    std::shared_ptr<AtlasSprite> m_MistSprite;
    std::shared_ptr<Util::GameObject> m_MistLeft = nullptr;
    std::shared_ptr<Util::GameObject> m_MistRight = nullptr;

    void UpdateAnimation();
    void UpdateConversion();

public:
    Overlay(const std::shared_ptr<SpriteAtlas>& atlas, int row, int col, Element element,
            const glm::vec2& startPos, int width, float tileSize);

    [[nodiscard]] int GetRow() const { return m_Row; }
    [[nodiscard]] int GetStartCol() const { return m_StartCol; }

    void Update();
    // void SwitchWaterAndIceElement();
    void StartConversion(const glm::vec2& triggerPos);
    std::vector<int> ConsumeNewlyConvertedIndex(){
        std::vector<int> result = m_NewlyConvertedIndex;
        m_NewlyConvertedIndex.clear();
        return result;
    }
};


#endif //OVERLAY_HPP