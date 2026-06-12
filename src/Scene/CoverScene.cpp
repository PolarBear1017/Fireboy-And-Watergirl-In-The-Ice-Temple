#include "Scene/CoverScene.hpp"
#include "App.hpp"
#include "Scene/MapScene.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"
#include "config.hpp"
#include <cmath>

namespace {
constexpr float kBackgroundZ = -10.0F;
constexpr float kDecorationZ = -5.0F;
constexpr float kTitleZ = 0.0F;
constexpr float kButtonZ = 5.0F;

std::string BuildReferencePath() {
  return std::string(RESOURCE_DIR) + "/reference/fireboy_and_watergirl_3";
}

std::string BuildImagePath(const std::string &fileName) {
  return BuildReferencePath() + "/images/" + fileName;
}

float ComputeContainScale(const glm::vec2 &size, const float maxWidth,
                          const float maxHeight) {
  if (size.x <= 0.0F || size.y <= 0.0F) {
    return 1.0F;
  }
  return std::max(0.1F, std::min(maxWidth / size.x, maxHeight / size.y));
}

float ComputeCoverScale(const glm::vec2 &size, const float minWidth,
                        const float minHeight) {
  if (size.x <= 0.0F || size.y <= 0.0F) {
    return 1.0F;
  }
  return std::max(minWidth / size.x, minHeight / size.y);
}

std::shared_ptr<Util::GameObject>
MakeAtlasObject(const std::shared_ptr<AtlasSprite> &drawable,
                const glm::vec2 &translation, const float zIndex,
                const float scale) {
  auto object = std::make_shared<Util::GameObject>(drawable, zIndex);
  object->m_Transform.translation = translation;
  object->m_Transform.scale = {scale, scale};
  return object;
}
} // namespace

CoverScene::CoverScene(App& app) : m_App(app) {}

void CoverScene::Init(std::shared_ptr<Util::GameObject> sceneRoot) {
    m_SceneRoot = std::move(sceneRoot);

    const auto background = std::make_shared<Util::Image>(BuildImagePath("TempleHallIce.jpg"));
    const auto backgroundScale = ComputeCoverScale(background->GetSize(), static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT));
    auto backgroundObject = std::make_shared<Util::GameObject>(background, kBackgroundZ);
    backgroundObject->m_Transform.scale = {backgroundScale, backgroundScale};
    m_SceneRoot->AddChild(backgroundObject);

    const auto beamImage = std::make_shared<Util::Image>(BuildImagePath("Beam.png"));
    const float baseScale = ComputeContainScale(beamImage->GetSize(), 520.0F, 340.0F);
    const glm::vec2 beamSize = beamImage->GetSize();

    m_Beams.clear();
    m_BeamDatas.clear();
    for (int i = 0; i < 5; ++i) {
        auto beamObj = std::make_shared<Util::GameObject>(beamImage, kDecorationZ - (i * 0.1f));
        beamObj->SetPivot({0.0f, beamSize.y / 2.0f});
        beamObj->m_Transform.translation = {0.0F, 550.0F};

        BeamData data;
        data.speed = 0.3f + (static_cast<float>(rand()) / RAND_MAX) * 0.4f;
        data.offset = static_cast<float>(i) * 1.5f;
        data.baseScale = baseScale * (2.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.8f);
        beamObj->m_Transform.scale = {data.baseScale, data.baseScale};

        m_Beams.push_back(beamObj);
        m_BeamDatas.push_back(data);
        m_SceneRoot->AddChild(beamObj);
    }

    const auto title = std::make_shared<Util::Image>(BuildImagePath("GameNameIce.png"));
    const auto titleScale = ComputeContainScale(title->GetSize(), 760.0F, 220.0F);
    auto titleObject = std::make_shared<Util::GameObject>(title, kTitleZ);
    titleObject->m_Transform.translation = {0.0F, 120.0F};
    titleObject->m_Transform.scale = {titleScale, titleScale};
    m_SceneRoot->AddChild(titleObject);

    m_StartButtonSprite = std::make_shared<AtlasSprite>(m_App.GetMenuAtlas(), "StoneButton0000");
    m_StartButtonBaseScale = ComputeContainScale(m_StartButtonSprite->GetSize(), 320.0F, 110.0F);
    m_StartButtonObject = MakeAtlasObject(m_StartButtonSprite, {0.0F, -245.0F}, kButtonZ, m_StartButtonBaseScale);

    auto buildWord = [&](const std::string &word, glm::vec2 basePos, float scale,
                          std::vector<std::shared_ptr<Util::GameObject>> &targetVector, float spacing) {
        targetVector.clear();
        float totalWidth = (word.length() - 1) * spacing;
        float startX = -totalWidth / 2.0f;

        for (size_t i = 0; i < word.length(); ++i) {
            std::string letter(1, word[i]);
            if (letter == " ") {
                startX += spacing;
                continue;
            }
            auto sprite = std::make_shared<AtlasSprite>(m_App.GetFontAtlas(), letter);
            auto object = std::make_shared<Util::GameObject>(sprite, kButtonZ + 1.0F);
            object->m_Transform.translation = basePos + glm::vec2(startX, 0.0F);
            object->m_Transform.scale = {scale, scale};
            targetVector.push_back(object);
            m_SceneRoot->AddChild(object);
            startX += spacing;
        }
    };

    buildWord("PLAY", {0.0F, -40.0F}, 0.65F, m_PlayTextObjects, 45.0F);
    buildWord("MORE GAMES", {0.0F, -135.0F}, 0.38F, m_MoreGamesTextObjects, 28.0F);
    buildWord("WALKTHROUGH", {0.0F, -205.0F}, 0.38F, m_WalkthroughTextObjects, 28.0F);

    LOG_INFO("Cover scene loaded. Press ENTER or SPACE to continue.");
}

void CoverScene::Update() {
    const auto elapsedSeconds = Util::Time::GetElapsedTimeMs() / 1000.0F;

    for (size_t i = 0; i < m_Beams.size(); ++i) {
        const auto &data = m_BeamDatas[i];
        auto &beam = m_Beams[i];

        float baseAngle = glm::radians(-50.0f + i * 25.0f);
        float swing = 0.12f * std::sin(data.offset + elapsedSeconds * data.speed);
        beam->m_Transform.rotation = baseAngle + swing;

        float scalePulse = 0.4f + 0.3f * std::abs(std::sin(data.offset + elapsedSeconds * data.speed));
        beam->m_Transform.scale.x = data.baseScale * scalePulse;
        beam->m_Transform.scale.y = data.baseScale;

        beam->m_Transform.translation.y = 550.0f + 10.0f * std::sin(data.offset + elapsedSeconds * 0.5f);
    }

    if (Util::Input::IsKeyUp(Util::Keycode::RETURN) ||
        Util::Input::IsKeyUp(Util::Keycode::KP_ENTER) ||
        Util::Input::IsKeyUp(Util::Keycode::SPACE)) {
        m_App.SwitchScene(std::make_unique<MapScene>(m_App));
        return;
    }

    if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
        const auto cursor = Util::Input::GetCursorPosition();
        const float worldX = cursor.x;
        const float worldY = cursor.y;

        if (worldX >= -85.0F && worldX <= 85.0F && worldY >= -65.0F && worldY <= -15.0F) {
            LOG_INFO("PLAY clicked!");
            m_App.SwitchScene(std::make_unique<MapScene>(m_App));
        }
    }
}
