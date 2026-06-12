#ifndef SCENE_COVER_SCENE_HPP
#define SCENE_COVER_SCENE_HPP

#include "IScene.hpp"
#include "AtlasSprite.hpp"
#include "Util/GameObject.hpp"
#include <memory>
#include <vector>

class App; // Forward declaration

/**
 * @brief Game Cover / Title Scene (Component 1 - State Pattern).
 * Handles the animated beam background and primary landing options.
 */
class CoverScene : public IScene {
private:
    App& m_App;
    std::shared_ptr<Util::GameObject> m_SceneRoot;
    
    std::shared_ptr<AtlasSprite> m_StartButtonSprite;
    std::shared_ptr<Util::GameObject> m_StartButtonObject;
    std::vector<std::shared_ptr<Util::GameObject>> m_Beams;
    
    struct BeamData {
        float speed;
        float offset;
        float baseScale;
    };
    std::vector<BeamData> m_BeamDatas;
    
    std::vector<std::shared_ptr<Util::GameObject>> m_PlayTextObjects;
    std::vector<std::shared_ptr<Util::GameObject>> m_MoreGamesTextObjects;
    std::vector<std::shared_ptr<Util::GameObject>> m_WalkthroughTextObjects;

    float m_StartButtonBaseScale = 1.0F;

public:
    explicit CoverScene(App& app);
    
    void Init(std::shared_ptr<Util::GameObject> sceneRoot) override;
    void Update() override;
};

#endif // SCENE_COVER_SCENE_HPP
