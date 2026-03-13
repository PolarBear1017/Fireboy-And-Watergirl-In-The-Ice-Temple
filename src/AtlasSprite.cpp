#include "AtlasSprite.hpp"

#include "Util/Logger.hpp"

AtlasSprite::AtlasSprite(std::shared_ptr<SpriteAtlas> atlas,
                         std::string initialFrame)
    : m_Atlas(std::move(atlas)) {
    if (s_Program == nullptr) {
        InitProgram();
    }
    m_UniformBuffer = std::make_unique<Core::UniformBuffer<Core::Matrices>>(
        *s_Program, "Matrices", 0);

    if (m_Atlas == nullptr) {
        LOG_ERROR("AtlasSprite initialized with null SpriteAtlas.");
        return;
    }

    if (initialFrame.empty() || !m_Atlas->HasFrame(initialFrame)) {
        initialFrame = m_Atlas->GetFrameNames().empty()
                           ? "__fallback__"
                           : m_Atlas->GetFrameNames().front();
    }
    SetFrame(initialFrame);
}

void AtlasSprite::Draw(const Core::Matrices &data) {
    if (m_Atlas == nullptr || m_VertexArray == nullptr || s_Program == nullptr) {
        return;
    }

    m_UniformBuffer->SetData(0, data);
    m_Atlas->GetTexture()->Bind(UNIFORM_SURFACE_LOCATION);

    s_Program->Bind();
    s_Program->Validate();
    m_VertexArray->Bind();
    m_VertexArray->DrawTriangles();
}

void AtlasSprite::SetFrame(const std::string &name) {
    if (m_Atlas == nullptr) {
        LOG_ERROR("SetFrame('{}') failed: atlas is null.", name);
        return;
    }

    std::string resolvedName = name;
    if (!m_Atlas->HasFrame(resolvedName)) {
        LOG_ERROR("Frame '{}' not found in atlas. Fallback to first frame.",
                  name);
        if (m_Atlas->GetFrameNames().empty()) {
            return;
        }
        resolvedName = m_Atlas->GetFrameNames().front();
    }

    const auto &frame = m_Atlas->GetFrame(resolvedName);
    RebuildVertexArray(frame);
    m_CurrentFrame = resolvedName;
    m_Size = {static_cast<float>(frame.w), static_cast<float>(frame.h)};
}

void AtlasSprite::InitProgram() {
    s_Program =
        std::make_unique<Core::Program>(RESOURCE_DIR "/shaders/Base.vert",
                                        RESOURCE_DIR "/shaders/Base.frag");
    s_Program->Bind();

    const GLint location = glGetUniformLocation(s_Program->GetId(), "surface");
    glUniform1i(location, UNIFORM_SURFACE_LOCATION);
}

void AtlasSprite::RebuildVertexArray(const AtlasFrame &frame) {
    m_VertexArray = std::make_unique<Core::VertexArray>();

    m_VertexArray->AddVertexBuffer(std::make_unique<Core::VertexBuffer>(
        std::vector<float>{
            -0.5F, 0.5F,  //
            -0.5F, -0.5F, //
            0.5F, -0.5F,  //
            0.5F, 0.5F,   //
        },
        2));

    const auto textureSize = m_Atlas->GetTextureSize();
    const float texW = static_cast<float>(textureSize.x);
    const float texH = static_cast<float>(textureSize.y);
    const float u0 = static_cast<float>(frame.x) / texW;
    const float v0 = static_cast<float>(frame.y) / texH;
    const float u1 = static_cast<float>(frame.x + frame.w) / texW;
    const float v1 = static_cast<float>(frame.y + frame.h) / texH;

    m_VertexArray->AddVertexBuffer(std::make_unique<Core::VertexBuffer>(
        std::vector<float>{
            u0, v0, //
            u0, v1, //
            u1, v1, //
            u1, v0, //
        },
        2));

    m_VertexArray->SetIndexBuffer(
        std::make_unique<Core::IndexBuffer>(std::vector<unsigned int>{
            0, 1, 2, //
            0, 2, 3, //
        }));
}

std::unique_ptr<Core::Program> AtlasSprite::s_Program = nullptr;
