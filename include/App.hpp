#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp" // IWYU pragma: export

#include "AtlasSprite.hpp"
#include "SpriteAtlas.hpp"
#include "Util/Renderer.hpp"

class App {
public:
    enum class State {
        START,
        UPDATE,
        END,
    };

    State GetCurrentState() const { return m_CurrentState; }

    void Start();

    void Update();

    void End(); // NOLINT(readability-convert-member-functions-to-static)

private:
    void ShiftFrame(int delta);
    void SetFrameByIndex(std::size_t index);

private:
    State m_CurrentState = State::START;
    Util::Renderer m_Root;

    std::shared_ptr<SpriteAtlas> m_Atlas;
    std::shared_ptr<AtlasSprite> m_AtlasSprite;
    std::shared_ptr<Util::GameObject> m_AtlasObject;
    std::vector<std::string> m_FrameNames;
    std::size_t m_CurrentFrameIndex = 0;
};

#endif
