#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <vector>
#include <string>

namespace Modele {

class MapManager {
public:
    MapManager();

    bool loadFloorFromPaths(const std::vector<std::string>& paths);
    bool setTileTexture(int id, const std::string& path);
    const sf::Texture* getTileTexture(int id) const;
    const sf::Texture& getFloorTexture() const;

    const std::vector<std::vector<int>>& getFloorMatrix() const;
    void setFloorMatrix(const std::vector<std::vector<int>>& m);

    int getTileSize() const { return tileSize; }
    void setTileSize(int s) { tileSize = s; }
    bool loadWallTextures(const std::vector<std::string>& tryPathsWall);
    const std::vector<sf::Texture>& getWallTextures() const { return wallTextures; }
    bool loadDefaults(const std::vector<std::string>& floorPaths,
                      const std::vector<std::string>& floor02Paths,
                      const std::vector<std::string>& tryPathsWall);

private:
    sf::Texture floorTexture;
    std::map<int, sf::Texture> tileTextures;
    std::vector<std::vector<int>> floorMatrix;
    int tileSize = 64;
    std::vector<sf::Texture> wallTextures;
};

} // namespace Modele
