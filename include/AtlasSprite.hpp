#ifndef ATLAS_SPRITE_HPP
#define ATLAS_SPRITE_HPP

#include "pch.hpp" // IWYU pragma: export

#include "Core/Drawable.hpp"
#include "Core/Program.hpp"
#include "Core/UniformBuffer.hpp"
#include "Core/VertexArray.hpp"

#include "SpriteAtlas.hpp"

class AtlasSprite : public Core::Drawable {
public:
    AtlasSprite(std::shared_ptr<SpriteAtlas> atlas, std::string initialFrame);

    glm::vec2 GetSize() const override { return m_Size; }
    void Draw(const Core::Matrices &data) override;
    void SetFrame(const std::string &name);

    void SetColorTint(const glm::vec4 &tint) { m_ColorTint = tint; }
    glm::vec4 GetColorTint() const { return m_ColorTint; }

    void SetUsePureColor(bool usePureColor) { m_UsePureColor = usePureColor; }
    bool GetUsePureColor() const { return m_UsePureColor; }

    void SetClockMask(bool useMask, float timeRatio) {
        m_UseClockMask = useMask;
        m_TimeRatio = timeRatio;
    }

private:
    void InitProgram();
    void RebuildVertexArray(const AtlasFrame &frame);

private:
    static constexpr int UNIFORM_SURFACE_LOCATION = 0;

    static std::unique_ptr<Core::Program> s_Program;
    std::unique_ptr<Core::UniformBuffer<Core::Matrices>> m_UniformBuffer;
    std::unique_ptr<Core::VertexArray> m_VertexArray;

    std::shared_ptr<SpriteAtlas> m_Atlas;
    std::string m_CurrentFrame;
    glm::vec2 m_Size = {1.0F, 1.0F};
    glm::vec4 m_ColorTint = {1.0F, 1.0F, 1.0F, 1.0F};

    bool m_UseClockMask = false;
    float m_TimeRatio = 1.0f;
    bool m_UsePureColor = false;
};

#endif
