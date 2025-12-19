#include "MapManager.h"
#include <iostream>

namespace Modele {

MapManager::MapManager() = default;

bool MapManager::loadFloorFromPaths(const std::vector<std::string>& paths)
{
    for (const auto& p : paths) {
        if (floorTexture.loadFromFile(p)) {
            std::cout << "[DEBUG] MapManager: floor texture loaded: " << p << std::endl;
            // keep backward compatibility: store default floor as id 1
            try {
                tileTextures[1] = floorTexture;
            } catch (...) {}
            return true;
        } else {
            std::cout << "[DEBUG] MapManager: failed to load floor texture: " << p << std::endl;
        }
    }
    std::cerr << "[DEBUG] MapManager: unable to load any floor texture from given paths" << std::endl;
    return false;
}

bool MapManager::setTileTexture(int id, const std::string& path)
{
    try {
        bool ok = tileTextures[id].loadFromFile(path);
        if (!ok) tileTextures.erase(id);
        return ok;
    } catch (...) {
        return false;
    }
}

const sf::Texture* MapManager::getTileTexture(int id) const
{
    auto it = tileTextures.find(id);
    if (it != tileTextures.end()) return &it->second;
    return nullptr;
}

const sf::Texture& MapManager::getFloorTexture() const
{
    return floorTexture;
}

const std::vector<std::vector<int>>& MapManager::getFloorMatrix() const
{
    return floorMatrix;
}

void MapManager::setFloorMatrix(const std::vector<std::vector<int>>& m)
{
    floorMatrix = m;
}

bool MapManager::loadWallTextures(const std::vector<std::string>& tryPathsWall)
{
    wallTextures.clear();
    wallTextures.resize(8);
    for (size_t i = 0; i < 8; ++i)
    {
        std::string primary = (i < tryPathsWall.size()) ? tryPathsWall[i] : std::string();
        std::string assetPath = "Asset/Wall/Wall1_" + std::to_string(i+1) + ".png";
        bool loaded = false;
        if (!primary.empty()) {
            if (wallTextures[i].loadFromFile(primary)) { loaded = true; std::cout << "[DEBUG] MapManager: loaded wall texture: " << primary << std::endl; }
            else std::cout << "[DEBUG] MapManager: failed to load wall texture: " << primary << std::endl;
        }
        if (!loaded) {
            if (wallTextures[i].loadFromFile(assetPath)) { loaded = true; std::cout << "[DEBUG] MapManager: loaded wall texture: " << assetPath << std::endl; }
            else std::cout << "[DEBUG] MapManager: failed to load wall texture: " << assetPath << std::endl;
        }
        if (!loaded) {
            std::cerr << "[DEBUG] MapManager: warning: unable to load wall texture index " << i << std::endl;
        }
    }
    return true;
}

bool MapManager::loadDefaults(const std::vector<std::string>& floorPaths,
                              const std::vector<std::string>& floor02Paths,
                              const std::vector<std::string>& tryPathsWall)
{
    bool ok = loadFloorFromPaths(floorPaths);
    // register floor_01 as id 22 if possible
    for (const auto& p : floorPaths) {
        if (setTileTexture(22, p)) { std::cout << "[DEBUG] MapManager: set tile 22 -> " << p << std::endl; break; }
    }
    // register floor_02 as id 21
    for (const auto& p : floor02Paths) {
        if (setTileTexture(21, p)) { std::cout << "[DEBUG] MapManager: set tile 21 -> " << p << std::endl; break; }
    }
    // load walls
    loadWallTextures(tryPathsWall);
    return ok;
}

} // namespace Modele
