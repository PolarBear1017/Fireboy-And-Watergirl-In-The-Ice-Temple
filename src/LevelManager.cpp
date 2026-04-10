#include "LevelManager.hpp"

#include "AtlasSprite.hpp"
#include "LevelParser.hpp"
#include "Util/Logger.hpp"
#include "config.hpp"
#include <cmath>

LevelManager::LevelManager(std::shared_ptr<SpriteAtlas> atlas)
    : m_Atlas(std::move(atlas)) {}

bool LevelManager::IsValidGroundCoord(const int row, const int col) const {
    if (row < 0 || col < 0) {
        return false;
    }

    if (row >= static_cast<int>(m_LevelDefinition.ground.size())) {
        return false;
    }

    if (col >= static_cast<int>(
                   m_LevelDefinition.ground[static_cast<size_t>(row)].size())) {
        return false;
    }

    return true;
}

bool LevelManager::IsTileInPool(const LevelPool& pool, const int row,
                                const int col) const {
    for (const auto& tile : pool.tiles) {
        if (tile.row == row && tile.col == col) {
            return true;
        }
    }

    return false;
}

bool LevelManager::IsSolidTile(const int row, const int col) const {
    if (!IsValidGroundCoord(row, col)) {
        return false;
    }

    const TerrainType terrain =
        m_LevelDefinition.ground[static_cast<size_t>(row)][static_cast<size_t>(col)];
    if (terrain == TerrainType::Solid || terrain == TerrainType::Ice ||
        terrain == TerrainType::Snow) {
        return true;
    }

    const LevelPool* pool = GetPoolAt(row, col);
    return pool != nullptr && pool->state == PoolState::Frozen;
}

const LevelPool* LevelManager::GetPoolAt(const int row, const int col) const {
    if (!IsValidGroundCoord(row, col)) {
        return nullptr;
    }

    for (const auto& pool : m_LevelDefinition.pools) {
        if (IsTileInPool(pool, row, col)) {
            return &pool;
        }
    }

    return nullptr;
}

bool LevelManager::IsHazardousFor(const Element element, const int row,
                                  const int col) const {
    const LevelPool* pool = GetPoolAt(row, col);
    if (pool == nullptr || pool->state == PoolState::Frozen) {
        return false;
    }

    if (pool->kind == PoolKind::Fire) {
        return element == Element::WATER;
    }

    if (pool->kind == PoolKind::Water) {
        return element == Element::FIRE;
    }

    return false;
}

glm::vec2 LevelManager::TileToWorldPosition(const int row, const int col) const {
    const float startX =
        -(static_cast<float>(WINDOW_WIDTH) / 2.0F) + (m_TileSize / 2.0F);
    const float startY =
        (static_cast<float>(WINDOW_HEIGHT) / 2.0F) - (m_TileSize / 2.0F);

    return {
        startX + static_cast<float>(col) * m_TileSize,
        startY - static_cast<float>(row) * m_TileSize,
    };
}

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
                case 'I':
                case 'S':
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

bool LevelManager::ValidateLevelDefinition(const LevelDefinition& level) const {
    if (level.width <= 0 || level.height <= 0) {
        LOG_ERROR("LevelManager: level definition size is invalid.");
        return false;
    }

    if (level.tileSize <= 0) {
        LOG_ERROR("LevelManager: tileSize must be positive.");
        return false;
    }

    if (static_cast<int>(level.ground.size()) != level.height) {
        LOG_ERROR("LevelManager: ground row count does not match level height.");
        return false;
    }

    int fireSpawnCount = 0;
    int waterSpawnCount = 0;
    int fireDoorCount = 0;
    int waterDoorCount = 0;

    for (int row = 0; row < level.height; ++row) {
        if (static_cast<int>(level.ground[static_cast<size_t>(row)].size()) !=
            level.width) {
            LOG_ERROR("LevelManager: ground column count does not match level width.");
            return false;
        }
    }

    std::vector<std::vector<bool>> occupiedPoolTiles(
        static_cast<size_t>(level.height),
        std::vector<bool>(static_cast<size_t>(level.width), false));

    for (const auto& pool : level.pools) {
        if (pool.tiles.empty()) {
            LOG_ERROR("LevelManager: pool must contain at least one tile.");
            return false;
        }

        for (const auto& tile : pool.tiles) {
            if (tile.row < 0 || tile.col < 0 || tile.row >= level.height ||
                tile.col >= level.width) {
                LOG_ERROR("LevelManager: pool tile coordinate is out of range.");
                return false;
            }

            auto occupied =
                occupiedPoolTiles[static_cast<size_t>(tile.row)][static_cast<size_t>(tile.col)];
            if (occupied) {
                LOG_ERROR("LevelManager: pool tiles cannot overlap.");
                return false;
            }

            occupiedPoolTiles[static_cast<size_t>(tile.row)][static_cast<size_t>(tile.col)] =
                true;
        }
    }

    for (const auto& object : level.objects) {
        if (object.coord.row < 0 || object.coord.col < 0 ||
            object.coord.row >= level.height || object.coord.col >= level.width) {
            LOG_ERROR("LevelManager: object coordinate is out of range.");
            return false;
        }

        fireSpawnCount += object.type == LevelObjectType::FireSpawn ? 1 : 0;
        waterSpawnCount += object.type == LevelObjectType::WaterSpawn ? 1 : 0;
        fireDoorCount += object.type == LevelObjectType::FireDoor ? 1 : 0;
        waterDoorCount += object.type == LevelObjectType::WaterDoor ? 1 : 0;
    }

    if (fireSpawnCount != 1 || waterSpawnCount != 1) {
        LOG_ERROR(
            "LevelManager: level must contain exactly one fire spawn and one water spawn.");
        return false;
    }

    if (fireDoorCount > 1 || waterDoorCount > 1) {
        LOG_ERROR(
            "LevelManager: level can contain at most one fire door and one water door.");
        return false;
    }

    return true;
}

void LevelManager::RegisterTerrain(const TerrainType terrain, const int row,
                                   const int col) {
    if (terrain == TerrainType::Empty) {
        return;
    }

    m_LevelData.solidTiles.push_back(TileCoord{row, col});
}

void LevelManager::RegisterObject(const LevelObject& object) {
    const TileCoord coord{object.coord.row, object.coord.col};

    switch (object.type) {
        case LevelObjectType::FireSpawn:
            m_LevelData.fireSpawn = coord;
            m_LevelData.hasFireSpawn = true;
            break;
        case LevelObjectType::WaterSpawn:
            m_LevelData.waterSpawn = coord;
            m_LevelData.hasWaterSpawn = true;
            break;
        case LevelObjectType::FireDoor:
            m_LevelData.fireDoor = coord;
            m_LevelData.hasFireDoor = true;
            break;
        case LevelObjectType::WaterDoor:
            m_LevelData.waterDoor = coord;
            m_LevelData.hasWaterDoor = true;
            break;
        default:
            break;
    }
}

bool LevelManager::HasSameTerrain(const LevelDefinition& level, const int row,
                                  const int col,
                                  const TerrainType terrain) const {
    if (row < 0 || col < 0) {
        return false;
    }

    if (row >= static_cast<int>(level.ground.size())) {
        return false;
    }

    if (col >= static_cast<int>(level.ground[static_cast<size_t>(row)].size())) {
        return false;
    }

    return level.ground[static_cast<size_t>(row)][static_cast<size_t>(col)] ==
           terrain;
}

bool LevelManager::HasSamePoolVisual(const int row, const int col,
                                     const PoolKind kind,
                                     const PoolState state) const {
    const LevelPool* pool = GetPoolAt(row, col);
    if (pool == nullptr) {
        return false;
    }

    if (state == PoolState::Frozen) {
        return pool->state == PoolState::Frozen;
    }

    return pool->kind == kind && pool->state == state;
}

std::string LevelManager::ResolveGroundFrameName(const LevelDefinition& level,
                                                 const int row, const int col,
                                                 const TerrainType terrain) const {
    const bool hasLeft = HasSameTerrain(level, row, col - 1, terrain);
    const bool hasRight = HasSameTerrain(level, row, col + 1, terrain);

    switch (terrain) {
        case TerrainType::Solid:
            if (!hasLeft && hasRight) {
                return "BlackBoxLeft0000";
            }
            if (hasLeft && !hasRight) {
                return "BlackBoxRight0000";
            }
            return "BlackBox0000";
        case TerrainType::Ice:
            if (!hasLeft && hasRight) {
                return "IceBoxLeft0000";
            }
            if (hasLeft && !hasRight) {
                return "IceBoxRight0000";
            }
            return "IceBox0000";
        case TerrainType::Snow:
            return "SnowFlat0000";
        case TerrainType::Empty:
        default:
            return "";
    }
}

std::string LevelManager::ResolvePoolFrameName(const int row, const int col,
                                               const LevelPool& pool) const {
    const bool hasLeft =
        HasSamePoolVisual(row, col - 1, pool.kind, pool.state);
    const bool hasRight =
        HasSamePoolVisual(row, col + 1, pool.kind, pool.state);

    if (pool.state == PoolState::Frozen) {
        if (!hasLeft && hasRight) {
            return "IceBoxLeft0000";
        }
        if (hasLeft && !hasRight) {
            return "IceBoxRight0000";
        }
        return "IceBox0000";
    }

    if (pool.kind == PoolKind::Fire) {
        if (!hasLeft && hasRight) {
            return "FireBoxLeft0000";
        }
        if (hasLeft && !hasRight) {
            return "FireBoxRight0000";
        }
        return "FireBox0000";
    }

    if (!hasLeft && hasRight) {
        return "WaterBoxLeft0000";
    }
    if (hasLeft && !hasRight) {
        return "WaterBoxRight0000";
    }
    return "WaterBox0000";
}

float LevelManager::ResolveGroundZIndex(const TerrainType terrain) const {
    switch (terrain) {
        case TerrainType::Ice:
        case TerrainType::Snow:
        case TerrainType::Solid:
        case TerrainType::Empty:
        default:
            return -1.0F;
    }
}

float LevelManager::ResolvePoolZIndex(const PoolState state) const {
    return state == PoolState::Frozen ? -1.0F : -2.0F;
}

std::string LevelManager::ResolveObjectFrameName(const LevelObjectType type) const {
    switch (type) {
        case LevelObjectType::FireSpawn:
        case LevelObjectType::FireDoor:
            return "FireBox0000";
        case LevelObjectType::WaterSpawn:
        case LevelObjectType::WaterDoor:
            return "WaterBox0000";
        default:
            return "";
    }
}

float LevelManager::ResolveObjectZIndex(const LevelObjectType type) const {
    switch (type) {
        case LevelObjectType::FireSpawn:
        case LevelObjectType::WaterSpawn:
        case LevelObjectType::FireDoor:
        case LevelObjectType::WaterDoor:
            return 2.0F;
        default:
            return -1.0F;
    }
}

bool LevelManager::LoadLevel(const std::vector<std::string>& mapData,
                             const std::shared_ptr<Util::GameObject>& root) {
    if (!ValidateLevel(mapData)) {
        return false;
    }

    const LevelDefinition level = BuildLevelDefinitionFromChars(mapData);
    return LoadLevel(level, root);
}

bool LevelManager::LoadLevel(const LevelDefinition& level,
                             const std::shared_ptr<Util::GameObject>& root) {
    if (!ValidateLevelDefinition(level)) {
        return false;
    }

    m_LevelData = {};
    m_LevelDefinition = level;
    m_TileSize = static_cast<float>(level.tileSize);

    for (const auto& object : level.objects) {
        RegisterObject(object);
    }

    const int rows = level.height;
    const int cols = level.width;
    const float startX =
        -(static_cast<float>(WINDOW_WIDTH) / 2.0F) + (m_TileSize / 2.0F);
    const float startY =
        (static_cast<float>(WINDOW_HEIGHT) / 2.0F) - (m_TileSize / 2.0F);
    const float logicalCoreSize = 32.0F;

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            const TerrainType terrain =
                level.ground[static_cast<size_t>(y)][static_cast<size_t>(x)];
            RegisterTerrain(terrain, y, x);

            const std::string frameName =
                ResolveGroundFrameName(level, y, x, terrain);
            if (frameName.empty() || !m_Atlas->HasFrame(frameName)) {
                continue;
            }

            auto sprite = std::make_shared<AtlasSprite>(m_Atlas, frameName);
            auto tileObj = std::make_shared<Util::GameObject>(
                sprite, ResolveGroundZIndex(terrain));

            tileObj->m_Transform.translation = {
                std::round(startX + static_cast<float>(x) * m_TileSize),
                std::round(startY - static_cast<float>(y) * m_TileSize),
            };
            tileObj->m_Transform.scale = {
                (m_TileSize / logicalCoreSize + 0.035F),
                (m_TileSize / logicalCoreSize + 0.035F),
            };

            root->AddChild(tileObj);
        }
    }

    for (const auto& pool : level.pools) {
        for (const auto& tile : pool.tiles) {
            const std::string frameName =
                ResolvePoolFrameName(tile.row, tile.col, pool);
            if (frameName.empty() || !m_Atlas->HasFrame(frameName)) {
                continue;
            }

            auto sprite = std::make_shared<AtlasSprite>(m_Atlas, frameName);
            auto tileObj = std::make_shared<Util::GameObject>(
                sprite, ResolvePoolZIndex(pool.state));

            tileObj->m_Transform.translation = {
                std::round(startX + static_cast<float>(tile.col) * m_TileSize),
                std::round(startY - static_cast<float>(tile.row) * m_TileSize),
            };
            tileObj->m_Transform.scale = {
                (m_TileSize / logicalCoreSize + 0.033F),
                (m_TileSize / logicalCoreSize + 0.033F),
            };

            root->AddChild(tileObj);
        }
    }

    for (const auto& object : level.objects) {
        if (object.type == LevelObjectType::FireSpawn ||
            object.type == LevelObjectType::WaterSpawn ||
            object.type == LevelObjectType::FireDoor ||
            object.type == LevelObjectType::WaterDoor) {
            continue;
        }

        const std::string frameName = ResolveObjectFrameName(object.type);
        if (frameName.empty() || !m_Atlas->HasFrame(frameName)) {
            continue;
        }

        auto sprite = std::make_shared<AtlasSprite>(m_Atlas, frameName);
        auto tileObj =
            std::make_shared<Util::GameObject>(sprite,
                                               ResolveObjectZIndex(object.type));

        tileObj->m_Transform.translation = {
            startX + static_cast<float>(object.coord.col) * m_TileSize,
            startY - static_cast<float>(object.coord.row) * m_TileSize,
        };
        tileObj->m_Transform.scale = {
            m_TileSize / logicalCoreSize,
            m_TileSize / logicalCoreSize,
        };
        tileObj->m_Transform.scale *= 0.75F;
        root->AddChild(tileObj);
    }

    return true;
}
