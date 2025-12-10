#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <string>
#include <memory>

namespace Modele {

struct ObstacleDefinition {
    std::string type;
    sf::Vector2f position;
    sf::Vector2f size;
};

struct Door {
    std::string direction;
    int targetRoomIndex;
    sf::FloatRect bounds;
    std::unique_ptr<sf::Shape> visualShape;
};

struct Room {
    std::string name;
    std::vector<ObstacleDefinition> obstacleDefs;
    std::vector<Door> doors;
    std::vector<std::unique_ptr<sf::Shape>> obstacleShapes;
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

    const std::vector<std::unique_ptr<sf::Shape>>& getObstacleShapes() const;
    const std::vector<Door>& getCurrentRoomDoors() const;
    std::string getCurrentRoomName() const;
    bool changeRoom(int newRoomIndex, const std::string& entryDirection, sf::RectangleShape& joueur);

    int getCurrentRoomIndex() const { return currentRoomIndex_; }
    std::map<int, Room>& getRooms() { return rooms_; }
    float getScreenW() const { return screenW; }
    float getScreenH() const { return screenH; }
};

}
