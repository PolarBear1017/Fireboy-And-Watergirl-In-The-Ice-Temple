#include "Mechanics/Elevator.hpp"
#include "Util/Time.hpp"
#include <cmath>
#include <glm/geometric.hpp>

Elevator::Elevator(const std::shared_ptr<SpriteAtlas> &atlas,
                   const glm::vec2 &startPos, const glm::vec2 &endPos,
                   const glm::vec2 &size, int groupId, bool isHorizontal)
    : Receiver(groupId), m_StartPos(startPos), m_EndPos(endPos), m_Size(size) {
  // Compute tile length
  m_TileLength = isHorizontal ? (size.x / 32.0f) : (size.y / 32.0f);

  // Initialize light overlay
  m_Sprite =
      std::make_shared<AtlasSprite>(atlas, "moving_platform_light_off0000");
  m_Sprite->SetColorTint(GetGroupColor(groupId));
  SetDrawable(m_Sprite);
  SetZIndex(-1.0f);

  m_Transform.translation = startPos;
  m_LastPosition = startPos;

  // Setup collision dimensions based on stone block bounds (height/thickness = 32.0f)
  if (isHorizontal) {
    m_Size = glm::vec2(m_TileLength * 32.0f, 32.0f);
  } else {
    m_Transform.rotation = glm::radians(90.0f);
    m_Size = glm::vec2(32.0f, m_TileLength * 32.0f);
  }

  // Scale of light sprite: scale.x = (L - 1.0f) / 3.0f, scale.y = 32.0f / 48.0f
  m_Transform.scale = glm::vec2((m_TileLength - 1.0f) / 3.0f, 32.0f / 48.0f);

  // Load stone components
  m_LeftCapSprite = std::make_shared<AtlasSprite>(atlas, "BarCapLeft0000");
  m_RightCapSprite = std::make_shared<AtlasSprite>(atlas, "BarCapRight0000");
  m_CenterSprite = std::make_shared<AtlasSprite>(atlas, "BarCenter0000");

  m_LeftCapObj = std::make_shared<Util::GameObject>(m_LeftCapSprite, -1.1f);
  m_RightCapObj = std::make_shared<Util::GameObject>(m_RightCapSprite, -1.1f);
  m_CenterObj = std::make_shared<Util::GameObject>(m_CenterSprite, -1.1f);

  m_LeftCapObj->m_Transform.scale = glm::vec2(1.0f, 32.0f / 48.0f);
  m_RightCapObj->m_Transform.scale = glm::vec2(1.0f, 32.0f / 48.0f);

  // Scale of center block: scale.x = (L - 2.0f) / 3.0f, scale.y = 32.0f / 48.0f
  m_CenterObj->m_Transform.scale = glm::vec2((m_TileLength - 2.0f) / 3.0f, 32.0f / 48.0f);

  // Add children
  AddChild(m_LeftCapObj);
  AddChild(m_RightCapObj);
  AddChild(m_CenterObj);
}

void Elevator::SetActivated(bool active) { m_IsOn = active; }

void Elevator::Update() {
  float dt = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;
  if (m_IsOn && m_Progress < 1.0f) {
    m_Progress += dt / m_Speed;
    if (m_Progress > 1.0f)
      m_Progress = 1.0f;
  } else if (!m_IsOn && m_Progress > 0.0f) {
    m_Progress -= dt / m_Speed;
    if (m_Progress < 0.0f)
      m_Progress = 0.0f;
  }

  // Update light state
  if (m_Progress > 0.0f || m_IsOn) {
    m_Sprite->SetFrame("moving_platform_light_on0000");
  } else {
    m_Sprite->SetFrame("moving_platform_light_off0000");
  }

  // Smoothstep or linear interpolation
  m_LastPosition = m_Transform.translation;
  m_Transform.translation = m_StartPos + (m_EndPos - m_StartPos) * m_Progress;

  UpdateChildren();
}

void Elevator::UpdateChildren() {
  float theta = m_Transform.rotation;
  float cosT = std::cos(theta);
  float sinT = std::sin(theta);
  float offset = (m_TileLength - 1.0f) * 16.0f;

  // Left Cap: local translation (-offset, 0)
  m_LeftCapObj->m_Transform.translation.x = m_Transform.translation.x + cosT * (-offset);
  m_LeftCapObj->m_Transform.translation.y = m_Transform.translation.y + sinT * (-offset);
  m_LeftCapObj->m_Transform.rotation = theta;

  // Right Cap: local translation (offset, 0)
  m_RightCapObj->m_Transform.translation.x = m_Transform.translation.x + cosT * offset;
  m_RightCapObj->m_Transform.translation.y = m_Transform.translation.y + sinT * offset;
  m_RightCapObj->m_Transform.rotation = theta;

  // Center: local translation (0, 0)
  m_CenterObj->m_Transform.translation = m_Transform.translation;
  m_CenterObj->m_Transform.rotation = theta;
}

void Elevator::SetPosition(const glm::vec2& newPos) {
  glm::vec2 direction = m_EndPos - m_StartPos;
  float len2 = glm::dot(direction, direction);
  if (len2 > 0.0001f) {
    m_Progress = glm::dot(newPos - m_StartPos, direction) / len2;
    m_Progress = glm::clamp(m_Progress, 0.0f, 1.0f);
  }
  m_Transform.translation = m_StartPos + direction * m_Progress;
  UpdateChildren();
}
