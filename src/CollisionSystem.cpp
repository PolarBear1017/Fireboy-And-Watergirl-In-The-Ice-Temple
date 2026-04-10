#include "CollisionSystem.hpp"
#include "Mechanics/Elevator.hpp"

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

    const auto& levelData = levelManager.GetLevelData();
    const bool shouldRespawn =
        levelManager.IsHazardousFor(character.GetElement(), footTile.row,
                                    footTile.col) &&
        ((character.GetElement() == Element::FIRE && levelData.hasFireSpawn) ||
         (character.GetElement() == Element::WATER && levelData.hasWaterSpawn));

    if (!shouldRespawn) {
        return;
    }

    const TileCoord spawn =
        character.GetElement() == Element::FIRE ? levelData.fireSpawn
                                                : levelData.waterSpawn;
    character.SetPosition(levelManager.TileToWorldPosition(spawn.row, spawn.col));
    character.SetVelocity({0.0F, 0.0F});
    character.SetGroundState(GroundState::AIR);
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

    character.SetGroundState(GroundState::AIR);
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
            character.SetGroundState(GroundState::GROUND);
        }
    }

    character.SetVelocity(velocity);
    ResolveCharacterHazards(character, levelManager);
}

void CollisionSystem::ResolveCharacterMechanics(
    Character& character,
    const std::vector<std::shared_ptr<BaseMechanism>>& mechanisms) const {
    
    glm::vec2 pos = character.GetPosition();
    glm::vec2 vel = character.GetVelocity();
    glm::vec2 halfChar = character.GetCollisionSize() * 0.5f;

    for (const auto& mech : mechanisms) {
        auto colliderOpt = mech->GetCollider();
        if (!colliderOpt) continue;

        Collider collider = *colliderOpt;
        glm::vec2 halfColl = collider.size * 0.5f;

        // Check AABB overlap
        if (std::abs(pos.x - collider.center.x) < (halfChar.x + halfColl.x) &&
            std::abs(pos.y - collider.center.y) < (halfChar.y + halfColl.y)) {
            
            // Calculate overlap on both axes
            float overlapX = (halfChar.x + halfColl.x) - std::abs(pos.x - collider.center.x);
            float overlapY = (halfChar.y + halfColl.y) - std::abs(pos.y - collider.center.y);

            // Resolve on the axis with smaller overlap (simplistic AABB resolution)
            if (overlapX < overlapY) {
                if (pos.x > collider.center.x) pos.x += overlapX;
                else pos.x -= overlapX;
                vel.x = 0;
            } else {
                if (pos.y > collider.center.y) {
                    // Standing on top
                    pos.y += overlapY;
                    vel.y = 0;
                    character.SetGroundState(GroundState::GROUND);
                    
                    // If it's an elevator, inherit delta movement
                    auto elevator = std::dynamic_pointer_cast<Elevator>(mech);
                    if (elevator) {
                        pos += elevator->GetDeltaPosition();
                    }
                } else {
                    // Hitting head
                    pos.y -= overlapY;
                    vel.y = 0;
                }
            }
        }
    }

    character.SetPosition(pos);
    character.SetVelocity(vel);
}
