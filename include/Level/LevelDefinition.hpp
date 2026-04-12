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
};

struct LevelObject {
    LevelObjectType type;
    Element element;
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
