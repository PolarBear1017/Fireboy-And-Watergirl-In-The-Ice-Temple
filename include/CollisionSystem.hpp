#ifndef COLLISION_SYSTEM_HPP
#define COLLISION_SYSTEM_HPP

#include "Character.hpp"
#include "LevelManager.hpp"
#include "Mechanics/BaseMechanism.hpp"

class CollisionSystem {
public:
    void ResolveCharacterTerrain(Character& character,
                                 const LevelManager& levelManager) const;
    
    void ResolveCharacterMechanics(Character& character,
                                    const std::vector<std::shared_ptr<BaseMechanism>>& mechanisms) const;

private:
    void ResolveCharacterHazards(Character& character,
                                 const LevelManager& levelManager) const;
};

#endif
