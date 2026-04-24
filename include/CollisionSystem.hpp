#ifndef COLLISION_SYSTEM_HPP
#define COLLISION_SYSTEM_HPP

#include "Character.hpp"
#include "Level/LevelManager.hpp"
#include "Mechanics/BaseMechanism.hpp"

class CollisionSystem {
public:
    void ResolveCharacterTerrain(Character& character,
                                 const LevelManager& levelManager) const;

    void ResolveCharacterMechanics(Character& character,
                                    const std::vector<std::shared_ptr<BaseMechanism>>& mechanisms) const;

    bool CheckOverlap(const glm::vec2& center1, const glm::vec2& size1, 
                      const glm::vec2& center2, const glm::vec2& size2) const;

private:
    void ResolveCharacterHazards(Character& character,
                                 const LevelManager& levelManager) const;
};

#endif
