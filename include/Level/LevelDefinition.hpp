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
    SlopeTR = 13
};

enum class OverlayType {
    None = 0,
    Water = 1,
    Fire = 2,
    Toxic = 3,
    Ice = 10,
    Snow = 11
};

enum class LevelObjectType {
    Spawn,
    Door,
    Button,
    Lever,
    Elevator
};

struct GridCoord {
    int row = 0;
    int col = 0;
};

struct LevelOverlay {
    Element element;
    OverlayType type;
    std::vector<GridCoord> tiles;
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
    std::vector<std::vector<TerrainType>> groundLayer;
    std::vector<std::vector<OverlayType>> overlayLayer;
    std::vector<LevelOverlay> overlays;
    std::vector<LevelObject> objects;
};

#endif
