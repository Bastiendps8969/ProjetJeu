
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
//   and to prevent accidental mutation from outside.

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
        std::string direction;                 // "up"/"down"/"left"/"right"
        int targetRoomIndex;                   // destination room id (or <0 for exit)
        sf::FloatRect bounds;                  // cached bounds for collision/checks
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
        std::vector<sf::Vector2f> patrolPoints;

        // Speed for patrolling enemies (ignored/overridden for stationary types).
        float speed = 2.0f;

        // Optional texture name (used by enemy factory).
        std::string textureName;

        // -------------------------
        // Type-specific flags / data
        // -------------------------

        // Camera type flag (stationary cone-of-vision).
        bool isCamera = false;

        // Facing direction as string (data-driven from JSON).
        // Will be converted to a direction vector by the factory.
        std::string facing;

        // Cone-of-vision parameters.
        float visionRange = 300.f;
        float visionAngle = 60.f;

        // Laser type flag (stationary beam).
        bool isLaser = false;

        // Laser beam length.
        float laserLength = 600.f;
    };

    // ------------------------------------------------------------
    // ObstacleDefinition: data-only obstacle descriptor.
    // ------------------------------------------------------------
    struct ObstacleDefinition
    {
        std::string type;           // e.g., "rect"
        sf::Vector2f position;      // scaled position
        sf::Vector2f size;          // scaled size
        std::string textureName;    // decoration texture name
    };

    // ------------------------------------------------------------
    // Room: holds all data relevant to a room.
    // ------------------------------------------------------------
    struct Room
    {
        std::string name;

        // Doors and enemy definitions are stored as plain data.
        std::vector<Door> doors;
        std::vector<EnemyDefinition> enemyDefs;

        // Obstacles definitions and runtime shapes (rectangles, etc.).
        std::vector<ObstacleDefinition> obstacleDefs;
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
        RoomManager(float w, float h);

        // Load and parse rooms from JSON file.
        // Returns true on success, false on failure.
        bool loadRoomsFromJson(const std::string& filename);

        // Initialize SFML shapes for doors + obstacles in a given room.
        void initializeRoomShapes(Room& room);

        // Access current room door list (const ref avoids copy; caller should not modify).
        const std::vector<Door>& getCurrentRoomDoors() const;

        // Return current room name (for UI/log).
        std::string getCurrentRoomName() const;

        // Change room and reposition player based on entry direction.
        // Player passed by reference because we need to modify its position.
        bool changeRoom(int newRoomIndex,
                        const std::string& entryDirection,
                        sf::RectangleShape& joueur);

        // Access enemy definitions for current room (data-only).
        const std::vector<EnemyDefinition>& getCurrentRoomEnemies() const;

        // Dialogue metadata for current room.
        std::string getCurrentRoomDialogueRef() const;
        bool isCurrentRoomDialogueShown() const;
        void markCurrentRoomDialogueShown();

        int getCurrentRoomIndex() const { return currentRoomIndex_; }

        // Non-const ref getter: allows external systems to edit rooms map if needed
        // (use carefully; could break encapsulation).
        std::map<int, Room>& getRooms() { return rooms_; }

        float getScreenW() const { return screenW; }
        float getScreenH() const { return screenH; }
    };

} // namespace Modele
