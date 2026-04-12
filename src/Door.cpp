#include "Door.hpp"
#include <sstream>
#include <iomanip>
#include <glm/geometric.hpp>

Door::Door(const std::shared_ptr<SpriteAtlas>& atlas, const Element element, const glm::vec2& pos)
    : m_Element(element) {
    SetZIndex(5);

    m_Transform.translation = pos;
    m_Transform.scale = {0.65f, 0.65f};

    const std::string prefix = (m_Element == Element::FIRE) ? "FinishBoy" : "FinishGirl";
    m_Sprite = std::make_shared<AtlasSprite>(atlas,  prefix + "0000");

    SetDrawable(m_Sprite);
}

void Door::Update(const glm::vec2& playerPos) {
    // 計算與玩家的距離
    float dist = glm::distance(m_Transform.translation, playerPos);

    // 判斷玩家是不是進入了門的感應圈
    if (dist < m_ActivationRadius) {
        if (m_State == DoorState::CLOSED || m_State == DoorState::CLOSING) {
            m_State = DoorState::OPENING;
        }
    } else {
        if (m_State == DoorState::OPEN || m_State == DoorState::OPENING) {
            m_State = DoorState::CLOSING;
        }
    }

    UpdateAnimation();
}

void Door::UpdateAnimation() {
    if (m_State == DoorState::CLOSED || m_State == DoorState::OPEN) {
        return;
    }

    m_AnimationTimer += 0.016f;
    if (m_AnimationTimer >= 0.03f) {
        m_AnimationTimer = 0.0f;

        if (m_State == DoorState::OPENING) {
            m_AnimationFrame++;

            if (m_AnimationFrame >= m_MaxFrames - 1) {
                m_AnimationFrame = m_MaxFrames - 1;
                m_State = DoorState::OPEN;
            }
        } else if (m_State == DoorState::CLOSING) {
            m_AnimationFrame--;

            if (m_AnimationFrame <= 0) {
                m_AnimationFrame = 0;
                m_State = DoorState::CLOSED;
            }
        }

        std::string prefix = (m_Element == Element::FIRE) ? "FinishBoy" : "FinishGirl";
        std::ostringstream ss;
        ss << prefix << std::setw(4) << std::setfill('0') << m_AnimationFrame;

        m_Sprite->SetFrame(ss.str());
    }
}