#include "RoomManager.h"
#include <fstream>
#include <iostream>
#include "../cmake-build-debug/json.hpp"
#include <cmath>

using json = nlohmann::json;

namespace Modele {

RoomManager::RoomManager(float w, float h)
    : screenW(w), screenH(h)
{}

bool RoomManager::loadRoomsFromJson(const std::string& filename)
{
    rooms_.clear();
    std::ifstream i(filename);
    if (!i.is_open())
    {
        std::cerr << "Erreur: Impossible d'ouvrir le fichier JSON " << filename << ". Vérifiez qu'il est dans le dossier de l'exécutable." << std::endl;
        return false;
    }

    json j;
    try
    {
        i >> j;
        for (auto it = j.begin(); it != j.end(); ++it)
        {
            int roomId = std::stoi(it.key());
            json roomJson = it.value();
            Room newRoom;

            newRoom.name = roomJson.at("name").get<std::string>();

            if (roomJson.contains("obstacles"))
            {
                for (const auto& obsJson : roomJson.at("obstacles"))
                {
                    ObstacleDefinition def;
                    def.type = obsJson.at("type").get<std::string>();
                    def.position.x = obsJson.at("x").get<float>();
                    def.position.y = obsJson.at("y").get<float>();
                    def.size.x = obsJson.at("w").get<float>();
                    def.size.y = obsJson.at("h").get<float>();
                    newRoom.obstacleDefs.emplace_back(def);
                }
            }

            if (roomJson.contains("doors"))
            {
                for (auto doorIt = roomJson.at("doors").begin(); doorIt != roomJson.at("doors").end(); ++doorIt)
                {
                    Door door;
                    door.direction = doorIt.key();
                    door.targetRoomIndex = doorIt.value().get<int>();
                    newRoom.doors.emplace_back(std::move(door));
                }
            }

            rooms_[roomId] = std::move(newRoom);
        }

        for (auto& pair : rooms_)
        {
            initializeRoomShapes(pair.second);
        }

        return true;
    }
    catch (const json::exception& e)
    {
        std::cerr << "Erreur de parsing JSON: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Erreur lors du chargement des pièces: " << e.what() << std::endl;
        return false;
    }
}

void RoomManager::initializeRoomShapes(Room& room)
{
    room.obstacleShapes.clear();

    for (const auto& def : room.obstacleDefs)
    {
        if (def.type == "rect")
        {
            std::unique_ptr<sf::RectangleShape> rect = std::make_unique<sf::RectangleShape>(def.size);
            rect->setFillColor(sf::Color::Red);
            rect->setPosition(def.position);
            room.obstacleShapes.emplace_back(std::move(rect));
        }
    }

    for (auto& door : room.doors)
    {
        if (door.direction == "up")
        {
            door.visualShape = std::make_unique<sf::RectangleShape>(sf::Vector2f(DOOR_SIZE, DOOR_THICKNESS));
            door.visualShape->setPosition(screenW / 2.f - DOOR_SIZE / 2.f, 0.f);
        }
        else if (door.direction == "down")
        {
            door.visualShape = std::make_unique<sf::RectangleShape>(sf::Vector2f(DOOR_SIZE, DOOR_THICKNESS));
            door.visualShape->setPosition(screenW / 2.f - DOOR_SIZE / 2.f, screenH - DOOR_THICKNESS);
        }
        else if (door.direction == "left")
        {
            door.visualShape = std::make_unique<sf::RectangleShape>(sf::Vector2f(DOOR_THICKNESS, DOOR_SIZE));
            door.visualShape->setPosition(0.f, screenH / 2.f - DOOR_SIZE / 2.f);
        }
        else if (door.direction == "right")
        {
            door.visualShape = std::make_unique<sf::RectangleShape>(sf::Vector2f(DOOR_THICKNESS, DOOR_SIZE));
            door.visualShape->setPosition(screenW - DOOR_THICKNESS, screenH / 2.f - DOOR_SIZE / 2.f);
        }

        if (door.visualShape) {
            door.visualShape->setFillColor(sf::Color(0, 150, 255, 128));
            door.bounds = door.visualShape->getGlobalBounds();
        }
    }
}

const std::vector<std::unique_ptr<sf::Shape>>& RoomManager::getObstacleShapes() const
{
    static const std::vector<std::unique_ptr<sf::Shape>> emptyShapes;
    auto it = rooms_.find(currentRoomIndex_);
    if (it != rooms_.end())
    {
        return it->second.obstacleShapes;
    }
    return emptyShapes;
}

const std::vector<Door>& RoomManager::getCurrentRoomDoors() const
{
    static const std::vector<Door> emptyDoors;
    auto it = rooms_.find(currentRoomIndex_);
    if (it != rooms_.end())
    {
        return it->second.doors;
    }
    return emptyDoors;
}

std::string RoomManager::getCurrentRoomName() const
{
    auto it = rooms_.find(currentRoomIndex_);
    if (it != rooms_.end())
    {
        return it->second.name;
    }
    return "Pièce inconnue (ID:" + std::to_string(currentRoomIndex_) + ")";
}

bool RoomManager::changeRoom(int newRoomIndex, const std::string& entryDirection, sf::RectangleShape& joueur)
{
    if (currentRoomIndex_ == newRoomIndex) return true;

    auto it = rooms_.find(newRoomIndex);
    if (it == rooms_.end())
    {
        std::cerr << "Erreur: Pièce cible " << newRoomIndex << " introuvable." << std::endl;
        return false;
    }

    currentRoomIndex_ = newRoomIndex;

    float playerW = joueur.getSize().x;
    float playerH = joueur.getSize().y;
    float halfW = playerW * 0.5f;
    float halfH = playerH * 0.5f;

    if (entryDirection == "up")
    {
        joueur.setPosition(screenW / 2.f - halfW, DOOR_THICKNESS + 20.f);
    }
    else if (entryDirection == "down")
    {
        joueur.setPosition(screenW / 2.f - halfW, screenH - DOOR_THICKNESS - 20.f - playerH);
    }
    else if (entryDirection == "left")
    {
        joueur.setPosition(DOOR_THICKNESS + 20.f, screenH / 2.f - halfH);
    }
    else if (entryDirection == "right")
    {
        joueur.setPosition(screenW - DOOR_THICKNESS - 20.f - playerW, screenH / 2.f - halfH);
    }
    else
    {
        joueur.setPosition(screenW * 0.5f - halfW, screenH * 0.5f - halfH);
    }

    return true;
}

}
