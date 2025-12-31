
#include "RoomManager.h"
#include <fstream>
#include <iostream>
#include "../cmake-build-debug/json.hpp"
#include <cmath>
#include <unordered_map>
#include <memory>
#include <filesystem>

using json = nlohmann::json;

namespace Modele {

RoomManager::RoomManager(float w, float h)
    : screenW(w), screenH(h)
{}

// ✅ NEW: le "screenW/screenH" devient la taille MONDE (map)
void RoomManager::setWorldSize(float w, float h)
{
    screenW = w;
    screenH = h;

    // Recalcule les portes car elles dépendent de screenW/screenH
    for (auto& pair : rooms_)
        initializeRoomShapes(pair.second);
}

bool RoomManager::loadRoomsFromJson(const std::string& filename)
{
    rooms_.clear();
    std::ifstream i(filename);
    if (!i.is_open())
    {
        std::cerr << "Erreur: Impossible d'ouvrir le fichier JSON " << filename
                  << ". Verifiez qu'il est dans le dossier de l'executable." << std::endl;
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

            // Doors
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

            // Obstacles
            if (roomJson.contains("obstacles"))
            {
                for (const auto& obsJson : roomJson.at("obstacles"))
                {
                    ObstacleDefinition def;
                    def.type = obsJson.value("type", "rect");
                    def.position.x = obsJson.value("x", 0.f);
                    def.position.y = obsJson.value("y", 0.f);
                    def.size.x = obsJson.value("w", 64.f);
                    def.size.y = obsJson.value("h", 64.f);
                    def.textureName = obsJson.value("textureName", "");
                    newRoom.obstacleDefs.emplace_back(std::move(def));
                }
            }

            // Optional map file
            if (roomJson.contains("mapFile"))
            {
                try { newRoom.mapFile = roomJson.at("mapFile").get<std::string>(); }
                catch (...) {}
            }

            // Optional room dialogueRef
            if (roomJson.contains("dialogueRef"))
            {
                try { newRoom.dialogueRef = roomJson.at("dialogueRef").get<std::string>(); }
                catch (...) { newRoom.dialogueRef = std::string(); }
            }

            // Objectives
            if (roomJson.contains("objectives"))
            {
                for (const auto& objJson : roomJson.at("objectives"))
                {
                    Objective objective;
                    objective.setTitle(objJson.value("title", std::string("")));
                    objective.setDescription(objJson.value("description", std::string("")));
                    objective.setTexture(objJson.value("texture", std::string("")));
                    objective.setPrimary(objJson.value("primary", false));
                    objective.setHitboxPosition(objJson.value("x", 0.f), objJson.value("y", 0.f));
                    objective.setHitboxSize(objJson.value("w", 32.f), objJson.value("h", 32.f));
                    objective.setDialogueFile(objJson.value("dialogueFile", std::string("")));
                    objective.setDialogueRef(objJson.value("dialogueRef", std::string("")));
                    objective.setCesar(objJson.value("cesar", false));
                    objective.setCode(objJson.value("code", std::string("")));
                    objective.setchangeValue(objJson.value("changeValue", 0));
                    newRoom.objectives.emplace_back(std::move(objective));
                }
            }

            // Enemies definitions (pure data)
            if (roomJson.contains("enemies"))
            {
                for (const auto& enemyJson : roomJson.at("enemies"))
                {
                    EnemyDefinition ed;
                    ed.position.x = enemyJson.at("x").get<float>();
                    ed.position.y = enemyJson.at("y").get<float>();

                    ed.speed = enemyJson.value("speed", 2.0f);

                    if (enemyJson.contains("patrol"))
                    {
                        for (const auto& pt : enemyJson.at("patrol"))
                        {
                            sf::Vector2f p;
                            p.x = pt.at(0).get<float>();
                            p.y = pt.at(1).get<float>();
                            ed.patrolPoints.push_back(p);
                        }
                    }

                    ed.textureName = enemyJson.value("texture", "ennemy_textures1");

                    std::string type = enemyJson.value("type", "");

                    if (type == "camera")
                    {
                        ed.isCamera = true;
                        ed.facing = enemyJson.value("facing", "left");
                        ed.speed = 0.f;
                        ed.patrolPoints.clear();
                    }

                    if (type == "laser")
                    {
                        ed.isLaser = true;
                        ed.facing = enemyJson.value("facing", "right");
                        ed.speed = 0.f;
                        ed.patrolPoints.clear();
                        ed.laserLength = enemyJson.value("laserLength", 600.f);
                    }

                    ed.visionRange = enemyJson.value("visionRange", ed.isCamera ? 400.f : 300.f);
                    ed.visionAngle = enemyJson.value("visionAngle", ed.isCamera ? 70.f : 60.f);

                    newRoom.enemyDefs.push_back(ed);
                }
            }

            rooms_[roomId] = std::move(newRoom);
        }

        // init shapes (portes + obstacles) selon screenW/screenH
        for (auto& pair : rooms_)
            initializeRoomShapes(pair.second);

        return true;
    }
    catch (const json::exception& e)
    {
        std::cerr << "Erreur de parsing JSON: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Erreur lors du chargement des pieces: " << e.what() << std::endl;
        return false;
    }
}

void RoomManager::initializeRoomShapes(Room& room)
{
    room.obstacleShapes.clear();

    static std::unordered_map<std::string, std::shared_ptr<sf::Texture>> texCache;
    namespace fs = std::filesystem;

    for (const auto& def : room.obstacleDefs)
    {
        if (def.type == "rect")
        {
            auto rect = std::make_unique<sf::RectangleShape>(def.size);
            rect->setPosition(def.position);
            rect->setOutlineColor(sf::Color::Green);
            rect->setOutlineThickness(2.f);

            if (!def.textureName.empty())
            {
                std::string rel = std::string("Asset/Decoration/") + def.textureName;

                std::vector<std::string> candidates = {
                    rel, "./" + rel, "../" + rel, "../../" + rel,
                    std::string("cmake-build-debug/") + rel,
                    std::string("./cmake-build-debug/") + rel
                };

                std::shared_ptr<sf::Texture> tex;
                std::string usedPath;

                for (const auto& c : candidates)
                {
                    fs::path p = c;
                    bool exists = false;
                    try { exists = fs::exists(p); } catch (...) { exists = false; }

                    if (exists) { usedPath = c; break; }
                }
                if (usedPath.empty()) usedPath = rel;

                auto it = texCache.find(usedPath);
                if (it != texCache.end())
                {
                    tex = it->second;
                }
                else
                {
                    tex = std::make_shared<sf::Texture>();
                    if (tex->loadFromFile(usedPath))
                    {
                        tex->setSmooth(true);
                        texCache[usedPath] = tex;
                    }
                    else
                    {
                        std::cerr << "[RoomManager] Failed to load obstacle texture: " << usedPath << std::endl;
                        tex.reset();
                    }
                }

                if (tex) rect->setTexture(tex.get());
                else rect->setFillColor(sf::Color::Red);
            }
            else
            {
                rect->setFillColor(sf::Color::Red);
            }

            room.obstacleShapes.emplace_back(std::move(rect));
        }
    }

    // ✅ Portes: placées aux bords du MONDE (screenW/screenH = world)
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

        if (door.visualShape)
        {
            if (door.targetRoomIndex < 0)
                door.visualShape->setFillColor(sf::Color(255, 140, 0, 200));
            else
                door.visualShape->setFillColor(sf::Color(0, 150, 255, 128));

            door.bounds = door.visualShape->getGlobalBounds();
        }
    }
}

const std::vector<Door>& RoomManager::getCurrentRoomDoors() const
{
    static const std::vector<Door> emptyDoors;
    auto it = rooms_.find(currentRoomIndex_);
    if (it != rooms_.end()) return it->second.doors;
    return emptyDoors;
}

const std::vector<EnemyDefinition>& RoomManager::getCurrentRoomEnemies() const
{
    static const std::vector<EnemyDefinition> empty;
    auto it = rooms_.find(currentRoomIndex_);
    if (it != rooms_.end()) return it->second.enemyDefs;
    return empty;
}

std::string RoomManager::getCurrentRoomDialogueRef() const
{
    auto it = rooms_.find(currentRoomIndex_);
    if (it != rooms_.end()) return it->second.dialogueRef;
    return std::string();
}

bool RoomManager::isCurrentRoomDialogueShown() const
{
    auto it = rooms_.find(currentRoomIndex_);
    if (it != rooms_.end()) return it->second.dialogueShown;
    return false;
}

void RoomManager::markCurrentRoomDialogueShown()
{
    auto it = rooms_.find(currentRoomIndex_);
    if (it != rooms_.end()) it->second.dialogueShown = true;
}

std::string RoomManager::getCurrentRoomName() const
{
    auto it = rooms_.find(currentRoomIndex_);
    if (it != rooms_.end()) return it->second.name;
    return "Piece inconnue (ID:" + std::to_string(currentRoomIndex_) + ")";
}

// ✅ Ici aussi: teleport basé sur MONDE (screenW/screenH)
bool RoomManager::changeRoom(int newRoomIndex, const std::string& entryDirection, sf::RectangleShape& joueur)
{
    if (currentRoomIndex_ == newRoomIndex) return true;

    auto it = rooms_.find(newRoomIndex);
    if (it == rooms_.end())
    {
        std::cerr << "Erreur: Piece cible " << newRoomIndex << " introuvable." << std::endl;
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

} // namespace Modele
