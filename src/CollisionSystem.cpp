#include "CollisionSystem.hpp"
#include "Mechanics/Elevator.hpp"
#include "Mechanics/Block.hpp"

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
        switch (type) {
            case TerrainType::SlopeBL:
            case TerrainType::SlopeBR:
            case TerrainType::SnowSlopeBL:
            case TerrainType::SnowSlopeBR:
                return true;
            default:
                return false;
        }
    }

    float CalculateSlopeSurfaceY(TerrainType type, const glm::vec2& tileCenter, float tileSize, float playerX) {
        const float tileLeft = tileCenter.x - tileSize * 0.5F;
        const float tileBottom = tileCenter.y - tileSize * 0.5F;

        float rx = playerX - tileLeft; // 計算角色在磚塊內的相對 X 座標 (0 ~ tileSize)
        rx = std::clamp(rx, 0.0F, tileSize); // 防呆，確保不出界

        if (type == TerrainType::SlopeBL || type == TerrainType::SnowSlopeBL) {
            return tileBottom + (tileSize - rx);
        } else {
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

void CollisionSystem::ResolveCharacterTerrain(
    Character& character, const LevelManager& levelManager) const {
    glm::vec2 position = character.GetPosition();
    glm::vec2 velocity = character.GetVelocity();
    const glm::vec2 size = character.GetCollisionSize();
    const float tileSize = levelManager.GetTileSize();
    const float halfWidth = size.x * 0.5F;

    // 雙層斜坡探測器 (解決陷下去的關鍵)
    // 會先查腳底下，如果找不到，會自動往下挖半格查接縫處
    auto GetSlopeAt = [&](float x, float y, TerrainType& outTerrain, GridCoord& outTile) {
        GridCoord c = WorldToTile({x, y}, tileSize);
        TerrainType t = levelManager.GetTerrain(c.row, c.col);
        if (IsSlope(t)) {
            outTerrain = t; outTile = c; return true;
        }
        // 往下深探半個磚塊
        GridCoord cBelow = WorldToTile({x, y - tileSize * 0.5f}, tileSize);
        TerrainType tBelow = levelManager.GetTerrain(cBelow.row, cBelow.col);
        if (IsSlope(tBelow)) {
            outTerrain = tBelow; outTile = cBelow; return true;
        }
        return false;
    };

    // 判斷角色目前是否在斜坡上 (用左腳、中心、右腳三點探測)
    bool onSlope = false;
    TerrainType dummyT; GridCoord dummyC;
    if (GetSlopeAt(position.x, position.y + 2.0F, dummyT, dummyC) ||
        GetSlopeAt(position.x - halfWidth + 2.0F, position.y + 2.0F, dummyT, dummyC) ||
        GetSlopeAt(position.x + halfWidth - 2.0F, position.y + 2.0F, dummyT, dummyC)) {
        onSlope = true;
    }

    // 水平牆壁判定
    // 增加 isBottom 參數，用來判斷現在是不是在測量腳部的碰撞
    auto IsSolidWall = [&](const GridCoord& coord, bool isBottom) {
        TerrainType t = levelManager.GetTerrain(coord.row, coord.col);

        if (t == TerrainType::Empty) return false;
        if (IsSlope(t)) return false;
        // 寬容下半身碰撞感測器
        if (isBottom) {
            if (onSlope) return false;
            TerrainType tileAbove = levelManager.GetTerrain(coord.row - 1, coord.col);
            if (IsSlope(tileAbove)) return false;
        }
        return true;
    };

    auto IsCeiling = [&](const GridCoord& coord) {
        TerrainType t = levelManager.GetTerrain(coord.row, coord.col);
        return t != TerrainType::Empty;
    };

    // === 水平碰撞 ===
    if (velocity.x > 0.0F) {
        // 頂部、底部感測器
        const GridCoord topRight = WorldToTile({position.x + halfWidth, position.y + size.y - 8.0F}, tileSize);
        const GridCoord bottomRight = WorldToTile({position.x + halfWidth, position.y + 12.0F}, tileSize);

        // 🌟 呼叫新工具：topRight 是頭(false)，bottomRight 是腳(true)
        if (IsSolidWall(topRight, false) || IsSolidWall(bottomRight, true)) {
            const GridCoord hitTile = IsSolidWall(topRight, false) ? topRight : bottomRight;
            const glm::vec2 tileCenter = levelManager.TileToWorldPosition(hitTile.row, hitTile.col);
            position.x = tileCenter.x - tileSize * 0.5F - halfWidth;
            velocity.x = 0.0F;
        }
    } else if (velocity.x < 0.0F) {
        const GridCoord topLeft = WorldToTile({position.x - halfWidth, position.y + size.y - 8.0F}, tileSize);
        const GridCoord bottomLeft = WorldToTile({position.x - halfWidth, position.y + 12.0F}, tileSize);

        if (IsSolidWall(topLeft, false) || IsSolidWall(bottomLeft, true)) {
            const GridCoord hitTile = IsSolidWall(topLeft, false) ? topLeft : bottomLeft;
            const glm::vec2 tileCenter = levelManager.TileToWorldPosition(hitTile.row, hitTile.col);
            position.x = tileCenter.x + tileSize * 0.5F + halfWidth;
            velocity.x = 0.0F;
        }
    }

    // === 向上碰撞 (撞天花板) ===
    if (velocity.y > 0.0F) {
        const GridCoord topLeft = WorldToTile({position.x - halfWidth + 1.0F, position.y + size.y}, tileSize);
        const GridCoord topRight = WorldToTile({position.x + halfWidth - 1.0F, position.y + size.y}, tileSize);

        if (IsCeiling(topLeft) || IsCeiling(topRight)) {
            const GridCoord hitTile = IsCeiling(topLeft) ? topLeft : topRight;
            const glm::vec2 tileCenter = levelManager.TileToWorldPosition(hitTile.row, hitTile.col);

            // 撞到天花板後，腳底位置 = 天花板下緣「再往下扣掉整個身高」
            position.y = tileCenter.y - tileSize * 0.5F - size.y;
            velocity.y = 0.0F;
        }
    }

    character.SetGroundState(GroundState::AIR);
    bool currentlyOnSlope = false;

    // === 3. 終極斜坡踩踏與爬坡檢查 ===
    TerrainType slopeTerrain;
    GridCoord slopeTile;

    // 魔法展開：不管左邊、中間、還是右邊，只要抓到斜坡就立刻回傳 true
    if (GetSlopeAt(position.x, position.y + 2.0F, slopeTerrain, slopeTile) ||
        GetSlopeAt(position.x - halfWidth + 2.0F, position.y + 2.0F, slopeTerrain, slopeTile) ||
        GetSlopeAt(position.x + halfWidth - 2.0F, position.y + 2.0F, slopeTerrain, slopeTile)) {

        const glm::vec2 tileCenter = levelManager.TileToWorldPosition(slopeTile.row, slopeTile.col);
        const float surfaceY = CalculateSlopeSurfaceY(slopeTerrain, tileCenter, tileSize, position.x);

        if (velocity.y <= 0.0F) {
            // 磁鐵吸附力加大到 20.0F，確保高速下坡時不會因為慣性而飛離斜坡
            if (position.y <= surfaceY + 20.0F) {
                position.y = surfaceY;
                velocity.y = 0.0F;

                if (slopeTerrain == TerrainType::SnowSlopeBL || slopeTerrain == TerrainType::SnowSlopeBR) {
                    character.SetGroundState(GroundState::ICE);
                } else {
                    character.SetGroundState(GroundState::GROUND);
                }

                currentlyOnSlope = true;
            }
        }
        }

    // === 4. 一般平地踩踏檢查 ===
    // 只有當我們「不在斜坡上」時，才執行平地判定
    if (!currentlyOnSlope && velocity.y <= 0.0F) {
        const GridCoord bottomLeft = WorldToTile({position.x - halfWidth + 1.0F, position.y - 2.0F}, tileSize);
        const GridCoord bottomRight = WorldToTile({position.x + halfWidth - 1.0F, position.y - 2.0F}, tileSize);

        // 🌟 建立一個小巧的檢查器：確認這塊磚可以走，而且絕對不是斜坡
        auto IsWalkableFlat = [&](const GridCoord& c) {
            TerrainType t = levelManager.GetTerrain(c.row, c.col);
            return t != TerrainType::Empty && !IsSlope(t);
        };

        if (IsWalkableFlat(bottomLeft) || IsWalkableFlat(bottomRight)) {
            const GridCoord hitTile = IsWalkableFlat(bottomLeft) ? bottomLeft : bottomRight;
            const glm::vec2 tileCenter = levelManager.TileToWorldPosition(hitTile.row, hitTile.col);

            // 腳底完美貼齊平地
            position.y = tileCenter.y + tileSize * 0.5F;
            velocity.y = 0.0F;

            switch (levelManager.GetTerrain(hitTile.row, hitTile.col)) {
                case TerrainType::SnowBlock:
                case TerrainType::Ice:
                    character.SetGroundState(GroundState::ICE);
                    break;
                default:
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
    const std::vector<std::shared_ptr<BaseMechanism>>& mechanisms,
    const LevelManager& levelManager) const {
    
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
            // 🌟 修正邊緣卡住問題：如果角色的腳底 (pos.y) 非常接近平台頂部 (容錯 4.0f 像素)，
            // 代表他其實是站在上面，這時就算走到邊緣 (overlapX 變得很小)，也必須強制做 Y 軸推擠，避免被橫向推開而卡住。
            bool isStandingOnTop = (pos.y >= collider.center.y + halfColl.y - 4.0f);

            if (!isStandingOnTop && overlapX < overlapY) {
                // 水平推擠 (側面撞擊)
                auto block = std::dynamic_pointer_cast<Block>(mech);
                if (block) {
                    glm::vec2 bVel = block->GetVelocity();
                    glm::vec2 bPos = block->GetPosition();
                    
                    bool charPushesBlock = false;
                    if (charCenter.x > collider.center.x) {
                        if (vel.x < 0) charPushesBlock = true;
                    } else {
                        if (vel.x > 0) charPushesBlock = true;
                    }

                    if (charPushesBlock) {
                        if (charCenter.x > collider.center.x) {
                            bPos.x -= overlapX;
                            bVel.x = vel.x;
                        } else {
                            bPos.x += overlapX;
                            bVel.x = vel.x;
                        }
                        block->SetPosition(bPos);
                        block->SetVelocity(bVel);
                        
                        ResolveBlockTerrain(*block, levelManager);
                        
                        glm::vec2 newBPos = block->GetPosition();
                        if (charCenter.x > collider.center.x) {
                            pos.x = newBPos.x + halfColl.x + halfChar.x;
                        } else {
                            pos.x = newBPos.x - halfColl.x - halfChar.x;
                        }
                        vel.x = block->GetVelocity().x;
                    } else {
                        // Block pushes character
                        if (charCenter.x > collider.center.x) {
                            pos.x += overlapX;
                        } else {
                            pos.x -= overlapX;
                        }
                        
                        character.SetPosition(pos);
                        character.SetVelocity(vel);
                        ResolveCharacterTerrain(character, levelManager);
                        glm::vec2 resolvedPos = character.GetPosition();
                        
                        float pushBack = resolvedPos.x - pos.x;
                        if (std::abs(pushBack) > 0.001f) {
                            bPos.x += pushBack;
                            block->SetPosition(bPos);
                            block->SetVelocity({0, block->GetVelocity().y});
                            
                            // Adjust character one more time to perfectly touch the block
                            if (charCenter.x > collider.center.x) {
                                resolvedPos.x = bPos.x + halfColl.x + halfChar.x;
                            } else {
                                resolvedPos.x = bPos.x - halfColl.x - halfChar.x;
                            }
                        }
                        pos = resolvedPos;
                        vel = character.GetVelocity();
                    }
                } else {
                    if (charCenter.x > collider.center.x) pos.x += overlapX;
                    else pos.x -= overlapX;
                    vel.x = 0;
                }
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

bool CollisionSystem::CheckOverlap(const glm::vec2& center1, const glm::vec2& size1, 
                                   const glm::vec2& center2, const glm::vec2& size2) const {
    float left1 = center1.x - size1.x * 0.5f;
    float right1 = center1.x + size1.x * 0.5f;
    float bottom1 = center1.y - size1.y * 0.5f;
    float top1 = center1.y + size1.y * 0.5f;

    float left2 = center2.x - size2.x * 0.5f;
    float right2 = center2.x + size2.x * 0.5f;
    float bottom2 = center2.y - size2.y * 0.5f;
    float top2 = center2.y + size2.y * 0.5f;

    return !(left1 > right2 || right1 < left2 || bottom1 > top2 || top1 < bottom2);
}

void CollisionSystem::ResolveBlockTerrain(Block& block, const LevelManager& levelManager) const {
    glm::vec2 position = block.GetPosition();
    glm::vec2 velocity = block.GetVelocity();
    const glm::vec2 size = block.GetSize();
    const float tileSize = levelManager.GetTileSize();
    const float halfWidth = size.x * 0.5F;
    const float halfHeight = size.y * 0.5F;

    auto GetSlopeAt = [&](float x, float y, TerrainType& outTerrain, GridCoord& outTile) {
        GridCoord c = WorldToTile({x, y}, tileSize);
        TerrainType t = levelManager.GetTerrain(c.row, c.col);
        if (IsSlope(t)) {
            outTerrain = t; outTile = c; return true;
        }
        GridCoord cBelow = WorldToTile({x, y - tileSize * 0.5f}, tileSize);
        TerrainType tBelow = levelManager.GetTerrain(cBelow.row, cBelow.col);
        if (IsSlope(tBelow)) {
            outTerrain = tBelow; outTile = cBelow; return true;
        }
        return false;
    };

    bool currentlyOnSlope = false;
    TerrainType slopeTerrain;
    GridCoord slopeTile;

    if (GetSlopeAt(position.x, position.y + 2.0F, slopeTerrain, slopeTile) ||
        GetSlopeAt(position.x - halfWidth + 2.0F, position.y + 2.0F, slopeTerrain, slopeTile) ||
        GetSlopeAt(position.x + halfWidth - 2.0F, position.y + 2.0F, slopeTerrain, slopeTile)) {
        
        currentlyOnSlope = true;
        const glm::vec2 tileCenter = levelManager.TileToWorldPosition(slopeTile.row, slopeTile.col);
        const float surfaceY = CalculateSlopeSurfaceY(slopeTerrain, tileCenter, tileSize, position.x);

        if (velocity.y <= 0.0F) {
            if (position.y - halfHeight <= surfaceY + 20.0F) {
                position.y = surfaceY + halfHeight;
                velocity.y = 0.0F;
                
                // Slight slide down slope
                if (slopeTerrain == TerrainType::SlopeBL) {
                    velocity.x -= 2.0f;
                } else if (slopeTerrain == TerrainType::SlopeBR) {
                    velocity.x += 2.0f;
                }
            }
        }
    }

    auto IsSolidWall = [&](const GridCoord& coord) {
        TerrainType t = levelManager.GetTerrain(coord.row, coord.col);
        if (t == TerrainType::Block || t == TerrainType::Ice || t == TerrainType::SnowBlock) return true;

        if (IsSlope(t)) {
            // Only allow passing through the slope we are currently standing on
            if (currentlyOnSlope && coord.row == slopeTile.row && coord.col == slopeTile.col) {
                return false;
            }
            return true;
        }
        return false;
    };

    if (velocity.x > 0.0F) {
        const GridCoord rightTop = WorldToTile({position.x + halfWidth, position.y + halfHeight - 1.0F}, tileSize);
        const GridCoord rightBottom = WorldToTile({position.x + halfWidth, position.y - halfHeight + 12.0F}, tileSize);
        if (IsSolidWall(rightTop) || IsSolidWall(rightBottom)) {
            const GridCoord hitTile = IsSolidWall(rightTop) ? rightTop : rightBottom;
            const glm::vec2 tileCenter = levelManager.TileToWorldPosition(hitTile.row, hitTile.col);
            position.x = tileCenter.x - tileSize * 0.5F - halfWidth;
            velocity.x = 0.0F;
        }
    } else if (velocity.x < 0.0F) {
        const GridCoord leftTop = WorldToTile({position.x - halfWidth, position.y + halfHeight - 1.0F}, tileSize);
        const GridCoord leftBottom = WorldToTile({position.x - halfWidth, position.y - halfHeight + 12.0F}, tileSize);
        if (IsSolidWall(leftTop) || IsSolidWall(leftBottom)) {
            const GridCoord hitTile = IsSolidWall(leftTop) ? leftTop : leftBottom;
            const glm::vec2 tileCenter = levelManager.TileToWorldPosition(hitTile.row, hitTile.col);
            position.x = tileCenter.x + tileSize * 0.5F + halfWidth;
            velocity.x = 0.0F;
        }
    }

    if (!currentlyOnSlope && velocity.y <= 0.0F) {
        const GridCoord bottomLeft = WorldToTile({position.x - halfWidth + 1.0F, position.y - halfHeight - 2.0F}, tileSize);
        const GridCoord bottomRight = WorldToTile({position.x + halfWidth - 1.0F, position.y - halfHeight - 2.0F}, tileSize);

        auto IsWalkableFlat = [&](const GridCoord& c) {
            TerrainType t = levelManager.GetTerrain(c.row, c.col);
            return t != TerrainType::Empty && !IsSlope(t);
        };

        if (IsWalkableFlat(bottomLeft) || IsWalkableFlat(bottomRight)) {
            const GridCoord hitTile = IsWalkableFlat(bottomLeft) ? bottomLeft : bottomRight;
            const glm::vec2 tileCenter = levelManager.TileToWorldPosition(hitTile.row, hitTile.col);
            position.y = tileCenter.y + tileSize * 0.5F + halfHeight;
            velocity.y = 0.0F;
        }
    }

    block.SetPosition(position);
    block.SetVelocity(velocity);
}
