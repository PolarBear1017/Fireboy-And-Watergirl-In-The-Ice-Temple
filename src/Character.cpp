
#include "Character.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

Character::Character(const std::shared_ptr<AtlasSprite>& sprite, const Element element) : m_Element(element){
    SetDrawable(sprite);
    SetZIndex(10);
}

void Character::Update() {
    ProcessInput();
    ApplyGravity();

    m_Transform.translation += m_Velocity;

    if (m_Transform.translation.y <= -200.0f) {
        m_Transform.translation.y = -200.0f;
        m_Velocity.y = 0.0f;
        m_IsGrounded = true;
    }

    LOG_DEBUG("X: {}, Y: {}, Vx: {}",
              m_Transform.translation.x,
              m_Transform.translation.y,
              m_Velocity.x);
}

void Character::ProcessInput() {
    m_Velocity.x = 0.0f;
    if (m_Element == Element::FIRE) {
        if (Util::Input::IsKeyPressed(Util::Keycode::LEFT)) {m_Velocity.x -= 5.0f;}
        if (Util::Input::IsKeyPressed(Util::Keycode::RIGHT)) {m_Velocity.x += 5.0f;}
        if (Util::Input::IsKeyPressed(Util::Keycode::UP) && m_IsGrounded) {
            m_Velocity.y += m_JumpForce;
            m_IsGrounded = false;
        }
    }
    if (m_Element == Element::WATER) {
        if (Util::Input::IsKeyPressed(Util::Keycode::A)) {m_Velocity.x -= 5.0f;}
        if (Util::Input::IsKeyPressed(Util::Keycode::D)) {m_Velocity.x += 5.0f;}
        if (Util::Input::IsKeyPressed(Util::Keycode::W) && m_IsGrounded) {
            m_Velocity.y += m_JumpForce;
            m_IsGrounded = false;
        }
    }

}


void Character::ApplyGravity() {
    if (!m_IsGrounded) {
        m_Velocity.y -= m_Gravity;
    }
}
