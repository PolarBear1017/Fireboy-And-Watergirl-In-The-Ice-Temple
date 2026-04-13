#include "Level/LevelLoader.hpp" // 修正：建議使用搬移後的路徑
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace {
    TerrainType ParseTerrain(const int value) {
        switch (value) {
            case 0: return TerrainType::Empty;
            case 1: return TerrainType::Solid;
            case 4: return TerrainType::Ice;
            case 5: return TerrainType::Snow;
            default: throw std::runtime_error("LevelLoader: unsupported terrain value.");
        }
    }

    Element ParsePoolKind(const std::string& value) {
        if (value == "fire") return Element::FIRE;
        if (value == "water") return Element::WATER;
        throw std::runtime_error("LevelLoader: unsupported pool kind.");
    }

    PoolState ParsePoolState(const std::string& value) {
        if (value == "liquid") return PoolState::Liquid;
        if (value == "frozen") return PoolState::Frozen;
        throw std::runtime_error("LevelLoader: unsupported pool state.");
    }

    LevelObjectType ParseObjectType(const std::string& value) {
        if (value.find("spawn") != std::string::npos) return LevelObjectType::Spawn;
        if (value.find("door") != std::string::npos) return LevelObjectType::Door;
        if (value == "button") return LevelObjectType::Button;
        if (value == "lever") return LevelObjectType::Lever;
        if (value == "elevator") return LevelObjectType::Elevator;
        throw std::runtime_error("LevelLoader: unsupported object type: " + value);
    }

    Element ParseElement(const std::string& value) {
        if (value.find("fire") != std::string::npos) return Element::FIRE;
        if (value.find("water") != std::string::npos) return Element::WATER;
        return Element::NEUTRAL; // 出生點和門以外的物件（如電梯）可能沒有元素屬性
    }
}

// LevelDefinition LoadLevelDefinitionFromJsonFile(const std::string& path) {

//     std::ifstream input(path);
//     if (!input.is_open()) {
//         throw std::runtime_error("LevelLoader: failed to open level file: " + path);
//     }
//
//
//     nlohmann::json root;
//     input >> root;
//
//     LevelDefinition level;  // 最後要回傳的關卡資料
//     level.width = root.at("width").get<int>();
//     level.height = root.at("height").get<int>();
//     level.tileSize = root.value("tileSize", 32);
//
//     const auto& groundJson = root.at("ground");
//     level.ground.resize(static_cast<size_t>(level.height));
//
//     for (int row = 0; row < level.height; ++row) {
//         const auto& rowJson = groundJson.at(static_cast<size_t>(row));
//         level.ground[static_cast<size_t>(row)].reserve(static_cast<size_t>(level.width));
//
//         for (int col = 0; col < level.width; ++col) {
//             const int terrainValue = rowJson.at(static_cast<size_t>(col)).get<int>();
//             if (terrainValue == 2 || terrainValue == 3) {
//                 level.ground[static_cast<size_t>(row)].push_back(TerrainType::Empty);
//                 level.pools.push_back(LevelPool{
//                     terrainValue == 2 ? Element::FIRE : Element::WATER,
//                     PoolState::Liquid,
//                     {GridCoord{row, col}}
//                 });
//                 continue;
//             }
//
//             level.ground[static_cast<size_t>(row)].push_back(ParseTerrain(terrainValue));
//         }
//     }
//
//     if (root.contains("pools")) {
//         for (const auto& poolJson : root.at("pools")) {
//             LevelPool pool;
//             pool.element = ParsePoolKind(poolJson.at("kind").get<std::string>());
//             pool.state = ParsePoolState(poolJson.value("state", "liquid"));
//
//             for (const auto& tileJson : poolJson.at("tiles")) {
//                 pool.tiles.push_back(GridCoord{
//                     tileJson.at("row").get<int>(),
//                     tileJson.at("col").get<int>()
//                 });
//             }
//             level.pools.push_back(pool);
//         }
//     }
//
//     // 解析 objects 圖層，建立出生點與門等重要物件。
//     for (const auto& objectJson : root.at("objects")) {
//         LevelObject object;
//         std::string typeStr = objectJson.at("type").get<std::string>();
//         object.type = ParseObjectType(typeStr);
//         object.element = ParseElement(typeStr);
//         object.coord.row = objectJson.at("row").get<int>();
//         object.coord.col = objectJson.at("col").get<int>();
//         level.objects.push_back(object);
//         if (root.contains("objects")) {
//             for (const auto& objectJson : root.at("objects")) {
//                 LevelObject object;
//                 object.type = ParseObjectType(objectJson.at("type").get<std::string>());
//                 object.coord.row = objectJson.at("row").get<int>();
//                 object.coord.col = objectJson.at("col").get<int>();
//
//                 if (objectJson.contains("group_id")) {
//                     object.group_id = objectJson.at("group_id").get<int>();
//                 }
//                 if (objectJson.contains("length")) {
//                     object.length = objectJson.at("length").get<int>();
//                 }
//                 if (objectJson.contains("is_horizontal")) {
//                     object.is_horizontal = objectJson.at("is_horizontal").get<bool>();
//                 }
//                 if (objectJson.contains("target_row")) {
//                     object.target_row = objectJson.at("target_row").get<int>();
//                 }
//                 if (objectJson.contains("target_col")) {
//                     object.target_col = objectJson.at("target_col").get<int>();
//                 }
//
//                 level.objects.push_back(object);
//             }
//         }
//     }
//     return level;
// }


LevelDefinition LoadLevelDefinitionFromJsonFile(const std::string& path) {
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error("LevelLoader: failed to open level file: " + path);
    }

    nlohmann::json root;
    input >> root;

    LevelDefinition level;
    level.width = root.at("width").get<int>();
    level.height = root.at("height").get<int>();
    level.tileSize = root.value("tileSize", 32);

    // --- 1. 解析地形 (Ground Layer) ---
    const auto& groundJson = root.at("ground");
    level.ground.resize(static_cast<size_t>(level.height));

    for (int row = 0; row < level.height; ++row) {
        const auto& rowJson = groundJson.at(static_cast<size_t>(row));
        level.ground[static_cast<size_t>(row)].reserve(static_cast<size_t>(level.width));

        for (int col = 0; col < level.width; ++col) {
            const int terrainValue = rowJson.at(static_cast<size_t>(col)).get<int>();

            level.ground[static_cast<size_t>(row)].push_back(ParseTerrain(terrainValue));
        }
    }

    // 水池邏輯
    if (root.contains("pools")) {
        for (const auto& poolJson : root.at("pools")) {
            LevelPool pool;
            pool.element = ParsePoolKind(poolJson.at("kind").get<std::string>());
            pool.state = ParsePoolState(poolJson.value("state", "liquid"));

            for (const auto& tileJson : poolJson.at("tiles")) {
                pool.tiles.push_back(GridCoord{
                    tileJson.at("row").get<int>(),
                    tileJson.at("col").get<int>()
                });
            }

            level.pools.push_back(pool);
        }
    }

    // --- 2. 解析物件 (Objects Layer) ---
    if (root.contains("objects")) {
        for (const auto& objectJson : root.at("objects")) {
            LevelObject object;
            std::string typeStr = objectJson.at("type").get<std::string>();

            // 基本屬性
            object.type = ParseObjectType(typeStr);
            object.element = ParseElement(typeStr);
            object.coord.row = objectJson.at("row").get<int>();
            object.coord.col = objectJson.at("col").get<int>();

            // 隊友新增的擴充屬性 (電梯、機關用)
            if (objectJson.contains("group_id")) object.group_id = objectJson.at("group_id").get<int>();
            if (objectJson.contains("length")) object.length = objectJson.at("length").get<int>();
            if (objectJson.contains("is_horizontal")) object.is_horizontal = objectJson.at("is_horizontal").get<bool>();
            if (objectJson.contains("target_row")) object.target_row = objectJson.at("target_row").get<int>();
            if (objectJson.contains("target_col")) object.target_col = objectJson.at("target_col").get<int>();

            level.objects.push_back(object);
        }
    }

    return level;
}