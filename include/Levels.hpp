#ifndef LEVELS_HPP
#define LEVELS_HPP

#include "LevelDefinition.hpp"
#include <string>
#include <vector>

namespace Levels {
namespace detail {

inline TerrainType CharToTerrain(const char tileChar) {
    switch (tileChar) {
        case 'X':
            return TerrainType::Solid;
        case 'I':
            return TerrainType::Ice;
        case 'S':
            return TerrainType::Snow;
        case '.':
        default:
            return TerrainType::Empty;
    }
}

inline std::vector<std::vector<TerrainType>> BuildGroundLayer(
    const std::vector<std::string>& rows) {
    std::vector<std::vector<TerrainType>> ground;
    ground.reserve(rows.size());

    for (const auto& row : rows) {
        std::vector<TerrainType> groundRow;
        groundRow.reserve(row.size());

        for (const char tileChar : row) {
            groundRow.push_back(CharToTerrain(tileChar));
        }

        ground.push_back(std::move(groundRow));
    }

    return ground;
}

} // namespace detail

inline LevelDefinition BuildLevel1Definition() {
    const std::vector<std::string> groundRows = {
        "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
        "X.....................................X",
        "X.....................................X",
        "X.............XXXX....................X",
        "X.....................................X",
        "X......XXXX..............XXXX.........X",
        "X.....................................X",
        "X.................XXXXX...............X",
        "X.....................................X",
        "X....XXXX.............................X",
        "X.......................XXXX..........X",
        "X.....................................X",
        "X........XXXX..................XXXX...X",
        "X.....................................X",
        "X..XXX.......III.........SSS....X.....X",
        "X.....................................X",
        "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
    };

    LevelDefinition level;
    level.width = static_cast<int>(groundRows.front().size());
    level.height = static_cast<int>(groundRows.size());
    level.tileSize = 32;
    level.ground = detail::BuildGroundLayer(groundRows);

    level.pools = {
        LevelPool{
            PoolKind::Fire,
            PoolState::Liquid,
            {{14, 15}, {14, 16}, {14, 17}},
        },
        LevelPool{
            PoolKind::Water,
            PoolState::Liquid,
            {{14, 21}, {14, 22}, {14, 23}},
        },
    };

    level.objects = {
        LevelObject{LevelObjectType::FireSpawn, {14, 3}},
        LevelObject{LevelObjectType::FireDoor, {14, 30}},
        LevelObject{LevelObjectType::WaterSpawn, {14, 35}},
        LevelObject{LevelObjectType::WaterDoor, {15, 34}},
    };

    return level;
}

} // namespace Levels

#endif
