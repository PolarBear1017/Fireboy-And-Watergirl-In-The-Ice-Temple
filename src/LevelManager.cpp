#include "LevelManager.hpp"

#include "AtlasSprite.hpp"
#include "Util/Logger.hpp"
#include "config.hpp" // For WINDOW_WIDTH / WINDOW_HEIGHT

LevelManager::LevelManager(std::shared_ptr<SpriteAtlas> atlas)
    : m_Atlas(std::move(atlas)) {}

bool LevelManager::ValidateLevel(const std::vector<std::string>& mapData) const {
    if (mapData.empty()) {
        LOG_ERROR("LevelManager: mapData is empty.");
        return false;
    }

    const size_t expectedWidth = mapData.front().size();
    if (expectedWidth == 0U) {
        LOG_ERROR("LevelManager: first row is empty.");
        return false;
    }

    int fireSpawnCount = 0;
    int waterSpawnCount = 0;
    int fireDoorCount = 0;
    int waterDoorCount = 0;

    for (size_t row = 0; row < mapData.size(); ++row) {
        if (mapData[row].size() != expectedWidth) {
            LOG_ERROR("LevelManager: row " + std::to_string(row) +
                      " width mismatch.");
            return false;
        }

        for (const char tileChar : mapData[row]) {
            switch (tileChar) {
                case '.':
                case 'X':
                case 'F':
                case 'G':
                case 'L':
                case 'W':
                case 'R':
                case 'B':
                    break;
                default:
                    LOG_ERROR("LevelManager: unsupported tile character.");
                    return false;
            }

            fireSpawnCount += tileChar == 'F' ? 1 : 0;
            waterSpawnCount += tileChar == 'G' ? 1 : 0;
            fireDoorCount += tileChar == 'R' ? 1 : 0;
            waterDoorCount += tileChar == 'B' ? 1 : 0;
        }
    }

    if (fireSpawnCount != 1 || waterSpawnCount != 1) {
        LOG_ERROR("LevelManager: level must contain exactly one `F` and one `G`.");
        return false;
    }

    if (fireDoorCount > 1 || waterDoorCount > 1) {
        LOG_ERROR("LevelManager: level can contain at most one `R` and one `B`.");
        return false;
    }

    return true;
}

void LevelManager::RegisterTile(const char tileChar, const int row, const int col) {
    const TileCoord coord{row, col};

    switch (tileChar) {
        case 'X':
            m_LevelData.solidTiles.push_back(coord);
            break;
        case 'L':
            m_LevelData.lavaTiles.push_back(coord);
            break;
        case 'W':
            m_LevelData.waterTiles.push_back(coord);
            break;
        case 'F':
            m_LevelData.fireSpawn = coord;
            m_LevelData.hasFireSpawn = true;
            break;
        case 'G':
            m_LevelData.waterSpawn = coord;
            m_LevelData.hasWaterSpawn = true;
            break;
        case 'R':
            m_LevelData.fireDoor = coord;
            m_LevelData.hasFireDoor = true;
            break;
        case 'B':
            m_LevelData.waterDoor = coord;
            m_LevelData.hasWaterDoor = true;
            break;
        default:
            break;
    }
}

bool LevelManager::HasSameTile(const std::vector<std::string>& mapData, const int row,
                               const int col, const char tileChar) const {
    if (row < 0 || col < 0) {
        return false;
    }

    if (row >= static_cast<int>(mapData.size())) {
        return false;
    }

    if (col >= static_cast<int>(mapData[row].size())) {
        return false;
    }

    return mapData[row][col] == tileChar;
}

std::string LevelManager::ResolveFrameName(const std::vector<std::string>& mapData,
                                           const int row, const int col,
                                           const char tileChar) const {
    const bool hasLeft = HasSameTile(mapData, row, col - 1, tileChar);
    const bool hasRight = HasSameTile(mapData, row, col + 1, tileChar);

    switch (tileChar) {
        case 'X':
            if (!hasLeft && hasRight) {
                return "BlackBoxLeft0000";
            }
            if (hasLeft && !hasRight) {
                return "BlackBoxRight0000";
            }
            return "BlackBox0000";
        case 'L':
            if (!hasLeft && hasRight) {
                return "FireBoxLeft0000";
            }
            if (hasLeft && !hasRight) {
                return "FireBoxRight0000";
            }
            return "FireBox0000";
        case 'W':
            if (!hasLeft && hasRight) {
                return "WaterBoxLeft0000";
            }
            if (hasLeft && !hasRight) {
                return "WaterBoxRight0000";
            }
            return "WaterBox0000";
        case 'F':
            return "FireBox0000";
        case 'G':
            return "WaterBox0000";
        case 'R':
            return "FireBox0000";
        case 'B':
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
        case 'R':
        case 'B':
            return 2.0F;
        case 'X':
        default:
            return -1.0F;
    }
}

void LevelManager::LoadLevel(const std::vector<std::string>& mapData,
                             const std::shared_ptr<Util::GameObject>& root) {
    if (!ValidateLevel(mapData)) {
        return;
    }

    m_LevelData = {};

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
            RegisterTile(tileChar, y, x);
            const std::string frameName = ResolveFrameName(mapData, y, x, tileChar);
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
            if (tileChar == 'F' || tileChar == 'G' ||
                tileChar == 'R' || tileChar == 'B') {
                tileObj->m_Transform.scale *= 0.75F;
            }

            root->AddChild(tileObj);
        }
    }
}
