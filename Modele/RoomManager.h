
// RoomManager.h
//
// Room/level loading + access layer.
//
// ===============================
// DESIGN / RATIONALE
// ===============================
// The RoomManager is responsible for:
// - Loading rooms from a JSON configuration file.
// - Storing *definitions* (data-only descriptors) such as EnemyDefinition.
// - Preparing simple SFML shapes for doors (for rendering/debug/collisions).
//
// Important: EnemyDefinition is NOT a runtime enemy (no AI logic here).
// It is a data-only description that will later be consumed by a factory
// (e.g., createEnemyFromDefinition) to build actual Enemy instances.
//
// OWNERSHIP NOTES
// - Doors store a unique_ptr<sf::Shape> to own the visual shape (RAII).
// - Rooms store obstacleShapes as unique_ptr<sf::Shape> as well.
// - Accessors often return const references to internal vectors to avoid copies,
// and to prevent accidental mutation from outside.

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include "Objective.h"

namespace Modele {

    // ------------------------------------------------------------
    // Door: minimal door data + an owned visual representation.
    // ------------------------------------------------------------
    struct Door
    {
        std::string direction;          // "up"/"down"/"left"/"right"
        int targetRoomIndex;            // destination room id (or <0 for exit)
        sf::FloatRect bounds;           // cached bounds for collision/checks

        // WHY unique_ptr here:
        // - exclusive ownership: a door "owns" its shape instance
        // - RAII: automatic deletion when Door is destroyed or moved
        // - movable but not copyable: prevents accidental expensive or invalid copies
        std::unique_ptr<sf::Shape> visualShape; // owned SFML shape for rendering/debug
    };

    // ------------------------------------------------------------
    // EnemyDefinition: data-only descriptor loaded from JSON.
    // ------------------------------------------------------------
    struct EnemyDefinition
    {
        // Base position in the room (pixels in the room coordinate system).
        sf::Vector2f position;

        // Optional patrol route (used by generic patrol enemies).
        // WHY vector by value here:
        // - the definition owns its route points as configuration data
        std::vector<sf::Vector2f> patrolPoints;

        // Speed for patrolling enemies (ignored/overridden for stationary types).
        float speed = 2.0f;

        // Optional texture name (used by enemy factory).
        std::string textureName;

        // -------------------------
        // Type-specific flags / data
        // -------------------------
        bool isCamera = false;
        std::string facing;
        float visionRange = 300.f;
        float visionAngle = 60.f;

        bool isLaser = false;
        float laserLength = 600.f;
    };

    // ------------------------------------------------------------
    // ObstacleDefinition: data-only obstacle descriptor.
    // ------------------------------------------------------------
    struct ObstacleDefinition
    {
        std::string type;       // e.g., "rect"
        sf::Vector2f position;  // scaled position
        sf::Vector2f size;      // scaled size
        std::string textureName;// decoration texture name
    };

    // ------------------------------------------------------------
    // Room: holds all data relevant to a room.
    // ------------------------------------------------------------
    struct Room
    {
        std::string name;

        // Doors and enemy definitions are stored as plain data.
        // WHY vectors of values:
        // - rooms fully own their configuration data
        // - contiguous storage improves iteration performance
        std::vector<Door> doors;
        std::vector<EnemyDefinition> enemyDefs;

        // Obstacles definitions and runtime shapes (rectangles, etc.).
        std::vector<ObstacleDefinition> obstacleDefs;

        // WHY vector<unique_ptr<sf::Shape>>:
        // - polymorphism: sf::Shape is a base class (RectangleShape, CircleShape, ...)
        // - ownership: Room owns the created shapes for its lifetime
        std::vector<std::unique_ptr<sf::Shape>> obstacleShapes;

        // Objectives present in the room.
        std::vector<Objective> objectives;

        // Optional dialogue metadata.
        std::string dialogueRef;
        bool dialogueShown = false;

        // Optional external map file.
        std::string mapFile;
    };

    // ------------------------------------------------------------
    // RoomManager: loads JSON and exposes current-room data.
    // ------------------------------------------------------------
    class RoomManager
    {
    private:
        // Map of roomId -> Room data.
        // WHY std::map:
        // - stable key-based lookup by room id
        // - stores Rooms by value (RoomManager owns the rooms)
        std::map<int, Room> rooms_;

        // Current active room index.
        int currentRoomIndex_ = -1;

        // Screen size used for scaling and for placing door shapes.
        float screenW, screenH;

        // Door visual parameters (in pixels).
        const float DOOR_SIZE = 120.f;
        const float DOOR_THICKNESS = 100.f;

    public:
        // Constructor stores screen size used for scaling / door placement.
        // WHY pass by value (float):
        // - primitive type, trivial to copy, keeps call simple
        RoomManager(float w, float h);

        // Load and parse rooms from JSON file.
        // Returns true on success, false on failure.
        // WHY const std::string&:
        // - avoids copying the filename string
        // - communicates read-only usage
        bool loadRoomsFromJson(const std::string& filename);

        // Initialize SFML shapes for doors + obstacles in a given room.
        // WHY Room& (non-const reference):
        // - this function mutates room (clears/creates shapes and updates door bounds)
        void initializeRoomShapes(Room& room);

        // Access current room door list (const ref avoids copy; caller should not modify).
        // WHY const ref return:
        // - avoids copying potentially large vectors
        // - keeps RoomManager in control of mutation (encapsulation)
        const std::vector<Door>& getCurrentRoomDoors() const;

        // Return current room name (for UI/log).
        // Returned by value to give the caller an independent string.
        std::string getCurrentRoomName() const;

        // Change room and reposition player based on entry direction.
        // Player passed by reference because we need to modify its position.
        // WHY sf::RectangleShape&:
        // - we must update the caller's player object in-place (setPosition)
        // WHY const std::string& entryDirection:
        // - avoid copying and ensure read-only usage
        bool changeRoom(int newRoomIndex,
                        const std::string& entryDirection,
                        sf::RectangleShape& joueur);

        // Access enemy definitions for current room (data-only).
        // WHY const ref return:
        // - avoids copying the vector of definitions
        const std::vector<EnemyDefinition>& getCurrentRoomEnemies() const;

        // Dialogue metadata for current room.
        std::string getCurrentRoomDialogueRef() const;

        bool isCurrentRoomDialogueShown() const;
        void markCurrentRoomDialogueShown();

        int getCurrentRoomIndex() const { return currentRoomIndex_; }

        // Non-const ref getter: allows external systems to edit rooms map if needed
        // WHY non-const ref:
        // - enables advanced systems (editors/debug) to modify rooms directly
        // - avoids copying the map
        std::map<int, Room>& getRooms() { return rooms_; }

        float getScreenW() const { return screenW; }
        float getScreenH() const { return screenH; }
    };

} // namespace Modele
