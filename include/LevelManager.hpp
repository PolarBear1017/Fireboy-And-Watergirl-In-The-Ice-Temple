#ifndef LEVEL_MANAGER_HPP
#define LEVEL_MANAGER_HPP

#include "LevelDefinition.hpp"
#include "SpriteAtlas.hpp"
#include "Util/GameObject.hpp"
#include <memory>
#include <string>
#include <vector>

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
    void LoadLevel(const LevelDefinition& level,
                   const std::shared_ptr<Util::GameObject>& root);

    [[nodiscard]] const LevelParseResult& GetLevelData() const { return m_LevelData; }
    [[nodiscard]] bool IsSolidTile(int row, int col) const;
    [[nodiscard]] bool IsLavaTile(int row, int col) const;
    [[nodiscard]] bool IsWaterTile(int row, int col) const;
    [[nodiscard]] glm::vec2 TileToWorldPosition(int row, int col) const;
    [[nodiscard]] float GetTileSize() const { return m_TileSize; }

private:
    bool ValidateLevel(const std::vector<std::string>& mapData) const;
    bool ValidateLevelDefinition(const LevelDefinition& level) const;
    void RegisterTerrain(TerrainType terrain, int row, int col);
    void RegisterObject(const LevelObject& object);
    [[nodiscard]] bool IsValidGroundCoord(int row, int col) const;
    [[nodiscard]] bool HasSameTerrain(const LevelDefinition& level, int row, int col,
                                      TerrainType terrain) const;
    [[nodiscard]] std::string ResolveGroundFrameName(const LevelDefinition& level, int row,
                                                     int col, TerrainType terrain) const;
    [[nodiscard]] float ResolveGroundZIndex(TerrainType terrain) const;
    [[nodiscard]] std::string ResolveObjectFrameName(LevelObjectType type) const;
    [[nodiscard]] float ResolveObjectZIndex(LevelObjectType type) const;

    std::shared_ptr<SpriteAtlas> m_Atlas;
    LevelParseResult m_LevelData;
    LevelDefinition m_LevelDefinition;
    float m_TileSize = 32.0F;
};

#endif
