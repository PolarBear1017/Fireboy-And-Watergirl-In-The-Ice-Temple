#include "Scene/GameScene.hpp"
#include <imgui.h>
#include "App.hpp"
#include "Scene/MapScene.hpp"
#include "Input/KeyboardInputController.hpp"
#include "Level/LevelLoader.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "config.hpp"
#include "Mechanics/Button.hpp"
#include "Mechanics/TimedButton.hpp"
#include "Mechanics/Lever.hpp"
#include "Mechanics/Elevator.hpp"
#include "../../include/GeneralGameObject/Diamond.hpp"
#include "Mechanics/Block.hpp"
#include "../../include/GeneralGameObject/Overlay.hpp"
#include "../../include/GeneralGameObject/Door.hpp"
#include <algorithm>
#include <cmath>

namespace {
constexpr float kBackgroundZ = -10.0F;

std::string BuildReferencePath() {
  return std::string(RESOURCE_DIR) + "/reference/fireboy_and_watergirl_3";
}

std::string BuildLevelPath(const std::string &fileName) {
  return std::string(RESOURCE_DIR) + "/levels/" + fileName;
}
} // namespace

GameScene::GameScene(App& app)
    : m_App(app),
      m_LevelManager(std::make_shared<LevelManager>(app.GetGroundAtlas(), app.GetOverlayAtlas())),
      m_TriggerMediator(std::make_shared<TriggerMediator>()) {}

void GameScene::Init(std::shared_ptr<Util::GameObject> sceneRoot) {
    m_SceneRoot = std::move(sceneRoot);
    BuildGameScene();
}

void GameScene::BuildGameScene() {
    // 1. Clear the renderer completely to prevent old overlays/children from leaking
    m_App.GetRenderer() = Util::Renderer{};

    // 2. Recreate m_SceneRoot and register it to the new renderer
    m_SceneRoot = std::make_shared<Util::GameObject>();
    m_App.GetRenderer().AddChild(m_SceneRoot);

    // Clear dynamic vectors first (cleanup on restart)
    m_Overlays.clear();
    m_Activators.clear();
    m_Receivers.clear();
    m_Diamonds.clear();
    m_Blocks.clear();
    m_DebugBoxes.clear();
    m_TriggerMediator->Reset();

    // 1. Build Background (MenuBackground0000 Tiling)
    const float bgTileSize = 512.0F;
    const int cols = (WINDOW_WIDTH / static_cast<int>(bgTileSize)) + 2;
    const int rows = (WINDOW_HEIGHT / static_cast<int>(bgTileSize)) + 2;

    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < rows; ++j) {
            auto bgSprite = std::make_shared<AtlasSprite>(m_App.GetTempleAtlas(), "BackGround0000");
            auto bgObject = std::make_shared<Util::GameObject>(bgSprite, kBackgroundZ);

            float x = -static_cast<float>(WINDOW_WIDTH) / 2.0F + (bgTileSize / 2.0F) + static_cast<float>(i) * bgTileSize;
            float y = static_cast<float>(WINDOW_HEIGHT) / 2.0F - (bgTileSize / 2.0F) - static_cast<float>(j) * bgTileSize;

            bgObject->m_Transform.translation = {x, y};
            m_SceneRoot->AddChild(bgObject);
        }
    }

    const LevelDefinition level = LoadLevelDefinitionFromJsonFile(BuildLevelPath(m_App.GetCurrentLevelPath()));
    if (!m_LevelManager->LoadLevel(level, m_SceneRoot)) {
        LOG_ERROR("Level validation failed after JSON load.");
        return;
    }

    // Load level water/fire/toxic overlays
    for (const auto& overlayData : level.overlays) {
        glm::vec2 startPos = m_LevelManager->TileToWorldPosition(overlayData.coord.row, overlayData.coord.col);
        auto dynamicOverlay = std::make_shared<Overlay>(
            m_App.GetOverlayAtlas(),
            overlayData.coord.row,
            overlayData.coord.col,
            overlayData.element,
            startPos,
            overlayData.width,
            level.tileSize
        );
        m_App.GetRenderer().AddChild(dynamicOverlay);
        m_Overlays.push_back(dynamicOverlay);
    }

    // 2. Load Mechanisms / Interactive Objects
    for (const auto &obj : level.objects) {
        glm::vec2 pos = m_LevelManager->TileToWorldPosition(obj.coord.row, obj.coord.col);
        
        if (obj.type == LevelObjectType::Button) {
            auto button = std::make_shared<Button>(m_App.GetMechAtlas(), pos, obj.group_id, m_TriggerMediator);
            m_Activators.push_back(button);
            m_SceneRoot->AddChild(button);
        } else if (obj.type == LevelObjectType::TimedButton) {
            auto button = std::make_shared<TimedButton>(m_App.GetMechAtlas(), pos, obj.group_id, obj.time, m_TriggerMediator);
            m_Activators.push_back(button);
            m_SceneRoot->AddChild(button);
        } else if (obj.type == LevelObjectType::Lever) {
            auto lever = std::make_shared<Lever>(m_App.GetMechAtlas(), pos, obj.group_id, m_TriggerMediator);
            m_Activators.push_back(lever);
            m_SceneRoot->AddChild(lever);
        } else if (obj.type == LevelObjectType::Elevator) {
            glm::vec2 targetPos = m_LevelManager->TileToWorldPosition(obj.target_row, obj.target_col);
            float tileSize = static_cast<float>(level.tileSize);
            glm::vec2 size = obj.is_horizontal
                                 ? glm::vec2(tileSize * obj.length, tileSize * 0.5f)
                                 : glm::vec2(tileSize, tileSize * obj.length);

            if (obj.is_horizontal) {
                float offset = (tileSize * obj.length) / 2.0f - (tileSize / 2.0f);
                pos.x += offset;
                targetPos.x += offset;
            } else {
                float offset = (tileSize * obj.length) / 2.0f - (tileSize / 2.0f);
                pos.y -= offset;
                targetPos.y -= offset;
            }

            auto elevator = std::make_shared<Elevator>(
                m_App.GetMechAtlas(), pos, targetPos, size, obj.group_id, obj.is_horizontal);
            m_Receivers.push_back(elevator);
            m_TriggerMediator->RegisterReceiver(obj.group_id, elevator);
            m_SceneRoot->AddChild(elevator);
        } else if (obj.type == LevelObjectType::Diamond) {
            std::string frameName = (obj.element == Element::FIRE) ? "diamond_fb0000" : "diamond_wg0000";
            auto sprite = std::make_shared<AtlasSprite>(m_App.GetGameAtlas(), frameName);
            auto diamond = std::make_shared<Diamond>(sprite, 5.0f, obj.element);
            diamond->m_Transform.translation = pos;
            diamond->m_Transform.scale = glm::vec2(0.8f);
            m_Diamonds.push_back(diamond);
            m_SceneRoot->AddChild(diamond);
        } else if (obj.type == LevelObjectType::Block) {
            auto block = std::make_shared<Block>(m_App.GetMechAtlas(), pos);
            m_Blocks.push_back(block);
            m_SceneRoot->AddChild(block);
        }
    }

    const auto &levelData = m_LevelManager->GetLevelData();

    // 3. Setup Characters with Keyboard Input Controllers (Dependency Injection & Strategy)
    auto fireboyCtrl = std::make_unique<KeyboardInputController>(
        Util::Keycode::LEFT, Util::Keycode::RIGHT, Util::Keycode::UP
    );
    m_FireBoy = std::make_shared<Character>(m_App.GetGameAtlas(), Element::FIRE, std::move(fireboyCtrl));
    if (levelData.hasFireSpawn) {
        m_FireBoy->m_Transform.translation = m_LevelManager->TileToWorldPosition(levelData.fireSpawn.row, levelData.fireSpawn.col);
    } else {
        m_FireBoy->m_Transform.translation = {0.0F, 0.0F};
    }
    m_SceneRoot->AddChild(m_FireBoy);

    auto watergirlCtrl = std::make_unique<KeyboardInputController>(
        Util::Keycode::A, Util::Keycode::D, Util::Keycode::W
    );
    m_WaterGirl = std::make_shared<Character>(m_App.GetGameAtlas(), Element::WATER, std::move(watergirlCtrl));
    if (levelData.hasWaterSpawn) {
        m_WaterGirl->m_Transform.translation = m_LevelManager->TileToWorldPosition(levelData.waterSpawn.row, levelData.waterSpawn.col);
    } else {
        m_WaterGirl->m_Transform.translation = {0.0F, 0.0F};
    }
    m_SceneRoot->AddChild(m_WaterGirl);

    // 4. Setup Doors
    if (levelData.hasFireDoor) {
        glm::vec2 fireDoorPos = m_LevelManager->TileToWorldPosition(levelData.fireDoor.row, levelData.fireDoor.col);
        m_FireDoor = std::make_shared<Door>(m_App.GetTempleAtlas(), Element::FIRE, fireDoorPos);
        m_SceneRoot->AddChild(m_FireDoor);
    }

    if (levelData.hasWaterDoor) {
        glm::vec2 waterDoorPos = m_LevelManager->TileToWorldPosition(levelData.waterDoor.row, levelData.waterDoor.col);
        m_WaterDoor = std::make_shared<Door>(m_App.GetTempleAtlas(), Element::WATER, waterDoorPos);
        m_SceneRoot->AddChild(m_WaterDoor);
    }

    m_LevelFinished = false;
    m_LevelFinishTimer = 0.0f;
    m_FireboyGems = 0;
    m_WatergirlGems = 0;

    LOG_INFO("Entered Level: {}", m_App.GetCurrentLevelPath());
}

void GameScene::Update() {
    const auto elapsedSeconds = Util::Time::GetDeltaTime();

    if (m_LevelFinished) {
        m_LevelFinishTimer += elapsedSeconds;
        if (m_FireBoy) m_FireBoy->SetInputEnabled(false);
        if (m_WaterGirl) m_WaterGirl->SetInputEnabled(false);
        
        if (m_LevelFinishTimer >= 1.6f) {
            LOG_INFO("Level Complete! Returning to Menu.");
            m_App.MarkCurrentLevelCompleted();
            m_App.SwitchScene(std::make_unique<MapScene>(m_App));
            return;
        }
    }

    if (Util::Input::IsKeyUp(Util::Keycode::BACKSPACE)) {
        m_App.SwitchScene(std::make_unique<MapScene>(m_App));
        return;
    }

    // Apply God Mode state
    if (m_FireBoy) m_FireBoy->SetGodMode(m_App.GetGodMode());
    if (m_WaterGirl) m_WaterGirl->SetGodMode(m_App.GetGodMode());

    // Respawn / Death logic
    if (m_App.GetIndependentRespawn()) {
        const auto& levelData = m_LevelManager->GetLevelData();

        if (m_FireBoy && (m_FireBoy->IsDying() || m_FireBoy->IsDead())) {
            m_FireBoy->SetInputEnabled(false);
            m_FireBoy->SetVelocity({0.0f, 0.0f});
            m_FireBoy->Update();
            if (m_FireBoy->IsDead()) {
                glm::vec2 spawn = levelData.hasFireSpawn ? m_LevelManager->TileToWorldPosition(
                    levelData.fireSpawn.row, levelData.fireSpawn.col) : glm::vec2(0.0f);
                m_FireBoy->Respawn(spawn);
                LOG_INFO("Fireboy respawned at starting point.");
            }
        }
        if (m_WaterGirl && (m_WaterGirl->IsDying() || m_WaterGirl->IsDead())) {
            m_WaterGirl->SetInputEnabled(false);
            m_WaterGirl->SetVelocity({0.0f, 0.0f});
            m_WaterGirl->Update();
            if (m_WaterGirl->IsDead()) {
                glm::vec2 spawn = levelData.hasWaterSpawn ? m_LevelManager->TileToWorldPosition(
                    levelData.waterSpawn.row, levelData.waterSpawn.col) : glm::vec2(0.0f);
                m_WaterGirl->Respawn(spawn);
                LOG_INFO("Watergirl respawned at starting point.");
            }
        }
    } else {
        bool isAnyDying = (m_FireBoy && m_FireBoy->IsDying()) || (m_WaterGirl && m_WaterGirl->IsDying());
        bool isAnyDead = (m_FireBoy && m_FireBoy->IsDead()) || (m_WaterGirl && m_WaterGirl->IsDead());

        if (isAnyDying || isAnyDead) {
            if (m_FireBoy) {
                m_FireBoy->SetInputEnabled(false);
                m_FireBoy->SetVelocity({0.0f, 0.0f});
                m_FireBoy->Update();
            }
            if (m_WaterGirl) {
                m_WaterGirl->SetInputEnabled(false);
                m_WaterGirl->SetVelocity({0.0f, 0.0f});
                m_WaterGirl->Update();
            }
            if (isAnyDead) {
                LOG_INFO("Player died! Restarting level.");
                BuildGameScene();
            }
            return;
        }
    }

    // Water flowing & freezing
    for (auto& overlay : m_Overlays) {
        overlay->Update();
        auto newIndex = overlay->ConsumeNewlyConvertedIndex();
        for (int localCol : newIndex) {
            m_LevelManager->SwitchWaterAndIceTerrain(overlay->GetRow(), overlay->GetStartCol() + localCol);
        }
    }

    // Update characters
    if (m_FireBoy && !(m_App.GetIndependentRespawn() && (m_FireBoy->IsDying() || m_FireBoy->IsDead()))) {
        m_FireBoy->Update();
    }
    if (m_WaterGirl && !(m_App.GetIndependentRespawn() && (m_WaterGirl->IsDying() || m_WaterGirl->IsDead()))) {
        m_WaterGirl->Update();
    }

    // Diamond Collection logic
    for (auto& diamond : m_Diamonds) {
        if (diamond->IsCollected()) continue;
        glm::vec2 diamondPos = diamond->GetTransform().translation;
        glm::vec2 diamondSize(30.0f, 30.0f);
    
        if (m_FireBoy && !(m_App.GetIndependentRespawn() && (m_FireBoy->IsDying() || m_FireBoy->IsDead()))) {
            glm::vec2 fbSize = m_FireBoy->GetCollisionSize();
            glm::vec2 fbCenter = m_FireBoy->GetPosition();
            fbCenter.y += fbSize.y * 0.5f;
            if (m_CollisionSystem.CheckOverlap(fbCenter, fbSize, diamondPos, diamondSize)) {
                if (diamond->GetElement() == Element::FIRE || diamond->GetElement() == Element::NEUTRAL) {
                    diamond->Collect();
                    m_FireboyGems++;
                    LOG_INFO("Fireboy collected a diamond! Total: {}", m_FireboyGems);
                }
            }
        }
    
        if (m_WaterGirl && !(m_App.GetIndependentRespawn() && (m_WaterGirl->IsDying() || m_WaterGirl->IsDead()))) {
            glm::vec2 wgSize = m_WaterGirl->GetCollisionSize();
            glm::vec2 wgCenter = m_WaterGirl->GetPosition();
            wgCenter.y += wgSize.y * 0.5f;
            if (m_CollisionSystem.CheckOverlap(wgCenter, wgSize, diamondPos, diamondSize)) {
                if (diamond->GetElement() == Element::WATER || diamond->GetElement() == Element::NEUTRAL) {
                    diamond->Collect();
                    m_WatergirlGems++;
                    LOG_INFO("Watergirl collected a diamond! Total: {}", m_WatergirlGems);
                }
            }
        }
    }

    // Mechanics Update (Callbacks driven by Mediator pattern)
    glm::vec2 fPos = m_FireBoy ? m_FireBoy->GetPosition() : glm::vec2(0.0f);
    glm::vec2 wPos = m_WaterGirl ? m_WaterGirl->GetPosition() : glm::vec2(0.0f);

    std::vector<glm::vec2> interactorPositions;
    if (m_FireBoy && !(m_App.GetIndependentRespawn() && (m_FireBoy->IsDying() || m_FireBoy->IsDead()))) interactorPositions.push_back(fPos);
    if (m_WaterGirl && !(m_App.GetIndependentRespawn() && (m_WaterGirl->IsDying() || m_WaterGirl->IsDead()))) interactorPositions.push_back(wPos);

    for (const auto &block : m_Blocks) {
        interactorPositions.push_back(block->GetPosition());
    }

    // Activators update (internally alerts m_TriggerMediator of state mutations)
    for (auto& activator : m_Activators) {
        activator->Update(interactorPositions);
        
        // Handle Ice conversion activator special case
        if (activator->IsActivated()) {
            int groupId = activator->GetGroupId();
            if (groupId < 0) {
                size_t overlayIdx = std::abs(groupId) - 1;
                if (overlayIdx < m_Overlays.size())
                    m_Overlays[overlayIdx]->StartConversion(activator->m_Transform.translation);
            }
        }
    }

    // Receivers update visual position shifts
    for (auto &receiver : m_Receivers) {
        receiver->Update();
    }

    // Physics Phase: Mechanics Collisions
    std::vector<std::shared_ptr<BaseMechanism>> allMechs;
    for (auto &a : m_Activators) allMechs.push_back(a);
    for (auto &r : m_Receivers) allMechs.push_back(r);
    for (auto &b : m_Blocks) {
        b->Update();
        m_CollisionSystem.ResolveBlockTerrain(*b, *m_LevelManager);
        allMechs.push_back(b);
    }

    if (m_FireBoy && !(m_App.GetIndependentRespawn() && (m_FireBoy->IsDying() || m_FireBoy->IsDead()))) {
        m_FireBoy->SetGroundState(GroundState::AIR);
        m_CollisionSystem.ResolveCharacterTerrain(*m_FireBoy, *m_LevelManager);
        m_CollisionSystem.ResolveCharacterMechanics(*m_FireBoy, allMechs, *m_LevelManager);
    }
    if (m_WaterGirl && !(m_App.GetIndependentRespawn() && (m_WaterGirl->IsDying() || m_WaterGirl->IsDead()))) {
        m_WaterGirl->SetGroundState(GroundState::AIR);
        m_CollisionSystem.ResolveCharacterTerrain(*m_WaterGirl, *m_LevelManager);
        m_CollisionSystem.ResolveCharacterMechanics(*m_WaterGirl, allMechs, *m_LevelManager);
    }

    // Doors updating
    if (m_FireDoor && m_FireBoy && !(m_App.GetIndependentRespawn() && (m_FireBoy->IsDying() || m_FireBoy->IsDead()))) {
        m_FireDoor->Update(m_FireBoy->m_Transform.translation);
    }
    if (m_WaterDoor && m_WaterGirl && !(m_App.GetIndependentRespawn() && (m_WaterGirl->IsDying() || m_WaterGirl->IsDead()))) {
        m_WaterDoor->Update(m_WaterGirl->m_Transform.translation);
    }

    // Check Victory Condition
    if (!m_LevelFinished && m_FireDoor && m_WaterDoor) {
        if (m_FireDoor->IsFullyOpen() && m_WaterDoor->IsFullyOpen()) {
            LOG_INFO("Both players reached the doors!");
            m_LevelFinished = true;
            if (m_FireBoy) m_FireBoy->PlayEnterDoorAnimation(m_FireDoor->m_Transform.translation);
            if (m_WaterGirl) m_WaterGirl->PlayEnterDoorAnimation(m_WaterDoor->m_Transform.translation);
        }
    }

    // Debug boxes drawing (Show Colliders / F4)
    struct DebugColliderInfo {
        glm::vec2 center;
        glm::vec2 size;
        glm::vec4 color;
    };
    std::vector<DebugColliderInfo> debugColliders;

    if (m_App.GetShowDebugBoxes()) {
        if (m_FireBoy && m_FireBoy->IsVisible() && !(m_App.GetIndependentRespawn() && (m_FireBoy->IsDying() || m_FireBoy->IsDead()))) {
            glm::vec2 size = m_FireBoy->GetCollisionSize();
            glm::vec2 pos = m_FireBoy->GetPosition();
            debugColliders.push_back({{pos.x, pos.y + size.y * 0.5f}, size, {1.0f, 0.0f, 0.0f, 0.4f}});
        }
        if (m_WaterGirl && m_WaterGirl->IsVisible() && !(m_App.GetIndependentRespawn() && (m_WaterGirl->IsDying() || m_WaterGirl->IsDead()))) {
            glm::vec2 size = m_WaterGirl->GetCollisionSize();
            glm::vec2 pos = m_WaterGirl->GetPosition();
            debugColliders.push_back({{pos.x, pos.y + size.y * 0.5f}, size, {0.0f, 0.0f, 1.0f, 0.4f}});
        }
        for (const auto& block : m_Blocks) {
            if (auto coll = block->GetCollider()) {
                debugColliders.push_back({coll->center, coll->size, {0.0f, 1.0f, 0.0f, 0.4f}});
            }
        }
        for (const auto& activator : m_Activators) {
            if (auto coll = activator->GetCollider()) {
                debugColliders.push_back({coll->center, coll->size, {0.0f, 1.0f, 0.0f, 0.4f}});
            }
        }
        for (const auto& receiver : m_Receivers) {
            if (auto coll = receiver->GetCollider()) {
                debugColliders.push_back({coll->center, coll->size, {0.0f, 1.0f, 0.0f, 0.4f}});
            }
        }
    }

    while (m_DebugBoxes.size() < debugColliders.size()) {
        auto sprite = std::make_shared<AtlasSprite>(m_App.GetMechAtlas(), "movingbox0000");
        sprite->SetUsePureColor(true);
        auto box = std::make_shared<Util::GameObject>(sprite, 100.0f);
        m_DebugBoxes.push_back(box);
        m_SceneRoot->AddChild(box);
    }

    for (size_t i = 0; i < m_DebugBoxes.size(); ++i) {
        if (i < debugColliders.size()) {
            const auto& info = debugColliders[i];
            m_DebugBoxes[i]->SetVisible(true);
            m_DebugBoxes[i]->m_Transform.translation = info.center;
            m_DebugBoxes[i]->m_Transform.scale = info.size / glm::vec2(74.0f, 76.0f);
            auto sprite = std::make_shared<AtlasSprite>(m_App.GetMechAtlas(), "movingbox0000");
            sprite->SetUsePureColor(true);
            sprite->SetColorTint(info.color);
            m_DebugBoxes[i]->SetDrawable(sprite);
        } else {
            m_DebugBoxes[i]->SetVisible(false);
        }
    }
}

void GameScene::TeleportToDoors() {
    if (m_FireBoy && m_FireDoor) {
        m_FireBoy->SetPosition(m_FireDoor->m_Transform.translation);
    }
    if (m_WaterGirl && m_WaterDoor) {
        m_WaterGirl->SetPosition(m_WaterDoor->m_Transform.translation);
    }
}

void GameScene::InstantWin() {
    m_LevelFinished = true;
    if (m_FireBoy && m_FireDoor) {
        m_FireBoy->PlayEnterDoorAnimation(m_FireDoor->m_Transform.translation);
    }
    if (m_WaterGirl && m_WaterDoor) {
        m_WaterGirl->PlayEnterDoorAnimation(m_WaterDoor->m_Transform.translation);
    }
}

void GameScene::ResetLevel() {
    BuildGameScene();
}

void GameScene::DrawDevMenu() {
    ImGui::Text("Game Cheats:");
    if (ImGui::Button("Teleport to Doors")) {
        TeleportToDoors();
    }
    if (ImGui::Button("Instant Win")) {
        InstantWin();
    }
    if (ImGui::Button("Reset Level")) {
        ResetLevel();
    }
}
