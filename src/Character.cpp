#include "Character.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Time.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>
#include "config.hpp"
// #include "Util/Logger.hpp"

Character::Character(const std::shared_ptr<SpriteAtlas>& atlas, const Element element, std::unique_ptr<IInputController> inputCtrl)
    : m_Element(element), m_InputController(std::move(inputCtrl)) {

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

    m_StairsObject = std::make_shared<Util::GameObject>();
    m_StairsObject->SetZIndex(zIndex + 2);
    m_StairsSprite = std::make_shared<AtlasSprite>(atlas, prefix + "_stairs0000");
    m_StairsObject->SetDrawable(m_StairsSprite);
    m_StairsObject->m_Transform.scale = {0.0f, 0.0f}; // Hidden by default

    m_DeathObject = std::make_shared<Util::GameObject>();
    m_DeathObject->SetZIndex(zIndex + 3);
    m_DeathSprite = std::make_shared<AtlasSprite>(atlas, "death_smoke0000");
    m_DeathObject->SetDrawable(m_DeathSprite);
    m_DeathObject->m_Transform.scale = {0.0f, 0.0f}; // Hidden by default

    AddChild(m_HeadObject);
    AddChild(m_LegsObject);
    AddChild(m_StairsObject);
    AddChild(m_DeathObject);
}

void Character::Update() {
    if (m_IsDying) {
        m_AnimationTimer += Util::Time::GetDeltaTimeMs() / 1000.0f;
        if (m_AnimationTimer > 0.03f) { // ~30 fps
            m_AnimationFrame++;
            m_AnimationTimer = 0.0f;
        }

        int maxFrames = 45; // death_smoke0000 to death_smoke0044
        if (m_AnimationFrame >= maxFrames) {
            m_IsDying = false;
            m_IsDead = true;
            m_DeathObject->m_Transform.scale = {0.0f, 0.0f};
        } else {
            std::ostringstream ss_death;
            ss_death << "death_smoke" << std::setw(4) << std::setfill('0') << m_AnimationFrame;
            m_DeathSprite->SetFrame(ss_death.str());
            m_DeathObject->m_Transform.translation = m_Transform.translation + m_VisualOffset;
            float baseScale = 0.8f;
            m_DeathObject->m_Transform.scale = {m_Transform.scale.x, baseScale};
        }

        m_HeadObject->m_Transform.scale = {0.0f, 0.0f};
        m_LegsObject->m_Transform.scale = {0.0f, 0.0f};
        m_StairsObject->m_Transform.scale = {0.0f, 0.0f};
        return;
    }

    if (m_IsDead) {
        m_HeadObject->m_Transform.scale = {0.0f, 0.0f};
        m_LegsObject->m_Transform.scale = {0.0f, 0.0f};
        m_StairsObject->m_Transform.scale = {0.0f, 0.0f};
        m_DeathObject->m_Transform.scale = {0.0f, 0.0f};
        return;
    }

    if (m_IsEnteringDoor) {
        if (m_EnterDoorTimer > 0.0f) {
            m_EnterDoorTimer -= Util::Time::GetDeltaTimeMs() / 1000.0f;
            float progress = 1.0f - (m_EnterDoorTimer / 0.36f);
            if (progress > 1.0f) progress = 1.0f;
            float scale = 0.8f - (0.16f * progress); // Scale from 0.8 to ~0.64 (since base scale is 0.8)
            m_StairsObject->m_Transform.scale = {scale, scale};
        }
        
        // Update stairs animation (about 30 frames, play at ~25fps)
        m_AnimationTimer += Util::Time::GetDeltaTimeMs() / 1000.0f;
        if (m_AnimationTimer > 0.04f) {
            m_AnimationFrame++;
            m_AnimationTimer = 0.0f;
        }
        
        int maxFrames = 30; // both have exactly 30 frames (0000 to 0029)
        int currentStairsFrame = std::min(m_AnimationFrame, maxFrames - 1); // Stop at last frame
        
        std::string prefix = (m_Element == Element::FIRE) ? "fire" : "water";
        std::ostringstream ss_stairs;
        ss_stairs << prefix << "_stairs" << std::setw(4) << std::setfill('0') << currentStairsFrame;
        m_StairsSprite->SetFrame(ss_stairs.str());

        m_StairsObject->m_Transform.translation = m_Transform.translation;
        return; // Skip normal physics and animation
    }

    ProcessInput();
    ApplyGravity();

    m_Transform.translation += m_Velocity;

    UpdateAnimation();

    if (!m_Visible) {
        m_HeadObject->m_Transform.scale = {0.0f, 0.0f};
        m_LegsObject->m_Transform.scale = {0.0f, 0.0f};
        m_StairsObject->m_Transform.scale = {0.0f, 0.0f};
    } else {
        m_HeadObject->m_Transform.scale = m_Transform.scale;
        m_LegsObject->m_Transform.scale = m_Transform.scale;
    }

    glm::vec2 visualPos = m_Transform.translation + m_VisualOffset;
    m_LegsObject->m_Transform.translation = visualPos;
    m_HeadObject->m_Transform.translation = {visualPos.x, visualPos.y + 4.0f};
}

void Character::ProcessInput() {
    if (!m_InputEnabled || !m_InputController) return;
    m_RunningState = RunningState::Idle;

    float acceleration = (m_GroundState == GroundState::ICE) ? 0.2F : 1.0f;
    float maxSpeed;
    float friction = (m_GroundState == GroundState::ICE)? 0.1F : 0.8f;

    switch (m_GroundState) {
        case GroundState::AIR:
            maxSpeed = 5.5F;
            break;
        case GroundState::ICE:
            maxSpeed = (m_Element == Element::WATER) ? 1.0F: 4.5F;
            break;
        default:
            maxSpeed = 4.0F;
    }

    float axis = m_InputController->GetHorizontalAxis();
    if (axis < 0.0f) {
        m_Velocity.x -= acceleration;
        m_RunningState = RunningState::Left;
    } else if (axis > 0.0f) {
        m_Velocity.x += acceleration;
        m_RunningState = RunningState::Right;
    }

    if (m_InputController->IsJumpPressed() && IsGrounded()) {
        m_Velocity.y += m_JumpForce;
        m_GroundState = GroundState::AIR;
        if (axis < 0.0f) m_Velocity.x = -maxSpeed;
        if (axis > 0.0f) m_Velocity.x = maxSpeed;
    }

    if (m_Velocity.x > 0.0f) {
        m_Velocity.x -= friction;
        if (m_Velocity.x < 0.0f) m_Velocity.x = 0.0f;
    } else if (m_Velocity.x < 0.0f) {
        m_Velocity.x += friction;
        if (m_Velocity.x > 0.0f) m_Velocity.x = 0.0f;
    }


    if (m_Velocity.x > maxSpeed) m_Velocity.x = maxSpeed;
    else if (m_Velocity.x < -maxSpeed) m_Velocity.x = -maxSpeed;
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
    int legsMaxFrames;
    int headMaxFrames;

    if (m_RunningState != RunningState::Idle) {
        legsAction = "legs_running";
        legsMaxFrames = 8;
        headAction = "head_jumping";
        headMaxFrames = 11;
    } else {
        legsAction = "legs_idle";
        legsMaxFrames = 1;

        if (m_Velocity.y > 0.5f && !IsGrounded()) {
            headAction = "head_rising";
            headMaxFrames = (m_Element == Element::FIRE) ? 5 : 11;
        } else if (m_Velocity.y < -0.5f && !IsGrounded()) {
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

    if (m_RunningState == RunningState::Right) {
        m_Transform.scale.x = std::abs(m_Transform.scale.x);
    } else if (m_RunningState == RunningState::Left) {
        m_Transform.scale.x = -std::abs(m_Transform.scale.x);
    }
}

void Character::PlayEnterDoorAnimation(const glm::vec2& doorPos) {
    m_IsEnteringDoor = true;
    m_InputEnabled = false;
    m_Velocity = {0.0f, 0.0f};
    
    // Hide standard body
    m_HeadObject->SetVisible(false);
    m_LegsObject->SetVisible(false);
    m_Visible = false; // Prevents Update() from scaling them back up
    
    // Snap to door center, but offset downwards so the character's feet stay near the door base
    m_Transform.translation = {doorPos.x, doorPos.y - 12.0f};
    
    // Show stairs object
    m_StairsObject->SetVisible(true);
    m_StairsObject->m_Transform.scale = {0.8f, 0.8f}; // Start at base character scale
    
    m_AnimationFrame = 0;
    m_AnimationTimer = 0.0f;
    m_EnterDoorTimer = 0.36f;
}

void Character::Kill() {
    if (m_IsDying || m_IsDead) return;
    m_IsDying = true;
    m_InputEnabled = false;
    m_Velocity = {0.0f, 0.0f};
    m_Visible = false;
    m_AnimationFrame = 0;
    m_AnimationTimer = 0.0f;
    m_DeathObject->m_Transform.scale = {0.8f, 0.8f};
}

void Character::Respawn(const glm::vec2& position) {
    m_IsDying = false;
    m_IsDead = false;
    m_InputEnabled = true;
    m_Visible = true;
    m_Velocity = {0.0f, 0.0f};
    SetPosition(position);
    m_AnimationFrame = 0;
    m_AnimationTimer = 0.0f;
    m_DeathObject->m_Transform.scale = {0.0f, 0.0f};
    m_HeadObject->m_Transform.scale = m_Transform.scale;
    m_LegsObject->m_Transform.scale = m_Transform.scale;
    m_HeadObject->SetVisible(true);
    m_LegsObject->SetVisible(true);
}


