#include "App.hpp"
#include <imgui.h>
#include <algorithm>
#include <exception>
#include <string>
#include <vector>
#include "config.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

// Include IScene implementations (State Pattern)
#include "Scene/CoverScene.hpp"
#include "Scene/MapScene.hpp"
#include "Scene/GameScene.hpp"

namespace {
std::string BuildReferencePath() {
  return std::string(RESOURCE_DIR) + "/reference/fireboy_and_watergirl_3";
}

std::string BuildAtlasPath(const std::string &fileName) {
  return BuildReferencePath() + "/atlasses/" + fileName;
}
} // namespace

void App::Start() {
  LOG_TRACE("Start");

  m_MenuAtlas = std::make_shared<SpriteAtlas>(BuildAtlasPath("MenuAssets.png"), BuildAtlasPath("MenuAssets.json"));
  m_MenuBackgroundAtlas = std::make_shared<SpriteAtlas>(BuildAtlasPath("MenuBackgrounds.png"), BuildAtlasPath("MenuBackgrounds.json"));
  m_GameAtlas = std::make_shared<SpriteAtlas>(BuildAtlasPath("CharAssets.png"), BuildAtlasPath("CharAssets.json"));
  m_MechAtlas = std::make_shared<SpriteAtlas>(BuildAtlasPath("MechAssets.png"), BuildAtlasPath("MechAssets.json"));
  m_GroundAtlas = std::make_shared<SpriteAtlas>(BuildAtlasPath("GroundAssets.png"), BuildAtlasPath("GroundAssets.json"));
  m_OverlayAtlas = std::make_shared<SpriteAtlas>(BuildAtlasPath("OverlayAssets.png"), BuildAtlasPath("OverlayAssets.json"));
  m_TempleAtlas = std::make_shared<SpriteAtlas>(BuildAtlasPath("TempleAssets.png"), BuildAtlasPath("TempleAssets.json"));
  m_FontAtlas = std::make_shared<SpriteAtlas>(std::string(RESOURCE_DIR) + "/fonts/font.png", std::string(RESOURCE_DIR) + "/sprites/font_play.json");

  SetupMapNodes();
  SwitchScene(std::make_unique<CoverScene>(*this));
  m_CurrentState = State::UPDATE;
}

void App::Update() {
  if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
    m_CurrentState = State::END;
    return;
  }

  if (Util::Input::IsKeyDown(Util::Keycode::F3)) {
    m_ShowDevMenu = !m_ShowDevMenu;
  }

  if (Util::Input::IsKeyDown(Util::Keycode::F4)) {
    m_ShowDebugBoxes = !m_ShowDebugBoxes;
  }

  if (m_ShowDevMenu) {
    DrawDevMenu();
  }

  // Delegate update step to active state scene (Polymorphism)
  if (m_CurrentScene) {
    m_CurrentScene->Update();
  }

  m_Root.Update();
}

void App::End() {
  LOG_TRACE("End");
}

void App::SwitchScene(std::unique_ptr<IScene> nextScene) {
  // Reset the global renderer root to prevent ghost renderables
  m_Root = Util::Renderer{};
  m_SceneRoot = std::make_shared<Util::GameObject>();
  m_Root.AddChild(m_SceneRoot);

  m_CurrentScene = std::move(nextScene);
  if (m_CurrentScene) {
    m_CurrentScene->Init(m_SceneRoot);
  }
}

void App::SetupMapNodes() {
    m_MapNodes.clear();

    // Node 0 (Normal): Start level
    m_MapNodes.push_back({0, "level0.json", "", 0.5f, 0.85f, {}, true, false});
    
    // Node 2 (Normal): Requires level 0
    m_MapNodes.push_back({2, "level2.json", "", 0.5f, 0.65f, {0}, false, false});
    
    // Node 6 (Speed): Branch Left, Requires level 2
    m_MapNodes.push_back({6, "level6.json", "speed", 0.35f, 0.45f, {2}, false, false});
    
    // Node 7 (Normal): Branch Right, Requires level 2
    m_MapNodes.push_back({7, "level7.json", "", 0.65f, 0.45f, {2}, false, false});
    
    // Node 21 (Puzzle): End level, Requires either level 6 or level 7
    m_MapNodes.push_back({21, "level21.json", "puzzle", 0.5f, 0.25f, {6, 7}, false, false});

    m_CurrentLevelPath = "level0.json";
}

void App::MarkCurrentLevelCompleted() {
    for (auto& node : m_MapNodes) {
        if (node.filename == m_CurrentLevelPath) {
            node.completed = true;
            break;
        }
    }
}

void App::DrawDevMenu() {
    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(WINDOW_WIDTH) - 260.0f, 10.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(240.0f, 320.0f), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Dev Tools (F3)", &m_ShowDevMenu, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Separator();

        ImGui::Checkbox("Independent Respawn", &m_IndependentRespawn);
        ImGui::Checkbox("God Mode", &m_GodMode);
        ImGui::Checkbox("Show Colliders (F4)", &m_ShowDebugBoxes);

        ImGui::Separator();

        if (m_CurrentScene) {
            m_CurrentScene->DrawDevMenu();
        }

        ImGui::Separator();
        ImGui::Text("Scene Navigation:");
        if (ImGui::Button("Go to Map")) {
            SwitchScene(std::make_unique<MapScene>(*this));
        }
        ImGui::SameLine();
        if (ImGui::Button("Go to Cover")) {
            SwitchScene(std::make_unique<CoverScene>(*this));
        }

        ImGui::Text("Load Level:");
        if (ImGui::Button("L0")) {
            m_CurrentLevelPath = "level0.json";
            SwitchScene(std::make_unique<GameScene>(*this));
        }
        ImGui::SameLine();
        if (ImGui::Button("L2")) {
            m_CurrentLevelPath = "level2.json";
            SwitchScene(std::make_unique<GameScene>(*this));
        }
        ImGui::SameLine();
        if (ImGui::Button("L6")) {
            m_CurrentLevelPath = "level6.json";
            SwitchScene(std::make_unique<GameScene>(*this));
        }
        ImGui::SameLine();
        if (ImGui::Button("L7")) {
            m_CurrentLevelPath = "level7.json";
            SwitchScene(std::make_unique<GameScene>(*this));
        }
        ImGui::SameLine();
        if (ImGui::Button("L21")) {
            m_CurrentLevelPath = "level21.json";
            SwitchScene(std::make_unique<GameScene>(*this));
        }
    }
    ImGui::End();
}
