
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
//   (RoomManager, MapManager, Level) to express clear ownership and lifetime.
// - Enemies are stored as std::unique_ptr<Enemy> to keep polymorphism + RAII.
// - Enemy instantiation is data-driven: RoomManager loads EnemyDefinition from JSON,
//   then Modele converts definitions to runtime objects via createEnemyFromDefinition().
//
// OWNERSHIP
// - unique_ptr expresses exclusive ownership (single owner, automatic destruction).
// - Functions returning const references avoid copies and prevent external mutation.
//
// NOTE ABOUT PLAYER REPRESENTATION
// - Player uses a RectangleShape as collision/hitbox and a Sprite as visual.
//   This decoupling is common: collisions use simple shapes; visuals can be scaled/cropped.
//
// NOTE ABOUT ANIMATION
// - The player sprite uses a sprite-sheet with movement frames + idle frames,
//   and a separate idle row region (playerRow + 4).

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <string>
#include <memory>

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
    sf::RectangleShape joueur;      // collision/hitbox representation

    // Sprite-sheet resources for player visuals
    sf::Texture playerTexture;
    sf::Sprite  playerSprite;

    // Animation state (movement + idle)
    int   playerFrameCount = 9;
    int   playerFrameIndex = 0;
    int   playerRow = 3;            // 1=up,2=left,3=down,4=right (default: down)
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
    sf::IntRect computePlayerTextureRect() const;

    // Subsystems owned by the model (unique ownership).
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
    Objective* objectiveContact = nullptr;
    bool objectiveContactDetectee = false;

    bool dialogueTriggeredFlag = false;

    // Score memory (non persistent).
    std::vector<int> playerScores = std::vector<int>(12, 0);

public:
    Modele();
    ~Modele() = default;

    void reset();

    // Player access:
    // Returns non-const reference because other systems (controller) may move the hitbox.
    sf::RectangleShape& getJoueur() { return joueur; }

    // Map queries delegate to MapManager.
    const std::vector<std::vector<int>>& getFloorMatrix() const;
    const sf::Texture& getFloorTexture() const;

    void setFloorMatrix(const std::vector<std::vector<int>>& m);
    bool setTileTexture(int id, const std::string& path);
    const sf::Texture* getTileTexture(int id) const;
    int getTileSize() const;
    const std::vector<sf::Texture>& getWallTextures() const;

    // Obstacles/doors from current room.
    const std::vector<std::unique_ptr<sf::Shape>>& getObstacleShapes() const;
    const std::vector<Door>& getCurrentRoomDoors() const;
    std::string getCurrentRoomName() const;

    // Objectives are returned by non-const reference to allow modification (accomplished flags).
    std::vector<Objective>& getCurrentRoomObjectives();
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
    std::vector<std::unique_ptr<Enemy>> enemies;
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
    bool changeRoom(int newRoomIndex, const std::string& entryDirection);

    // Player animation interface.
    void setPlayerDirection(int row);
    void updatePlayerAnimation(bool moving);
    const sf::Sprite& getPlayerSprite() const { return playerSprite; }
    void syncPlayerSprite();

    // Level load from JSON (rebuilds RoomManager and reloads enemies).
    bool loadLevelFromFile(const std::string& levelJsonPath);

    // Objective contact pointers.
    void setObjectiveContact(Objective* obj);
    void setObjectiveContactDetectee(const bool b);
    Objective* getObjectiveContact() const;
    bool getObjectiveContactDetectee() const;

    // Scores.
    void setPlayerScore(int levelIndex, int score);
    std::vector<int> getPlayerScores() const;
};

} // namespace Modele
