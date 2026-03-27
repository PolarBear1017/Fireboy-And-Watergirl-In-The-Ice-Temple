#include "Character.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>
// #include "Util/Logger.hpp"

Character::Character(const std::shared_ptr<SpriteAtlas>& atlas, const Element element)
    : m_Element(element) {

    SetZIndex(10);

    m_HeadObject = std::make_shared<Util::GameObject>();
    m_LegsObject = std::make_shared<Util::GameObject>();

    m_LegsObject->SetZIndex(10);
    m_HeadObject->SetZIndex(11);

    std::string prefix = (element == Element::FIRE) ? "fire" : "water";
    m_HeadSprite = std::make_shared<AtlasSprite>(atlas, prefix + "_head_idle0000");
    m_LegsSprite = std::make_shared<AtlasSprite>(atlas, prefix + "_legs_idle0000");
    m_HeadObject->SetDrawable(m_HeadSprite);
    m_LegsObject->SetDrawable(m_LegsSprite);

    AddChild(m_HeadObject);
    AddChild(m_LegsObject);
}

void Character::AddChildrenTo(const std::shared_ptr<Util::GameObject>& root) {
    root->AddChild(m_LegsObject);
    root->AddChild(m_HeadObject);
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

    UpdateAnimation();

    m_HeadObject->m_Transform.scale = m_Transform.scale;
    m_LegsObject->m_Transform.scale = m_Transform.scale;

    glm::vec2 visualPos = m_Transform.translation + m_VisualOffset;
    m_LegsObject->m_Transform.translation = visualPos;
    m_HeadObject->m_Transform.translation = {visualPos.x, visualPos.y + 5.0f};
}

void Character::ProcessInput() {
    m_Velocity.x = 0.0f;
    if (m_Element == Element::FIRE) {
        if (Util::Input::IsKeyPressed(Util::Keycode::LEFT)) {m_Velocity.x -= m_Speed;}
        if (Util::Input::IsKeyPressed(Util::Keycode::RIGHT)) {m_Velocity.x += m_Speed;}
        if (Util::Input::IsKeyPressed(Util::Keycode::UP) && IsGrounded()) {
            m_Velocity.y += m_JumpForce;
            m_GroundState = GroundState::AIR;
        }
    }
    if (m_Element == Element::WATER) {
        if (Util::Input::IsKeyPressed(Util::Keycode::A)) {m_Velocity.x -= m_Speed;}
        if (Util::Input::IsKeyPressed(Util::Keycode::D)) {m_Velocity.x += m_Speed;}
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

void Character::UpdateAnimation() {
    // std::string prefix = (m_Element == Element::FIRE) ? "fire" : "water";
    //
    // std::string action = "legs_idle";
    // int maxFrames = 1;
    //
    // if (!IsGrounded()) {
    //     action = "legs_idle";
    //     maxFrames = 1;
    // } else if (m_Velocity.x != 0.0f) {
    //     action = "legs_running";
    //     maxFrames = 8;
    // }
    //
    // m_AnimationTimer += 0.016f;
    // if (m_AnimationTimer > 0.1f) {
    //     m_AnimationFrame++;
    //     m_AnimationTimer = 0.0f;
    // }
    //
    // if (m_AnimationFrame >= maxFrames) {
    //     m_AnimationFrame = 0;
    // }
    //
    // std::ostringstream ss;
    // ss << prefix << "_" << action
    //    << std::setw(4) << std::setfill('0') << m_AnimationFrame;
    //
    // m_Sprite->SetFrame(ss.str());
    //
    // if (m_Velocity.x > 0) {
    //     m_Transform.scale.x = std::abs(m_Transform.scale.x);
    // } else if (m_Velocity.x < 0) {
    //     m_Transform.scale.x = -std::abs(m_Transform.scale.x);
    // }

    std::string prefix = (m_Element == Element::FIRE) ? "fire" : "water";
    std::string legsAction = "legs_idle";
    std::string headAction = "head_idle";
    int legsMaxFrames = 1;

    if (!IsGrounded()) {
        legsAction = "legs_idle";
        headAction = "head_idle";
    } else if (std::abs(m_Velocity.x) > 0.1f) {
        legsAction = "legs_running";
        headAction = "head_idle";
        legsMaxFrames = 8;
    }

    m_AnimationTimer += 0.016f;
    if (m_AnimationTimer > 0.08f) {
        m_AnimationFrame++;
        m_AnimationTimer = 0.0f;
    }
    if (m_AnimationFrame >= legsMaxFrames) { m_AnimationFrame = 0; }

    std::ostringstream ss_legs;
    ss_legs << prefix << "_" << legsAction << std::setw(4) << std::setfill('0') << m_AnimationFrame;
    m_LegsSprite->SetFrame(ss_legs.str());

    std::ostringstream ss_head;
    ss_head << prefix << "_" << headAction << "0000";
    m_HeadSprite->SetFrame(ss_head.str());

    if (m_Velocity.x > 0) { m_Transform.scale.x = std::abs(m_Transform.scale.x); }
    else if (m_Velocity.x < 0) { m_Transform.scale.x = -std::abs(m_Transform.scale.x); }
}
