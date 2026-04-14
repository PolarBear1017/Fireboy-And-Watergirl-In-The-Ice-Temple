#ifndef LEVEL_MANAGER_HPP
#define LEVEL_MANAGER_HPP

#include "../Element.hpp"
#include "LevelDefinition.hpp"
#include "../SpriteAtlas.hpp"
#include "Util/GameObject.hpp"
#include <memory>
#include <string>
#include <vector>

struct LevelParseResult {
    std::vector<GridCoord> solidTiles;
    GridCoord fireSpawn;
    GridCoord waterSpawn;
    GridCoord fireDoor;
    GridCoord waterDoor;
    bool hasFireSpawn = false;
    bool hasWaterSpawn = false;
    bool hasFireDoor = false;
    bool hasWaterDoor = false;
};

class LevelManager {
private:
    std::shared_ptr<SpriteAtlas> m_GroundAtlas;
    std::shared_ptr<SpriteAtlas> m_OverlayAtlas;
    LevelParseResult m_LevelData;
    LevelDefinition m_LevelDefinition;
    float m_TileSize = 32.0F;
    static constexpr float GROUND_Z_INDEX = 0.0F;

public:
    explicit LevelManager(std::shared_ptr<SpriteAtlas> groundAtlas, std::shared_ptr<SpriteAtlas> overlayAtlas);

    [[nodiscard]] const LevelParseResult& GetLevelData() const { return m_LevelData; }
    [[nodiscard]] float GetTileSize() const { return m_TileSize; }

    [[nodiscard]] bool IsWalkable(const int row, const int col) const;
    [[nodiscard]] bool IsHazardousFor(Element element, int row, int col) const;
    [[nodiscard]] glm::vec2 TileToWorldPosition(int row, int col) const;

    bool LoadLevel(const LevelDefinition& level, const std::shared_ptr<Util::GameObject>& root);

private:
    bool ValidateLevelDefinition(const LevelDefinition& level) const;
    // RegisterTerrain 有刪除疑慮
    void RegisterTerrain(TerrainType terrain, int row, int col);
    void RegisterObject(const LevelObject& object);

    // 判斷座標是否合理、左右是否為同圖
    [[nodiscard]] bool IsValidCoord(const int row, const int col) const;
    [[nodiscard]] bool HasSameTerrain(const LevelDefinition& level, int row, int col, TerrainType terrain) const;
    [[nodiscard]] bool HasSameOverlay(const int row, const int col, const OverlayType type) const;

    // 取得各種 FrameName 與 ZIndex
    [[nodiscard]] std::string DetermineGroundFrameName(const LevelDefinition& level, int row, int col, TerrainType terrain) const;
    [[nodiscard]] std::string DetermineOverlayFrameName(const int row, const int col, const OverlayType type) const;
    float DetermineOverlayZIndex(const OverlayType type) const;
    [[nodiscard]] std::string DetermineObjectFrameName(Element e) const;
    [[nodiscard]] float DetermineObjectZIndex(LevelObjectType type) const;


    // 直接丟掉
    // bool LoadLevel(const std::vector<std::string>& mapData, const std::shared_ptr<Util::GameObject>& root);
    // [[nodiscard]] const LevelPool* GetPoolAt(int row, int col) const;
    // [[nodiscard]] bool IsTileInPool(const LevelPool& pool, int row, int col) const;
    // [[nodiscard]] float ResolveGroundZIndex(TerrainType terrain) const;

    // 替換
    // [[nodiscard]] bool IsSolidTile(int row, int col) const;
    // [[nodiscard]] bool IsValidGroundCoord(int row, int col) const;
    // [[nodiscard]] bool HasSamePoolVisual(int row, int col, Element element, PoolState state) const;
    // [[nodiscard]] std::string ResolvePoolFrameName(int row, int col, const LevelOverlay& overlay) const;
    // [[nodiscard]] float ResolvePoolZIndex(PoolState state) const;
};

#endif
