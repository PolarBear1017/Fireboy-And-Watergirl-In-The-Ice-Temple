#include "App.hpp"

#include "config.hpp"

#include "Util/GameObject.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

void App::Start() {
    LOG_TRACE("Start");

    const std::string atlasImagePath = std::string(RESOURCE_DIR) + "/sprites/atlas.png";
    const std::string atlasJsonPath = std::string(RESOURCE_DIR) + "/sprites/atlas.json";

    m_Atlas = std::make_shared<SpriteAtlas>(atlasImagePath, atlasJsonPath);
    m_FrameNames = m_Atlas->GetFrameNames();

    if (m_FrameNames.empty()) {
        LOG_ERROR("No available atlas frames. App enters END state.");
        m_CurrentState = State::END;
        return;
    }

    std::string initialFrame = m_FrameNames.front();
    if (m_Atlas->HasFrame("fire_head_idle0000")) {
        initialFrame = "fire_head_idle0000";
    } else if (m_Atlas->HasFrame("water_head_idle0000")) {
        initialFrame = "water_head_idle0000";
    }

    m_AtlasSprite = std::make_shared<AtlasSprite>(m_Atlas, initialFrame);
    m_AtlasObject = std::make_shared<Util::GameObject>(m_AtlasSprite, 0.0F);
    m_AtlasObject->m_Transform.scale = {4.0F, 4.0F};
    m_Root.AddChild(m_AtlasObject);

    LOG_INFO("Atlas loaded with {} frames.", m_FrameNames.size());
    LOG_INFO("Use LEFT/RIGHT key to switch frame.");
    auto startIt = std::find(m_FrameNames.begin(), m_FrameNames.end(), initialFrame);
    const auto startIndex =
        startIt == m_FrameNames.end()
            ? 0
            : static_cast<std::size_t>(std::distance(m_FrameNames.begin(), startIt));
    SetFrameByIndex(startIndex);

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    if (Util::Input::IsKeyUp(Util::Keycode::LEFT)) {
        ShiftFrame(-1);
    }
    if (Util::Input::IsKeyUp(Util::Keycode::RIGHT)) {
        ShiftFrame(1);
    }

    m_Root.Update();

    /*
     * Do not touch the code below as they serve the purpose for
     * closing the window.
     */
    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}

void App::ShiftFrame(const int delta) {
    if (m_FrameNames.empty()) {
        return;
    }
    const auto frameCount = static_cast<int>(m_FrameNames.size());
    auto next = static_cast<int>(m_CurrentFrameIndex) + delta;
    if (next < 0) {
        next = frameCount - 1;
    } else if (next >= frameCount) {
        next = 0;
    }
    SetFrameByIndex(static_cast<std::size_t>(next));
}

void App::SetFrameByIndex(const std::size_t index) {
    if (m_AtlasSprite == nullptr || m_FrameNames.empty()) {
        return;
    }
    m_CurrentFrameIndex = index % m_FrameNames.size();
    m_AtlasSprite->SetFrame(m_FrameNames[m_CurrentFrameIndex]);
    LOG_INFO("Current atlas frame: {}", m_FrameNames[m_CurrentFrameIndex]);
}
