#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp" // IWYU pragma: export

#include "AtlasSprite.hpp"
#include "Core/Drawable.hpp"
#include "SpriteAtlas.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Renderer.hpp"
#include "LevelManager.hpp"
#include "Character.hpp"

class App {
public:
    enum class State {
        START,
        UPDATE,
        END,
    };

    enum class Scene {
        Cover,
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

private:
    State m_CurrentState = State::START;
    Scene m_CurrentScene = Scene::Cover;
    Util::Renderer m_Root;
    std::shared_ptr<Util::GameObject> m_SceneRoot;

    std::shared_ptr<SpriteAtlas> m_MenuAtlas;
    std::shared_ptr<SpriteAtlas> m_MenuBackgroundAtlas;
    std::shared_ptr<AtlasSprite> m_StartButtonSprite;
    std::shared_ptr<Util::GameObject> m_StartButtonObject;
    std::shared_ptr<AtlasSprite> m_GamePlaceholderSprite;
    float m_StartButtonBaseScale = 1.0F;

    std::shared_ptr<SpriteAtlas> m_GameAtlas;
    std::shared_ptr<SpriteAtlas> m_GroundAtlas;
    std::shared_ptr<SpriteAtlas> m_TempleAtlas;
    std::shared_ptr<LevelManager> m_LevelManager;

    std::shared_ptr<Character> m_FireBoy;
};

#endif
