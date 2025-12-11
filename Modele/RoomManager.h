#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <string>
#include <memory>

namespace Modele {

struct Door {
    std::string direction;
    int targetRoomIndex;
    sf::FloatRect bounds;
    std::unique_ptr<sf::Shape> visualShape;
};

struct EnemyDefinition {
    sf::Vector2f position;
    std::vector<sf::Vector2f> patrolPoints;
    float speed = 2.0f;
    std::string textureName; // Ajout
    // Ajout pour caméra
    bool isCamera = false;
    std::string facing; // "left", "right", "up", "down"
    // Ajout pour cône de vision
    float visionRange = 300.f;
    float visionAngle = 60.f;
};

struct Room {
    std::string name;
    std::vector<Door> doors;
    std::vector<EnemyDefinition> enemyDefs;
};

class RoomManager {
private:
    std::map<int, Room> rooms_;
    int currentRoomIndex_ = -1;
    float screenW, screenH;
    const float DOOR_SIZE = 120.f;
    const float DOOR_THICKNESS = 100.f;

public:
    RoomManager(float w, float h);

    bool loadRoomsFromJson(const std::string& filename);
    void initializeRoomShapes(Room& room);

    const std::vector<Door>& getCurrentRoomDoors() const;
    std::string getCurrentRoomName() const;
    bool changeRoom(int newRoomIndex, const std::string& entryDirection, sf::RectangleShape& joueur);
    const std::vector<EnemyDefinition>& getCurrentRoomEnemies() const;

    int getCurrentRoomIndex() const { return currentRoomIndex_; }
    std::map<int, Room>& getRooms() { return rooms_; }
    float getScreenW() const { return screenW; }
    float getScreenH() const { return screenH; }
};

}
