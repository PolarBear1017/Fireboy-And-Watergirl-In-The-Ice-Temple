
#include "Character.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

Character::Character(const std::string &imagePath, const Element element) : m_Element(element){
    SetDrawable(std::make_shared<Util::Image>(imagePath));
    SetZIndex(10);
}

void Character::Update() {
    ProcessInput();
    ApplyGravity();

    m_Transform.translation += m_Velocity;
}

void Character::ProcessInput() {
    m_Velocity.x = 0.0f;
    if (Util::Input::IsKeyDown(Util::Keycode::A)) {
        m_Velocity.x -= 5.0f;
    }
    if (Util::Input::IsKeyDown(Util::Keycode::D)) {
        m_Velocity.x += 5.0f;
    }
    if (Util::Input::IsKeyPressed(Util::Keycode::W) && m_IsGrounded) {
        m_Velocity.y += m_JumpForce;
        m_IsGrounded = false;
    }
}


void Character::ApplyGravity() {
    if (!m_IsGrounded) {
        m_Velocity.y -= m_Gravity;
    }
}
