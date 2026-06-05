#include "App.hpp"

#include <algorithm>
#include <cmath>
#include <exception>
#include <string>
#include <vector>

#include "config.hpp"

#include "Level/LevelLoader.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

#include "Mechanics/Button.hpp"
#include "Mechanics/Elevator.hpp"
#include "Mechanics/Lever.hpp"
#include "Mechanics/TimedButton.hpp"
#include "Util/Text.hpp"
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

void App::Start() {
  LOG_TRACE("Start");

  m_MenuAtlas = std::make_shared<SpriteAtlas>(
      BuildAtlasPath("MenuAssets.png"), BuildAtlasPath("MenuAssets.json"));
  m_MenuBackgroundAtlas =
      std::make_shared<SpriteAtlas>(BuildAtlasPath("MenuBackgrounds.png"),
                                    BuildAtlasPath("MenuBackgrounds.json"));

  m_GameAtlas = std::make_shared<SpriteAtlas>(
      BuildAtlasPath("CharAssets.png"), BuildAtlasPath("CharAssets.json"));

  m_MechAtlas = std::make_shared<SpriteAtlas>(
      BuildAtlasPath("MechAssets.png"), BuildAtlasPath("MechAssets.json"));

  m_GroundAtlas = std::make_shared<SpriteAtlas>(
      BuildAtlasPath("GroundAssets.png"), BuildAtlasPath("GroundAssets.json"));
  m_OverlayAtlas =
      std::make_shared<SpriteAtlas>(BuildAtlasPath("OverlayAssets.png"),
                                    BuildAtlasPath("OverlayAssets.json"));
  m_TempleAtlas = std::make_shared<SpriteAtlas>(
      BuildAtlasPath("TempleAssets.png"), BuildAtlasPath("TempleAssets.json"));
  m_FontAtlas = std::make_shared<SpriteAtlas>(
      std::string(RESOURCE_DIR) + "/fonts/font.png",
      std::string(RESOURCE_DIR) + "/sprites/font_play.json");
  m_LevelManager =
      std::make_shared<LevelManager>(m_GroundAtlas, m_OverlayAtlas);

  SetupMapNodes();
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

  case Scene::Map:
    UpdateMapScene();
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

  case Scene::Map:
    BuildMapScene();
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
  m_Beams.clear();
  m_BeamDatas.clear();
  m_PlayTextObjects.clear();
  m_MoreGamesTextObjects.clear();
  m_WalkthroughTextObjects.clear();
  m_GamePlaceholderSprite.reset();

  m_Overlays.clear();
  m_Activators.clear();
  m_Receivers.clear();
  m_Diamonds.clear();
  m_Blocks.clear();
}

void App::BuildCoverScene() {
  ResetSceneRoot();

  const auto background =
      std::make_shared<Util::Image>(BuildImagePath("TempleHallIce.jpg"));
  const auto backgroundScale =
      ComputeCoverScale(background->GetSize(), static_cast<float>(WINDOW_WIDTH),
                        static_cast<float>(WINDOW_HEIGHT));
  auto backgroundObject =
      std::make_shared<Util::GameObject>(background, kBackgroundZ);
  backgroundObject->m_Transform.scale = {backgroundScale, backgroundScale};
  m_SceneRoot->AddChild(backgroundObject);

  const auto beamImage =
      std::make_shared<Util::Image>(BuildImagePath("Beam.png"));
  const float baseScale =
      ComputeContainScale(beamImage->GetSize(), 520.0F, 340.0F);
  const glm::vec2 beamSize = beamImage->GetSize();

  for (int i = 0; i < 5; ++i) {
    auto beamObj = std::make_shared<Util::GameObject>(
        beamImage, kDecorationZ - (i * 0.1f));

    // 設置樞紐點在圖片頂部中心
    beamObj->SetPivot({0.0f, beamSize.y / 2.0f});

    // 從畫面外頂部發射 (螢幕邊界是 300)
    beamObj->m_Transform.translation = {0.0F, 550.0F};

    BeamData data;
    data.speed = 0.3f + (static_cast<float>(rand()) / RAND_MAX) * 0.4f;
    data.offset = static_cast<float>(i) * 1.5f;
    data.baseScale =
        baseScale * (2.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.8f);

    beamObj->m_Transform.scale = {data.baseScale, data.baseScale};

    m_Beams.push_back(beamObj);
    m_BeamDatas.push_back(data);
    m_SceneRoot->AddChild(beamObj);
  }

  const auto title =
      std::make_shared<Util::Image>(BuildImagePath("GameNameIce.png"));
  const auto titleScale = ComputeContainScale(title->GetSize(), 760.0F, 220.0F);
  auto titleObject = std::make_shared<Util::GameObject>(title, kTitleZ);
  titleObject->m_Transform.translation = {0.0F, 120.0F};
  titleObject->m_Transform.scale = {titleScale, titleScale};
  m_SceneRoot->AddChild(titleObject);

  m_StartButtonSprite =
      std::make_shared<AtlasSprite>(m_MenuAtlas, "StoneButton0000");
  m_StartButtonBaseScale =
      ComputeContainScale(m_StartButtonSprite->GetSize(), 320.0F, 110.0F);
  m_StartButtonObject = MakeAtlasObject(m_StartButtonSprite, {0.0F, -245.0F},
                                        kButtonZ, m_StartButtonBaseScale);
  // 移除按鈕背景物件的 AddChild，僅保留變數初始化以防萬一

  // 1. 各項字樣實作 (PLAY, MORE GAMES, WALKTHROUGH)
  auto buildWord =
      [&](const std::string &word, glm::vec2 basePos, float scale,
          std::vector<std::shared_ptr<Util::GameObject>> &targetVector,
          float spacing) {
        targetVector.clear();
        float totalWidth = (word.length() - 1) * spacing;
        float startX = -totalWidth / 2.0f;

        for (size_t i = 0; i < word.length(); ++i) {
          std::string letter(1, word[i]);
          if (letter == " ") {
            startX += spacing;
            continue;
          }
          auto sprite = std::make_shared<AtlasSprite>(m_FontAtlas, letter);
          auto object =
              std::make_shared<Util::GameObject>(sprite, kButtonZ + 1.0F);
          object->m_Transform.translation = basePos + glm::vec2(startX, 0.0F);
          object->m_Transform.scale = {scale, scale};
          targetVector.push_back(object);
          m_SceneRoot->AddChild(object);
          startX += spacing;
        }
      };

  // 依照原版截圖佈置位置
  buildWord("PLAY", {0.0F, -40.0F}, 0.65F, m_PlayTextObjects, 45.0F);
  buildWord("MORE GAMES", {0.0F, -135.0F}, 0.38F, m_MoreGamesTextObjects,
            28.0F);
  buildWord("WALKTHROUGH", {0.0F, -205.0F}, 0.38F, m_WalkthroughTextObjects,
            28.0F);

  LOG_INFO("Cover scene loaded. Press ENTER or SPACE to continue.");
}

void App::BuildGameScene() {
  ResetSceneRoot();

  // 1. 建立背景 (拼貼磁磚寫法)
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

  const LevelDefinition level =
      LoadLevelDefinitionFromJsonFile(BuildLevelPath(m_CurrentLevelPath));
  if (!m_LevelManager->LoadLevel(level, m_SceneRoot)) {
    LOG_ERROR("Level validation failed after JSON load.");
    return;
  }

    for (const auto& overlayData : level.overlays) {
        // 從資料算出世界座標
        glm::vec2 startPos = m_LevelManager->TileToWorldPosition(overlayData.coord.row, overlayData.coord.col);

        // 實體化 Overlay 物件
        auto dynamicOverlay = std::make_shared<Overlay>(
            m_OverlayAtlas,
            overlayData.coord.row,
            overlayData.coord.col,
            overlayData.element,
            startPos,
            overlayData.width,
            level.tileSize
        );

        m_Root.AddChild(dynamicOverlay);
        m_Overlays.push_back(dynamicOverlay);
    }

  // 3. 🌟 實體化機關 (讓電梯跟按鈕出現)
  for (const auto &obj : level.objects) {
    glm::vec2 pos =
        m_LevelManager->TileToWorldPosition(obj.coord.row, obj.coord.col);
    if (obj.type == LevelObjectType::Button) {
      auto button = std::make_shared<Button>(m_MechAtlas, pos, obj.group_id);
      m_Activators.push_back(button);
      m_SceneRoot->AddChild(button);
    } else if (obj.type == LevelObjectType::TimedButton) {
      auto button = std::make_shared<TimedButton>(m_MechAtlas, pos, obj.group_id, obj.time);
      m_Activators.push_back(button);
      m_SceneRoot->AddChild(button);
    } else if (obj.type == LevelObjectType::Lever) {
      auto lever = std::make_shared<Lever>(m_MechAtlas, pos, obj.group_id);
      m_Activators.push_back(lever);
      m_SceneRoot->AddChild(lever);
    } else if (obj.type == LevelObjectType::Elevator) {
      glm::vec2 targetPos =
          m_LevelManager->TileToWorldPosition(obj.target_row, obj.target_col);

      // 計算電梯長度
      float tileSize = static_cast<float>(level.tileSize);
      glm::vec2 size = obj.is_horizontal
                           ? glm::vec2(tileSize * obj.length, tileSize * 0.5f)
                           : glm::vec2(tileSize, tileSize * obj.length);

      // 修正：TileToWorldPosition
      // 回傳的是第一格的中心，但電梯的中心點應該在整段長度的中間
      if (obj.is_horizontal) {
        float offset = (tileSize * obj.length) / 2.0f - (tileSize / 2.0f);
        pos.x += offset;
        targetPos.x += offset;
      } else {
        // 世界座標 Y 軸向上為正，向下為負
        float offset = (tileSize * obj.length) / 2.0f - (tileSize / 2.0f);
        pos.y -= offset;
        targetPos.y -= offset;
      }

      auto elevator = std::make_shared<Elevator>(
          m_MechAtlas, pos, targetPos, size, obj.group_id, obj.is_horizontal);
      m_Receivers.push_back(elevator);
      m_SceneRoot->AddChild(elevator);
    } else if (obj.type == LevelObjectType::Diamond) {
      std::string frameName =
          (obj.element == Element::FIRE) ? "diamond_fb0000" : "diamond_wg0000";
      auto sprite = std::make_shared<AtlasSprite>(m_GameAtlas, frameName);
      auto diamond = std::make_shared<Diamond>(sprite, 5.0f, obj.element);
      diamond->m_Transform.translation = pos;
      diamond->m_Transform.scale = glm::vec2(0.8f);
      m_Diamonds.push_back(diamond);
      m_SceneRoot->AddChild(diamond);
    } else if (obj.type == LevelObjectType::Block) {
      auto block = std::make_shared<Block>(m_MechAtlas, pos);
      m_Blocks.push_back(block);
      m_SceneRoot->AddChild(block);
    }
  }

  const auto &levelData = m_LevelManager->GetLevelData();

  // 4. 建立火娃與水娃 (乾淨的動態讀取)
  m_FireBoy = std::make_shared<Character>(m_GameAtlas, Element::FIRE);
  if (levelData.hasFireSpawn) {
    m_FireBoy->m_Transform.translation = m_LevelManager->TileToWorldPosition(
        levelData.fireSpawn.row, levelData.fireSpawn.col);
  } else {
    m_FireBoy->m_Transform.translation = {0.0F, 0.0F};
  }
  m_SceneRoot->AddChild(m_FireBoy);

  m_WaterGirl = std::make_shared<Character>(m_GameAtlas, Element::WATER);
  if (levelData.hasWaterSpawn) {
    m_WaterGirl->m_Transform.translation = m_LevelManager->TileToWorldPosition(
        levelData.waterSpawn.row, levelData.waterSpawn.col);
  } else {
    m_WaterGirl->m_Transform.translation = {0.0F, 0.0F};
  }
  m_SceneRoot->AddChild(m_WaterGirl);

  // 5. 建立過關大門 (動態計算座標)
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

  m_LevelFinished = false;
  m_LevelFinishTimer = 0.0f;

  LOG_INFO("Entered Level 1.");
}

void App::UpdateCoverScene() {
  const auto elapsedSeconds = Util::Time::GetElapsedTimeMs() / 1000.0F;

  if (m_StartButtonObject != nullptr && m_StartButtonSprite != nullptr) {
    // 呼吸燈與縮放邏輯已移除，保持靜態顯示
  }

  for (size_t i = 0; i < m_Beams.size(); ++i) {
    const auto &data = m_BeamDatas[i];
    auto &beam = m_Beams[i];

    // 分佈更開以減少顏色疊加導致的過亮
    float baseAngle = glm::radians(-50.0f + i * 25.0f);
    float swing = 0.12f * std::sin(data.offset + elapsedSeconds * data.speed);
    beam->m_Transform.rotation = baseAngle + swing;

    // 變細：縮小 Scale X 脈動範圍
    float scalePulse =
        0.4f +
        0.3f * std::abs(std::sin(data.offset + elapsedSeconds * data.speed));
    beam->m_Transform.scale.x = data.baseScale * scalePulse;
    beam->m_Transform.scale.y = data.baseScale;

    // 保持 Y 在畫面上方外
    beam->m_Transform.translation.y =
        550.0f + 10.0f * std::sin(data.offset + elapsedSeconds * 0.5f);
  }

  if (Util::Input::IsKeyUp(Util::Keycode::RETURN) ||
      Util::Input::IsKeyUp(Util::Keycode::KP_ENTER) ||
      Util::Input::IsKeyUp(Util::Keycode::SPACE)) {
    SwitchScene(Scene::Map);
  }

  if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
    const auto cursor = Util::Input::GetCursorPosition();
    const float worldX = cursor.x;
    const float worldY = cursor.y;

    // 精確判定範圍 (x: [-85, 85], y: [-65, -15])，確保只有點擊到 "PLAY"
    // 字樣才觸發
    if (worldX >= -85.0F && worldX <= 85.0F && worldY >= -65.0F &&
        worldY <= -15.0F) {
      LOG_INFO("PLAY clicked!");
      SwitchScene(Scene::Map);
    }
  }
}

void App::UpdateGameScene() {
    const auto elapsedSeconds = Util::Time::GetDeltaTime();

    if (m_LevelFinished) {
        m_LevelFinishTimer += elapsedSeconds;
        if (m_FireBoy) {
            m_FireBoy->SetInputEnabled(false);
            // Removed m_FireBoy->SetVisible(false); because PlayEnterDoorAnimation
            // handles visibility
        }
        if (m_WaterGirl) {
            m_WaterGirl->SetInputEnabled(false);
            // Removed m_WaterGirl->SetVisible(false); because PlayEnterDoorAnimation
            // handles visibility
        }
        if (m_LevelFinishTimer >= 1.6f) { // Adjusted from 2.0f to 1.6f
            LOG_INFO("Level Complete! Returning to Menu.");
            MarkCurrentLevelCompleted();
            SwitchScene(Scene::Map);
            return;
        }
        // Still need to update doors and other things to let animations finish
    }

    if (Util::Input::IsKeyUp(Util::Keycode::BACKSPACE)) {
        SwitchScene(Scene::Map);
        return;
    }

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

    // 讓水池流動
    for (auto& overlay : m_Overlays) {
        overlay->Update();

        auto newIndex = overlay->ConsumeNewlyConvertedIndex();
        for (int localCol : newIndex) {
            m_LevelManager->SwitchWaterAndIceTerrain(overlay->GetRow(), overlay->GetStartCol() + localCol);
        }
    }

    // 呼叫機關邏輯 (Polymorphic Update)
    glm::vec2 fPos = m_FireBoy ? m_FireBoy->GetPosition() : glm::vec2(0.0f);
    glm::vec2 wPos = m_WaterGirl ? m_WaterGirl->GetPosition() : glm::vec2(0.0f);

    // --- 呼叫角色更新 ---
    if (m_FireBoy != nullptr) {m_FireBoy->Update();}
    if (m_WaterGirl != nullptr) {m_WaterGirl->Update();}

    for (auto& diamond : m_Diamonds) {
        if (diamond->IsCollected()) continue;
        glm::vec2 diamondPos = diamond->GetTransform().translation;
        glm::vec2 diamondSize(30.0f, 30.0f); // Default approx size for diamonds
    
        if (m_FireBoy) {
            glm::vec2 fbSize = m_FireBoy->GetCollisionSize();
            glm::vec2 fbCenter = m_FireBoy->GetPosition();
            fbCenter.y += fbSize.y * 0.5f; // Adjust feet to center
            if (m_CollisionSystem.CheckOverlap(fbCenter, fbSize, diamondPos, diamondSize)) {
                if (diamond->GetElement() == Element::FIRE || diamond->GetElement() == Element::NEUTRAL) {
                    diamond->Collect();
                    m_FireboyGems++;
                    LOG_INFO("Fireboy collected a diamond! Total: {}", m_FireboyGems);
                }
            }
        }
    
        if (m_WaterGirl) {
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

    std::unordered_map<int, bool> groupStates;

    std::vector<glm::vec2> interactorPositions;
    if (m_FireBoy) interactorPositions.push_back(fPos);
    if (m_WaterGirl) interactorPositions.push_back(wPos);

    for (const auto &block : m_Blocks) {
      interactorPositions.push_back(block->GetPosition());
    }

    for (auto& activator : m_Activators) {
        activator->Update(interactorPositions);
        if (activator->IsActivated()) {
            int groupId = activator->GetGroupId();

            if (groupId < 0) {
                size_t overlayIdx = std::abs(groupId) - 1;
                if (overlayIdx < m_Overlays.size())
                    m_Overlays[overlayIdx]->StartConversion(activator->m_Transform.translation);
            } else {
                groupStates[groupId] = true;
            }
        }
    }

    for (auto &receiver : m_Receivers) {
        bool isOn = groupStates[receiver->GetGroupId()];
        receiver->SetActivated(isOn);
        receiver->Update();
    }

    // 第二階段：處理機關碰撞 (動態平台)
    std::vector<std::shared_ptr<BaseMechanism>> allMechs;
    for (auto &a : m_Activators) allMechs.push_back(a);
    for (auto &r : m_Receivers) allMechs.push_back(r);
    for (auto &b : m_Blocks) {
        b->Update();
        // Resolve block against terrain
        m_CollisionSystem.ResolveBlockTerrain(*b, *m_LevelManager);
        allMechs.push_back(b);
    }

    if (m_FireBoy) {
        m_FireBoy->SetGroundState(GroundState::AIR);
        m_CollisionSystem.ResolveCharacterTerrain(*m_FireBoy, *m_LevelManager);
        m_CollisionSystem.ResolveCharacterMechanics(*m_FireBoy, allMechs, *m_LevelManager);
    }
    if (m_WaterGirl) {
        m_WaterGirl->SetGroundState(GroundState::AIR);
        m_CollisionSystem.ResolveCharacterTerrain(*m_WaterGirl, *m_LevelManager);
        m_CollisionSystem.ResolveCharacterMechanics(*m_WaterGirl, allMechs, *m_LevelManager);
    }

    // 把玩家的座標給大門
    // (注意：如果 Character 沒有 GetPosition() 函數，請直接用
    // m_Transform.translation)
    if (m_FireDoor != nullptr && m_FireBoy != nullptr) {
        m_FireDoor->Update(m_FireBoy->m_Transform.translation);
    }
    if (m_WaterDoor != nullptr && m_WaterGirl != nullptr) {
        m_WaterDoor->Update(m_WaterGirl->m_Transform.translation);
    }

    // Check for victory
    if (!m_LevelFinished && m_FireDoor && m_WaterDoor) {
        if (m_FireDoor->IsFullyOpen() && m_WaterDoor->IsFullyOpen()) {
            LOG_INFO("Both players reached the doors!");
            m_LevelFinished = true;
            if (m_FireBoy)
                m_FireBoy->PlayEnterDoorAnimation(m_FireDoor->m_Transform.translation);
            if (m_WaterGirl)
                m_WaterGirl->PlayEnterDoorAnimation(m_WaterDoor->m_Transform.translation);
        }
    }
}

void App::SetupMapNodes() {
    m_MapNodes.clear();

    // 關卡 0 (普通)：起點
    m_MapNodes.push_back({0, "level0.json", "", 0.5f, 0.85f, {}, true, false});
    
    // 關卡 2 (普通)：需要先過關卡 0
    m_MapNodes.push_back({2, "level2.json", "", 0.5f, 0.65f, {0}, false, false});
    
    // 關卡 6 (速度)：分支左，需要先過關卡 2
    m_MapNodes.push_back({6, "level6.json", "speed", 0.35f, 0.45f, {2}, false, false});
    
    // 關卡 7 (普通)：分支右，需要先過關卡 2
    m_MapNodes.push_back({7, "level7.json", "", 0.65f, 0.45f, {2}, false, false});
    
    // 關卡 21 (解謎)：終點，只要過關卡 6 或關卡 7 其中一關即可解鎖
    m_MapNodes.push_back({21, "level21.json", "puzzle", 0.5f, 0.25f, {6, 7}, false, false});

    m_CurrentLevelPath = "level0.json";
}

void App::DrawPathLine(const glm::vec2& start, const glm::vec2& end) {
    auto lineSprite = std::make_shared<AtlasSprite>(m_TempleAtlas, "MenuBackground0000");
    auto lineObj = std::make_shared<Util::GameObject>(lineSprite, kDecorationZ);
    
    glm::vec2 diff = end - start;
    float len = glm::length(diff);
    float angle = std::atan2(diff.y, diff.x);
    
    lineObj->m_Transform.translation = start + diff * 0.5f;
    lineObj->m_Transform.rotation = angle;
    // MenuBackground0000 的大小為 512x512
    lineObj->m_Transform.scale = { len / 512.0f, 6.0f / 512.0f };
    // 染成原版地圖的灰色 (#99A5AA)
    lineSprite->SetColorTint({ 0.60f, 0.65f, 0.67f, 1.0f });
    
    m_SceneRoot->AddChild(lineObj);
}

void App::BuildMapScene() {
    ResetSceneRoot();

    // 1. 設置地圖背景 (拼貼 MenuBackground0000 磁磚寫法)
    const float bgTileSize = 512.0F;
    const int cols = (WINDOW_WIDTH / static_cast<int>(bgTileSize)) + 2;
    const int rows = (WINDOW_HEIGHT / static_cast<int>(bgTileSize)) + 2;

    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < rows; ++j) {
            auto bgSprite =
                std::make_shared<AtlasSprite>(m_TempleAtlas, "MenuBackground0000");
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

    // 重新評估各關卡解鎖狀態 (測試用：全部解鎖)
    for (auto& node : m_MapNodes) {
        node.unlocked = true;
    }

    // 2. 繪製關卡連線 (依序連接)
    auto getWorldPos = [&](const MapNode& node) -> glm::vec2 {
        float mapW = 800.0f * m_MapBackgroundScale;
        float mapH = 395.0f * m_MapBackgroundScale;
        return { (node.x - 0.5f) * mapW, (0.5f - node.y) * mapH };
    };

    // 畫線：0 -> 2, 2 -> 6, 2 -> 7, 6 -> 21, 7 -> 21
    DrawPathLine(getWorldPos(m_MapNodes[0]), getWorldPos(m_MapNodes[1])); // 0 -> 2
    DrawPathLine(getWorldPos(m_MapNodes[1]), getWorldPos(m_MapNodes[2])); // 2 -> 6
    DrawPathLine(getWorldPos(m_MapNodes[1]), getWorldPos(m_MapNodes[3])); // 2 -> 7
    DrawPathLine(getWorldPos(m_MapNodes[2]), getWorldPos(m_MapNodes[4])); // 6 -> 21
    DrawPathLine(getWorldPos(m_MapNodes[3]), getWorldPos(m_MapNodes[4])); // 7 -> 21

    // 3. 實體化鑽石按鈕與外框
    for (auto& node : m_MapNodes) {
        glm::vec2 pos = getWorldPos(node);
        std::string frameName = "DiamondDark0000"; // 預設鎖定
        std::string maskFrameName = "DiamondDark_Bkg_Mask0000"; // 預設鎖定外框

        if (node.unlocked) {
            std::string prefix = "Diamond";
            if (node.type == "puzzle") prefix = "DiamondPuzzle";
            else if (node.type == "speed") prefix = "DiamondSpeed";
            
            // 已通關顯示 0003 幀，未通關顯示 0000 幀
            frameName = prefix + (node.completed ? "0003" : "0000");
            maskFrameName = prefix + "_Bkg_Mask0000";
        }

        // 建立黃色/鎖定外框 (ZIndex 稍微低於鑽石)
        auto maskSprite = std::make_shared<AtlasSprite>(m_MenuAtlas, maskFrameName);
        node.maskObject = std::make_shared<Util::GameObject>(maskSprite, kButtonZ - 0.2f);
        node.maskObject->m_Transform.translation = pos;
        node.maskObject->m_Transform.scale = {0.7f, 0.7f};
        m_SceneRoot->AddChild(node.maskObject);

        // 建立鑽石
        node.sprite = std::make_shared<AtlasSprite>(m_MenuAtlas, frameName);
        node.gameObject = std::make_shared<Util::GameObject>(node.sprite, kButtonZ);
        node.gameObject->m_Transform.translation = pos;
        node.gameObject->m_Transform.scale = {0.7f, 0.7f};
        m_SceneRoot->AddChild(node.gameObject);

        // 如果鎖定，疊加鎖頭圖示
        if (!node.unlocked) {
            auto lockSprite = std::make_shared<AtlasSprite>(m_MenuAtlas, "Lock0000");
            node.lockObject = std::make_shared<Util::GameObject>(lockSprite, kButtonZ + 1.0f);
            node.lockObject->m_Transform.translation = pos;
            node.lockObject->m_Transform.scale = {0.6f, 0.6f};
            m_SceneRoot->AddChild(node.lockObject);
        } else {
            node.lockObject.reset();
        }
    }
}

void App::UpdateMapScene() {
    const auto cursor = Util::Input::GetCursorPosition();
    const float worldX = cursor.x;
    const float worldY = cursor.y;

    auto getWorldPos = [&](const MapNode& node) -> glm::vec2 {
        float mapW = 800.0f * m_MapBackgroundScale;
        float mapH = 395.0f * m_MapBackgroundScale;
        return { (node.x - 0.5f) * mapW, (0.5f - node.y) * mapH };
    };

    for (auto& node : m_MapNodes) {
        if (!node.unlocked) continue;

        glm::vec2 pos = getWorldPos(node);
        float dist = glm::distance(pos, glm::vec2(worldX, worldY));

        // 懸停偵測 (半徑 40 像素)
        if (dist < 40.0f) {
            // 點擊時縮小按鈕與外框
            if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
                node.gameObject->m_Transform.scale = {0.65f, 0.65f};
                if (node.maskObject) node.maskObject->m_Transform.scale = {0.65f, 0.65f};
                LOG_INFO("Entering level: {}", node.filename);
                m_CurrentLevelPath = node.filename;
                SwitchScene(Scene::Game);
                return;
            } else {
                // 懸停時放大
                node.gameObject->m_Transform.scale = {0.8f, 0.8f};
                if (node.maskObject) node.maskObject->m_Transform.scale = {0.8f, 0.8f};
            }
        } else {
            // 還原為預設縮放
            node.gameObject->m_Transform.scale = {0.7f, 0.7f};
            if (node.maskObject) node.maskObject->m_Transform.scale = {0.7f, 0.7f};
        }
    }

    // 按下 Backspace 可回主封面
    if (Util::Input::IsKeyUp(Util::Keycode::BACKSPACE)) {
        SwitchScene(Scene::Cover);
    }
}

void App::MarkCurrentLevelCompleted() {
    for (auto& node : m_MapNodes) {
        if (node.filename == m_CurrentLevelPath) {
            node.completed = true;
            break;
        }
    }
}
