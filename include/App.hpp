#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp" // IWYU pragma: export

#include "AtlasSprite.hpp"
#include "Core/Drawable.hpp"
#include "SpriteAtlas.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Renderer.hpp"
#include "CollisionSystem.hpp"
#include "Level/LevelManager.hpp"
#include "Character.hpp"
#include "Door.hpp"
#include "Overlay.hpp"
#include "Mechanics/Activator.hpp"
#include "Mechanics/Receiver.hpp"
#include "Mechanics/Diamond.hpp"
#include "Mechanics/Block.hpp"

class App {
public:
    enum class State {
        START,
        UPDATE,
        END,
    };

    enum class Scene {
        Cover,
        Map,
        Game,
    };

    State GetCurrentState() const { return m_CurrentState; }

    void Start();

    void Update();

    void End(); // NOLINT(readability-convert-member-functions-to-static)

private:
    void SwitchScene(Scene scene);
    void ResetSceneRoot();
    void BuildCoverScene();
    void BuildGameScene();
    void UpdateCoverScene();
    void UpdateGameScene();
    void BuildMapScene();
    void UpdateMapScene();
    void DrawPathLine(const glm::vec2& start, const glm::vec2& end);
    void MarkCurrentLevelCompleted();
    void SetupMapNodes();
    void DrawDevMenu();

private:
    State m_CurrentState = State::START;
    Scene m_CurrentScene = Scene::Cover;
    Util::Renderer m_Root;
    std::shared_ptr<Util::GameObject> m_SceneRoot;

    std::shared_ptr<SpriteAtlas> m_MenuAtlas;
    std::shared_ptr<SpriteAtlas> m_MenuBackgroundAtlas;
    std::shared_ptr<SpriteAtlas> m_FontAtlas;
    struct BeamData {
        float speed;
        float offset;
        float baseScale;
    };
    std::shared_ptr<AtlasSprite> m_StartButtonSprite;
    std::shared_ptr<Util::GameObject> m_StartButtonObject;
    std::vector<std::shared_ptr<Util::GameObject>> m_Beams;
    std::vector<BeamData> m_BeamDatas;
    std::shared_ptr<AtlasSprite> m_GamePlaceholderSprite;
    float m_StartButtonBaseScale = 1.0F;
    float m_PlayTextBaseScale = 1.0F;
    std::vector<std::shared_ptr<Util::GameObject>> m_PlayTextObjects;
    std::vector<std::shared_ptr<Util::GameObject>> m_MoreGamesTextObjects;
    std::vector<std::shared_ptr<Util::GameObject>> m_WalkthroughTextObjects;

    std::shared_ptr<SpriteAtlas> m_GameAtlas;
    std::shared_ptr<SpriteAtlas> m_GroundAtlas;
    std::shared_ptr<SpriteAtlas> m_OverlayAtlas;
    std::shared_ptr<SpriteAtlas> m_TempleAtlas;
    std::shared_ptr<LevelManager> m_LevelManager;
    std::shared_ptr<SpriteAtlas> m_MechAtlas;

    CollisionSystem m_CollisionSystem;

    std::shared_ptr<Character> m_FireBoy;
    std::shared_ptr<Character> m_WaterGirl;

    std::shared_ptr<Door> m_FireDoor;
    std::shared_ptr<Door> m_WaterDoor;

    std::vector<std::shared_ptr<Overlay>> m_Overlays;

    // Mechanics
    std::vector<std::shared_ptr<Activator>> m_Activators;
    std::vector<std::shared_ptr<Receiver>> m_Receivers;
    std::vector<std::shared_ptr<Diamond>> m_Diamonds;
    std::vector<std::shared_ptr<Block>> m_Blocks;
    
    int m_FireboyGems = 0;
    int m_WatergirlGems = 0;
    
    // Level State
    bool m_LevelFinished = false;
    float m_LevelFinishTimer = 0.0f;

    struct MapNode {
        int id;
        std::string filename;
        std::string type; // "" (normal), "puzzle", "speed"
        float x;          // 0.0 ~ 1.0
        float y;          // 0.0 ~ 1.0
        std::vector<int> requiredLevels;
        
        bool unlocked = false;
        bool completed = false;
        
        std::shared_ptr<AtlasSprite> sprite;
        std::shared_ptr<Util::GameObject> gameObject;
        std::shared_ptr<Util::GameObject> lockObject;
        std::shared_ptr<Util::GameObject> maskObject;
    };

    std::vector<MapNode> m_MapNodes;
    std::string m_CurrentLevelPath;
    float m_MapBackgroundScale = 1.5F;

    bool m_IndependentRespawn = false;
    bool m_GodMode = false;
    bool m_ShowDevMenu = true;
    bool m_AllLevelsUnlocked = false;
};

#endif
