
// Modele.h
//
// Central "Model" class of the game.
// It aggregates core game state and systems:
// - player hitbox (RectangleShape) + visual sprite/animation
// - RoomManager (rooms/doors/enemy definitions/objectives)
// - MapManager (tile/floor matrix + textures)
// - Level (lives/game over)
// - runtime enemies + enemy prototypes (Prototype + Factory pattern)
//
// DESIGN / RATIONALE
// - This class acts as an orchestrator: it owns the main subsystems through unique_ptr
// (RoomManager, MapManager, Level) to express clear ownership and lifetime.
// - Enemies are stored as std::unique_ptr<Enemy> to keep polymorphism + RAII.
// - Enemy instantiation is data-driven: RoomManager loads EnemyDefinition from JSON,
// then Modele converts definitions to runtime objects via createEnemyFromDefinition().
//
// OWNERSHIP
// - unique_ptr expresses exclusive ownership (single owner, automatic destruction).
// - Functions returning const references avoid copies and prevent external mutation.
//
// NOTE ABOUT PLAYER REPRESENTATION
// - Player uses a RectangleShape as collision/hitbox and a Sprite as visual.
// This decoupling is common: collisions use simple shapes; visuals can be scaled/cropped.
//
// NOTE ABOUT ANIMATION
// - The player sprite uses a sprite-sheet with movement frames + idle frames,
// and a separate idle row region (playerRow + 4).

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <set>
#include "Objective.h"
#include "ScoreCalculator.h"
#include "Level.h"
#include "../cmake-build-debug/json.hpp"
#include "Enemy.h"
#include "RoomManager.h"
#include "MapManager.h"

namespace Modele {

class Modele {
private:
    // -------------------------
    // Player: collision vs visual
    // -------------------------
    sf::RectangleShape joueur; // collision/hitbox representation

    // Sprite-sheet resources for player visuals
    sf::Texture playerTexture;
    sf::Sprite playerSprite;

    // Animation state (movement + idle)
    int playerFrameCount = 9;
    int playerFrameIndex = 0;
    int playerRow = 3; // 1=up,2=left,3=down,4=right (default: down)
    float playerFrameDuration = 0.08f;
    float playerIdleFrameDuration = 0.24f;
    sf::Clock playerClock;
    int playerTileSize = 64;

    // Number of idle frames appended after movement frames in each row.
    int idleFrameCount = 2;

    // Whether the player is currently moving (drives which rows are used).
    bool playerIsMoving = false;

    // Crop/zoom settings for texture rect and display scaling.
    float playerTextureZoom = 1.20f;
    float playerSpriteDisplayScaleX = 1.0f;
    float playerSpriteDisplayScaleY = 1.0f;

    // Compute current sprite-sheet rectangle (includes zoom/crop logic).
    // WHY const method:
    // - pure computation from internal state
    // - does not modify the model; safe to call from rendering code
    sf::IntRect computePlayerTextureRect() const;

    // Subsystems owned by the model (unique ownership).
    // WHY unique_ptr:
    // - expresses a single owner (Modele) for these subsystems
    // - automatic destruction when Modele resets / is destroyed (RAII)
    std::unique_ptr<MapManager> mapManager;
    std::unique_ptr<RoomManager> roomManager;
    std::unique_ptr<Level> currentLevel;

    // Global state flags/counters.
    bool collisionDetectee = false;
    bool joueurDetecte = false;
    int detectionCount = 0;

    // Current level path used to reload/reset.
    std::string currentLevelPath;

    // Objective contact (pointer references an object owned by Room/Modele containers).
    // Rationale: pointer allows modifying the original objective (no copy).
    //
    // WHY raw pointer here:
    // - we want a "non-owning" link to an Objective stored in a room's vector
    // - allows in-place modification (accomplished/dialogue flags) without copying
    // - nullptr expresses "no current contact"
    const float DOOR_SIZE = 120.f;
    const float DOOR_THICKNESS = 100.f;

    // Collision with objective
    Objective* objectiveContact = nullptr;
    bool objectiveContactDetectee = false;

    // Dialogue triggered flag (keeps state about whether a dialogue was triggered)
    bool dialogueTriggeredFlag = false;

    // Tracks which room indices have had their room-level dialogue shown
    // This set persists for the duration of the current level/session
    std::set<int> shownRoomDialogues;

    // Score memory (non persistent).
    std::vector<int> playerScores = std::vector<int>(12, 0);

    // Currently loaded mission index (maps to playerScores slots).
    // 0 = tutorial, 1 = test, 2.. = main missions.
    int currentMissionIndex = 0;

public:
    Modele();
    ~Modele() = default;

    void reset();

    // Player access:
    // Returns non-const reference because other systems (controller) may move the hitbox.
    //
    // WHY return non-const reference:
    // - controller needs to change player position/size directly
    // - avoids copying SFML shapes (which could be expensive and would desync state)
    sf::RectangleShape& getJoueur() { return joueur; }

    // Map queries delegate to MapManager.
    //
    // WHY const reference returns:
    // - avoids copying large matrices/vectors
    // - enforces read-only access from outside
    const std::vector<std::vector<int>>& getFloorMatrix() const;
    const sf::Texture& getFloorTexture() const;

    // WHY const reference parameter:
    // - avoids copying the matrix when setting it
    void setFloorMatrix(const std::vector<std::vector<int>>& m);

    // WHY const std::string&:
    // - avoid copying a potentially long path string
    bool setTileTexture(int id, const std::string& path);

    // WHY return pointer (const sf::Texture*):
    // - texture may be absent for an id, so nullptr is a clear "not found" signal
    // - avoids copying sf::Texture
    const sf::Texture* getTileTexture(int id) const;

    int getTileSize() const;
    const std::vector<sf::Texture>& getWallTextures() const;

    // Obstacles/doors from current room.
    //
    // WHY const reference returns:
    // - obstacles are stored as unique_ptr; returning by value is not possible (move-only)
    // - returning const ref avoids copies and prevents external mutation/ownership transfer
    const std::vector<std::unique_ptr<sf::Shape>>& getObstacleShapes() const;

    const std::vector<Door>& getCurrentRoomDoors() const;
    std::string getCurrentRoomName() const;

    // Objectives are returned by non-const reference to allow modification (accomplished flags).
    //
    // WHY non-const reference:
    // - gameplay systems need to mark objectives accomplished in-place
    // - avoids copying Objective objects (which include SFML resources)
    std::vector<Objective>& getCurrentRoomObjectives();

    // WHY return by value here:
    // - builds a flattened snapshot across rooms (caller gets its own independent list)
    std::vector<Objective> getAllLevelObjectives() const;

    int getCurrentRoomIndex() const;
    std::string getCurrentRoomDialogueRef() const;
    bool isCurrentRoomDialogueShown() const;
    void markCurrentRoomDialogueShown();

    float getScreenW() const;
    float getScreenH() const;

    // Obstacles update (currently empty/no-op in cpp).
    void mettreAJourObstacles();

    // Collision/detection flags.
    void setCollisionDetectee(bool v);
    bool isCollisionDetectee() const;
    void setJoueurDetecte(bool v);
    bool isJoueurDetecte() const;

    // Runtime enemies + prototypes:
    // - enemies: actual instances in current room
    // - enemyPrototypes: templates used to create enemies by cloning (Prototype pattern)
    //
    // WHY vector<unique_ptr<Enemy>>:
    // - polymorphism: store different enemy derived types through Enemy base pointer
    // - RAII: automatic destruction, no manual delete
    std::vector<std::unique_ptr<Enemy>> enemies;

    // WHY map<string, unique_ptr<Enemy>> for prototypes:
    // - prototypes are looked up by type string ("generic", "camera", "laser")
    // - unique ownership of each prototype instance by the model
    std::map<std::string, std::unique_ptr<Enemy>> enemyPrototypes;

    const std::vector<std::unique_ptr<Enemy>>& getEnemies() const;
    void reloadEnemiesForCurrentRoom();
    void updateEnemies();

    // Dialogue state.
    bool hasDialogueTriggered() const;
    void setDialogueTriggered(bool v);
    void resetDialogueTriggered();

    // Life system.
    int getLives() const;
    void loseLives(int amount);
    bool isGameOver() const;

    // Detection counter (how many times player got detected).
    int getDetectionCount() const;
    void incrementDetectionCount();
    void resetDetectionCount();

    // Obstacle center/forward (forward currently defaulted in cpp).
    sf::Vector2f getObstacleCenter(size_t idx = 0) const;
    sf::Vector2f getObstacleForward(size_t idx = 0) const;

    // Change room and respawn player based on entry.
    //
    // WHY const std::string&:
    // - avoids copying entry direction string
    bool changeRoom(int newRoomIndex, const std::string& entryDirection);

    // Player animation interface.
    void setPlayerDirection(int row);

    // WHY bool by value:
    // - trivial type; simplest and fastest
    void updatePlayerAnimation(bool moving);

    const sf::Sprite& getPlayerSprite() const { return playerSprite; }

    void syncPlayerSprite();

    // Level load from JSON (rebuilds RoomManager and reloads enemies).
    //
    // WHY const std::string&:
    // - avoid copying path string
    bool loadLevelFromFile(const std::string& levelJsonPath);

    // Objective contact pointers.
    //
    // WHY Objective* parameter:
    // - non-owning pointer to an Objective stored elsewhere (room vector)
    // - allows modifying the original objective without copying
    void setObjectiveContact(Objective* obj);

    void setObjectiveContactDetectee(const bool b);
    Objective* getObjectiveContact() const;
    bool getObjectiveContactDetectee() const;

    // Scores.
    void setPlayerScore(int levelIndex, int score);

    // WHY return by value:
    // - returns a snapshot copy of the current scores array/vector
    std::vector<int> getPlayerScores() const;

    // Mission index API: allow controller to tell the model which mission is active.
    void setCurrentMissionIndex(int idx);
    int getCurrentMissionIndex() const;
};

} // namespace Modele
