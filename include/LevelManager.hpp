#ifndef LEVEL_MANAGER_HPP
#define LEVEL_MANAGER_HPP

#include "SpriteAtlas.hpp"
#include "Util/GameObject.hpp"
#include <vector>
#include <string>
#include <memory>

struct TileCoord {
    int row = 0;
    int col = 0;
};

struct LevelParseResult {
    std::vector<TileCoord> solidTiles;
    std::vector<TileCoord> lavaTiles;
    std::vector<TileCoord> waterTiles;
    TileCoord fireSpawn;
    TileCoord waterSpawn;
    TileCoord fireDoor;
    TileCoord waterDoor;
    bool hasFireSpawn = false;
    bool hasWaterSpawn = false;
    bool hasFireDoor = false;
    bool hasWaterDoor = false;
};

class LevelManager {
public:
    explicit LevelManager(std::shared_ptr<SpriteAtlas> atlas);
    
    // Loads the level and populates the given root node with tiles
    void LoadLevel(const std::vector<std::string>& mapData, const std::shared_ptr<Util::GameObject>& root);

    [[nodiscard]] const LevelParseResult& GetLevelData() const { return m_LevelData; }
    [[nodiscard]] bool IsSolidTile(int row, int col) const;
    [[nodiscard]] bool IsLavaTile(int row, int col) const;
    [[nodiscard]] bool IsWaterTile(int row, int col) const;
    [[nodiscard]] glm::vec2 TileToWorldPosition(int row, int col) const;
    [[nodiscard]] float GetTileSize() const { return m_TileSize; }

private:
    bool ValidateLevel(const std::vector<std::string>& mapData) const;
    void RegisterTile(char tileChar, int row, int col);
    [[nodiscard]] bool HasSameTile(const std::vector<std::string>& mapData, int row,
                                   int col, char tileChar) const;
    [[nodiscard]] std::string ResolveFrameName(
        const std::vector<std::string>& mapData, int row, int col,
        char tileChar) const;
    float ResolveZIndex(char tileChar) const;

    std::shared_ptr<SpriteAtlas> m_Atlas;
    LevelParseResult m_LevelData;
    std::vector<std::string> m_MapData;
    float m_TileSize = 32.0F;
};

#endif
