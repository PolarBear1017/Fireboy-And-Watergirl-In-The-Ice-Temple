#ifndef COLLISION_SYSTEM_HPP
#define COLLISION_SYSTEM_HPP

#include "Character.hpp"
#include "LevelManager.hpp"

class CollisionSystem {
public:
    void ResolveCharacterTerrain(Character& character,
                                 const LevelManager& levelManager) const;

private:
    void ResolveCharacterHazards(Character& character,
                                 const LevelManager& levelManager) const;
};

#endif
