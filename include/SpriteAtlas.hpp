#ifndef SPRITE_ATLAS_HPP
#define SPRITE_ATLAS_HPP

#include "pch.hpp" // IWYU pragma: export

#include <nlohmann/json.hpp>

#include "Core/Texture.hpp"

struct AtlasFrame {
    int x = 0;
    int y = 0;
    int w = 1;
    int h = 1;
};

class SpriteAtlas {
public:
    SpriteAtlas(std::string imagePath, std::string jsonPath);

    bool HasFrame(const std::string &name) const;
    const AtlasFrame &GetFrame(const std::string &name) const;
    glm::ivec2 GetTextureSize() const { return m_TextureSize; }
    const std::vector<std::string> &GetFrameNames() const { return m_FrameOrder; }
    std::shared_ptr<Core::Texture> GetTexture() const { return m_Texture; }

private:
    void ParseAtlasJson(const std::string &jsonPath);
    bool TryParseFrameRect(const nlohmann::json &source, AtlasFrame &out) const;
    bool InsertFrame(const std::string &name, const AtlasFrame &frame);
    void InsertFallbackFrame();

private:
    std::shared_ptr<Core::Texture> m_Texture;
    glm::ivec2 m_TextureSize = {1, 1};

    std::unordered_map<std::string, AtlasFrame> m_Frames;
    std::vector<std::string> m_FrameOrder;
};

#endif
