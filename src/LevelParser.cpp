#include "LevelParser.hpp"

#include "Util/Logger.hpp"

namespace {     // 把裡面的函式或變數限制在這個 .cpp 檔案內使用
TerrainType CharToTerrainType(const char tileChar) {
    switch (tileChar) {
        case 'X':
            return TerrainType::Solid;
        case 'L':
            return TerrainType::Lava;
        case 'W':
            return TerrainType::Water;
        case 'I':
            return TerrainType::Ice;
        case 'S':
            return TerrainType::Snow;
        case '.':
        case 'F':
        case 'G':
        case 'R':
        case 'B':
        default:
            return TerrainType::Empty;
    }
}

bool IsObjectChar(const char tileChar) {
    switch (tileChar) {
        case 'F':
        case 'G':
        case 'R':
        case 'B':
            return true;
        default:
            return false;
    }
}

LevelObjectType CharToObjectType(const char tileChar) {
    switch (tileChar) {
        case 'F':
            return LevelObjectType::FireSpawn;
        case 'G':
            return LevelObjectType::WaterSpawn;
        case 'R':
            return LevelObjectType::FireDoor;
        case 'B':
            return LevelObjectType::WaterDoor;
        default:
            return LevelObjectType::FireSpawn;
    }
}
} // namespace

LevelDefinition BuildLevelDefinitionFromChars(
    const std::vector<std::string>& mapData) {
    LevelDefinition level;

    if (mapData.empty()) {
        return level;
    }

    level.height = static_cast<int>(mapData.size());
    level.width = static_cast<int>(mapData[0].size());
    level.tileSize = 32;

    for (int row = 0; row < level.height; ++row) {
        if (static_cast<int>(mapData[static_cast<size_t>(row)].size()) !=
            level.width) {
            LOG_ERROR("LevelParser: row " + std::to_string(row) +
                      " width mismatch.");
            return {};
        }
    }

    level.ground.resize(level.height, std::vector<TerrainType>(level.width, TerrainType::Empty));

    for (int row = 0; row < level.height; ++row) {
        for (int col = 0; col < level.width; ++col) {
            const char tileChar = mapData[row][col];
            level.ground[row][col] = CharToTerrainType(tileChar);

            if (IsObjectChar(tileChar)) {
                level.objects.push_back(LevelObject{
                    CharToObjectType(tileChar),
                    GridCoord{row, col}
                });
            }
        }
    }

    return level;
}
