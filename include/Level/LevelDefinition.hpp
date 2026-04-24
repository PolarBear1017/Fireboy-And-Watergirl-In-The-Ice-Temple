#ifndef LEVEL_DEFINITION_HPP
#define LEVEL_DEFINITION_HPP

#include <string>
#include <vector>
#include "../Element.hpp"

enum class TerrainType {
    Empty = 0,
    Block = 1,
    SlopeBL = 10,
    SlopeBR = 11,
    SlopeTL = 12,
    SlopeTR = 13,
    SnowSlopeBL = 20,
    SnowSlopeBR = 21,
    SnowBlock = 22,
    Ice = 30,
    Water = 31,
    Fire = 32,
    Toxic = 33,
};

enum class LevelObjectType {
    Spawn,
    Door,
    Button,
    Lever,
    Elevator,
    Diamond
};

struct GridCoord {
    int row = 0;
    int col = 0;
};

struct LevelOverlay {
    Element element;
    GridCoord coord;
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
    std::vector<std::vector<TerrainType>> terrainLayer;
    std::vector<LevelOverlay> overlays;
    std::vector<LevelObject> objects;
};

#endif
