#include "../include/Level/LevelManager.hpp"
#include "AtlasSprite.hpp"
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

bool LevelManager::IsTileInPool(const LevelPool& pool, const int row, const int col) const {
    for (const auto& tile : pool.tiles) {
        if (tile.row == row && tile.col == col) {
            return true;
        }
    }

    return false;
}

bool LevelManager::IsSolidTile(const int row, const int col) const {
    if (!IsValidGroundCoord(row, col)) {
        // Horizontal boundaries and top boundary are solid
        if (col < 0 || col >= static_cast<int>(m_LevelDefinition.width) || row < 0) {
            return true;
        }
        // Bottom boundary is not solid (it's a pit)
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

bool LevelManager::IsHazardousFor(const Element element, const int row, const int col) const {
    const LevelPool* pool = GetPoolAt(row, col);
    if (pool == nullptr || pool->state == PoolState::Frozen) {
        return false;
    }

    if (pool->element == Element::FIRE) {
        return element == Element::WATER;
    }

    if (pool->element == Element::WATER) {
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

        if (object.type == LevelObjectType::Spawn) {
            if (object.element == Element::FIRE) {fireSpawnCount += 1;}
            else {waterSpawnCount += 1;}
        }else if (object.type == LevelObjectType::Door) {
            if (object.element == Element::FIRE) {fireDoorCount += 1;}
            else {waterDoorCount += 1;}
        }
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

void LevelManager::RegisterTerrain(const TerrainType terrain, const int row, const int col) {
    if (terrain == TerrainType::Empty) {
        return;
    }

    m_LevelData.solidTiles.push_back(TileCoord{row, col});
}

void LevelManager::RegisterObject(const LevelObject& object) {
    const TileCoord coord{object.coord.row, object.coord.col};

    switch (object.type) {
        case LevelObjectType::Spawn:
            if (object.element == Element::FIRE) {
                m_LevelData.fireSpawn = coord;
                m_LevelData.hasFireSpawn = true;
            } else if (object.element == Element::WATER) {
                m_LevelData.waterSpawn = coord;
                m_LevelData.hasWaterSpawn = true;
            }
            break; // 務必記得 break

        case LevelObjectType::Door:
            if (object.element == Element::FIRE) {
                m_LevelData.fireDoor = coord;
                m_LevelData.hasFireDoor = true;
            } else if (object.element == Element::WATER) {
                m_LevelData.waterDoor = coord;
                m_LevelData.hasWaterDoor = true;
            }
            break; // 務必記得 break

        default:
            break;
    }
}

bool LevelManager::HasSameTerrain(const LevelDefinition& level, const int row, const int col, const TerrainType terrain) const {
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

bool LevelManager::HasSamePoolVisual(const int row, const int col, const Element element, const PoolState state) const {
    const LevelPool* pool = GetPoolAt(row, col);
    if (pool == nullptr) {
        return false;
    }

    if (state == PoolState::Frozen) {
        return pool->state == PoolState::Frozen;
    }

    return pool->element == element && pool->state == state;
}

std::string LevelManager::ResolveGroundFrameName(
    const LevelDefinition& level,
    const int row,
    const int col,
    const TerrainType terrain
    ) const{
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

std::string LevelManager::ResolvePoolFrameName(const int row, const int col, const LevelPool& pool) const {
    const bool hasLeft =
        HasSamePoolVisual(row, col - 1, pool.element, pool.state);
    const bool hasRight =
        HasSamePoolVisual(row, col + 1, pool.element, pool.state);

    if (pool.state == PoolState::Frozen) {
        if (!hasLeft && hasRight) {
            return "IceBoxLeft0000";
        }
        if (hasLeft && !hasRight) {
            return "IceBoxRight0000";
        }
        return "IceBox0000";
    }

    if (pool.element == Element::FIRE) {
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

std::string LevelManager::ResolveObjectFrameName(const Element e) const {
    switch (e) {
        case Element::FIRE:
            return "FireBox0000";
        case Element::WATER:
            return "WaterBox0000";
        default:
            return "";
    }
}

float LevelManager::ResolveObjectZIndex(const LevelObjectType type) const {
    switch (type) {
        case LevelObjectType::Spawn:
        case LevelObjectType::Door:
            return 2.0F;
        default:
            return -1.0F;
    }
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

    const float logicalCoreSize = 32.0F;

    // --- 1. 處理地形 ---
    for (int y = 0; y < level.height; ++y) {
        for (int x = 0; x < level.width; ++x) {
            const TerrainType terrain = level.ground[y][x];
            RegisterTerrain(terrain, y, x);

            const std::string frameName = ResolveGroundFrameName(level, y, x, terrain);
            if (frameName.empty() || !m_Atlas->HasFrame(frameName)) continue;

            auto sprite = std::make_shared<AtlasSprite>(m_Atlas, frameName);
            auto tileObj = std::make_shared<Util::GameObject>(sprite, ResolveGroundZIndex(terrain));

            // 🌟 使用統一函式並四捨五入確保對齊
            glm::vec2 pos = TileToWorldPosition(y, x);
            tileObj->m_Transform.translation = { std::round(pos.x), std::round(pos.y) };

            tileObj->m_Transform.scale = glm::vec2(m_TileSize / logicalCoreSize + 0.035F);
            root->AddChild(tileObj);
        }
    }

    // --- 2. 處理水池 ---
    for (const auto& pool : level.pools) {
        for (const auto& tile : pool.tiles) {
            const std::string frameName = ResolvePoolFrameName(tile.row, tile.col, pool);
            if (frameName.empty() || !m_Atlas->HasFrame(frameName)) continue;

            auto sprite = std::make_shared<AtlasSprite>(m_Atlas, frameName);
            auto tileObj = std::make_shared<Util::GameObject>(sprite, ResolvePoolZIndex(pool.state));

            // 🌟 使用統一函式
            glm::vec2 pos = TileToWorldPosition(tile.row, tile.col);
            tileObj->m_Transform.translation = { std::round(pos.x), std::round(pos.y) };

            tileObj->m_Transform.scale = glm::vec2(m_TileSize / logicalCoreSize + 0.033F);
            root->AddChild(tileObj);
        }
    }

    // --- 3. 處理物件 (出生點、門) ---
    for (const auto& object : level.objects) {
        if (object.type == LevelObjectType::Spawn ||
            object.type == LevelObjectType::Door) {
            continue;
            }

        // 2. 處理其他的機關 (如 Button, Lever, Elevator)
        // 🚨 注意：目前 ResolveObjectFrameName 只吃 Element(火/水)
        // 如果是電梯 (Element::NEUTRAL)，目前會回傳空字串並跳過，這在現階段是安全的防呆機制。
        const std::string frameName = ResolveObjectFrameName(object.element);

        if (frameName.empty() || !m_Atlas->HasFrame(frameName)) {
            continue;
        }

        auto sprite = std::make_shared<AtlasSprite>(m_Atlas, frameName);
        auto tileObj = std::make_shared<Util::GameObject>(sprite, ResolveObjectZIndex(object.type));

        // 確保座標對齊
        glm::vec2 pos = TileToWorldPosition(object.coord.row, object.coord.col);
        tileObj->m_Transform.translation = { std::round(pos.x), std::round(pos.y) };
        tileObj->m_Transform.scale = glm::vec2(m_TileSize / logicalCoreSize * 0.75F);
        root->AddChild(tileObj);
    }
    return true;
}