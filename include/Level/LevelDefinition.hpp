#ifndef LEVEL_DEFINITION_HPP
#define LEVEL_DEFINITION_HPP

#include <string>
#include <vector>
#include "../Element.hpp"

enum class TerrainType {
    Empty,
    Solid,
    Ice,
    Snow
};

struct GridCoord {
    int row = 0;
    int col = 0;
};

enum class PoolState {
    Liquid,
    Frozen
};

struct LevelPool {
    Element element;
    PoolState state = PoolState::Liquid;
    std::vector<GridCoord> tiles;
};

enum class LevelObjectType {
    Spawn,
    Door,
    Mechanics
    FireSpawn,
    WaterSpawn,
    FireDoor,
    WaterDoor,
    Button,
    Lever,
    Elevator
};

struct LevelObject {
    LevelObjectType type;
    Element element;
    GridCoord coord;
    int group_id = -1;
    int length = 1;
    bool is_horizontal = true;
    int target_row = -1;
    int target_col = -1;
};

struct LevelDefinition {
    int width = 0;
    int height = 0;
    int tileSize = 32;
    std::vector<std::vector<TerrainType>> ground;
    std::vector<LevelPool> pools;
    std::vector<LevelObject> objects;
};

#endif
