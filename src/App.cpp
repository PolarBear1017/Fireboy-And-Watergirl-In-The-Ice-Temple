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
  m_Beams.clear();
  m_BeamDatas.clear();
  m_PlayTextObjects.clear();
  m_MoreGamesTextObjects.clear();
  m_WalkthroughTextObjects.clear();
  m_GamePlaceholderSprite.reset();
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

  m_Activators.clear();
  m_Receivers.clear();
  m_Diamonds.clear();

  // 2. 嚴格載入 JSON 關卡 (拔掉 try-catch 強制依賴 JSON)
  const LevelDefinition level =
      LoadLevelDefinitionFromJsonFile(BuildLevelPath("level1.json"));
  if (!m_LevelManager->LoadLevel(level, m_SceneRoot)) {
    LOG_ERROR("Level validation failed after JSON load.");
    return;
  }

  // 3. 🌟 實體化機關 (讓電梯跟按鈕出現)
  for (const auto &obj : level.objects) {
    glm::vec2 pos =
        m_LevelManager->TileToWorldPosition(obj.coord.row, obj.coord.col);
    if (obj.type == LevelObjectType::Button) {
      auto button = std::make_shared<Button>(m_MechAtlas, pos, obj.group_id);
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

      auto elevator = std::make_shared<Elevator>(m_MechAtlas, pos, targetPos,
                                                 size, obj.group_id);
      m_Receivers.push_back(elevator);
      m_SceneRoot->AddChild(elevator);
    } else if (obj.type == LevelObjectType::Diamond) {
      std::string frameName = (obj.element == Element::FIRE) ? "diamond_fb0000" : "diamond_wg0000";
      auto sprite = std::make_shared<AtlasSprite>(m_GameAtlas, frameName);
      auto diamond = std::make_shared<Diamond>(sprite, 5.0f, obj.element);
      diamond->m_Transform.translation = pos;
      diamond->m_Transform.scale = glm::vec2(0.8f);
      m_Diamonds.push_back(diamond);
      m_SceneRoot->AddChild(diamond);
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
    SwitchScene(Scene::Game);
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
      SwitchScene(Scene::Game);
    }
  }
}

void App::UpdateGameScene() {
  const auto elapsedSeconds = Util::Time::GetDeltaTime();

  if (m_LevelFinished) {
    m_LevelFinishTimer += elapsedSeconds;

    if (m_FireBoy) {
      m_FireBoy->SetInputEnabled(false);
      // Removed m_FireBoy->SetVisible(false); because PlayEnterDoorAnimation handles visibility
    }
    if (m_WaterGirl) {
      m_WaterGirl->SetInputEnabled(false);
      // Removed m_WaterGirl->SetVisible(false); because PlayEnterDoorAnimation handles visibility
    }

    if (m_LevelFinishTimer >= 1.6f) { // Adjusted from 2.0f to 1.6f
      LOG_INFO("Level Complete! Returning to Menu.");
      SwitchScene(Scene::Cover);
    }
    // Still need to update doors and other things to let animations finish
  }

  if (Util::Input::IsKeyUp(Util::Keycode::BACKSPACE)) {
    SwitchScene(Scene::Cover);
  }

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

  // 1. 呼叫機關邏輯 (Polymorphic Update)
  glm::vec2 fPos = m_FireBoy ? m_FireBoy->GetPosition() : glm::vec2(0.0f);
  glm::vec2 wPos = m_WaterGirl ? m_WaterGirl->GetPosition() : glm::vec2(0.0f);

  std::unordered_map<int, bool> groupStates;
  for (auto &activator : m_Activators) {
    activator->Update(fPos, wPos);
    if (activator->IsActivated()) {
      groupStates[activator->GetGroupId()] = true;
    }
  }

  for (auto &receiver : m_Receivers) {
    bool isOn = groupStates[receiver->GetGroupId()];
    receiver->SetActivated(isOn);
    receiver->Update();
  }

  // 第二階段：處理機關碰撞 (動態平台)
  std::vector<std::shared_ptr<BaseMechanism>> allMechs;
  for (auto &a : m_Activators)
    allMechs.push_back(a);
  for (auto &r : m_Receivers)
    allMechs.push_back(r);

  if (m_FireBoy) {
    m_FireBoy->SetGroundState(GroundState::AIR);
    m_CollisionSystem.ResolveCharacterTerrain(*m_FireBoy, *m_LevelManager);
    m_CollisionSystem.ResolveCharacterMechanics(*m_FireBoy, allMechs);
  }
  if (m_WaterGirl) {
    m_WaterGirl->SetGroundState(GroundState::AIR);
    m_CollisionSystem.ResolveCharacterTerrain(*m_WaterGirl, *m_LevelManager);
    m_CollisionSystem.ResolveCharacterMechanics(*m_WaterGirl, allMechs);
  }

  // --- 呼叫角色更新 ---
  if (m_FireBoy != nullptr) {
    m_FireBoy->Update();
  }
  if (m_WaterGirl != nullptr) {
    m_WaterGirl->Update();
  }

  // 🌟 呼叫大門的邏輯：把玩家目前的精確座標餵給大門！
  // (注意：如果你們 Character 沒有 GetPosition() 函數，請直接用
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
      if (m_FireBoy) m_FireBoy->PlayEnterDoorAnimation(m_FireDoor->m_Transform.translation);
      if (m_WaterGirl) m_WaterGirl->PlayEnterDoorAnimation(m_WaterDoor->m_Transform.translation);
    }
  }
}
