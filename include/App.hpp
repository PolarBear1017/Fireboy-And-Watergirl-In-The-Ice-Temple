#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp" // IWYU pragma: export

#include "AtlasSprite.hpp"
#include "SpriteAtlas.hpp"
#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include "Scene/IScene.hpp"
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Main application driver (Context class for the State Pattern).
 * Acts as a central cache for game resources and global state.
 */
class App {
public:
    enum class State {
        START,
        UPDATE,
        END,
    };

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

    State GetCurrentState() const { return m_CurrentState; }

    void Start();
    void Update();
    void End();

    /**
     * @brief Performs scene switching by resetting the active renderer and initializing the next scene.
     */
    void SwitchScene(std::unique_ptr<IScene> nextScene);

    /**
     * @brief Marks a level completion in global node records.
     */
    void MarkCurrentLevelCompleted();

    // Global resource accessors (Dependency Injection facilitation)
    [[nodiscard]] std::shared_ptr<SpriteAtlas> GetMenuAtlas() const { return m_MenuAtlas; }
    [[nodiscard]] std::shared_ptr<SpriteAtlas> GetMenuBackgroundAtlas() const { return m_MenuBackgroundAtlas; }
    [[nodiscard]] std::shared_ptr<SpriteAtlas> GetFontAtlas() const { return m_FontAtlas; }
    [[nodiscard]] std::shared_ptr<SpriteAtlas> GetGameAtlas() const { return m_GameAtlas; }
    [[nodiscard]] std::shared_ptr<SpriteAtlas> GetGroundAtlas() const { return m_GroundAtlas; }
    [[nodiscard]] std::shared_ptr<SpriteAtlas> GetOverlayAtlas() const { return m_OverlayAtlas; }
    [[nodiscard]] std::shared_ptr<SpriteAtlas> GetTempleAtlas() const { return m_TempleAtlas; }
    [[nodiscard]] std::shared_ptr<SpriteAtlas> GetMechAtlas() const { return m_MechAtlas; }

    [[nodiscard]] Util::Renderer& GetRenderer() { return m_Root; }
    [[nodiscard]] float GetMapBackgroundScale() const { return m_MapBackgroundScale; }
    [[nodiscard]] std::vector<MapNode>& GetMapNodes() { return m_MapNodes; }
    [[nodiscard]] std::string& GetCurrentLevelPath() { return m_CurrentLevelPath; }

    // Cheat tool variables & toggles (read-write access)
    [[nodiscard]] bool& GetIndependentRespawn() { return m_IndependentRespawn; }
    [[nodiscard]] bool& GetGodMode() { return m_GodMode; }
    [[nodiscard]] bool& GetShowDebugBoxes() { return m_ShowDebugBoxes; }
    [[nodiscard]] bool& GetAllLevelsUnlocked() { return m_AllLevelsUnlocked; }

private:
    void SetupMapNodes();
    void DrawDevMenu();

private:
    State m_CurrentState = State::START;
    std::unique_ptr<IScene> m_CurrentScene; // Current State / Polymorphic Scene Reference
    Util::Renderer m_Root;
    std::shared_ptr<Util::GameObject> m_SceneRoot;

    // Sprite atlas resource caches (shared across multiple scenes)
    std::shared_ptr<SpriteAtlas> m_MenuAtlas;
    std::shared_ptr<SpriteAtlas> m_MenuBackgroundAtlas;
    std::shared_ptr<SpriteAtlas> m_FontAtlas;
    std::shared_ptr<SpriteAtlas> m_GameAtlas;
    std::shared_ptr<SpriteAtlas> m_GroundAtlas;
    std::shared_ptr<SpriteAtlas> m_OverlayAtlas;
    std::shared_ptr<SpriteAtlas> m_TempleAtlas;
    std::shared_ptr<SpriteAtlas> m_MechAtlas;

    // Persistent game context
    std::vector<MapNode> m_MapNodes;
    std::string m_CurrentLevelPath;
    float m_MapBackgroundScale = 1.5F;

    // Dev settings
    bool m_IndependentRespawn = false;
    bool m_GodMode = false;
    bool m_ShowDevMenu = true;
    bool m_AllLevelsUnlocked = false;
    bool m_ShowDebugBoxes = false;
};

#endif // APP_HPP
