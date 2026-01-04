
#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <vector>
#include <string>

namespace Modele {

// MapManager: centralise la gestion des ressources liées à la carte (sols, murs)
// - Chargement et stockage des textures de sol (floorTexture) et des tuiles
// (tileTextures).
// - Chargement et stockage des textures de murs (`wallTextures`).
// - Stockage de la matrice de la carte (`floorMatrix`) et API pour y accéder.
// Objectif: déléguer à cette classe toute la logique d'assets/tiles pour
// que `Modele` n'ait plus à manipuler directement les textures ou les ids.
class MapManager {
public:
    MapManager();

    // Standard tile IDs (centralized here so all modules use same values)
    static constexpr int TILE_WALL_TL = 11;
    static constexpr int TILE_WALL_LEFT = 12;
    static constexpr int TILE_WALL_BL = 13;
    static constexpr int TILE_WALL_BOTTOM = 14;
    static constexpr int TILE_WALL_TOP = 15;
    static constexpr int TILE_WALL_TR = 16;
    static constexpr int TILE_WALL_RIGHT = 17;
    static constexpr int TILE_WALL_BR = 18;
    static constexpr int TILE_FLOOR_02 = 21; // floor_02 default id
    static constexpr int TILE_FLOOR_01 = 22; // floor_01 default id

    // Try to load a floor texture from a list of candidate file paths.
    // Returns true on first successful load and sets `floorTexture`.
    // Also keeps backward compatibility by (optionally) registering
    // a default tile id inside `tileTextures`.
    //
    // WHY pass paths by const reference:
    // - avoids copying a potentially long vector of strings (file paths)
    // - makes it explicit the function won't modify the caller's list
    bool loadFloorFromPaths(const std::vector<std::string>& paths);

    // Associate a texture file with a tile id. On success the texture is
    // stored in `tileTextures[id]`. Returns true if the file was loaded.
    //
    // WHY id by value:
    // - int is trivial/cheap to copy
    // WHY path by const reference:
    // - avoids copying potentially long file path string
    bool setTileTexture(int id, const std::string& path);

    // Return pointer to a texture previously registered with `setTileTexture`.
    // Returns nullptr if no texture exists for the id.
    //
    // WHY return const sf::Texture*:
    // - some ids may be missing -> nullptr cleanly signals "not found"
    // - avoids copying sf::Texture (heavy resource object)
    // - pointer remains valid as long as the texture stays in tileTextures
    const sf::Texture* getTileTexture(int id) const;

    // Access the fallback floor texture loaded by `loadFloorFromPaths`.
    //
    // WHY return const reference:
    // - avoids copying texture
    // - ensures external code cannot mutate internal resource accidentally
    const sf::Texture& getFloorTexture() const;

    // Accessors for the floor matrix (tile id grid) used by renderers.
    // The matrix contains integer tile ids which reference textures.
    //
    // WHY return const reference:
    // - avoids copying potentially large matrices
    // - preserves encapsulation (read-only view)
    const std::vector<std::vector<int>>& getFloorMatrix() const;

    // Replace the internal floor matrix with `m` (copy).
    //
    // WHY parameter by const reference:
    // - avoids copying the matrix on entry
    // - copy occurs only once when assigning to floorMatrix
    void setFloorMatrix(const std::vector<std::vector<int>>& m);

    // Tile size in pixels used by renderers when drawing the grid.
    int getTileSize() const { return tileSize; }
    void setTileSize(int s) { tileSize = s; } // WHY by value: int is trivial

    // Load wall textures. Accepts a list of primary paths; each wall index
    // will try its primary path first and fall back to embedded assets.
    //
    // WHY pass by const reference:
    // - avoids copying the list of candidate paths
    bool loadWallTextures(const std::vector<std::string>& tryPathsWall);

    // Access to loaded wall textures (order corresponds to wall tile codes)
    //
    // WHY return const reference:
    // - avoids copying the vector of textures
    // - read-only access from outside (ownership stays inside MapManager)
    const std::vector<sf::Texture>& getWallTextures() const { return wallTextures; }

    // Convenience: load a set of default floor/wall textures using the
    // provided candidate paths. Registers common floor tile IDs too.
    //
    // WHY pass vectors by const reference:
    // - avoids copying lists of paths
    bool loadDefaults(const std::vector<std::string>& floorPaths,
                      const std::vector<std::string>& floor02Paths,
                      const std::vector<std::string>& tryPathsWall);

    // Load a map JSON file containing a `matrix` field (array of arrays of
    // integers). The matrix is stored in `floorMatrix` and can be read by
    // renderers to draw tiles by id.
    //
    // WHY path by const reference:
    // - avoid copying file path string
    bool loadMapFromFile(const std::string& path);

    // Clear loaded matrix and reset map-related state (does not free
    // textures cached in `tileTextures`).
    //
    // WHY keep textures:
    // - textures are expensive to load; keeping them cached allows fast map reloads
    void clearMap();

private:
    // WHY store textures by value:
    // - MapManager owns resource objects (RAII)
    // - lifetime is managed by the manager; no external delete/free
    sf::Texture floorTexture;

    // WHY std::map<int, sf::Texture>:
    // - direct association from tile id -> texture
    // - stable addresses for textures (used by getTileTexture pointer return)
    std::map<int, sf::Texture> tileTextures;

    // WHY matrix stored as vector<vector<int>>:
    // - represents a grid of tile ids
    // - easy to serialize/deserialize from JSON
    std::vector<std::vector<int>> floorMatrix;

    int tileSize = 64;

    // Wall textures stored by value (owned by manager).
    std::vector<sf::Texture> wallTextures;
};

} // namespace Modele
