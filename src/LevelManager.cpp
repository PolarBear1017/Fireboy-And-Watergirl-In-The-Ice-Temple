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
    // 只要檢查 groundLayer 的邊界，因為 overlayLayer 跟它一樣大
    if (row >= static_cast<int>(m_LevelDefinition.groundLayer.size())) {
        return false;
    }
    if (col >= static_cast<int>(m_LevelDefinition.groundLayer[static_cast<size_t>(row)].size())) {
        return false;
    }
    return true;
}

bool LevelManager::IsWalkable(const int row, const int col) const {
    if (!IsValidCoord(row, col)) {
        return false; // 掉出地圖外不能走
    }

    // 1. 查地磚層：是實體方塊或斜坡就可以走
    const TerrainType terrain = m_LevelDefinition.groundLayer[row][col];
    if (terrain != TerrainType::Empty) {
        return true;
    }

    // 2. 查疊加層：如果地磚是空的，但疊加層是「冰」，也可以走！
    const OverlayType overlay = m_LevelDefinition.overlayLayer[row][col];
    if (overlay == OverlayType::Ice) {
        return true;
    }

    return false;
}

bool LevelManager::IsHazardousFor(const Element element, const int row, const int col) const {
    if (!IsValidCoord(row, col)) {return false;}

    const OverlayType overlay = m_LevelDefinition.overlayLayer[row][col];

    switch (overlay) {
        case OverlayType::Toxic:
            return true;
        case OverlayType::Fire:
            return element == Element::WATER;
        case OverlayType::Water:
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
    if (static_cast<int>(level.groundLayer.size()) != level.height) {
        LOG_ERROR("LevelManager: ground row count does not match level height.");
        return false;
    }
    if (static_cast<int>(level.overlayLayer.size()) != level.height) {
        LOG_ERROR("LevelManager: overlay row count does not match level height.");
        return false;
    }

    int fireSpawnCount = 0; int waterSpawnCount = 0;
    int fireDoorCount = 0; int waterDoorCount = 0;

    for (int row = 0; row < level.height; ++row) {
        if (static_cast<int>(level.groundLayer[static_cast<size_t>(row)].size()) !=
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

bool LevelManager::HasSameTerrain(const LevelDefinition& level, const int row, const int col, const TerrainType terrain) const {
    if (!IsValidCoord(row, col)) { return false; }
    return level.groundLayer[row][col] == terrain;
}

bool LevelManager::HasSameOverlay(const int row, const int col, const OverlayType type) const {
    if (!IsValidCoord(row, col)) { return false; }
    return m_LevelDefinition.overlayLayer[row][col] == type;
}

std::string LevelManager::DetermineGroundFrameName(
    const LevelDefinition& level,
    const int row,
    const int col,
    const TerrainType terrain) const {

    const bool hasLeft = HasSameTerrain(level, row, col - 1, terrain);
    const bool hasRight = HasSameTerrain(level, row, col + 1, terrain);

    switch (terrain) {
        case TerrainType::Block:
            if (!hasLeft && hasRight) return "SlopeBlockTR";
            if (hasLeft && !hasRight) return "SlopeBlockTL";
            return "GroundBlock";

            // 🚧 未來你可以把斜坡 (SlopeBL, SlopeBR) 的圖塊名稱寫在這裡

        case TerrainType::Empty:
        default:
            return "";
    }
}

std::string LevelManager::DetermineOverlayFrameName(const int row, const int col, const OverlayType type) const {
    // 快速查表，看看左右兩邊是不是也是一樣的疊加物
    const bool hasLeft = HasSameOverlay(row, col - 1, type);
    const bool hasRight = HasSameOverlay(row, col + 1, type);

    switch (type) {
        case OverlayType::Ice:
            if (!hasLeft && hasRight) return "IceBoxLeft0000";
            if (hasLeft && !hasRight) return "IceBoxRight0000";
            return "IceBox0000";

        case OverlayType::Fire:
            if (!hasLeft && hasRight) return "FireBoxLeft0000";
            if (hasLeft && !hasRight) return "FireBoxRight0000";
            return "FireBox0000";

        case OverlayType::Water:
            if (!hasLeft && hasRight) return "WaterBoxLeft0000";
            if (hasLeft && !hasRight) return "WaterBoxRight0000";
            return "WaterBox0000";

        case OverlayType::Toxic:
            // 假設你有毒液圖塊，名稱請依據你的 Atlas 修改
            if (!hasLeft && hasRight) return "ToxicBoxLeft0000";
            if (hasLeft && !hasRight) return "ToxicBoxRight0000";
            return "ToxicBox0000";

        case OverlayType::Snow:
            return "SnowFlat0000"; // 雪地直接回傳單張鋪平的圖

        case OverlayType::None:
        default:
            return "";
    }
}

float LevelManager::DetermineOverlayZIndex(const OverlayType type) const {
    switch (type) {
        case OverlayType::Ice:
        case OverlayType::Snow:
            return 1.0F;
        default:
            return 15.0F;
    }
}

std::string LevelManager::DetermineObjectFrameName(const Element e) const {
    switch (e) {
        case Element::FIRE:
            return "FireBox0000";
        case Element::WATER:
            return "WaterBox0000";
        default:
            return "";
    }
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
            const TerrainType terrain = level.groundLayer[y][x];
            RegisterTerrain(terrain, y, x);

            const std::string frameName = DetermineGroundFrameName(level, y, x, terrain);
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

    // --- 2. 處理水池 ---
    // for (const auto& pool : level.pools) {
    //     for (const auto& tile : pool.tiles) {
    //         const std::string frameName = ResolvePoolFrameName(tile.row, tile.col, pool);
    //         if (frameName.empty() || !m_Atlas->HasFrame(frameName)) continue;
    //
    //         auto sprite = std::make_shared<AtlasSprite>(m_Atlas, frameName);
    //         auto tileObj = std::make_shared<Util::GameObject>(sprite, ResolvePoolZIndex(pool.state));
    //
    //         // 🌟 使用統一函式
    //         glm::vec2 pos = TileToWorldPosition(tile.row, tile.col);
    //         tileObj->m_Transform.translation = { std::round(pos.x), std::round(pos.y) };
    //
    //         tileObj->m_Transform.scale = glm::vec2(m_TileSize / logicalCoreSize + 0.033F);
    //         root->AddChild(tileObj);
    //     }
    // }

    for (const auto& overlayObj : level.overlays) {
        for (const auto& tile : overlayObj.tiles) {
            // 呼叫我們剛剛寫好的新函式！
            const std::string frameName = DetermineOverlayFrameName(tile.row, tile.col, overlayObj.type);

            if (frameName.empty() || !m_OverlayAtlas->HasFrame(frameName)) continue;

            auto sprite = std::make_shared<AtlasSprite>(m_OverlayAtlas, frameName);
            auto tileObj = std::make_shared<Util::GameObject>(sprite, DetermineOverlayZIndex(overlayObj.type));

            glm::vec2 pos = TileToWorldPosition(tile.row, tile.col);
            tileObj->m_Transform.translation = { std::round(pos.x), std::round(pos.y) };

            // 這個 0.033F 是你原本微調接縫用的，我們保留
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
        // 🚨 注意：目前 DetermineObjectFrameName 只吃 Element(火/水)
        // 如果是電梯 (Element::NEUTRAL)，目前會回傳空字串並跳過，這在現階段是安全的防呆機制。
        const std::string frameName = DetermineObjectFrameName(object.element);

        if (frameName.empty() || !m_OverlayAtlas->HasFrame(frameName)) {
            continue;
        }

        auto sprite = std::make_shared<AtlasSprite>(m_OverlayAtlas, frameName);
        auto tileObj = std::make_shared<Util::GameObject>(sprite, DetermineObjectZIndex(object.type));

        // 確保座標對齊
        glm::vec2 pos = TileToWorldPosition(object.coord.row, object.coord.col);
        tileObj->m_Transform.translation = { std::round(pos.x), std::round(pos.y) };
        tileObj->m_Transform.scale = glm::vec2(m_TileSize / logicalCoreSize * 0.75F);
        root->AddChild(tileObj);
    }
    return true;
}