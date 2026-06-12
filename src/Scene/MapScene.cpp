#include "Scene/MapScene.hpp"
#include <imgui.h>
#include "App.hpp"
#include "Scene/CoverScene.hpp"
#include "Scene/GameScene.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "config.hpp"
#include <algorithm>
#include <cmath>

namespace {
constexpr float kBackgroundZ = -10.0F;
constexpr float kDecorationZ = -5.0F;
constexpr float kButtonZ = 5.0F;
} // namespace

MapScene::MapScene(App& app) : m_App(app) {}

void MapScene::DrawPathLine(const glm::vec2& start, const glm::vec2& end) {
    auto lineSprite = std::make_shared<AtlasSprite>(m_App.GetTempleAtlas(), "MenuBackground0000");
    auto lineObj = std::make_shared<Util::GameObject>(lineSprite, kDecorationZ);
    
    glm::vec2 diff = end - start;
    float len = glm::length(diff);
    float angle = std::atan2(diff.y, diff.x);
    
    lineObj->m_Transform.translation = start + diff * 0.5f;
    lineObj->m_Transform.rotation = angle;
    lineObj->m_Transform.scale = { len / 512.0f, 6.0f / 512.0f };
    lineSprite->SetColorTint({ 0.60f, 0.65f, 0.67f, 1.0f });
    
    m_SceneRoot->AddChild(lineObj);
}

void MapScene::Init(std::shared_ptr<Util::GameObject> sceneRoot) {
    m_SceneRoot = std::move(sceneRoot);

    const float bgTileSize = 512.0F;
    const int cols = (WINDOW_WIDTH / static_cast<int>(bgTileSize)) + 2;
    const int rows = (WINDOW_HEIGHT / static_cast<int>(bgTileSize)) + 2;

    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < rows; ++j) {
            auto bgSprite = std::make_shared<AtlasSprite>(m_App.GetTempleAtlas(), "MenuBackground0000");
            auto bgObject = std::make_shared<Util::GameObject>(bgSprite, kBackgroundZ);

            float x = -static_cast<float>(WINDOW_WIDTH) / 2.0F + (bgTileSize / 2.0F) + static_cast<float>(i) * bgTileSize;
            float y = static_cast<float>(WINDOW_HEIGHT) / 2.0F - (bgTileSize / 2.0F) - static_cast<float>(j) * bgTileSize;

            bgObject->m_Transform.translation = {x, y};
            m_SceneRoot->AddChild(bgObject);
        }
    }

    auto& mapNodes = m_App.GetMapNodes();

    // Re-evaluate unlock statuses
    for (auto& node : mapNodes) {
        if (m_App.GetAllLevelsUnlocked()) {
            node.unlocked = true;
        } else if (node.id == 0) {
            node.unlocked = true;
        } else {
            if (node.id == 21) {
                node.unlocked = false;
                for (int req : node.requiredLevels) {
                    auto it = std::find_if(mapNodes.begin(), mapNodes.end(), [req](const App::MapNode& n){ return n.id == req; });
                    if (it != mapNodes.end() && it->completed) {
                        node.unlocked = true;
                        break;
                    }
                }
            } else {
                node.unlocked = true;
                for (int req : node.requiredLevels) {
                    auto it = std::find_if(mapNodes.begin(), mapNodes.end(), [req](const App::MapNode& n){ return n.id == req; });
                    if (it != mapNodes.end() && !it->completed) {
                        node.unlocked = false;
                        break;
                    }
                }
            }
        }
    }

    auto getWorldPos = [&](const App::MapNode& node) -> glm::vec2 {
        float mapW = 800.0f * m_App.GetMapBackgroundScale();
        float mapH = 395.0f * m_App.GetMapBackgroundScale();
        return { (node.x - 0.5f) * mapW, (0.5f - node.y) * mapH };
    };

    // Draw paths: 0 -> 2, 2 -> 6, 2 -> 7, 6 -> 21, 7 -> 21
    DrawPathLine(getWorldPos(mapNodes[0]), getWorldPos(mapNodes[1])); // 0 -> 2
    DrawPathLine(getWorldPos(mapNodes[1]), getWorldPos(mapNodes[2])); // 2 -> 6
    DrawPathLine(getWorldPos(mapNodes[1]), getWorldPos(mapNodes[3])); // 2 -> 7
    DrawPathLine(getWorldPos(mapNodes[2]), getWorldPos(mapNodes[4])); // 6 -> 21
    DrawPathLine(getWorldPos(mapNodes[3]), getWorldPos(mapNodes[4])); // 7 -> 21

    // Instantiate buttons & frames
    for (auto& node : mapNodes) {
        glm::vec2 pos = getWorldPos(node);
        std::string frameName = "DiamondDark0000";
        std::string maskFrameName = "DiamondDark_Bkg_Mask0000";

        if (node.unlocked) {
            std::string prefix = "Diamond";
            if (node.type == "puzzle") prefix = "DiamondPuzzle";
            else if (node.type == "speed") prefix = "DiamondSpeed";
            
            frameName = prefix + (node.completed ? "0003" : "0000");
            maskFrameName = prefix + "_Bkg_Mask0000";
        }

        auto maskSprite = std::make_shared<AtlasSprite>(m_App.GetMenuAtlas(), maskFrameName);
        node.maskObject = std::make_shared<Util::GameObject>(maskSprite, kButtonZ - 0.2f);
        node.maskObject->m_Transform.translation = pos;
        node.maskObject->m_Transform.scale = {0.7f, 0.7f};
        m_SceneRoot->AddChild(node.maskObject);

        node.sprite = std::make_shared<AtlasSprite>(m_App.GetMenuAtlas(), frameName);
        node.gameObject = std::make_shared<Util::GameObject>(node.sprite, kButtonZ);
        node.gameObject->m_Transform.translation = pos;
        node.gameObject->m_Transform.scale = {0.7f, 0.7f};
        m_SceneRoot->AddChild(node.gameObject);

        if (!node.unlocked) {
            auto lockSprite = std::make_shared<AtlasSprite>(m_App.GetMenuAtlas(), "Lock0000");
            node.lockObject = std::make_shared<Util::GameObject>(lockSprite, kButtonZ + 1.0f);
            node.lockObject->m_Transform.translation = pos;
            node.lockObject->m_Transform.scale = {0.6f, 0.6f};
            m_SceneRoot->AddChild(node.lockObject);
        } else {
            node.lockObject.reset();
        }
    }
}

void MapScene::Update() {
    const auto cursor = Util::Input::GetCursorPosition();
    const float worldX = cursor.x;
    const float worldY = cursor.y;

    auto getWorldPos = [&](const App::MapNode& node) -> glm::vec2 {
        float mapW = 800.0f * m_App.GetMapBackgroundScale();
        float mapH = 395.0f * m_App.GetMapBackgroundScale();
        return { (node.x - 0.5f) * mapW, (0.5f - node.y) * mapH };
    };

    auto& mapNodes = m_App.GetMapNodes();
    for (auto& node : mapNodes) {
        if (!node.unlocked) continue;

        glm::vec2 pos = getWorldPos(node);
        float dist = glm::distance(pos, glm::vec2(worldX, worldY));

        if (dist < 40.0f) {
            if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
                node.gameObject->m_Transform.scale = {0.65f, 0.65f};
                if (node.maskObject) node.maskObject->m_Transform.scale = {0.65f, 0.65f};
                LOG_INFO("Entering level: {}", node.filename);
                m_App.GetCurrentLevelPath() = node.filename;
                m_App.SwitchScene(std::make_unique<GameScene>(m_App));
                return;
            } else {
                node.gameObject->m_Transform.scale = {0.8f, 0.8f};
                if (node.maskObject) node.maskObject->m_Transform.scale = {0.8f, 0.8f};
            }
        } else {
            node.gameObject->m_Transform.scale = {0.7f, 0.7f};
            if (node.maskObject) node.maskObject->m_Transform.scale = {0.7f, 0.7f};
        }
    }

    if (Util::Input::IsKeyUp(Util::Keycode::BACKSPACE)) {
        m_App.SwitchScene(std::make_unique<CoverScene>(m_App));
    }
}

void MapScene::DrawDevMenu() {
    ImGui::Text("Map Cheats:");
    if (ImGui::Checkbox("Unlock All Levels", &m_App.GetAllLevelsUnlocked())) {
        m_App.SwitchScene(std::make_unique<MapScene>(m_App));
    }
}
