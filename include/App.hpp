#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp" // IWYU pragma: export

#include "AtlasSprite.hpp"
#include "Core/Drawable.hpp"
#include "SpriteAtlas.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Renderer.hpp"

class App {
public:
    enum class PreviewKind {
        Atlas,
        Image,
    };

    struct PreviewDefinition {
        PreviewKind kind = PreviewKind::Atlas;
        std::string name;
        std::string imagePath;
        std::string jsonPath;
    };

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
    void LoadPreviewByIndex(std::size_t index);
    void ApplyPreviewScale(const glm::vec2 &size);
    static std::string FindInitialFrame(const SpriteAtlas &atlas,
                                        const std::vector<std::string> &frameNames);
    void ShiftFrame(int delta);
    void ShiftPreview(int delta);
    void SetFrameByIndex(std::size_t index);

private:
    State m_CurrentState = State::START;
    Util::Renderer m_Root;

    std::shared_ptr<SpriteAtlas> m_Atlas;
    std::shared_ptr<AtlasSprite> m_AtlasSprite;
    std::shared_ptr<Util::Image> m_ImagePreview;
    std::shared_ptr<Util::GameObject> m_PreviewObject;
    std::vector<PreviewDefinition> m_PreviewDefinitions;
    std::vector<std::string> m_FrameNames;
    std::size_t m_CurrentPreviewIndex = 0;
    std::size_t m_CurrentFrameIndex = 0;
};

#endif
