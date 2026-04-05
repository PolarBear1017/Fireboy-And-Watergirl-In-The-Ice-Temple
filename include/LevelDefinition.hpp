#ifndef LEVEL_DEFINITION_HPP
#define LEVEL_DEFINITION_HPP

#include <string>
#include <vector>

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

enum class PoolKind {
    Fire,
    Water
};

enum class PoolState {
    Liquid,
    Frozen
};

struct LevelPool {
    PoolKind kind;
    PoolState state = PoolState::Liquid;
    std::vector<GridCoord> tiles;
};

enum class LevelObjectType {
    FireSpawn,
    WaterSpawn,
    FireDoor,
    WaterDoor
};

struct LevelObject {
    LevelObjectType type;
    GridCoord coord;
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
