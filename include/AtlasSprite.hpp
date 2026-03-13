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
};

#endif
