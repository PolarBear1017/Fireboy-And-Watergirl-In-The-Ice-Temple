#ifndef LEVEL_MANAGER_HPP
#define LEVEL_MANAGER_HPP

#include "SpriteAtlas.hpp"
#include "Util/GameObject.hpp"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

class LevelManager {
public:
    explicit LevelManager(std::shared_ptr<SpriteAtlas> atlas);
    
    // Loads the level and populates the given root node with tiles
    void LoadLevel(const std::vector<std::string>& mapData, const std::shared_ptr<Util::GameObject>& root);

private:
    std::string ResolveFrameName(char tileChar) const;
    float ResolveZIndex(char tileChar) const;
    std::shared_ptr<SpriteAtlas> m_Atlas;
};

#endif
