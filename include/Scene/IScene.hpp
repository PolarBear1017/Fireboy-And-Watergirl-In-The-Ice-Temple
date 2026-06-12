#ifndef SCENE_I_SCENE_HPP
#define SCENE_I_SCENE_HPP

#include <memory>

namespace Util {
class GameObject;
}

/**
 * @brief Interface representing a game scene (State Pattern / Polymorphism).
 * Each scene implements its own logic, lifecycle, and rendering root.
 */
class IScene {
public:
    virtual ~IScene() = default;
    
    /**
     * @brief Called when transitioning into this scene.
     */
    virtual void Init(std::shared_ptr<Util::GameObject> sceneRoot) = 0;
    
    /**
     * @brief Called every frame to update game state.
     */
    virtual void Update() = 0;

    /**
     * @brief Optional method to draw scene-specific developer options.
     */
    virtual void DrawDevMenu() {}
};

#endif // SCENE_I_SCENE_HPP
