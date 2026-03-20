#include "LevelManager.hpp"
#include "AtlasSprite.hpp"
#include "config.hpp" // For WINDOW_WIDTH / WINDOW_HEIGHT

LevelManager::LevelManager(std::shared_ptr<SpriteAtlas> atlas)
    : m_Atlas(std::move(atlas)) {}

void LevelManager::LoadLevel(const std::vector<std::string>& mapData, const std::shared_ptr<Util::GameObject>& root) {
    if (mapData.empty()) return;

    // A simple mapping layout. Make tiles 32x32 pixels.
    const float tileSize = 32.0f;
    
    // Calculate the total map size to center it on screen or place it relative to a corner.
    const int rows = static_cast<int>(mapData.size());
    const int cols = static_cast<int>(mapData[0].size());
    
    // We can start laying out from the top-left corner
    // The screen size is 1280x720 (assuming WINDOW_WIDTH=1280), so center is (0,0). Top-left is (-640, 360).
    const float startX = -(static_cast<float>(WINDOW_WIDTH) / 2.0f) + (tileSize / 2.0f);
    const float startY = (static_cast<float>(WINDOW_HEIGHT) / 2.0f) - (tileSize / 2.0f);

    constexpr float kWallZ = -1.0F; // Place walls above backgrounds

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            if (static_cast<size_t>(x) >= mapData[y].size()) continue;
            
            char tileChar = mapData[y][x];
            
            std::string frameName = "";
            switch (tileChar) {
                case 'X': frameName = "BlackBox0000"; break; // Terrain block visual
                // Add more types later (Ice, Snow, Water, Lava)
                default: continue; // Empty space '.'
            }

            if (!frameName.empty() && m_Atlas->HasFrame(frameName)) {
                auto sprite = std::make_shared<AtlasSprite>(m_Atlas, frameName);
                auto tileObj = std::make_shared<Util::GameObject>(sprite, kWallZ);
                
                // Set position
                tileObj->m_Transform.translation = {
                    startX + static_cast<float>(x) * tileSize,
                    startY - static_cast<float>(y) * tileSize
                };
                
                // Adjust scale so the tile matches tileSize visually
                const auto actualSize = sprite->GetSize();
                if (actualSize.x > 0 && actualSize.y > 0) {
                    tileObj->m_Transform.scale = {
                        tileSize / actualSize.x,
                        tileSize / actualSize.y
                    };
                }

                root->AddChild(tileObj);
            }
        }
    }
}
