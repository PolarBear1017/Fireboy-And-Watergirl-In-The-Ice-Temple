#include "Level/LevelLoader.hpp" // 修正：建議使用搬移後的路徑
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace {
    void ParseAndAssignTerrain(const int value, const int row, const int col, LevelDefinition& level) {
        switch (value) {
            case 0:
                // 空白
                break;
            case 1:
                level.groundLayer[row][col] = TerrainType::Block;
                break;
                // 🚧 未來斜坡加在這裡，例如： case 10: level.groundLayer[row][col] = TerrainType::SlopeBL; break;
            case 4:
                // 舊 JSON 的冰塊，自動被歸類到疊加層！
                level.overlayLayer[row][col] = OverlayType::Ice;
                break;
            case 5:
                // 舊 JSON 的雪地，自動被歸類到疊加層！
                level.overlayLayer[row][col] = OverlayType::Snow;
                break;
            default:
                // 未知數值先忽略，或是拋出例外
                break;
        }
    }

    // 將水池字串轉為新的 OverlayType
    OverlayType ParsePoolKindToOverlay(const std::string& value) {
        if (value == "fire") return OverlayType::Fire;
        if (value == "water") return OverlayType::Water;
        if (value == "toxic") return OverlayType::Toxic; // 預留毒液
        return OverlayType::None;
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
        return Element::NEUTRAL;
    }
}


// LevelDefinition LoadLevelDefinitionFromJsonFile(const std::string& path) {
//     std::ifstream input(path);
//     if (!input.is_open()) {
//         throw std::runtime_error("LevelLoader: failed to open level file: " + path);
//     }
//
//     nlohmann::json root;
//     input >> root;
//
//     LevelDefinition level;
//     level.width = root.at("width").get<int>();
//     level.height = root.at("height").get<int>();
//     level.tileSize = root.value("tileSize", 32);
//
//     // --- 1. 解析地形 (Ground Layer) ---
//     const auto& groundJson = root.at("ground");
//     level.groundLayer.resize(static_cast<size_t>(level.height));
//
//     for (int row = 0; row < level.height; ++row) {
//         const auto& rowJson = groundJson.at(static_cast<size_t>(row));
//         level.groundLayer[static_cast<size_t>(row)].reserve(static_cast<size_t>(level.width));
//
//         for (int col = 0; col < level.width; ++col) {
//             const int terrainValue = rowJson.at(static_cast<size_t>(col)).get<int>();
//
//             level.groundLayer[static_cast<size_t>(row)].push_back(ParseTerrain(terrainValue));
//         }
//     }
//
//     // 水池邏輯
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
//
//             level.pools.push_back(pool);
//         }
//     }
//
//     // --- 2. 解析物件 (Objects Layer) ---
//     if (root.contains("objects")) {
//         for (const auto& objectJson : root.at("objects")) {
//             LevelObject object;
//             std::string typeStr = objectJson.at("type").get<std::string>();
//
//             // 基本屬性
//             object.type = ParseObjectType(typeStr);
//             object.element = ParseElement(typeStr);
//             object.coord.row = objectJson.at("row").get<int>();
//             object.coord.col = objectJson.at("col").get<int>();
//
//             // 隊友新增的擴充屬性 (電梯、機關用)
//             if (objectJson.contains("group_id")) object.group_id = objectJson.at("group_id").get<int>();
//             if (objectJson.contains("length")) object.length = objectJson.at("length").get<int>();
//             if (objectJson.contains("is_horizontal")) object.is_horizontal = objectJson.at("is_horizontal").get<bool>();
//             if (objectJson.contains("target_row")) object.target_row = objectJson.at("target_row").get<int>();
//             if (objectJson.contains("target_col")) object.target_col = objectJson.at("target_col").get<int>();
//
//             level.objects.push_back(object);
//         }
//     }
//
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

    // 🌟 魔法 2：一口氣把兩個二維陣列撐開，並全部填滿 Empty 和 None
    level.groundLayer.assign(level.height, std::vector<TerrainType>(level.width, TerrainType::Empty));
    level.overlayLayer.assign(level.height, std::vector<OverlayType>(level.width, OverlayType::None));

    // --- 1. 解析地形 (並自動分層) ---
    const auto& groundJson = root.at("ground");
    for (int row = 0; row < level.height; ++row) {
        const auto& rowJson = groundJson.at(static_cast<size_t>(row));
        for (int col = 0; col < level.width; ++col) {
            const int terrainValue = rowJson.at(static_cast<size_t>(col)).get<int>();
            // 讓小幫手去決定這個數字該放哪一層
            ParseAndAssignTerrain(terrainValue, row, col, level);
        }
    }

    // --- 2. 解析水池 (轉換成 LevelOverlay，並同步更新疊加層陣列) ---
    if (root.contains("pools")) {
        for (const auto& poolJson : root.at("pools")) {
            LevelOverlay overlayObj;
            std::string kindStr = poolJson.at("kind").get<std::string>();

            overlayObj.element = ParseElement(kindStr);
            overlayObj.type = ParsePoolKindToOverlay(kindStr);

            for (const auto& tileJson : poolJson.at("tiles")) {
                int r = tileJson.at("row").get<int>();
                int c = tileJson.at("col").get<int>();
                overlayObj.tiles.push_back(GridCoord{r, c});

                // 🌟 魔法 3：雙重表述的精髓！把物件座標同步蓋印章到二維陣列上
                level.overlayLayer[r][c] = overlayObj.type;
            }

            level.overlays.push_back(overlayObj);
        }
    }

    // --- 3. 解析物件 (維持隊友寫好的強大擴充，完全不變) ---
    if (root.contains("objects")) {
        for (const auto& objectJson : root.at("objects")) {
            LevelObject object;
            std::string typeStr = objectJson.at("type").get<std::string>();

            object.type = ParseObjectType(typeStr);
            object.element = ParseElement(typeStr);
            object.coord.row = objectJson.at("row").get<int>();
            object.coord.col = objectJson.at("col").get<int>();

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