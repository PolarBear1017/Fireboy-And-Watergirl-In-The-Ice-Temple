#include "Level/LevelLoader.hpp" // 修正：建議使用搬移後的路徑
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace {
    void ProcessTerrainArray(const std::vector<std::vector<int>>& rawGrid, LevelDefinition& level) {
        for (int r = 0; r < level.height; ++r) {
            for (int c = 0; c < level.width; ++c) {
                int value = rawGrid[r][c];

                level.terrainLayer[r][c] = static_cast<TerrainType>(value);

                if (value >= 30 && value <= 33) {
                    LevelOverlay overlay;
                    overlay.coord = {r, c};

                    if (value == 30) overlay.element = Element::ICE;
                    if (value == 31) overlay.element = Element::WATER;
                    if (value == 32) overlay.element = Element::FIRE;
                    if (value == 33) overlay.element = Element::TOXIC;

                    level.overlays.push_back(overlay);
                }
            }
        }
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

    // 🌟 魔法 2：一口氣把二維陣列撐開，並全部填滿 Empty
    level.terrainLayer.assign(level.height, std::vector<TerrainType>(level.width, TerrainType::Empty));

    // --- 1. 解析地形 (極簡版) ---
    auto rawGrid = root.at("terrain").get<std::vector<std::vector<int>>>();
    ProcessTerrainArray(rawGrid, level);

    // --- 3. 解析物件 ---
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