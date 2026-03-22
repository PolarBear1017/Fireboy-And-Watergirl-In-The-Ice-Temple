#include "LevelManager.hpp"

#include "AtlasSprite.hpp"
#include "config.hpp" // For WINDOW_WIDTH / WINDOW_HEIGHT

LevelManager::LevelManager(std::shared_ptr<SpriteAtlas> atlas)
    : m_Atlas(std::move(atlas)) {}

std::string LevelManager::ResolveFrameName(const char tileChar) const {
    switch (tileChar) {
        case 'X':
            return "BlackBox0000";
        case 'L':
            return "FireBox0000";
        case 'W':
            return "WaterBox0000";
        case 'F':
            return "FireBox0000";
        case 'G':
            return "WaterBox0000";
        default:
            return "";
    }
}

float LevelManager::ResolveZIndex(const char tileChar) const {
    switch (tileChar) {
        case 'W':
        case 'L':
            return -2.0F;
        case 'F':
        case 'G':
            return 2.0F;
        case 'X':
        default:
            return -1.0F;
    }
}

void LevelManager::LoadLevel(const std::vector<std::string>& mapData,
                             const std::shared_ptr<Util::GameObject>& root) {
    if (mapData.empty()) {
        return;
    }

    const float tileSize = 32.0f;
    const int rows = static_cast<int>(mapData.size());
    const int cols = static_cast<int>(mapData[0].size());
    const float startX = -(static_cast<float>(WINDOW_WIDTH) / 2.0f) + (tileSize / 2.0f);
    const float startY = (static_cast<float>(WINDOW_HEIGHT) / 2.0f) - (tileSize / 2.0f);

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            if (static_cast<size_t>(x) >= mapData[y].size()) {
                continue;
            }

            const char tileChar = mapData[y][x];
            const std::string frameName = ResolveFrameName(tileChar);
            if (frameName.empty() || !m_Atlas->HasFrame(frameName)) {
                continue;
            }

            auto sprite = std::make_shared<AtlasSprite>(m_Atlas, frameName);
            auto tileObj =
                std::make_shared<Util::GameObject>(sprite, ResolveZIndex(tileChar));

            tileObj->m_Transform.translation = {
                startX + static_cast<float>(x) * tileSize,
                startY - static_cast<float>(y) * tileSize,
            };

            const auto actualSize = sprite->GetSize();
            if (actualSize.x > 0.0F && actualSize.y > 0.0F) {
                tileObj->m_Transform.scale = {
                    tileSize / actualSize.x,
                    tileSize / actualSize.y,
                };
            }

            // Spawn markers are temporary visual hints until Character objects
            // are instantiated from the map data.
            if (tileChar == 'F' || tileChar == 'G') {
                tileObj->m_Transform.scale *= 0.75F;
            }

            root->AddChild(tileObj);
        }
    }
}
