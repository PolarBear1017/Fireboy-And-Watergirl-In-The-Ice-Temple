#ifndef LEVEL_DEFINITION_HPP
#define LEVEL_DEFINITION_HPP

#include <string>
#include <vector>
#include "../Element.hpp"

enum class TerrainType {
    Empty = 0,              // 空氣
    Block = 1,              // 普通滿格磚塊
    SlopeBL = 10,           // 左斜坡
    SlopeBR = 11,           // 右斜坡
    SlopeTL = 12,           // 左斜頂
    SlopeTR = 13,           // 右斜頂
    SnowSlopeBL = 20,       // 積雪左斜坡
    SnowSlopeBR = 21,       // 積雪右斜坡
    SnowBlock = 22,         // 積雪磚塊
    Ice = 30,               // 冰（凍結水池）
    Water = 31,             // 水池
    Fire = 32,              // 岩漿
    Toxic = 33,             // 毒沼
    // 淺坑系列（Shallow_）為虛擬地形，作為水池池底，便於碰撞系統分類，並未實際存在於關卡的 TerrainArray 中
    ShallowSlopeBL = 40,    // 淺坑左斜坡
    ShallowSlopeBR = 41,    // 淺坑右斜坡
    ShallowBlock = 42       // 淺坑平地
};

enum class LevelObjectType {
    Spawn,
    Door,
    Button,
    Lever,
    Elevator,
    Diamond,
    Block,
    TimedButton
};

struct GridCoord {
    int row = 0;
    int col = 0;
};

struct LevelOverlay {
    Element element;
    GridCoord coord; // The start/left position
    int width = 1;
};

struct LevelObject {
    LevelObjectType type;
    Element element;
    GridCoord coord;
    int group_id = 0;
    int length = 1;
    bool is_horizontal = true;
    int target_row = -1;
    int target_col = -1;
    float time = 0.0f;
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
