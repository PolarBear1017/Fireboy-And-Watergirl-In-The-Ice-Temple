#include "SpriteAtlas.hpp"

#include <fstream>

#include <nlohmann/json.hpp>

#include "Core/TextureUtils.hpp"
#include "Util/Logger.hpp"
#include "Util/MissingTexture.hpp"

namespace {
std::shared_ptr<SDL_Surface> LoadAtlasSurface(const std::string &imagePath) {
    auto surface = std::shared_ptr<SDL_Surface>(IMG_Load(imagePath.c_str()),
                                                SDL_FreeSurface);
    if (surface != nullptr) {
        return surface;
    }

    LOG_ERROR("Failed to load atlas image: '{}'", imagePath);
    LOG_ERROR("{}", IMG_GetError());
    return {GetMissingTextureSDLSurface(), SDL_FreeSurface};
}

std::string ExtractFrameName(const nlohmann::json &source) {
    if (source.contains("filename") && source["filename"].is_string()) {
        return source["filename"].get<std::string>();
    }
    if (source.contains("name") && source["name"].is_string()) {
        return source["name"].get<std::string>();
    }
    if (source.contains("id") && source["id"].is_string()) {
        return source["id"].get<std::string>();
    }
    return "";
}
} // namespace

SpriteAtlas::SpriteAtlas(std::string imagePath, std::string jsonPath) {
    auto surface = LoadAtlasSurface(imagePath);
    m_TextureSize = {surface->w, surface->h};
    m_Texture = std::make_shared<Core::Texture>(
        Core::SdlFormatToGlFormat(surface->format->format), surface->w,
        surface->h, surface->pixels);

    ParseAtlasJson(jsonPath);
    if (m_FrameOrder.empty()) {
        InsertFallbackFrame();
    }
}

bool SpriteAtlas::HasFrame(const std::string &name) const {
    return m_Frames.find(name) != m_Frames.end();
}

const AtlasFrame &SpriteAtlas::GetFrame(const std::string &name) const {
    auto it = m_Frames.find(name);
    if (it != m_Frames.end()) {
        return it->second;
    }

    if (!m_FrameOrder.empty()) {
        return m_Frames.at(m_FrameOrder.front());
    }

    static const AtlasFrame fallback{0, 0, 1, 1};
    return fallback;
}

void SpriteAtlas::ParseAtlasJson(const std::string &jsonPath) {
    std::ifstream in(jsonPath);
    if (!in.is_open()) {
        LOG_ERROR("Failed to open atlas json: '{}'", jsonPath);
        return;
    }

    nlohmann::json root;
    try {
        in >> root;
    } catch (const std::exception &e) {
        LOG_ERROR("Failed to parse atlas json: '{}'", jsonPath);
        LOG_ERROR("{}", e.what());
        return;
    }

    if (root.contains("meta") && root["meta"].is_object() &&
        root["meta"].contains("size")) {
        const auto &size = root["meta"]["size"];
        if (size.contains("w") && size.contains("h") && size["w"].is_number() &&
            size["h"].is_number()) {
            const auto jsonW = size["w"].get<int>();
            const auto jsonH = size["h"].get<int>();
            if (jsonW != m_TextureSize.x || jsonH != m_TextureSize.y) {
                LOG_WARN(
                    "Atlas texture size mismatch. image=({}, {}), json=({}, "
                    "{})",
                    m_TextureSize.x, m_TextureSize.y, jsonW, jsonH);
            }
        }
    }

    if (root.contains("frames")) {
        const auto &frames = root["frames"];
        if (frames.is_object()) {
            for (const auto &[name, frameJson] : frames.items()) {
                AtlasFrame frame{};
                if (TryParseFrameRect(frameJson, frame)) {
                    InsertFrame(name, frame);
                }
            }
            return;
        }

        if (frames.is_array()) {
            for (const auto &frameJson : frames) {
                const auto name = ExtractFrameName(frameJson);
                AtlasFrame frame{};
                if (!name.empty() && TryParseFrameRect(frameJson, frame)) {
                    InsertFrame(name, frame);
                }
            }
            return;
        }
    }

    if (root.is_object()) {
        for (const auto &[name, frameJson] : root.items()) {
            if (name == "meta" || name == "frames") {
                continue;
            }
            AtlasFrame frame{};
            if (TryParseFrameRect(frameJson, frame)) {
                InsertFrame(name, frame);
            }
        }
    }
}

bool SpriteAtlas::TryParseFrameRect(const nlohmann::json &source,
                                    AtlasFrame &out) const {
    const nlohmann::json *frame = &source;
    if (source.contains("frame") && source["frame"].is_object()) {
        frame = &source["frame"];
    }

    if (frame->contains("x") && frame->contains("y") && frame->contains("w") &&
        frame->contains("h")) {
        out = {(*frame)["x"].get<int>(), (*frame)["y"].get<int>(),
               (*frame)["w"].get<int>(), (*frame)["h"].get<int>()};
        return true;
    }

    if (frame->contains("x") && frame->contains("y") &&
        frame->contains("width") && frame->contains("height")) {
        out = {(*frame)["x"].get<int>(), (*frame)["y"].get<int>(),
               (*frame)["width"].get<int>(), (*frame)["height"].get<int>()};
        return true;
    }

    if (frame->contains("left") && frame->contains("top") &&
        frame->contains("width") && frame->contains("height")) {
        out = {(*frame)["left"].get<int>(), (*frame)["top"].get<int>(),
               (*frame)["width"].get<int>(), (*frame)["height"].get<int>()};
        return true;
    }

    return false;
}

bool SpriteAtlas::InsertFrame(const std::string &name, const AtlasFrame &frame) {
    if (name.empty()) {
        return false;
    }

    if (frame.w <= 0 || frame.h <= 0) {
        LOG_WARN("Ignore frame '{}': non-positive frame size({}, {})", name,
                 frame.w, frame.h);
        return false;
    }

    AtlasFrame safeFrame = frame;
    safeFrame.x = std::max(0, std::min(safeFrame.x, m_TextureSize.x - 1));
    safeFrame.y = std::max(0, std::min(safeFrame.y, m_TextureSize.y - 1));

    const auto maxW = m_TextureSize.x - safeFrame.x;
    const auto maxH = m_TextureSize.y - safeFrame.y;
    safeFrame.w = std::max(1, std::min(safeFrame.w, maxW));
    safeFrame.h = std::max(1, std::min(safeFrame.h, maxH));

    if (m_Frames.find(name) != m_Frames.end()) {
        LOG_WARN("Duplicate frame '{}' found in atlas json. Keep first one.",
                 name);
        return false;
    }

    m_Frames.emplace(name, safeFrame);
    m_FrameOrder.push_back(name);
    return true;
}

void SpriteAtlas::InsertFallbackFrame() {
    LOG_WARN("Atlas has no valid frame definitions. Use fallback frame.");
    m_Frames.emplace("__fallback__", AtlasFrame{0, 0, m_TextureSize.x, m_TextureSize.y});
    m_FrameOrder.emplace_back("__fallback__");
}
