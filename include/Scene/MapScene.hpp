#ifndef SCENE_MAP_SCENE_HPP
#define SCENE_MAP_SCENE_HPP

#include "IScene.hpp"
#include "Util/GameObject.hpp"
#include <memory>
#include <glm/vec2.hpp>

class App; // Forward declaration

/**
 * @brief Map / Level Selection Scene (Component 1 - State Pattern).
 * Displays levels network, unlocking logic, and paths.
 */
class MapScene : public IScene {
private:
    App& m_App;
    std::shared_ptr<Util::GameObject> m_SceneRoot;

    void DrawPathLine(const glm::vec2& start, const glm::vec2& end);

public:
    explicit MapScene(App& app);

    void Init(std::shared_ptr<Util::GameObject> sceneRoot) override;
    void Update() override;
    void DrawDevMenu() override;
};

#endif // SCENE_MAP_SCENE_HPP
