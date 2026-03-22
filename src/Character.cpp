
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
        m_GroundState = GroundState::GROUND;
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
        if (Util::Input::IsKeyPressed(Util::Keycode::UP) && IsGrounded()) {
            m_Velocity.y += m_JumpForce;
            m_GroundState = GroundState::AIR;
        }
    }
    if (m_Element == Element::WATER) {
        if (Util::Input::IsKeyPressed(Util::Keycode::A)) {m_Velocity.x -= 5.0f;}
        if (Util::Input::IsKeyPressed(Util::Keycode::D)) {m_Velocity.x += 5.0f;}
        if (Util::Input::IsKeyPressed(Util::Keycode::W) && IsGrounded()) {
            m_Velocity.y += m_JumpForce;
            m_GroundState = GroundState::AIR;
        }
    }
}

void Character::ApplyGravity() {
    if (!IsGrounded()) {
        m_Velocity.y -= m_Gravity;
    }
}

