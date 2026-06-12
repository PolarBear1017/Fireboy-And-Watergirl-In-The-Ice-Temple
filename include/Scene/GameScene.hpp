#ifndef SCENE_GAME_SCENE_HPP
#define SCENE_GAME_SCENE_HPP

#include "IScene.hpp"
#include "CollisionSystem.hpp"
#include "Level/LevelManager.hpp"
#include "Character.hpp"
#include "Door.hpp"
#include "Overlay.hpp"
#include "Mechanics/Activator.hpp"
#include "Mechanics/Receiver.hpp"
#include "Mechanics/Diamond.hpp"
#include "Mechanics/Block.hpp"
#include "Mechanics/TriggerMediator.hpp"
#include <memory>
#include <vector>

class App; // Forward declaration

/**
 * @brief Core Gameplay Scene (Component 1 - State Pattern).
 * Manages characters, mechanisms interaction, levels collision, and gems collection.
 */
class GameScene : public IScene {
private:
    App& m_App;
    std::shared_ptr<Util::GameObject> m_SceneRoot;
    
    std::shared_ptr<LevelManager> m_LevelManager;
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
    
    std::shared_ptr<TriggerMediator> m_TriggerMediator; // Mediator Pattern
    
    int m_FireboyGems = 0;
    int m_WatergirlGems = 0;
    
    // Level State
    bool m_LevelFinished = false;
    float m_LevelFinishTimer = 0.0f;

    std::vector<std::shared_ptr<Util::GameObject>> m_DebugBoxes;

    void BuildGameScene();

public:
    explicit GameScene(App& app);
    virtual ~GameScene() = default;
    
    void Init(std::shared_ptr<Util::GameObject> sceneRoot) override;
    void Update() override;
    void DrawDevMenu() override;
    
    // Dev Tools / Cheat triggers
    void TeleportToDoors();
    void InstantWin();
    void ResetLevel();
    
    bool IsLevelFinished() const { return m_LevelFinished; }
};

#endif // SCENE_GAME_SCENE_HPP
