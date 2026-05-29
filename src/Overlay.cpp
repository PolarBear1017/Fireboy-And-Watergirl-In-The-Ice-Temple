#include "Overlay.hpp"

Overlay::Overlay(const std::shared_ptr<SpriteAtlas>& atlas, int row, int col, Element element, const glm::vec2& startPos, int width, float tileSize)
    : m_Row(row), m_StartCol(col), m_Width(width) {

    m_Elements.assign(width, element);
    float zIndex = (element == Element::ICE) ? 1.0F : 15.0F;

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

        auto childObj = std::make_shared<Util::GameObject>(sprite, zIndex);
        childObj->m_Transform.translation = {startPos.x + (i * tileSize), startPos.y};
        childObj->m_Transform.scale = glm::vec2(tileSize / 32.0F + 0.033F);
        AddChild(childObj);
        m_Puddles.push_back(childObj);
    }
    m_MistSprite = std::make_shared<AtlasSprite>(atlas, "FreezeEffect0000");
}

// void Overlay::SwitchWaterAndIceElement() {
//     if (m_Element == Element::WATER) {
//         m_Element = Element::ICE;
//     } else if (m_Element == Element::ICE) {
//         m_Element = Element::WATER;
//     }
// }

void Overlay::Update() {
    UpdateAnimation();
    UpdateConversion();
}

void Overlay::UpdateAnimation() {
    m_AnimTimer += 0.016f;

    if (m_AnimTimer >= 0.04f) {
        m_AnimTimer = 0.0f;
        m_AnimFrame = (m_AnimFrame + 1) % m_MaxFrames;

        if (m_Elements[0] == Element::FIRE || m_Elements[0] == Element::TOXIC) {
            std::string prefix = (m_Elements[0] == Element::FIRE) ? "FireBox" : "GreenBox";

            for (int i = 0; i < m_Width; ++i) {
                std::string posName = "";
                if (m_Width > 1) {
                    if (i == 0) posName = "Left";
                    else if (i == m_Width - 1) posName = "Right";
                }

                char frameNum[5];
                snprintf(frameNum, sizeof(frameNum), "%04d", m_AnimFrame);
                m_Sprites[i]->SetFrame(prefix + posName + frameNum);
            }
            return;
        }

        for (int i = 0; i < m_Width; ++i) {
            std::string posName = "";
            if (m_Width > 1) {
                if (i == 0) posName = "Left";
                else if (i == m_Width - 1) posName = "Right";
            }

            if (m_Elements[i] == Element::WATER) {
                char frameNum[5];
                snprintf(frameNum, sizeof(frameNum), "%04d", m_AnimFrame);
                m_Sprites[i]->SetFrame("WaterBox" + posName + frameNum);

            } else if (m_Elements[i] == Element::ICE) {
                m_Sprites[i]->SetFrame("IceBox" + posName + "0000");
            }
        }
    }
}

void Overlay::StartConversion(const glm::vec2& triggerPos) {
    if (m_IsConverting || m_Elements[0] == Element::FIRE || m_Elements[0] == Element::TOXIC) {
        return;
    }

    m_IsConverting = true;
    m_ConvTimer = 0.0f;
    m_ConvRadius = 0;
    m_NewlyConvertedIndex.clear();

    // 計算起點 Index 利用第一個子物件取得這個水池最左的真實世界 X 座標
    float poolStartX = m_Puddles[0]->m_Transform.translation.x;

    // 計算觸發點相對於水池起點的距離，除以磚塊大小 (預設 32.0f) 得到 index
    m_ConvStartIndex = static_cast<int>(std::round((triggerPos.x - poolStartX) / 32.0f));

    // 邊界防護
    m_ConvStartIndex = std::max(0, std::min(m_ConvStartIndex, m_Width - 1));

    // Mist effects
    m_MistLeft = std::make_shared<Util::GameObject>(m_MistSprite, 20.0F);
    m_MistLeft->m_Transform.translation = m_Puddles[m_ConvStartIndex]->m_Transform.translation;
    m_MistLeft->m_Transform.scale = m_Puddles[m_ConvStartIndex]->m_Transform.scale * 0.6f;
    AddChild(m_MistLeft);

    m_MistRight = std::make_shared<Util::GameObject>(m_MistSprite, 20.0F);
    m_MistRight->m_Transform.translation = m_Puddles[m_ConvStartIndex]->m_Transform.translation;
    m_MistRight->m_Transform.scale = m_Puddles[m_ConvStartIndex]->m_Transform.scale * 0.6f;
    AddChild(m_MistRight);
}

void Overlay::UpdateConversion() {
    if (!m_IsConverting) return;

    m_ConvTimer += 0.016f;
    if (m_ConvTimer >= 0.12f) {
        m_ConvTimer = 0.0f;

        int leftIdx = m_ConvStartIndex - m_ConvRadius;
        int rightIdx = m_ConvStartIndex + m_ConvRadius;
        bool stillSpreading = false;

        if (leftIdx >= 0) {
            m_Elements[leftIdx] = (m_Elements[leftIdx] == Element::WATER) ? Element::ICE : Element::WATER;
            m_Puddles[leftIdx]->SetZIndex((m_Elements[leftIdx] == Element::ICE) ? 1.0F : 15.0F);
            if (m_MistLeft) m_MistLeft->m_Transform.translation = m_Puddles[leftIdx]->m_Transform.translation+ glm::vec2(-16.0f, 0.0f);
            m_NewlyConvertedIndex.push_back(leftIdx);
            stillSpreading = true;
        }
        if ((leftIdx != rightIdx) && (rightIdx < m_Width)) {
            m_Elements[rightIdx] = (m_Elements[rightIdx] == Element::WATER) ? Element::ICE : Element::WATER;
            m_Puddles[rightIdx]->SetZIndex((m_Elements[rightIdx] == Element::ICE) ? 1.0F : 15.0F);
            if (m_MistRight) m_MistRight->m_Transform.translation = m_Puddles[rightIdx]->m_Transform.translation + glm::vec2(16.0f, 0.0f);
            m_NewlyConvertedIndex.push_back(rightIdx);
            stillSpreading = true;
        }

        m_ConvRadius++;

        if (!stillSpreading) {
            m_IsConverting = false;

            if (m_MistLeft) { RemoveChild(m_MistLeft); m_MistLeft = nullptr; }
            if (m_MistRight) { RemoveChild(m_MistRight); m_MistRight = nullptr; }
        }
    }
}