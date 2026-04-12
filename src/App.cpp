#include "App.hpp"

#include <algorithm>
#include <cmath>
#include <exception>
#include <string>
#include <vector>

#include "config.hpp"

#include "../include/Level/LevelLoader.hpp"
// #include "../include/Level/Levels.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

#include "Mechanics/Button.hpp"
#include "Mechanics/Lever.hpp"
#include "Mechanics/Elevator.hpp"
#include <unordered_map>


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

    m_MechAtlas = std::make_shared<SpriteAtlas>(
        BuildAtlasPath("MechAssets.png"),
        BuildAtlasPath("MechAssets.json")
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

// void App::BuildGameScene() {
//     ResetSceneRoot();
//
//     const auto background =
//         std::make_shared<Util::Image>(BuildImagePath("TempleHallIce.jpg"));
//     const auto backgroundScale = ComputeCoverScale(background->GetSize(),
//                                                    static_cast<float>(WINDOW_WIDTH),
//                                                    static_cast<float>(WINDOW_HEIGHT));
//     auto backgroundObject =
//         std::make_shared<Util::GameObject>(background, kBackgroundZ);
//     backgroundObject->m_Transform.scale = {backgroundScale, backgroundScale};
//     m_SceneRoot->AddChild(backgroundObject);
//
//     try {
//         // 優先從 JSON 檔載入關卡，這是新版關卡格式的主要入口。
//         const LevelDefinition level =
//             LoadLevelDefinitionFromJsonFile(BuildLevelPath("level1.json"));
//         if (!m_LevelManager->LoadLevel(level, m_SceneRoot)) {
//             throw std::runtime_error("Level validation failed after JSON load.");
//         }
//     } catch (const std::exception &e) {
//         // 若 JSON 載入失敗，退回程式內建的新版關卡資料，避免流程完全中斷。
//         LOG_ERROR(std::string("Failed to load JSON level: ") + e.what());
//         if (!m_LevelManager->LoadLevel(Levels::BuildLevel1Definition(), m_SceneRoot)) {
//             LOG_ERROR("Failed to load fallback built-in level definition.");
//         }
//     }
//
//     const auto &levelData = m_LevelManager->GetLevelData();
//     if (levelData.hasFireSpawn && levelData.hasWaterSpawn) {
//         LOG_INFO("Level parsed. Fire spawn=(" +
//                  std::to_string(levelData.fireSpawn.row) + "," +
//                  std::to_string(levelData.fireSpawn.col) + "), Water spawn=(" +
//                  std::to_string(levelData.waterSpawn.row) + "," +
//                  std::to_string(levelData.waterSpawn.col) + ")");
//     }
//
//     // Build the Fireboy character
//     m_FireBoy = std::make_shared<Character>(m_GameAtlas, Element::FIRE);
//     if (levelData.hasFireSpawn) {
//         m_FireBoy->m_Transform.translation =
//             m_LevelManager->TileToWorldPosition(levelData.fireSpawn.row,
//                                                 levelData.fireSpawn.col);
//     } else {
//         m_FireBoy->m_Transform.translation = {0.0F, 0.0F};
//     }
//     m_SceneRoot->AddChild(m_FireBoy);
//
//     // Build the Watergirl character
//     m_WaterGirl = std::make_shared<Character>(m_GameAtlas, Element::WATER);
//     if (levelData.hasFireSpawn) {
//         m_WaterGirl->m_Transform.translation =
//             m_LevelManager->TileToWorldPosition(levelData.waterSpawn.row,
//                                                 levelData.waterSpawn.col);
//     } else {
//         m_WaterGirl->m_Transform.translation = {0.0F, 0.0F};
//     }
//     m_SceneRoot->AddChild(m_WaterGirl);
//
//     // 假設你的地圖高度是透過 level 檔案讀取的（例如第 20 行是地板）
//     // 我們把門放在地圖底部偏中間的位置測試
//     const int groundRow = 14; // 這是地板的那一列索引
//     const int fireCol = 9;
//     const int waterCol = 28;
//
//     // 使用 LevelManager 幫我們算座標，保證門會乖乖站在格子點上
//     glm::vec2 fireDoorPos = m_LevelManager->TileToWorldPosition(groundRow, fireCol);
//     glm::vec2 waterDoorPos = m_LevelManager->TileToWorldPosition(groundRow, waterCol);
//
//     // 🌟 建立門的同時直接把算好的座標丟進去
//     m_FireDoor = std::make_shared<Door>(m_TempleAtlas, Element::FIRE, fireDoorPos);
//     m_WaterDoor = std::make_shared<Door>(m_TempleAtlas, Element::WATER, waterDoorPos);
//
//     // 最後別忘了加進場景樹
//     m_SceneRoot->AddChild(m_FireDoor);
//     m_SceneRoot->AddChild(m_WaterDoor);
//
//     LOG_INFO("Entered Level 1.");
// }

void App::BuildGameScene() {
    ResetSceneRoot();

    // 1. 建立背景
    const auto background = std::make_shared<Util::Image>(BuildImagePath("TempleHallIce.jpg"));
    const auto backgroundScale = ComputeCoverScale(background->GetSize(),
                                                   static_cast<float>(WINDOW_WIDTH),
                                                   static_cast<float>(WINDOW_HEIGHT));
    auto backgroundObject = std::make_shared<Util::GameObject>(background, kBackgroundZ);
    backgroundObject->m_Transform.scale = {backgroundScale, backgroundScale};
    m_SceneRoot->AddChild(backgroundObject);
    const float bgTileSize = 512.0F;
    const int cols = (WINDOW_WIDTH / static_cast<int>(bgTileSize)) + 2;
    const int rows = (WINDOW_HEIGHT / static_cast<int>(bgTileSize)) + 2;

    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < rows; ++j) {
            auto bgSprite =
                std::make_shared<AtlasSprite>(m_TempleAtlas, "BackGround0000");
            auto bgObject =
                std::make_shared<Util::GameObject>(bgSprite, kBackgroundZ);

            float x = -static_cast<float>(WINDOW_WIDTH) / 2.0F + (bgTileSize / 2.0F) +
                      static_cast<float>(i) * bgTileSize;
            float y = static_cast<float>(WINDOW_HEIGHT) / 2.0F - (bgTileSize / 2.0F) -
                      static_cast<float>(j) * bgTileSize;

            bgObject->m_Transform.translation = {x, y};
            m_SceneRoot->AddChild(bgObject);
        }
    }

    m_Activators.clear();
    m_Receivers.clear();

    // 2. 載入 JSON 關卡 (移除 try-catch，強制依賴 JSON)
    // 這樣如果沒讀到，程式就會明確報錯，方便你除錯
    const LevelDefinition level = LoadLevelDefinitionFromJsonFile(BuildLevelPath("level1.json")); // 記得換成你真正的檔名
    if (!m_LevelManager->LoadLevel(level, m_SceneRoot)) {
        LOG_ERROR("Level validation failed after JSON load.");
        return; // 直接中斷，不要硬跑
    }

    const auto &levelData = m_LevelManager->GetLevelData();

    // 3. 建立火娃與水娃 (動態讀取 JSON 算好的座標)
    m_FireBoy = std::make_shared<Character>(m_GameAtlas, Element::FIRE);
    if (levelData.hasFireSpawn) {
        m_FireBoy->m_Transform.translation = m_LevelManager->TileToWorldPosition(levelData.fireSpawn.row, levelData.fireSpawn.col);
    }
    m_SceneRoot->AddChild(m_FireBoy);

    // Build the Watergirl character
    m_WaterGirl = std::make_shared<Character>(m_GameAtlas, Element::WATER);
    if (levelData.hasWaterSpawn) {
        m_WaterGirl->m_Transform.translation = m_LevelManager->TileToWorldPosition(levelData.waterSpawn.row, levelData.waterSpawn.col);
    if (levelData.hasWaterSpawn) {
        m_WaterGirl->m_Transform.translation =
            m_LevelManager->TileToWorldPosition(levelData.waterSpawn.row,
                                                 levelData.waterSpawn.col);
    } else {
        m_WaterGirl->m_Transform.translation = {0.0F, 0.0F};
    }
    m_SceneRoot->AddChild(m_WaterGirl);

    // 4. 建立過關的門 (動態讀取 JSON 算好的座標，不再寫死 row = 14)
    if (levelData.hasFireDoor) {
        glm::vec2 fireDoorPos = m_LevelManager->TileToWorldPosition(levelData.fireDoor.row, levelData.fireDoor.col);
        m_FireDoor = std::make_shared<Door>(m_TempleAtlas, Element::FIRE, fireDoorPos);
        m_SceneRoot->AddChild(m_FireDoor);
    }

    if (levelData.hasWaterDoor) {
        glm::vec2 waterDoorPos = m_LevelManager->TileToWorldPosition(levelData.waterDoor.row, levelData.waterDoor.col);
        m_WaterDoor = std::make_shared<Door>(m_TempleAtlas, Element::WATER, waterDoorPos);
        m_SceneRoot->AddChild(m_WaterDoor);
    }
    // 使用 LevelManager 的資料動態建立大門
    if (levelData.hasFireDoor) {
        glm::vec2 fireDoorPos = m_LevelManager->TileToWorldPosition(
            levelData.fireDoor.row, levelData.fireDoor.col);
        m_FireDoor =
            std::make_shared<Door>(m_TempleAtlas, Element::FIRE, fireDoorPos);
        m_SceneRoot->AddChild(m_FireDoor);
    }

    if (levelData.hasWaterDoor) {
        glm::vec2 waterDoorPos = m_LevelManager->TileToWorldPosition(
            levelData.waterDoor.row, levelData.waterDoor.col);
        m_WaterDoor =
            std::make_shared<Door>(m_TempleAtlas, Element::WATER, waterDoorPos);
        m_SceneRoot->AddChild(m_WaterDoor);
    }

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
    if (m_WaterGirl != nullptr) {
        m_WaterGirl->Update();
        m_CollisionSystem.ResolveCharacterTerrain(*m_WaterGirl, *m_LevelManager);
    }

    // 🌟 呼叫大門的邏輯：把玩家目前的精確座標餵給大門！
    // (注意：如果你們 Character 沒有 GetPosition() 函數，請直接用 m_Transform.translation)
    if (m_FireDoor != nullptr && m_FireBoy != nullptr) {
        m_FireDoor->Update(m_FireBoy->m_Transform.translation);
    }
    if (m_WaterDoor != nullptr && m_WaterGirl != nullptr) {
        m_WaterDoor->Update(m_WaterGirl->m_Transform.translation);
    }

    // 呼叫機關邏輯 (Polymorphic Update)
    glm::vec2 fPos = m_FireBoy ? m_FireBoy->GetPosition() : glm::vec2(0.0f);
    glm::vec2 wPos = m_WaterGirl ? m_WaterGirl->GetPosition() : glm::vec2(0.0f);

    std::unordered_map<int, bool> groupStates;
    for (auto& activator : m_Activators) {
        activator->Update(fPos, wPos);
        if (activator->IsActivated()) {
            groupStates[activator->GetGroupId()] = true;
        }
    }

    for (auto& receiver : m_Receivers) {
        bool isOn = groupStates[receiver->GetGroupId()];
        receiver->SetActivated(isOn);
        receiver->Update();
    }

    // 第二階段：處理機關碰撞 (動態平台)
    std::vector<std::shared_ptr<BaseMechanism>> allMechs;
    for (auto& a : m_Activators) allMechs.push_back(a);
    for (auto& r : m_Receivers) allMechs.push_back(r);

    if (m_FireBoy) m_CollisionSystem.ResolveCharacterMechanics(*m_FireBoy, allMechs);
    if (m_WaterGirl) m_CollisionSystem.ResolveCharacterMechanics(*m_WaterGirl, allMechs);
}
