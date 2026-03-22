#include "CollisionSystem.hpp"

#include <cmath>

#include "config.hpp"

namespace {
TileCoord WorldToTile(const glm::vec2& world, const float tileSize) {
    return {
        static_cast<int>(std::floor(
            (static_cast<float>(WINDOW_HEIGHT) * 0.5F - world.y) / tileSize)),
        static_cast<int>(std::floor(
            (world.x + static_cast<float>(WINDOW_WIDTH) * 0.5F) / tileSize)),
    };
}
} // namespace

void CollisionSystem::ResolveCharacterHazards(
    Character& character, const LevelManager& levelManager) const {
    const glm::vec2 position = character.GetPosition();
    const glm::vec2 size = character.GetCollisionSize();
    const float tileSize = levelManager.GetTileSize();
    const TileCoord footTile =
        WorldToTile({position.x, position.y - size.y * 0.5F}, tileSize);

    const bool inLava = levelManager.IsLavaTile(footTile.row, footTile.col);
    const bool inWater = levelManager.IsWaterTile(footTile.row, footTile.col);

    bool shouldRespawn = false;
    const auto& levelData = levelManager.GetLevelData();

    if (character.GetElement() == Element::FIRE && inWater) {
        shouldRespawn = levelData.hasFireSpawn;
    } else if (character.GetElement() == Element::WATER && inLava) {
        shouldRespawn = levelData.hasWaterSpawn;
    }

    if (!shouldRespawn) {
        return;
    }

    const TileCoord spawn =
        character.GetElement() == Element::FIRE ? levelData.fireSpawn
                                                : levelData.waterSpawn;
    character.SetPosition(levelManager.TileToWorldPosition(spawn.row, spawn.col));
    character.SetVelocity({0.0F, 0.0F});
    character.SetGrounded(false);
}

void CollisionSystem::ResolveCharacterTerrain(
    Character& character, const LevelManager& levelManager) const {
    glm::vec2 position = character.GetPosition();
    glm::vec2 velocity = character.GetVelocity();
    const glm::vec2 size = character.GetCollisionSize();
    const float tileSize = levelManager.GetTileSize();
    const float halfWidth = size.x * 0.5F;
    const float halfHeight = size.y * 0.5F;

    if (velocity.x > 0.0F) {
        const TileCoord topRight =
            WorldToTile({position.x + halfWidth, position.y + halfHeight - 1.0F},
                        tileSize);
        const TileCoord bottomRight =
            WorldToTile({position.x + halfWidth, position.y - halfHeight + 1.0F},
                        tileSize);

        if (levelManager.IsSolidTile(topRight.row, topRight.col) ||
            levelManager.IsSolidTile(bottomRight.row, bottomRight.col)) {
            const TileCoord hitTile =
                levelManager.IsSolidTile(topRight.row, topRight.col)
                    ? topRight
                    : bottomRight;
            const glm::vec2 tileCenter =
                levelManager.TileToWorldPosition(hitTile.row, hitTile.col);
            position.x = tileCenter.x - tileSize * 0.5F - halfWidth;
            velocity.x = 0.0F;
        }
    } else if (velocity.x < 0.0F) {
        const TileCoord topLeft =
            WorldToTile({position.x - halfWidth, position.y + halfHeight - 1.0F},
                        tileSize);
        const TileCoord bottomLeft =
            WorldToTile({position.x - halfWidth, position.y - halfHeight + 1.0F},
                        tileSize);

        if (levelManager.IsSolidTile(topLeft.row, topLeft.col) ||
            levelManager.IsSolidTile(bottomLeft.row, bottomLeft.col)) {
            const TileCoord hitTile =
                levelManager.IsSolidTile(topLeft.row, topLeft.col)
                    ? topLeft
                    : bottomLeft;
            const glm::vec2 tileCenter =
                levelManager.TileToWorldPosition(hitTile.row, hitTile.col);
            position.x = tileCenter.x + tileSize * 0.5F + halfWidth;
            velocity.x = 0.0F;
        }
    }

    if (velocity.y > 0.0F) {
        const TileCoord topLeft =
            WorldToTile({position.x - halfWidth + 1.0F, position.y + halfHeight},
                        tileSize);
        const TileCoord topRight =
            WorldToTile({position.x + halfWidth - 1.0F, position.y + halfHeight},
                        tileSize);

        if (levelManager.IsSolidTile(topLeft.row, topLeft.col) ||
            levelManager.IsSolidTile(topRight.row, topRight.col)) {
            const TileCoord hitTile =
                levelManager.IsSolidTile(topLeft.row, topLeft.col)
                    ? topLeft
                    : topRight;
            const glm::vec2 tileCenter =
                levelManager.TileToWorldPosition(hitTile.row, hitTile.col);
            position.y = tileCenter.y - tileSize * 0.5F - halfHeight;
            velocity.y = 0.0F;
        }
    }

    character.SetGrounded(false);
    if (velocity.y <= 0.0F) {
        const TileCoord bottomLeft =
            WorldToTile({position.x - halfWidth + 1.0F, position.y - halfHeight},
                        tileSize);
        const TileCoord bottomRight =
            WorldToTile({position.x + halfWidth - 1.0F, position.y - halfHeight},
                        tileSize);

        if (levelManager.IsSolidTile(bottomLeft.row, bottomLeft.col) ||
            levelManager.IsSolidTile(bottomRight.row, bottomRight.col)) {
            const TileCoord hitTile =
                levelManager.IsSolidTile(bottomLeft.row, bottomLeft.col)
                    ? bottomLeft
                    : bottomRight;
            const glm::vec2 tileCenter =
                levelManager.TileToWorldPosition(hitTile.row, hitTile.col);
            position.y = tileCenter.y + tileSize * 0.5F + halfHeight;
            velocity.y = 0.0F;
            character.SetGrounded(true);
        }
    }

    character.SetPosition(position);
    character.SetVelocity(velocity);
    ResolveCharacterHazards(character, levelManager);
}
