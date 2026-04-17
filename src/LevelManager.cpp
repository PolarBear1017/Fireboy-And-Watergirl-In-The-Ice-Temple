#include "../include/Level/LevelManager.hpp"
#include "AtlasSprite.hpp"
#include "Util/Logger.hpp"
#include "config.hpp"
#include <cmath>

LevelManager::LevelManager(std::shared_ptr<SpriteAtlas> groundAtlas, std::shared_ptr<SpriteAtlas> overlayAtlas)
    : m_GroundAtlas(std::move(groundAtlas)), m_OverlayAtlas(std::move(overlayAtlas)) {}

bool LevelManager::IsValidCoord(const int row, const int col) const {
    if (row < 0 || col < 0) {
        return false;
    }
    if (row >= static_cast<int>(m_LevelDefinition.terrainLayer.size())) {
        return false;
    }
    if (col >= static_cast<int>(m_LevelDefinition.terrainLayer[static_cast<size_t>(row)].size())) {
        return false;
    }
    return true;
}

// 在 LevelManager.cpp 中新增：
TerrainType LevelManager::GetTerrain(const int row, const int col) const {
    if (!IsValidCoord(row, col)) return TerrainType::Empty;
    return m_LevelDefinition.terrainLayer[row][col];
}

bool LevelManager::IsWalkable(const int row, const int col) const {
    if (!IsValidCoord(row, col)) {return false;}
    return (m_LevelDefinition.terrainLayer[row][col] != TerrainType::Empty);
}

bool LevelManager::IsHazardousFor(const Element element, const int row, const int col) const {
    if (!IsValidCoord(row, col)) return false;

    switch (m_LevelDefinition.terrainLayer[row][col]) {
        case TerrainType::Toxic:
            return true;
        case TerrainType::Fire:
            return element == Element::WATER;
        case TerrainType::Water:
            return element == Element::FIRE;
        default:
            return false;
    }
}

// 網格座標轉成螢幕座標
glm::vec2 LevelManager::TileToWorldPosition(const int row, const int col) const {
    const float startX = -(static_cast<float>(WINDOW_WIDTH) / 2.0F) + (m_TileSize / 2.0F);
    const float startY = (static_cast<float>(WINDOW_HEIGHT) / 2.0F) - (m_TileSize / 2.0F);

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
    if (static_cast<int>(level.terrainLayer.size()) != level.height) {
        LOG_ERROR("LevelManager: ground row count does not match level height.");
        return false;
    }

    int fireSpawnCount = 0; int waterSpawnCount = 0;
    int fireDoorCount = 0; int waterDoorCount = 0;

    for (int row = 0; row < level.height; ++row) {
        if (static_cast<int>(level.terrainLayer[static_cast<size_t>(row)].size()) !=
            level.width) {
            LOG_ERROR("LevelManager: ground column count does not match level width.");
            return false;
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
        LOG_ERROR("LevelManager: level must contain exactly one fire spawn and one water spawn.");
        return false;
    }
    if (fireDoorCount != 1 || waterDoorCount != 1) {
        LOG_ERROR("LevelManager: level must contain exactly one fire door and one water door.");
        return false;
    }

    return true;
}

void LevelManager::RegisterTerrain(const TerrainType terrain, const int row, const int col) {
    if (terrain == TerrainType::Empty) {return;}
    m_LevelData.solidTiles.push_back(GridCoord{row, col});
}

void LevelManager::RegisterObject(const LevelObject& object) {
    const GridCoord coord{object.coord.row, object.coord.col};

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

bool LevelManager::HasSameTerrain(const int row, const int col, const TerrainType terrain) const {
    if (!IsValidCoord(row, col)) { return false; }
    return m_LevelDefinition.terrainLayer[row][col] == terrain;
}


std::string LevelManager::DetermineGroundFrameName(const int row, const int col, const TerrainType terrain) const {

    const bool hasLeft = HasSameTerrain(row, col - 1, terrain);
    const bool hasRight = HasSameTerrain(row, col + 1, terrain);

    switch (terrain) {
        case TerrainType::Block:
            // 遇到下面的覆蓋物時 應該用 HasSameTerrain() 判斷是否為覆蓋物邊緣（terrain 先轉成數字可能會比較好）
        case TerrainType::Ice:
        case TerrainType::Water:
        case TerrainType::Fire:
        case TerrainType::Toxic:
            return "GroundBlock";

        case TerrainType::SlopeBL: return "SlopeBlockBL";
        case TerrainType::SlopeBR: return "SlopeBlockBR";
        case TerrainType::SlopeTL: return "SlopeBlockTL";
        case TerrainType::SlopeTR: return "SlopeBlockTR";
        default: return "";
    }
}

std::string LevelManager::DetermineOverlayFrameName(const int row, const int col, const TerrainType terrain) const {
    const bool hasLeft = HasSameTerrain(row, col - 1, terrain);
    const bool hasRight = HasSameTerrain(row, col + 1, terrain);

    switch (terrain) {
        case TerrainType::Ice:
            if (!hasLeft && hasRight) return "IceBoxLeft0000";
            if (hasLeft && !hasRight) return "IceBoxRight0000";
            return "IceBox0000";

        case TerrainType::Fire:
            if (!hasLeft && hasRight) return "FireBoxLeft0000";
            if (hasLeft && !hasRight) return "FireBoxRight0000";
            return "FireBox0000";

        case TerrainType::Water:
            if (!hasLeft && hasRight) return "WaterBoxLeft0000";
            if (hasLeft && !hasRight) return "WaterBoxRight0000";
            return "WaterBox0000";

        case TerrainType::Toxic:
            if (!hasLeft && hasRight) return "GreenBoxLeft0000";
            if (hasLeft && !hasRight) return "GreenBoxRight0000";
            return "GreenBox0000";

        default:
            return "";
    }
}

float LevelManager::DetermineOverlayZIndex(const TerrainType type) const {
    int terrainNum = static_cast<int>(type);
    if (terrainNum >= 20 && terrainNum <= 30) {
        return 1.0F;
    }
    return 15.0F;
}

std::string LevelManager::DetermineObjectFrameName(const Element e) const {
    return "";
}

float LevelManager::DetermineObjectZIndex(const LevelObjectType type) const {
    switch (type) {
        case LevelObjectType::Spawn:
        case LevelObjectType::Door:
            return 2.0F;
        default:
            return -1.0F;
    }
}

bool LevelManager::LoadLevel(const LevelDefinition& level, const std::shared_ptr<Util::GameObject>& root) {
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
            const TerrainType terrain = level.terrainLayer[y][x];
            RegisterTerrain(terrain, y, x);

            const std::string frameName = DetermineGroundFrameName(y, x, terrain);
            if (frameName.empty() || !m_GroundAtlas->HasFrame(frameName)) continue;

            auto sprite = std::make_shared<AtlasSprite>(m_GroundAtlas, frameName);
            auto tileObj = std::make_shared<Util::GameObject>(sprite, GROUND_Z_INDEX);

            // 🌟 使用統一函式並四捨五入確保對齊
            glm::vec2 pos = TileToWorldPosition(y, x);
            tileObj->m_Transform.translation = { std::round(pos.x), std::round(pos.y) };

            tileObj->m_Transform.scale = glm::vec2(m_TileSize / logicalCoreSize + 0.035F);
            root->AddChild(tileObj);
        }
    }

    // --- 2. 處理水池/冰塊 (新版極簡寫法) ---
    for (const auto& overlayObj : level.overlays) {
        const int r = overlayObj.coord.row;
        const int c = overlayObj.coord.col;

        const TerrainType terrain = level.terrainLayer[r][c];
        const std::string frameName = DetermineOverlayFrameName(r, c, terrain);

        if (frameName.empty() || !m_OverlayAtlas->HasFrame(frameName)) continue;

        auto sprite = std::make_shared<AtlasSprite>(m_OverlayAtlas, frameName);
        auto tileObj = std::make_shared<Util::GameObject>(sprite, DetermineOverlayZIndex(terrain));

        glm::vec2 pos = TileToWorldPosition(r, c);
        tileObj->m_Transform.translation = { std::round(pos.x), std::round(pos.y) };

        tileObj->m_Transform.scale = glm::vec2(m_TileSize / logicalCoreSize + 0.033F);
        root->AddChild(tileObj);
    }

    // 以下仍由 app.cpp 處理 可刪
    // --- 3. 處理物件 (出生點、門) ---
    // for (const auto& object : level.objects) {
    //     if (object.type == LevelObjectType::Spawn ||
    //         object.type == LevelObjectType::Door) {
    //         continue;
    //         }
    //
    //     // 2. 處理其他的機關 (如 Button, Lever, Elevator)
    //     // 🚨 注意：目前 DetermineObjectFrameName 只吃 Element(火/水)
    //     // 如果是電梯 (Element::NEUTRAL)，目前會回傳空字串並跳過，這在現階段是安全的防呆機制。
    //     const std::string frameName = DetermineObjectFrameName(object.element);
    //
    //     if (frameName.empty() || !m_OverlayAtlas->HasFrame(frameName)) {
    //         continue;
    //     }
    //
    //     auto sprite = std::make_shared<AtlasSprite>(m_OverlayAtlas, frameName);
    //     auto tileObj = std::make_shared<Util::GameObject>(sprite, DetermineObjectZIndex(object.type));
    //
    //     // 確保座標對齊
    //     glm::vec2 pos = TileToWorldPosition(object.coord.row, object.coord.col);
    //     tileObj->m_Transform.translation = { std::round(pos.x), std::round(pos.y) };
    //     tileObj->m_Transform.scale = glm::vec2(m_TileSize / logicalCoreSize * 0.75F);
    //     root->AddChild(tileObj);
    // }
    return true;
}