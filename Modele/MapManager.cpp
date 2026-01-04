
#include "MapManager.h"
#include <iostream>
#include <fstream>
#include "../cmake-build-debug/json.hpp"

namespace Modele {

MapManager::MapManager() = default;

// loadFloorFromPaths:
// - Iterates candidate paths and attempts to load the first valid texture file.
// - On success sets `floorTexture` and (for backward compatibility) may
// register a default tile id in `tileTextures` so older code can still
// refer to a default floor tile.
bool MapManager::loadFloorFromPaths(const std::vector<std::string>& paths)
{
    // WHY const std::vector<std::string>&:
    // - avoids copying the entire list of candidate paths
    for (const auto& p : paths) {
        // WHY const auto&:
        // - avoids copying each path string while iterating
        if (floorTexture.loadFromFile(p)) {
            std::cout << "[DEBUG] MapManager: floor texture loaded: " << p << std::endl;

            // keep backward compatibility: store default floor as id 1
            // WHY store a copy into tileTextures[1]:
            // - legacy code may expect a default floor tile under id 1
            // - provides a consistent mapping even if newer ids (TILE_FLOOR_01/02) are used elsewhere
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
    // WHY id by value:
    // - int is trivial
    // WHY path by const reference:
    // - avoid copying potentially long path strings
    try {
        bool ok = tileTextures[id].loadFromFile(path);

        // If loading fails, erase the entry to avoid keeping an "empty" texture in the map.
        // WHY erase on failure:
        // - keeps tileTextures consistent: only valid textures remain registered
        if (!ok) tileTextures.erase(id);

        return ok;
    } catch (...) {
        return false;
    }
}

// clearMap: clears the floor matrix only (keeps loaded textures intact).
void MapManager::clearMap()
{
    // WHY only clear matrix:
    // - map layout changes frequently, textures do not
    // - keeps cached textures for faster reloads and reduced disk I/O
    floorMatrix.clear();
}

const sf::Texture* MapManager::getTileTexture(int id) const
{
    auto it = tileTextures.find(id);
    if (it != tileTextures.end()) return &it->second;

    // nullptr indicates "no texture registered for this id"
    return nullptr;
}

// getFloorTexture: returns the loaded fallback floor texture.
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
    // WHY const reference parameter:
    // - avoid copying on function entry, copy occurs once here via assignment
    floorMatrix = m;
}

bool MapManager::loadWallTextures(const std::vector<std::string>& tryPathsWall)
{
    // loadWallTextures:
    // - For each of the 8 wall variants, try the provided primary path first
    // then fall back to a built-in asset path `Asset/Wall/Wall1_N.png`.
    // - The resulting textures are stored in `wallTextures` in index order
    // corresponding to the wall tile ids used by the map matrix.
    wallTextures.clear();
    wallTextures.resize(8);

    for (size_t i = 0; i < 8; ++i)
    {
        std::string primary = (i < tryPathsWall.size()) ? tryPathsWall[i] : std::string();
        std::string assetPath = "Asset/Wall/Wall1_" + std::to_string(i+1) + ".png";

        bool loaded = false;

        if (!primary.empty()) {
            if (wallTextures[i].loadFromFile(primary)) {
                loaded = true;
                std::cout << "[DEBUG] MapManager: loaded wall texture: " << primary << std::endl;
            } else {
                std::cout << "[DEBUG] MapManager: failed to load wall texture: " << primary << std::endl;
            }
        }

        if (!loaded) {
            if (wallTextures[i].loadFromFile(assetPath)) {
                loaded = true;
                std::cout << "[DEBUG] MapManager: loaded wall texture: " << assetPath << std::endl;
            } else {
                std::cout << "[DEBUG] MapManager: failed to load wall texture: " << assetPath << std::endl;
            }
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
    // loadDefaults: convenience routine to populate common floor and wall
    // textures. `floorPaths`/`floor02Paths` are tried in order; on first
    // success the tile id constants `TILE_FLOOR_01` / `TILE_FLOOR_02` are
    // registered in `tileTextures` to allow map files to reference them.
    bool ok = loadFloorFromPaths(floorPaths);

    // Register floor_01 under TILE_FLOOR_01 using the first path that loads.
    // WHY loop and break:
    // - supports multiple working directories; picks first valid asset path
    for (const auto& p : floorPaths) {
        if (setTileTexture(TILE_FLOOR_01, p)) {
            std::cout << "[DEBUG] MapManager: set tile " << TILE_FLOOR_01 << " -> " << p << std::endl;
            break;
        }
    }

    // Register floor_02 under TILE_FLOOR_02 using the first path that loads.
    for (const auto& p : floor02Paths) {
        if (setTileTexture(TILE_FLOOR_02, p)) {
            std::cout << "[DEBUG] MapManager: set tile " << TILE_FLOOR_02 << " -> " << p << std::endl;
            break;
        }
    }

    // load walls
    loadWallTextures(tryPathsWall);

    return ok;
}

// loadMapFromFile:
// - Opens a JSON file at `path` and expects a top-level `matrix` field which
// is an array of rows (each row is an array of integers tile ids).
// - The parsed matrix is stored into `floorMatrix` and can later be used by
// the renderer to draw tiles by id. Returns true on successful parse.
bool MapManager::loadMapFromFile(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "[DEBUG] MapManager: cannot open map file: " << path << std::endl;
        return false;
    }

    try {
        nlohmann::json j;
        f >> j;

        if (j.contains("matrix") && j["matrix"].is_array()) {
            std::vector<std::vector<int>> m;

            for (const auto& row : j["matrix"]) {
                if (!row.is_array()) continue;

                std::vector<int> rvec;
                for (const auto& v : row) {
                    rvec.push_back(v.get<int>());
                }

                // WHY std::move(rvec):
                // - avoids copying the row vector into m; transfers its internal buffer instead
                m.push_back(std::move(rvec));
            }

            setFloorMatrix(m);
            std::cout << "[DEBUG] MapManager: loaded map matrix from " << path << std::endl;
            return true;
        } else {
            std::cerr << "[DEBUG] MapManager: map file missing 'matrix' array: " << path << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "[DEBUG] MapManager: error parsing map file " << path << " : " << e.what() << std::endl;
        return false;
    }
}

} // namespace Modele
