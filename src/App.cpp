#include "App.hpp"

#include <algorithm>

#include "config.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

namespace {
std::vector<App::PreviewDefinition> BuildPreviewDefinitions() {
    const auto basePath = std::string(RESOURCE_DIR);
    const auto referencePath =
        basePath + "/reference/fireboy_and_watergirl_3";
    const auto atlasPath = referencePath + "/atlasses";
    const auto imagePath = referencePath + "/images";

    return {
        {App::PreviewKind::Atlas, "char_assets", atlasPath + "/CharAssets.png",
         atlasPath + "/CharAssets.json"},
        {App::PreviewKind::Atlas, "ground_assets", atlasPath + "/GroundAssets.png",
         atlasPath + "/GroundAssets.json"},
        {App::PreviewKind::Atlas, "mechanics_assets", atlasPath + "/MechAssets.png",
         atlasPath + "/MechAssets.json"},
        {App::PreviewKind::Atlas, "menu_assets", atlasPath + "/MenuAssets.png",
         atlasPath + "/MenuAssets.json"},
        {App::PreviewKind::Atlas, "menu_backgrounds",
         atlasPath + "/MenuBackgrounds.png",
         atlasPath + "/MenuBackgrounds.json"},
        {App::PreviewKind::Atlas, "popup_assets", atlasPath + "/PopupAssets.png",
         atlasPath + "/PopupAssets.json"},
        {App::PreviewKind::Atlas, "ice_temple_assets",
         atlasPath + "/TempleAssets.png", atlasPath + "/TempleAssets.json"},
        {App::PreviewKind::Image, "beam", imagePath + "/Beam.png", ""},
        {App::PreviewKind::Image, "game_name_ice",
         imagePath + "/GameNameIce.png", ""},
        {App::PreviewKind::Image, "temple_hall_ice",
         imagePath + "/TempleHallIce.jpg", ""},
    };
}
} // namespace

void App::Start() {
    LOG_TRACE("Start");

    m_PreviewDefinitions = BuildPreviewDefinitions();
    if (m_PreviewDefinitions.empty()) {
        LOG_ERROR("No preview definitions configured. App enters END state.");
        m_CurrentState = State::END;
        return;
    }

    m_PreviewObject = std::make_shared<Util::GameObject>();
    m_Root.AddChild(m_PreviewObject);

    LoadPreviewByIndex(0);
    LOG_INFO("Use LEFT/RIGHT key to switch frame.");
    LOG_INFO("Use UP/DOWN key to switch preview asset.");

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    if (Util::Input::IsKeyUp(Util::Keycode::LEFT)) {
        ShiftFrame(-1);
    }
    if (Util::Input::IsKeyUp(Util::Keycode::RIGHT)) {
        ShiftFrame(1);
    }
    if (Util::Input::IsKeyUp(Util::Keycode::UP)) {
        ShiftPreview(-1);
    }
    if (Util::Input::IsKeyUp(Util::Keycode::DOWN)) {
        ShiftPreview(1);
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

void App::LoadPreviewByIndex(const std::size_t index) {
    if (m_PreviewDefinitions.empty()) {
        return;
    }

    m_CurrentPreviewIndex = index % m_PreviewDefinitions.size();
    const auto &definition = m_PreviewDefinitions[m_CurrentPreviewIndex];
    m_FrameNames.clear();
    m_CurrentFrameIndex = 0;

    if (definition.kind == PreviewKind::Atlas) {
        m_ImagePreview.reset();
        m_Atlas = std::make_shared<SpriteAtlas>(definition.imagePath, definition.jsonPath);
        m_FrameNames = m_Atlas->GetFrameNames();

        if (m_FrameNames.empty()) {
            LOG_ERROR("Atlas '{}' has no available frames.", definition.name);
            return;
        }

        const auto initialFrame = FindInitialFrame(*m_Atlas, m_FrameNames);
        m_AtlasSprite = std::make_shared<AtlasSprite>(m_Atlas, initialFrame);
        if (m_PreviewObject != nullptr) {
            m_PreviewObject->SetDrawable(m_AtlasSprite);
        }

        const auto startIt =
            std::find(m_FrameNames.begin(), m_FrameNames.end(), initialFrame);
        const auto startIndex =
            startIt == m_FrameNames.end()
                ? 0
                : static_cast<std::size_t>(std::distance(m_FrameNames.begin(), startIt));

        ApplyPreviewScale(m_AtlasSprite->GetSize());
        LOG_INFO("Atlas '{}' loaded with {} frames.", definition.name,
                 m_FrameNames.size());
        SetFrameByIndex(startIndex);
        return;
    }

    m_AtlasSprite.reset();
    m_Atlas.reset();
    m_ImagePreview = std::make_shared<Util::Image>(definition.imagePath);
    if (m_PreviewObject != nullptr) {
        m_PreviewObject->SetDrawable(m_ImagePreview);
    }
    ApplyPreviewScale(m_ImagePreview->GetSize());
    LOG_INFO("Image '{}' loaded.", definition.name);
}

std::string App::FindInitialFrame(const SpriteAtlas &atlas,
                                  const std::vector<std::string> &frameNames) {
    static const std::array<const char *, 8> preferredFrames = {
        "fire_head_idle0000",
        "water_head_idle0000",
        "FireBoy0000",
        "WaterGirl0000",
        "BackGround0000",
        "Block0000",
        "Button0000",
        "_default0000",
    };

    for (const auto *name : preferredFrames) {
        if (atlas.HasFrame(name)) {
            return name;
        }
    }

    return frameNames.front();
}

void App::ApplyPreviewScale(const glm::vec2 &size) {
    if (m_PreviewObject == nullptr || size.x <= 0.0F || size.y <= 0.0F) {
        return;
    }

    constexpr float maxPreviewWidth = WINDOW_WIDTH * 0.8F;
    constexpr float maxPreviewHeight = WINDOW_HEIGHT * 0.8F;

    const auto scaleX = maxPreviewWidth / size.x;
    const auto scaleY = maxPreviewHeight / size.y;
    const auto uniformScale = std::max(0.1F, std::min(scaleX, scaleY));

    m_PreviewObject->m_Transform.translation = {0.0F, 0.0F};
    m_PreviewObject->m_Transform.scale = {uniformScale, uniformScale};
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

void App::ShiftPreview(const int delta) {
    if (m_PreviewDefinitions.empty()) {
        return;
    }

    const auto previewCount = static_cast<int>(m_PreviewDefinitions.size());
    auto next = static_cast<int>(m_CurrentPreviewIndex) + delta;
    if (next < 0) {
        next = previewCount - 1;
    } else if (next >= previewCount) {
        next = 0;
    }

    LoadPreviewByIndex(static_cast<std::size_t>(next));
}

void App::SetFrameByIndex(const std::size_t index) {
    if (m_AtlasSprite == nullptr || m_FrameNames.empty()) {
        return;
    }
    m_CurrentFrameIndex = index % m_FrameNames.size();
    m_AtlasSprite->SetFrame(m_FrameNames[m_CurrentFrameIndex]);
    ApplyPreviewScale(m_AtlasSprite->GetSize());
    LOG_INFO("Atlas '{}' frame: {}", m_PreviewDefinitions[m_CurrentPreviewIndex].name,
             m_FrameNames[m_CurrentFrameIndex]);
}
