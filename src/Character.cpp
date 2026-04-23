#include "Character.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>
#include "config.hpp"
// #include "Util/Logger.hpp"

Character::Character(const std::shared_ptr<SpriteAtlas>& atlas, const Element element)
    : m_Element(element) {

    const float zIndex = (element == Element::FIRE) ? 10.0F : 12.0F;
    SetZIndex(zIndex);

    m_Transform.scale = {0.8f, 0.8f};

    m_HeadObject = std::make_shared<Util::GameObject>();
    m_LegsObject = std::make_shared<Util::GameObject>();

    m_LegsObject->SetZIndex(zIndex);
    m_HeadObject->SetZIndex(zIndex + 1);

    std::string prefix = (element == Element::FIRE) ? "fire" : "water";
    m_HeadSprite = std::make_shared<AtlasSprite>(atlas, prefix + "_head_idle0000");
    m_LegsSprite = std::make_shared<AtlasSprite>(atlas, prefix + "_legs_idle0000");
    m_HeadObject->SetDrawable(m_HeadSprite);
    m_LegsObject->SetDrawable(m_LegsSprite);

    AddChild(m_HeadObject);
    AddChild(m_LegsObject);

    // std::string debugPath = std::string(RESOURCE_DIR) + "/reference/fireboy_and_watergirl_3/images/debug_red.png";
    // auto debugShape = std::make_shared<Util::Image>(debugPath);
    // m_DebugBox = std::make_shared<Util::GameObject>(debugShape, 9.0F);
    //
    // glm::vec2 collSize = this->GetCollisionSize();
    // m_DebugBox->m_Transform.scale = { collSize.x / 10.0F, collSize.y / 10.0F };
    // this->AddChild(m_DebugBox); // 留一個 AddChild 就好
}

void Character::AddChildrenTo(const std::shared_ptr<Util::GameObject>& root) {
    root->AddChild(m_LegsObject);
    root->AddChild(m_HeadObject);
}

void Character::Update() {
    ProcessInput();
    ApplyGravity();

    m_Transform.translation += m_Velocity;

    UpdateAnimation();

    if (!m_Visible) {
        m_HeadObject->m_Transform.scale = {0.0f, 0.0f};
        m_LegsObject->m_Transform.scale = {0.0f, 0.0f};
    } else {
        m_HeadObject->m_Transform.scale = m_Transform.scale;
        m_LegsObject->m_Transform.scale = m_Transform.scale;
    }

    glm::vec2 visualPos = m_Transform.translation + m_VisualOffset;
    m_LegsObject->m_Transform.translation = visualPos;
    m_HeadObject->m_Transform.translation = {visualPos.x, visualPos.y + 4.0f};

    // if (m_DebugBox) {
    //     glm::vec2 pos = this->GetPosition();
    //     glm::vec2 collSize = this->GetCollisionSize();
    //     m_DebugBox->m_Transform.translation = {pos.x, pos.y + (collSize.y / 2.0F) + 20.0F};
    // }
}

void Character::ProcessInput() {
    m_Velocity.x = 0.0f;
    if (!m_InputEnabled) return;

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

    std::string prefix = (m_Element == Element::FIRE) ? "fire" : "water";

    // Default Status: Idle
    std::string legsAction = "legs_idle";
    std::string headAction = "head_idle";
    int legsMaxFrames = 1;
    int headMaxFrames = (m_Element == Element::FIRE) ? 19 : 30;

    if (m_Velocity.x != 0) {
        legsAction = "legs_running";
        legsMaxFrames = 8;
        headAction = "head_jumping";
        headMaxFrames = 11;
    } else {
        legsAction = "legs_idle";
        legsMaxFrames = 1;

        if (m_Velocity.y > 0.1f && !IsGrounded()) {
            headAction = "head_rising";
            headMaxFrames = (m_Element == Element::FIRE) ? 5 : 11;
        } else if (m_Velocity.y < -0.1f && !IsGrounded()) {
            headAction = "head_falling";
            headMaxFrames = (m_Element == Element::FIRE) ? 5 : 11;
        } else {
            headAction = "head_idle";
            headMaxFrames = (m_Element == Element::FIRE) ? 19 : 30;
        }
    }

    // if (m_Velocity.x != 0 && m_Velocity.y != 0) {
    //     float tiltAngle = m_Velocity.y * 0.02f;
    //
    //     // 防呆：限制最大旋轉角度，避免速度太快時頭被扭斷
    //     if (tiltAngle > 0.3f) tiltAngle = 0.3f;
    //     if (tiltAngle < -0.3f) tiltAngle = -0.3f;
    //     // 關鍵：如果角色正面向左邊 (scale.x < 0)，旋轉方向要反過來，
    //     // 不然往左跳時，頭會變成「往後仰」而不是往前看！
    //     if (m_Transform.scale.x < 0) {
    //         tiltAngle = -tiltAngle;
    //     }
    //     // 把算出來的角度套用到頭的 GameObject 上
    //     m_HeadObject->m_Transform.rotation = tiltAngle;
    // } else {
    //     m_HeadObject->m_Transform.rotation = 0.0f;
    // }

    m_AnimationTimer += 0.016f;
    if (m_AnimationTimer > 0.05f) {
        m_AnimationFrame++;
        m_AnimationTimer = 0.0f;
    }

    int currentLegsFrame = m_AnimationFrame % legsMaxFrames;
    int currentHeadFrame = m_AnimationFrame % headMaxFrames;

    std::ostringstream ss_legs;
    ss_legs << prefix << "_" << legsAction << std::setw(4) << std::setfill('0') << currentLegsFrame;
    m_LegsSprite->SetFrame(ss_legs.str());

    std::ostringstream ss_head;
    ss_head << prefix << "_" << headAction << std::setw(4) << std::setfill('0') << currentHeadFrame;
    m_HeadSprite->SetFrame(ss_head.str());

    if (m_Velocity.x > 0) {
        m_Transform.scale.x = std::abs(m_Transform.scale.x);
    } else if (m_Velocity.x < 0) {
        m_Transform.scale.x = -std::abs(m_Transform.scale.x);
    }
}
