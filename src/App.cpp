#include "App.hpp"

#include <algorithm>
#include <cmath>
#include <exception>
#include <string>
#include <vector>

#include "config.hpp"

#include "LevelLoader.hpp"
#include "Levels.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"


namespace {
constexpr float kBackgroundZ = -10.0F;
constexpr float kDecorationZ = -5.0F;
constexpr float kTitleZ = 0.0F;
constexpr float kButtonZ = 5.0F;

std::string BuildReferencePath() {
    return std::string(RESOURCE_DIR) + "/reference/fireboy_and_watergirl_3";
}

std::string BuildAtlasPath(const std::string &fileName) {
    return BuildReferencePath() + "/atlasses/" + fileName;
}

std::string BuildSpritePath(const std::string &fileName) {
    return std::string(RESOURCE_DIR) + "/sprites/" + fileName;
}

std::string BuildImagePath(const std::string &fileName) {
    return BuildReferencePath() + "/images/" + fileName;
}

std::string BuildLevelPath(const std::string &fileName) {
    return std::string(RESOURCE_DIR) + "/levels/" + fileName;
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

std::shared_ptr<Util::GameObject> MakeAtlasObject(
    const std::shared_ptr<AtlasSprite> &drawable, const glm::vec2 &translation,
    const float zIndex, const float scale) {
    auto object = std::make_shared<Util::GameObject>(drawable, zIndex);
    object->m_Transform.translation = translation;
    object->m_Transform.scale = {scale, scale};
    return object;
}
} // namespace

void App::Start() {
    LOG_TRACE("Start");

    m_MenuAtlas = std::make_shared<SpriteAtlas>(BuildAtlasPath("MenuAssets.png"),
                                                BuildAtlasPath("MenuAssets.json"));
    m_MenuBackgroundAtlas =
        std::make_shared<SpriteAtlas>(BuildAtlasPath("MenuBackgrounds.png"),
                                      BuildAtlasPath("MenuBackgrounds.json"));

    m_GameAtlas = std::make_shared<SpriteAtlas>(
        BuildAtlasPath("CharAssets.png"),
        BuildAtlasPath("CharAssets.json")
    );

    m_GroundAtlas = std::make_shared<SpriteAtlas>(BuildAtlasPath("GroundAssets.png"),
                                                  BuildAtlasPath("GroundAssets.json"));
    m_TempleAtlas = std::make_shared<SpriteAtlas>(BuildAtlasPath("TempleAssets.png"),
                                                  BuildAtlasPath("TempleAssets.json"));
    m_LevelManager = std::make_shared<LevelManager>(m_GroundAtlas);

    SwitchScene(Scene::Cover);
    m_CurrentState = State::UPDATE;
}

void App::Update() {
    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
        return;
    }

    switch (m_CurrentScene) {
        case Scene::Cover:
            UpdateCoverScene();
            break;

        case Scene::Game:
            UpdateGameScene();
            break;
    }

    m_Root.Update();
}

void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}

void App::SwitchScene(const Scene scene) {
    m_CurrentScene = scene;

    switch (scene) {
        case Scene::Cover:
            BuildCoverScene();
            break;

        case Scene::Game:
            BuildGameScene();
            break;
    }
}

void App::ResetSceneRoot() {
    m_Root = Util::Renderer{};
    m_SceneRoot = std::make_shared<Util::GameObject>();
    m_Root.AddChild(m_SceneRoot);
    m_StartButtonObject.reset();
    m_StartButtonSprite.reset();
    m_GamePlaceholderSprite.reset();
}

void App::BuildCoverScene() {
    ResetSceneRoot();

    const auto background =
        std::make_shared<Util::Image>(BuildImagePath("TempleHallIce.jpg"));
    const auto backgroundScale = ComputeCoverScale(background->GetSize(),
                                                   static_cast<float>(WINDOW_WIDTH),
                                                   static_cast<float>(WINDOW_HEIGHT));
    auto backgroundObject =
        std::make_shared<Util::GameObject>(background, kBackgroundZ);
    backgroundObject->m_Transform.scale = {backgroundScale, backgroundScale};
    m_SceneRoot->AddChild(backgroundObject);

    const auto beam =
        std::make_shared<Util::Image>(BuildImagePath("Beam.png"));
    const auto beamScale = ComputeContainScale(beam->GetSize(), 520.0F, 340.0F);
    auto beamObject = std::make_shared<Util::GameObject>(beam, kDecorationZ);
    beamObject->m_Transform.translation = {250.0F, -10.0F};
    beamObject->m_Transform.scale = {beamScale, beamScale};
    m_SceneRoot->AddChild(beamObject);

    const auto title = std::make_shared<Util::Image>(BuildImagePath("GameNameIce.png"));
    const auto titleScale = ComputeContainScale(title->GetSize(), 760.0F, 220.0F);
    auto titleObject = std::make_shared<Util::GameObject>(title, kTitleZ);
    titleObject->m_Transform.translation = {0.0F, 120.0F};
    titleObject->m_Transform.scale = {titleScale, titleScale};
    m_SceneRoot->AddChild(titleObject);

    m_StartButtonSprite =
        std::make_shared<AtlasSprite>(m_MenuAtlas, "StoneButton0000");
    m_StartButtonBaseScale =
        ComputeContainScale(m_StartButtonSprite->GetSize(), 320.0F, 110.0F);
    m_StartButtonObject =
        MakeAtlasObject(m_StartButtonSprite, {0.0F, -245.0F}, kButtonZ,
                        m_StartButtonBaseScale);
    m_SceneRoot->AddChild(m_StartButtonObject);

    LOG_INFO("Cover scene loaded. Press ENTER or SPACE to continue.");
}

void App::BuildGameScene() {
    ResetSceneRoot();

    const auto background =
        std::make_shared<Util::Image>(BuildImagePath("TempleHallIce.jpg"));
    const auto backgroundScale = ComputeCoverScale(background->GetSize(),
                                                   static_cast<float>(WINDOW_WIDTH),
                                                   static_cast<float>(WINDOW_HEIGHT));
    auto backgroundObject =
        std::make_shared<Util::GameObject>(background, kBackgroundZ);
    backgroundObject->m_Transform.scale = {backgroundScale, backgroundScale};
    m_SceneRoot->AddChild(backgroundObject);

    try {
        // 優先從 JSON 檔載入關卡，這是新版關卡格式的主要入口。
        const LevelDefinition level =
            LoadLevelDefinitionFromJsonFile(BuildLevelPath("level1.json"));
        if (!m_LevelManager->LoadLevel(level, m_SceneRoot)) {
            throw std::runtime_error("Level validation failed after JSON load.");
        }
    } catch (const std::exception &e) {
        // 若 JSON 載入失敗，先退回舊版字元地圖，避免目前流程完全中斷。
        LOG_ERROR(std::string("Failed to load JSON level: ") + e.what());
        if (!m_LevelManager->LoadLevel(Levels::kLevel1, m_SceneRoot)) {
            LOG_ERROR("Failed to load fallback character level.");
        }
    }

    const auto &levelData = m_LevelManager->GetLevelData();
    if (levelData.hasFireSpawn && levelData.hasWaterSpawn) {
        LOG_INFO("Level parsed. Fire spawn=(" +
                 std::to_string(levelData.fireSpawn.row) + "," +
                 std::to_string(levelData.fireSpawn.col) + "), Water spawn=(" +
                 std::to_string(levelData.waterSpawn.row) + "," +
                 std::to_string(levelData.waterSpawn.col) + ")");
    }

    // Build the Fireboy character
    m_FireBoy = std::make_shared<Character>(m_GameAtlas, Element::FIRE);
    if (levelData.hasFireSpawn) {
        m_FireBoy->m_Transform.translation =
            m_LevelManager->TileToWorldPosition(levelData.fireSpawn.row,
                                                levelData.fireSpawn.col);
    } else {
        m_FireBoy->m_Transform.translation = {0.0F, 0.0F};
    }
    m_SceneRoot->AddChild(m_FireBoy);

    LOG_INFO("Entered Level 1.");
}

void App::UpdateCoverScene() {
    if (m_StartButtonObject != nullptr && m_StartButtonSprite != nullptr) {
        const auto elapsedSeconds = Util::Time::GetElapsedTimeMs() / 1000.0F;
        const auto pulse = 1.0F + 0.04F * std::sin(elapsedSeconds * 4.0F);
        m_StartButtonObject->m_Transform.scale = {
            m_StartButtonBaseScale * pulse,
            m_StartButtonBaseScale * pulse,
        };

        const auto activeFrame =
            std::fmod(elapsedSeconds, 1.0F) < 0.5F ? "StoneButton0000"
                                                   : "StoneButton0001";
        m_StartButtonSprite->SetFrame(activeFrame);
    }

    if (Util::Input::IsKeyUp(Util::Keycode::RETURN) ||
        Util::Input::IsKeyUp(Util::Keycode::KP_ENTER) ||
        Util::Input::IsKeyUp(Util::Keycode::SPACE)) {
        SwitchScene(Scene::Game);
    }
}

void App::UpdateGameScene() {
    if (Util::Input::IsKeyUp(Util::Keycode::BACKSPACE)) {
        SwitchScene(Scene::Cover);
    }

    // --- 呼叫火娃的邏輯 ---
    if (m_FireBoy != nullptr) {
        m_FireBoy->Update();
        m_CollisionSystem.ResolveCharacterTerrain(*m_FireBoy, *m_LevelManager);
    }
}
