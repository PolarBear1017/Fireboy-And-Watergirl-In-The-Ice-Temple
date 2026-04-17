#include "CollisionSystem.hpp"
#include "Mechanics/Elevator.hpp"

#include <cmath>
#include <algorithm>
#include "config.hpp"

namespace {
    GridCoord WorldToTile(const glm::vec2& world, const float tileSize) {
        return {
            static_cast<int>(std::floor(
                (static_cast<float>(WINDOW_HEIGHT) * 0.5F - world.y) / tileSize)),
            static_cast<int>(std::floor(
                (world.x + static_cast<float>(WINDOW_WIDTH) * 0.5F) / tileSize)),
        };
    }

    bool IsSlope(TerrainType type) {
        return type == TerrainType::SlopeBL || type == TerrainType::SlopeBR;
    }

    float CalculateSlopeSurfaceY(TerrainType type, const glm::vec2& tileCenter, float tileSize, float playerX) {
        const float tileLeft = tileCenter.x - tileSize * 0.5F;
        float rx = playerX - tileLeft; // 計算角色在磚塊內的相對 X 座標 (0 ~ tileSize)
        rx = std::clamp(rx, 0.0F, tileSize); // 防呆，確保不出界

        const float tileBottom = tileCenter.y - tileSize * 0.5F;

        if (type == TerrainType::SlopeBL) {
            // SlopeBL: 左高右低。相對高度 = tileSize - rx
            return tileBottom + (tileSize - rx);
        } else {
            // SlopeBR: 左低右高。相對高度 = rx
            return tileBottom + rx;
        }
    }
} // namespace

void CollisionSystem::ResolveCharacterHazards(Character& character, const LevelManager& levelManager) const {
    const glm::vec2 position = character.GetPosition();
    const float tileSize = levelManager.GetTileSize();

    const GridCoord footTile = WorldToTile({position.x, position.y - 1.0F}, tileSize);

    const auto& levelData = levelManager.GetLevelData();
    const bool shouldRespawn =
        levelManager.IsHazardousFor(character.GetElement(), footTile.row,
                                    footTile.col) &&
        ((character.GetElement() == Element::FIRE && levelData.hasFireSpawn) ||
         (character.GetElement() == Element::WATER && levelData.hasWaterSpawn));

    if (!shouldRespawn) {return;}

    const GridCoord spawn = character.GetElement() == Element::FIRE ? levelData.fireSpawn : levelData.waterSpawn;

    character.SetPosition(levelManager.TileToWorldPosition(spawn.row, spawn.col));
    character.SetVelocity({0.0F, 0.0F});
    character.SetGroundState(GroundState::AIR);
}

// void CollisionSystem::ResolveCharacterTerrain(
//     Character& character, const LevelManager& levelManager) const {
//     glm::vec2 position = character.GetPosition();
//     glm::vec2 velocity = character.GetVelocity();
//     const glm::vec2 size = character.GetCollisionSize();
//     const float tileSize = levelManager.GetTileSize();
//     const float halfWidth = size.x * 0.5F;
//     const float halfHeight = size.y * 0.5F;
//
//     if (velocity.x > 0.0F) {
//         const GridCoord topRight =
//             WorldToTile({position.x + halfWidth, position.y + halfHeight - 1.0F},
//                         tileSize);
//         const GridCoord bottomRight =
//             WorldToTile({position.x + halfWidth, position.y - halfHeight + 1.0F},
//                         tileSize);
//
//         if (levelManager.IsWalkable(topRight.row, topRight.col) ||
//             levelManager.IsWalkable(bottomRight.row, bottomRight.col)) {
//             const GridCoord hitTile =
//                 levelManager.IsWalkable(topRight.row, topRight.col)
//                     ? topRight : bottomRight;
//             const glm::vec2 tileCenter = levelManager.TileToWorldPosition(hitTile.row, hitTile.col);
//             position.x = tileCenter.x - tileSize * 0.5F - halfWidth;
//             velocity.x = 0.0F;
//         }
//     } else if (velocity.x < 0.0F) {
//         const GridCoord topLeft =
//             WorldToTile({position.x - halfWidth, position.y + halfHeight - 1.0F},
//                         tileSize);
//         const GridCoord bottomLeft =
//             WorldToTile({position.x - halfWidth, position.y - halfHeight + 1.0F},
//                         tileSize);
//
//         if (levelManager.IsWalkable(topLeft.row, topLeft.col) ||
//             levelManager.IsWalkable(bottomLeft.row, bottomLeft.col)) {
//             const GridCoord hitTile =
//                 levelManager.IsWalkable(topLeft.row, topLeft.col)
//                     ? topLeft
//                     : bottomLeft;
//             const glm::vec2 tileCenter =
//                 levelManager.TileToWorldPosition(hitTile.row, hitTile.col);
//             position.x = tileCenter.x + tileSize * 0.5F + halfWidth;
//             velocity.x = 0.0F;
//         }
//     }
//
//     if (velocity.y > 0.0F) {
//         const GridCoord topLeft =
//             WorldToTile({position.x - halfWidth + 1.0F, position.y + halfHeight},
//                         tileSize);
//         const GridCoord topRight =
//             WorldToTile({position.x + halfWidth - 1.0F, position.y + halfHeight},
//                         tileSize);
//
//         if (levelManager.IsWalkable(topLeft.row, topLeft.col) ||
//             levelManager.IsWalkable(topRight.row, topRight.col)) {
//             const GridCoord hitTile =
//                 levelManager.IsWalkable(topLeft.row, topLeft.col)
//                     ? topLeft
//                     : topRight;
//             const glm::vec2 tileCenter =
//                 levelManager.TileToWorldPosition(hitTile.row, hitTile.col);
//             position.y = tileCenter.y - tileSize * 0.5F - halfHeight;
//             velocity.y = 0.0F;
//         }
//     }
//
//     character.SetGroundState(GroundState::AIR);
//     if (velocity.y <= 0.0F) {
//         const GridCoord bottomLeft =
//             WorldToTile({position.x - halfWidth + 1.0F, position.y - halfHeight},
//                         tileSize);
//         const GridCoord bottomRight =
//             WorldToTile({position.x + halfWidth - 1.0F, position.y - halfHeight},
//                         tileSize);
//
//         if (levelManager.IsWalkable(bottomLeft.row, bottomLeft.col) ||
//             levelManager.IsWalkable(bottomRight.row, bottomRight.col)) {
//             const GridCoord hitTile =
//                 levelManager.IsWalkable(bottomLeft.row, bottomLeft.col)
//                     ? bottomLeft
//                     : bottomRight;
//             const glm::vec2 tileCenter =
//                 levelManager.TileToWorldPosition(hitTile.row, hitTile.col);
//             position.y = tileCenter.y + tileSize * 0.5F + halfHeight;
//             velocity.y = 0.0F;
//             character.SetGroundState(GroundState::GROUND);
//         }
//     }
//
//     character.SetPosition(position);
//     character.SetVelocity(velocity);
//     ResolveCharacterHazards(character, levelManager);
// }

void CollisionSystem::ResolveCharacterTerrain(
    Character& character, const LevelManager& levelManager) const {
    glm::vec2 position = character.GetPosition();
    glm::vec2 velocity = character.GetVelocity();
    const glm::vec2 size = character.GetCollisionSize();
    const float tileSize = levelManager.GetTileSize();
    const float halfWidth = size.x * 0.5F;

    // 🌟 輔助 Lambda：判斷是否為「平坦的牆壁」(無視斜坡)
    auto IsSolidWall = [&](const GridCoord& coord) {
        TerrainType t = levelManager.GetTerrain(coord.row, coord.col);
        return t == TerrainType::Block;
    };

    auto IsCeiling = [&](const GridCoord& coord) {
        TerrainType t = levelManager.GetTerrain(coord.row, coord.col);
        return t != TerrainType::Empty;
    };

    // === 1. 水平碰撞 ===
    if (velocity.x > 0.0F) {
        const GridCoord topRight = WorldToTile({position.x + halfWidth, position.y + size.y - 1.0F}, tileSize);
        const GridCoord bottomRight = WorldToTile({position.x + halfWidth, position.y + 1.0F}, tileSize);

        if (IsSolidWall(topRight) || IsSolidWall(bottomRight)) {
            const GridCoord hitTile = IsSolidWall(topRight) ? topRight : bottomRight;
            const glm::vec2 tileCenter = levelManager.TileToWorldPosition(hitTile.row, hitTile.col);
            position.x = tileCenter.x - tileSize * 0.5F - halfWidth;
            velocity.x = 0.0F;
        }
    } else if (velocity.x < 0.0F) {
        const GridCoord topLeft = WorldToTile({position.x - halfWidth, position.y + size.y - 1.0F}, tileSize);
        const GridCoord bottomLeft = WorldToTile({position.x - halfWidth, position.y + 1.0F}, tileSize);

        if (IsSolidWall(topLeft) || IsSolidWall(bottomLeft)) {
            const GridCoord hitTile = IsSolidWall(topLeft) ? topLeft : bottomLeft;
            const glm::vec2 tileCenter = levelManager.TileToWorldPosition(hitTile.row, hitTile.col);
            position.x = tileCenter.x + tileSize * 0.5F + halfWidth;
            velocity.x = 0.0F;
        }
    }

    // === 2. 向上碰撞 (撞天花板) ===
    if (velocity.y > 0.0F) {
        const GridCoord topLeft = WorldToTile({position.x - halfWidth + 1.0F, position.y + size.y}, tileSize);
        const GridCoord topRight = WorldToTile({position.x + halfWidth - 1.0F, position.y + size.y}, tileSize);

        if (IsCeiling(topLeft) || IsCeiling(topRight)) {
            const GridCoord hitTile = IsSolidWall(topLeft) ? topLeft : topRight;
            const glm::vec2 tileCenter = levelManager.TileToWorldPosition(hitTile.row, hitTile.col);

            // 🌟 修正：撞到天花板後，腳底位置 = 天花板下緣「再往下扣掉整個身高」
            position.y = tileCenter.y - tileSize * 0.5F - size.y;
            velocity.y = 0.0F;
        }
    }

    character.SetGroundState(GroundState::AIR);
    bool isOnSlope = false;

    // === 3. 🌟 修正後的斜坡踩踏與爬坡檢查 ===
    // 腳底座標 + 2.0F，往上微探破解 1 像素死角
    const GridCoord footCenterTile = WorldToTile({position.x, position.y + 2.0F}, tileSize);
    const TerrainType footTerrain = levelManager.GetTerrain(footCenterTile.row, footCenterTile.col);

    if (IsSlope(footTerrain)) {
        const glm::vec2 tileCenter = levelManager.TileToWorldPosition(footCenterTile.row, footCenterTile.col);
        const float surfaceY = CalculateSlopeSurfaceY(footTerrain, tileCenter, tileSize, position.x);

        if (velocity.y <= 0.0F) {
            if ((position.y) <= surfaceY + 10.0F) {
                position.y = surfaceY;
                velocity.y = 0.0F;
                character.SetGroundState(GroundState::GROUND);
                isOnSlope = true;
            }
        }
    }

    // === 4. 一般平地踩踏檢查 ===
    if (!isOnSlope && velocity.y <= 0.0F) {
        const GridCoord bottomLeft = WorldToTile({position.x - halfWidth + 1.0F, position.y}, tileSize);
        const GridCoord bottomRight = WorldToTile({position.x + halfWidth - 1.0F, position.y}, tileSize);

        if (levelManager.IsWalkable(bottomLeft.row, bottomLeft.col) ||
            levelManager.IsWalkable(bottomRight.row, bottomRight.col)) {

            const GridCoord hitTile = levelManager.IsWalkable(bottomLeft.row, bottomLeft.col) ? bottomLeft : bottomRight;

            if (!IsSlope(levelManager.GetTerrain(hitTile.row, hitTile.col))) {
                const glm::vec2 tileCenter = levelManager.TileToWorldPosition(hitTile.row, hitTile.col);

                // 🌟 第二個魔法：踩到平地，腳底直接等於地磚的上緣！
                position.y = tileCenter.y + tileSize * 0.5F;
                velocity.y = 0.0F;
                character.SetGroundState(GroundState::GROUND);
            }
        }
    }

    character.SetPosition(position);
    character.SetVelocity(velocity);

    ResolveCharacterHazards(character, levelManager);
}

void CollisionSystem::ResolveCharacterMechanics(
    Character& character,
    const std::vector<std::shared_ptr<BaseMechanism>>& mechanisms) const {
    
    glm::vec2 pos = character.GetPosition(); // 🌟 記住：現在這是腳底！
    glm::vec2 vel = character.GetVelocity();
    glm::vec2 size = character.GetCollisionSize(); // 拿取完整的尺寸

    // 🌟 先算出角色的物理中心點 (X 軸不變，Y 軸往上推半個身高)
    glm::vec2 halfChar = size * 0.5f;
    glm::vec2 charCenter = {pos.x, pos.y + halfChar.y};

    for (const auto& mech : mechanisms) {
        auto colliderOpt = mech->GetCollider();
        if (!colliderOpt) continue;

        Collider collider = *colliderOpt;
        glm::vec2 halfColl = collider.size * 0.5f;

        // Check AABB overlap with a small downward tolerance (e.g., 5.0f) to catch platforms moving away
        float downTolerance = 5.0f;

        // 🌟 這裡把原本的 pos 替換成 charCenter 來進行 AABB 檢查
        if (std::abs(charCenter.x - collider.center.x) < (halfChar.x + halfColl.x) &&
            (charCenter.y - collider.center.y) < (halfChar.y + halfColl.y + downTolerance) &&
            (collider.center.y - charCenter.y) < (halfChar.y + halfColl.y)) {

            // 🌟 這裡加上 std::abs 來取得 Y 軸絕對的重疊量，這樣比大小才會準確
            float overlapX = (halfChar.x + halfColl.x) - std::abs(charCenter.x - collider.center.x);
            float overlapY = (halfChar.y + halfColl.y) - std::abs(charCenter.y - collider.center.y);

            // 判斷要推擠 X 軸還是 Y 軸 (取重疊比較小的那一邊)
            if (overlapX < overlapY) {
                // 水平推擠 (側面撞擊)
                if (charCenter.x > collider.center.x) pos.x += overlapX;
                else pos.x -= overlapX;
                vel.x = 0;
            } else {
                if (charCenter.y > collider.center.y) {
                    pos.y = collider.center.y + halfColl.y;
                    vel.y = 0;

                    // 電梯跟隨邏輯 (維持原樣，完美運作)
                    auto elevator = std::dynamic_pointer_cast<Elevator>(mech);
                    if (elevator) {
                        pos += elevator->GetDeltaPosition();
                        character.SetGroundState(GroundState::MOVING_PLATFORM);
                    } else {
                        character.SetGroundState(GroundState::GROUND);
                    }
                } else {
                    pos.y = collider.center.y - halfColl.y - size.y;
                    vel.y = 0;
                }
            }
        }
    }
    character.SetPosition(pos);
    character.SetVelocity(vel);
}
