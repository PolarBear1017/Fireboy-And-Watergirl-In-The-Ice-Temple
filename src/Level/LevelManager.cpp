#include "../../include/Level/LevelManager.hpp"
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

bool LevelManager::IsLeft(int row, int col, TerrainType terrain) const {
    if (!HasSameTerrain(row, col-1, terrain) && HasSameTerrain(row, col+1, terrain)) return true;
    return false;
}

bool LevelManager::IsRight(int row, int col, TerrainType terrain) const {
    if (HasSameTerrain(row, col-1, terrain) && !HasSameTerrain(row, col+1, terrain)) return true;
    return false;
}


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
            break;

        case LevelObjectType::Door:
            if (object.element == Element::FIRE) {
                m_LevelData.fireDoor = coord;
                m_LevelData.hasFireDoor = true;
            } else if (object.element == Element::WATER) {
                m_LevelData.waterDoor = coord;
                m_LevelData.hasWaterDoor = true;
            }
            break;

        default:
            break;
    }
}

bool LevelManager::HasSameTerrain(const int row, const int col, const TerrainType terrain) const {
    if (!IsValidCoord(row, col)) { return false; }
    return m_LevelDefinition.terrainLayer[row][col] == terrain;
}

std::string LevelManager::DetermineGroundFrameName(const int row, const int col, const TerrainType terrain) const {
    switch (terrain) {
        case TerrainType::Block:
        case TerrainType::SnowBlock:
            return "GroundBlock";
        case TerrainType::Ice:
        case TerrainType::Water:
        case TerrainType::Fire:
        case TerrainType::Toxic:
            if (IsLeft(row, col, terrain)) return "ShallowSlopeBL";
            if (IsRight(row, col, terrain)) return "ShallowSlopeBR";
            return "ShallowBlock";

        case TerrainType::SlopeBL:
        case TerrainType::SnowSlopeBL:
            return "SlopeBL";
        case TerrainType::SlopeBR:
        case TerrainType::SnowSlopeBR:
            return "SlopeBR";
        case TerrainType::SlopeTL: return "SlopeTL";
        case TerrainType::SlopeTR: return "SlopeTR";
        default: return "";
    }
}

std::string LevelManager::DetermineSnowFrameName(TerrainType terrain) const {
    switch (terrain) {
        case TerrainType::SnowSlopeBL: return "SnowSlope0000";
        case TerrainType::SnowSlopeBR: return "SnowSlope0001";
        case TerrainType::SnowBlock:
        default:
            return "SnowFlat0000";
    }
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

    for (const auto& object : level.objects) {RegisterObject(object);}

    const float logicalCoreSize = 32.0F;

    // 處理地形
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

            int terrainValue = static_cast<int>(terrain);
            // 20 ~ 22 is snow-related terrains
            if (terrainValue >= 20 && terrainValue <= 22) {
                std::string snowFrame = DetermineSnowFrameName(terrain);
                if (!snowFrame.empty() && m_OverlayAtlas->HasFrame(snowFrame)) {
                    auto snowSprite = std::make_shared<AtlasSprite>(m_OverlayAtlas, snowFrame);
                    auto snowObj = std::make_shared<Util::GameObject>(snowSprite, GROUND_Z_INDEX + 1.0F);

                    snowObj->m_Transform.translation = { std::round(pos.x), std::round(pos.y) };

                    glm::vec2 snowSize = snowSprite->GetSize();
                    if (terrain == TerrainType::SnowSlopeBL || terrain == TerrainType::SnowSlopeBR) {
                        snowObj->m_Transform.scale = {(m_TileSize / snowSize.x) + 0.25F, (m_TileSize / snowSize.y) + 0.25F};
                    } else {
                        // 平的雪塊：維持原本的 +0.5F，確保能包覆方塊邊緣
                        snowObj->m_Transform.scale = {(m_TileSize / snowSize.x) + 0.5F, (m_TileSize / snowSize.y) + 0.5F};
                    }
                    root->AddChild(snowObj);
                }
            }
        }
    }

    return true;
}

void LevelManager::SwitchWaterAndIceTerrain(int row, int col) {
    if (!IsValidCoord(row, col)) return;
    TerrainType curTerrain = m_LevelDefinition.terrainLayer[row][col];

    if (curTerrain == TerrainType::Water)
        m_LevelDefinition.terrainLayer[row][col] = TerrainType::Ice;
    else if (curTerrain == TerrainType::Ice)
        m_LevelDefinition.terrainLayer[row][col] = TerrainType::Water;
}