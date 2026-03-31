#include "LevelManager.hpp"

#include "AtlasSprite.hpp"
#include "LevelParser.hpp"
#include "Util/Logger.hpp"
#include "config.hpp" // For WINDOW_WIDTH / WINDOW_HEIGHT
// #include <cmath>

LevelManager::LevelManager(std::shared_ptr<SpriteAtlas> atlas)
    : m_Atlas(std::move(atlas)) {}

bool LevelManager::IsValidGroundCoord(const int row, const int col) const {
    if (row < 0 || col < 0) {
        return false;
    }

    if (row >= static_cast<int>(m_LevelDefinition.ground.size())) {
        return false;
    }

    if (col >= static_cast<int>(m_LevelDefinition.ground[static_cast<size_t>(row)].size())) {
        return false;
    }

    return true;
}

bool LevelManager::IsSolidTile(const int row, const int col) const {
    // 碰撞查詢優先使用新的地形資料，而不是舊版字元地圖。
    if (!IsValidGroundCoord(row, col)) {
        return false;
    }

    return m_LevelDefinition.ground[static_cast<size_t>(row)]
                                  [static_cast<size_t>(col)] ==
           TerrainType::Solid;
}

bool LevelManager::IsLavaTile(const int row, const int col) const {
    if (!IsValidGroundCoord(row, col)) {
        return false;
    }

    return m_LevelDefinition.ground[static_cast<size_t>(row)]
                                  [static_cast<size_t>(col)] ==
           TerrainType::Lava;
}

bool LevelManager::IsWaterTile(const int row, const int col) const {
    if (!IsValidGroundCoord(row, col)) {
        return false;
    }

    return m_LevelDefinition.ground[static_cast<size_t>(row)]
                                  [static_cast<size_t>(col)] ==
           TerrainType::Water;
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

    // 檢查 ground 圖層尺寸是否完整，避免後續以 row/col 存取時越界。
    for (int row = 0; row < level.height; ++row) {
        if (static_cast<int>(level.ground[static_cast<size_t>(row)].size()) !=
            level.width) {
            LOG_ERROR("LevelManager: ground column count does not match level width.");
            return false;
        }
    }

    // 檢查 objects 圖層中的關鍵物件數量與座標是否合法。
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
        LOG_ERROR("LevelManager: level must contain exactly one fire spawn and one water spawn.");
        return false;
    }

    if (fireDoorCount > 1 || waterDoorCount > 1) {
        LOG_ERROR("LevelManager: level can contain at most one fire door and one water door.");
        return false;
    }

    return true;
}

void LevelManager::RegisterTerrain(const TerrainType terrain, const int row,
                                   const int col) {
    const TileCoord coord{row, col};

    // 地形註冊直接依照新格式中的 TerrainType 進行。
    switch (terrain) {
        case TerrainType::Solid:
            m_LevelData.solidTiles.push_back(coord);
            break;
        case TerrainType::Lava:
            m_LevelData.lavaTiles.push_back(coord);
            break;
        case TerrainType::Water:
            m_LevelData.waterTiles.push_back(coord);
            break;
        case TerrainType::Empty:
        default:
            break;
    }
}

void LevelManager::RegisterObject(const LevelObject& object) {
    const TileCoord coord{object.coord.row, object.coord.col};

    // 角色出生點與門改成直接從新格式的 objects 圖層註冊。
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

// 判斷某格周圍是不是同種類型的地形
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
        case TerrainType::Lava:
            if (!hasLeft && hasRight) {
                return "FireBoxLeft0000";
            }
            if (hasLeft && !hasRight) {
                return "FireBoxRight0000";
            }
            return "FireBox0000";
        case TerrainType::Water:
            if (!hasLeft && hasRight) {
                return "WaterBoxLeft0000";
            }
            if (hasLeft && !hasRight) {
                return "WaterBoxRight0000";
            }
            return "WaterBox0000";
        case TerrainType::Empty:
        default:
            return "";  // 如果這格是空地，就不要畫圖。
    }
}

// 不同地形畫出來時，要放在前面還是後面。
float LevelManager::ResolveGroundZIndex(const TerrainType terrain) const {
    switch (terrain) {
        case TerrainType::Water:
        case TerrainType::Lava:
            return -2.0F;
        case TerrainType::Solid:
        case TerrainType::Empty:
        default:
            return -1.0F;
    }
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

void LevelManager::LoadLevel(const std::vector<std::string>& mapData,
                             const std::shared_ptr<Util::GameObject>& root) {
    if (!ValidateLevel(mapData)) {
        return;
    }

    // 舊版字元地圖先轉成新的標準關卡格式。
    const LevelDefinition level = BuildLevelDefinitionFromChars(mapData);

    // 後續統一交給新版 LoadLevel 處理，避免兩套載入邏輯分開維護。
    LoadLevel(level, root);
}

void LevelManager::LoadLevel(const LevelDefinition& level,
                             const std::shared_ptr<Util::GameObject>& root) {
    if (!ValidateLevelDefinition(level)) {
        return;
    }

    // 保存新格式的關卡資料，後續邏輯直接以 LevelDefinition 為主。
    m_LevelData = {};
    m_LevelDefinition = level;
    m_TileSize = static_cast<float>(level.tileSize);

    // 先根據新格式中的 objects 圖層註冊出生點與門。
    for (const auto& object : level.objects) {
        RegisterObject(object);
    }

    const int rows = level.height;
    const int cols = level.width;
    const float startX =
        -(static_cast<float>(WINDOW_WIDTH) / 2.0F) + (m_TileSize / 2.0F);
    const float startY =
        (static_cast<float>(WINDOW_HEIGHT) / 2.0F) - (m_TileSize / 2.0F);

    // 逐格建立牆壁、岩漿、水等地形，直接讀取新格式中的 ground 圖層。
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
            auto tileObj =
                std::make_shared<Util::GameObject>(sprite, ResolveGroundZIndex(terrain));

            tileObj->m_Transform.translation = {
                startX + static_cast<float>(x) * m_TileSize,
                startY - static_cast<float>(y) * m_TileSize,
            };

            // const auto actualSize = sprite->GetSize();
            // if (actualSize.x > 0.0F && actualSize.y > 0.0F) {
            //     tileObj->m_Transform.scale = {
            //         m_TileSize / actualSize.x,
            //         m_TileSize / actualSize.y,
            //     };
            // }

            const float logicalCoreSize = 32.0F;

            tileObj->m_Transform.scale = {
                m_TileSize / logicalCoreSize,
                m_TileSize / logicalCoreSize,
            };

            root->AddChild(tileObj);
        }
    }

    // 物件圖層直接根據 object type 決定提示圖示，不再依賴整張舊字元地圖。
    for (const auto& object : level.objects) {
        const std::string frameName = ResolveObjectFrameName(object.type);
        if (frameName.empty() || !m_Atlas->HasFrame(frameName)) {
            continue;
        }

        auto sprite = std::make_shared<AtlasSprite>(m_Atlas, frameName);
        auto tileObj =
            std::make_shared<Util::GameObject>(sprite, ResolveObjectZIndex(object.type));

        tileObj->m_Transform.translation = {
            startX + static_cast<float>(object.coord.col) * m_TileSize,
            startY - static_cast<float>(object.coord.row) * m_TileSize,
        };

        // 把圖片縮放到剛好符合一格 tile 的大小。
        // const auto actualSize = sprite->GetSize();
        // if (actualSize.x > 0.0F && actualSize.y > 0.0F) {
        //     tileObj->m_Transform.scale = {
        //         m_TileSize / actualSize.x,
        //         m_TileSize / actualSize.y,
        //     };
        // }

        const float logicalCoreSize = 32.0F;

        tileObj->m_Transform.scale = {
            m_TileSize / logicalCoreSize,
            m_TileSize / logicalCoreSize,
        };

        // 出生點與門目前仍用縮小圖示作為暫時標記，之後可再換成真正物件。
        tileObj->m_Transform.scale *= 0.75F;
        root->AddChild(tileObj);
    }
}
