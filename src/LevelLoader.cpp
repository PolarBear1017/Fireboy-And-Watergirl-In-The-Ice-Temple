#include "LevelLoader.hpp"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace {
TerrainType ParseTerrain(const int value) {
    switch (value) {
        case 0:
            return TerrainType::Empty;
        case 1:
            return TerrainType::Solid;
        case 4:
            return TerrainType::Ice;
        case 5:
            return TerrainType::Snow;
        default:
            throw std::runtime_error("LevelLoader: unsupported terrain value.");
    }
}

PoolKind ParsePoolKind(const std::string& value) {
    if (value == "fire") {
        return PoolKind::Fire;
    }
    if (value == "water") {
        return PoolKind::Water;
    }

    throw std::runtime_error("LevelLoader: unsupported pool kind.");
}

PoolState ParsePoolState(const std::string& value) {
    if (value == "liquid") {
        return PoolState::Liquid;
    }
    if (value == "frozen") {
        return PoolState::Frozen;
    }

    throw std::runtime_error("LevelLoader: unsupported pool state.");
}

LevelObjectType ParseObjectType(const std::string& value) {
    if (value == "fire_spawn") {
        return LevelObjectType::FireSpawn;
    }
    if (value == "water_spawn") {
        return LevelObjectType::WaterSpawn;
    }
    if (value == "fire_door") {
        return LevelObjectType::FireDoor;
    }
    if (value == "water_door") {
        return LevelObjectType::WaterDoor;
    }
    if (value == "button") {
        return LevelObjectType::Button;
    }
    if (value == "lever") {
        return LevelObjectType::Lever;
    }
    if (value == "elevator") {
        return LevelObjectType::Elevator;
    }

    throw std::runtime_error("LevelLoader: unsupported object type.");
}
} // namespace

LevelDefinition LoadLevelDefinitionFromJsonFile(const std::string& path) {
    // 開啟檔案
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error("LevelLoader: failed to open level file: " + path);
    }

    // 把 JSON 檔內容讀進 root
    nlohmann::json root;
    input >> root;

    LevelDefinition level;  // 最後要回傳的關卡資料
    // 讀取基本資訊
    level.width = root.at("width").get<int>();
    level.height = root.at("height").get<int>();
    level.tileSize = root.value("tileSize", 32);

    // 先拿到 JSON 裡的地形資料，再把程式裡要存地形的陣列準備好大小。
    const auto& groundJson = root.at("ground");
    level.ground.resize(static_cast<size_t>(level.height));

    // 逐列解析地形資料，將數字代碼轉成程式內部使用的 TerrainType。
    for (int row = 0; row < level.height; ++row) {
        const auto& rowJson = groundJson.at(static_cast<size_t>(row));
        level.ground[static_cast<size_t>(row)].reserve(static_cast<size_t>(level.width));

        for (int col = 0; col < level.width; ++col) {
            const int terrainValue = rowJson.at(static_cast<size_t>(col)).get<int>();
            if (terrainValue == 2 || terrainValue == 3) {
                level.ground[static_cast<size_t>(row)].push_back(TerrainType::Empty);
                level.pools.push_back(LevelPool{
                    terrainValue == 2 ? PoolKind::Fire : PoolKind::Water,
                    PoolState::Liquid,
                    {GridCoord{row, col}}
                });
                continue;
            }

            level.ground[static_cast<size_t>(row)].push_back(ParseTerrain(terrainValue));
        }
    }

    if (root.contains("pools")) {
        for (const auto& poolJson : root.at("pools")) {
            LevelPool pool;
            pool.kind = ParsePoolKind(poolJson.at("kind").get<std::string>());
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

    // 解析 objects 圖層，建立出生點與門等重要物件。
    if (root.contains("objects")) {
        for (const auto& objectJson : root.at("objects")) {
            LevelObject object;
            object.type = ParseObjectType(objectJson.at("type").get<std::string>());
            object.coord.row = objectJson.at("row").get<int>();
            object.coord.col = objectJson.at("col").get<int>();
            
            if (objectJson.contains("group_id")) {
                object.group_id = objectJson.at("group_id").get<int>();
            }
            if (objectJson.contains("length")) {
                object.length = objectJson.at("length").get<int>();
            }
            if (objectJson.contains("is_horizontal")) {
                object.is_horizontal = objectJson.at("is_horizontal").get<bool>();
            }
            if (objectJson.contains("target_row")) {
                object.target_row = objectJson.at("target_row").get<int>();
            }
            if (objectJson.contains("target_col")) {
                object.target_col = objectJson.at("target_col").get<int>();
            }

            level.objects.push_back(object);
        }
    }

    return level;
}
