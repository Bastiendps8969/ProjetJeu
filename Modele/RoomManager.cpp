
#include "RoomManager.h"

#include <fstream>        // std::ifstream
#include <iostream>       // std::cout, std::cerr
#include "../cmake-build-debug/json.hpp" // nlohmann::json (project-local include path)
#include <cmath>          // std::sqrt
#include <unordered_map>  // obstacle texture cache
#include <memory>         // std::unique_ptr, std::shared_ptr
#include <filesystem>     // to test candidate paths

using json = nlohmann::json;

namespace Modele {

// RoomManager.cpp
//
// ===============================
// DESIGN / RATIONALE
// ===============================
// This file loads a JSON configuration describing rooms.
// Each room contains:
// - a name
// - doors ("up","down","left","right" -> target room)
// - obstacles (rectangles) and their textures
// - objectives (with positions/sizes, dialogue metadata, etc.)
// - enemies (EnemyDefinition descriptors)
//
// IMPORTANT: enemies are loaded as EnemyDefinition (data-only).
// Actual Enemy objects are created later by a factory (Prototype + Factory approach).

RoomManager::RoomManager(float w, float h)
    : screenW(w), screenH(h)
{
    // Store screen dimensions for later scaling and door placement.
}

// Load rooms from JSON configuration file.
bool RoomManager::loadRoomsFromJson(const std::string& filename)
{
    // Clear previous content so reload starts from a clean state.
    rooms_.clear();

    // Open JSON file.
    std::ifstream i(filename);

    // Fail early if file cannot be opened.
    if (!i.is_open())
    {
        std::cerr << "Error: Cannot open JSON file " << filename
                  << ". Ensure it is located next to the executable."
                  << std::endl;
        return false;
    }

    // Parse JSON and populate rooms_ map.
    json j;

    try
    {
        // Read file into JSON object.
        i >> j;

        // Each top-level key is a room id (string) that we convert to int.
        for (auto it = j.begin(); it != j.end(); ++it)
        {
            int roomId = std::stoi(it.key());
            json roomJson = it.value();

            Room newRoom;

            // Mandatory field: room name.
            // Using at() throws if missing -> helps catch config errors early.
            newRoom.name = roomJson.at("name").get<std::string>();

            // ----------------------------
            // Doors: direction -> target id
            // ----------------------------
            if (roomJson.contains("doors"))
            {
                for (auto doorIt = roomJson.at("doors").begin();
                     doorIt != roomJson.at("doors").end();
                     ++doorIt)
                {
                    Door door;

                    // Door direction key: "up"/"down"/"left"/"right"
                    door.direction = doorIt.key();

                    // Target room index.
                    door.targetRoomIndex = doorIt.value().get<int>();

                    // Store the minimal door descriptor (shape will be created later).
                    newRoom.doors.emplace_back(std::move(door));
                }
            }

            // ----------------------------
            // Obstacles: rectangles from JSON
            // ----------------------------
            if (roomJson.contains("obstacles"))
            {
                // Scaling approach:
                // obstacle coordinates are authored for a reference resolution (2560x1440)
                // and scaled to current screenW/screenH.
                const float refW = 2560.f;
                const float refH = 1440.f;
                const float scaleW = screenW / refW;
                const float scaleH = screenH / refH;

                for (const auto& obsJson : roomJson.at("obstacles"))
                {
                    ObstacleDefinition def;

                    // Default obstacle type is "rect".
                    def.type = obsJson.value("type", "rect");

                    // Scale position and size.
                    def.position.x = obsJson.value("x", 0.f) * scaleW;
                    def.position.y = obsJson.value("y", 0.f) * scaleH;
                    def.size.x     = obsJson.value("w", 64.f) * scaleW;
                    def.size.y     = obsJson.value("h", 64.f) * scaleH;

                    // Optional texture name to apply to the obstacle rectangle.
                    def.textureName = obsJson.value("textureName", "");

                    newRoom.obstacleDefs.emplace_back(std::move(def));
                }
            }

            // Optional: external map file for the room.
            if (roomJson.contains("mapFile"))
            {
                try { newRoom.mapFile = roomJson.at("mapFile").get<std::string>(); }
                catch (...) {}
            }

            // Optional: room-level dialogue reference.
            if (roomJson.contains("dialogueRef"))
            {
                try { newRoom.dialogueRef = roomJson.at("dialogueRef").get<std::string>(); }
                catch (...) { newRoom.dialogueRef = std::string(); }
            }

            // ----------------------------
            // Objectives: parse JSON
            // ----------------------------
            if (roomJson.contains("objectives"))
            {
                // Use the same reference scaling as obstacles/enemies.
                const float refW = 2560.f;
                const float refH = 1440.f;
                const float scaleW = screenW / refW;
                const float scaleH = screenH / refH;

                for (const auto& objJson : roomJson.at("objectives"))
                {
                    Objective objective;

                    // Populate objective metadata.
                    objective.setTitle(objJson.value("title", std::string("")));
                    objective.setDescription(objJson.value("description", std::string("")));
                    objective.setTexture(objJson.value("texture", std::string("")));
                    objective.setPrimary(objJson.value("primary", false));

                    // Scale hitbox position/size.
                    float ox = objJson.value("x", 0.f) * scaleW;
                    float oy = objJson.value("y", 0.f) * scaleH;
                    float ow = objJson.value("w", 32.f) * scaleW;
                    float oh = objJson.value("h", 32.f) * scaleH;

                    objective.setHitboxPosition(ox, oy);
                    objective.setHitboxSize(ow, oh);

                    // Optional dialogue data for objective.
                    objective.setDialogueFile(objJson.value("dialogueFile", std::string("")));
                    objective.setDialogueRef(objJson.value("dialogueRef", std::string("")));

                    // Optional Cesar fields (project-specific).
                    objective.setCesar(objJson.value("cesar", false));
                    objective.setCode(objJson.value("code", std::string("")));
                    objective.setchangeValue(objJson.value("changeValue", 0));

                    newRoom.objectives.emplace_back(std::move(objective));
                }
            }

            // ----------------------------
            // Enemies: parse EnemyDefinition (data-only)
            // ----------------------------
            if (roomJson.contains("enemies"))
            {
                for (const auto& enemyJson : roomJson.at("enemies"))
                {
                    EnemyDefinition ed;

                    // Mandatory position.
                    ed.position.x = enemyJson.at("x").get<float>();
                    ed.position.y = enemyJson.at("y").get<float>();

                    // Optional speed (used for patrollers).
                    ed.speed = enemyJson.value("speed", 2.0f);

                    // Optional patrol route (list of [x,y] pairs).
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

                    // Texture name default to avoid repeating config for every enemy.
                    ed.textureName = enemyJson.value("texture", "ennemy_textures1");

                    // "type" decides additional flags/fields.
                    std::string type = enemyJson.value("type", "");

                    // If camera:
                    // - mark as camera
                    // - facing from JSON (default left)
                    // - speed forced to 0, patrol cleared (stationary)
                    if (type == "camera")
                    {
                        ed.isCamera = true;
                        ed.facing = enemyJson.value("facing", "left");
                        ed.speed = 0.f;
                        ed.patrolPoints.clear();
                    }

                    // If laser:
                    // - mark as laser
                    // - facing from JSON (default right)
                    // - speed forced to 0, patrol cleared (stationary)
                    // - laserLength may be customized
                    if (type == "laser")
                    {
                        ed.isLaser = true;
                        ed.facing = enemyJson.value("facing", "right");
                        ed.speed = 0.f;
                        ed.patrolPoints.clear();
                        ed.laserLength = enemyJson.value("laserLength", 600.f);
                    }

                    // Vision parameters are customizable per enemy.
                    // Default differs between camera and generic:
                    // - camera: range 400, angle 70
                    // - generic: range 300, angle 60
                    ed.visionRange = enemyJson.value("visionRange", ed.isCamera ? 400.f : 300.f);
                    ed.visionAngle = enemyJson.value("visionAngle", ed.isCamera ? 70.f : 60.f);

                    // Store the enemy definition in the room.
                    // Actual Enemy objects are created later by the runtime model/factory.
                    newRoom.enemyDefs.push_back(ed);
                }
            }

            // Store room in the map (move to avoid copies).
            rooms_[roomId] = std::move(newRoom);
        }

        // Create basic visual shapes for doors/obstacles based on current screen size.
        for (auto& pair : rooms_)
        {
            initializeRoomShapes(pair.second);
        }

        return true;
    }
    catch (const json::exception& e)
    {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Room loading error: " << e.what() << std::endl;
        return false;
    }
}

void RoomManager::initializeRoomShapes(Room& room)
{
    // Build obstacle shapes from obstacleDefs (currently rectangles).
    room.obstacleShapes.clear();

    // Texture cache (static => shared across calls/rooms):
    // Rationale: avoids reloading the same texture file multiple times.
    static std::unordered_map<std::string, std::shared_ptr<sf::Texture>> texCache;

    namespace fs = std::filesystem;

    for (const auto& def : room.obstacleDefs)
    {
        if (def.type == "rect")
        {
            // Create rectangle shape with size from definition.
            auto rect = std::make_unique<sf::RectangleShape>(def.size);

            // Position it.
            rect->setPosition(def.position);

            // Outline helps debug collisions/visibility.
            rect->setOutlineColor(sf::Color::Green);
            rect->setOutlineThickness(2.f);

            // If a texture name is provided, attempt to load/apply it.
            if (!def.textureName.empty())
            {
                std::string rel = std::string("Asset/Decoration/") + def.textureName;

                // Try multiple candidate paths to support different run directories.
                std::vector<std::string> candidates;
                candidates.push_back(rel);
                candidates.push_back("./" + rel);
                candidates.push_back("../" + rel);
                candidates.push_back("../../" + rel);
                candidates.push_back(std::string("cmake-build-debug/") + rel);
                candidates.push_back(std::string("./cmake-build-debug/") + rel);

                std::shared_ptr<sf::Texture> tex;
                std::string usedPath;

                // Find first candidate that exists.
                for (const auto& c : candidates)
                {
                    fs::path p = c;
                    bool exists = false;
                    try { exists = fs::exists(p); }
                    catch (...) { exists = false; }

                    std::cout << "[RoomManager] Trying texture path: '" << c
                              << "' (exists=" << (exists ? "yes" : "no") << ")"
                              << std::endl;

                    if (exists)
                    {
                        usedPath = c;
                        break;
                    }
                }

                // If none found, still try the original relative path.
                if (usedPath.empty()) usedPath = rel;

                // Check cache first.
                auto it = texCache.find(usedPath);
                if (it != texCache.end())
                {
                    tex = it->second;
                }
                else
                {
                    // Load texture and store in cache.
                    tex = std::make_shared<sf::Texture>();

                    std::cout << "[RoomManager] Loading texture from: " << usedPath << std::endl;

                    if (tex->loadFromFile(usedPath))
                    {
                        tex->setSmooth(true);
                        texCache[usedPath] = tex;

                        std::cout << "[RoomManager] Texture loaded OK: " << usedPath << std::endl;
                    }
                    else
                    {
                        std::cerr << "[RoomManager] Failed to load obstacle texture: " << usedPath << std::endl;
                        tex.reset();
                    }
                }

                // Apply texture if available; otherwise fill red as fallback.
                if (tex)
                    rect->setTexture(tex.get());
                else
                    rect->setFillColor(sf::Color::Red);
            }
            else
            {
                // No texture name -> fill red.
                rect->setFillColor(sf::Color::Red);
            }

            room.obstacleShapes.emplace_back(std::move(rect));
        }
    }

    // Create door shapes (rectangles placed at screen edges).
    for (auto& door : room.doors)
    {
        if (door.direction == "up")
        {
            door.visualShape = std::make_unique<sf::RectangleShape>(
                sf::Vector2f(DOOR_SIZE, DOOR_THICKNESS)
            );
            door.visualShape->setPosition(screenW / 2.f - DOOR_SIZE / 2.f, 0.f);
        }
        else if (door.direction == "down")
        {
            door.visualShape = std::make_unique<sf::RectangleShape>(
                sf::Vector2f(DOOR_SIZE, DOOR_THICKNESS)
            );
            door.visualShape->setPosition(screenW / 2.f - DOOR_SIZE / 2.f,
                                          screenH - DOOR_THICKNESS);
        }
        else if (door.direction == "left")
        {
            door.visualShape = std::make_unique<sf::RectangleShape>(
                sf::Vector2f(DOOR_THICKNESS, DOOR_SIZE)
            );
            door.visualShape->setPosition(0.f, screenH / 2.f - DOOR_SIZE / 2.f);
        }
        else if (door.direction == "right")
        {
            door.visualShape = std::make_unique<sf::RectangleShape>(
                sf::Vector2f(DOOR_THICKNESS, DOOR_SIZE)
            );
            door.visualShape->setPosition(screenW - DOOR_THICKNESS,
                                          screenH / 2.f - DOOR_SIZE / 2.f);
        }

        // If we created the shape, color it + store bounds.
        if (door.visualShape)
        {
            // Distinguish exit doors (targetRoomIndex < 0).
            if (door.targetRoomIndex < 0)
                door.visualShape->setFillColor(sf::Color(255, 140, 0, 200)); // orange-ish
            else
                door.visualShape->setFillColor(sf::Color(0, 150, 255, 128)); // bluish translucent

            door.bounds = door.visualShape->getGlobalBounds();
        }
    }
}

const std::vector<Door>& RoomManager::getCurrentRoomDoors() const
{
    // Return a const reference to avoid copying.
    // Provide a static empty vector if room not found.
    static const std::vector<Door> emptyDoors;

    auto it = rooms_.find(currentRoomIndex_);
    if (it != rooms_.end())
        return it->second.doors;

    return emptyDoors;
}

const std::vector<EnemyDefinition>& RoomManager::getCurrentRoomEnemies() const
{
    // Same pattern: return const ref to avoid copy, or empty fallback.
    static const std::vector<EnemyDefinition> empty;

    auto it = rooms_.find(currentRoomIndex_);
    if (it != rooms_.end())
        return it->second.enemyDefs;

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
    if (it != rooms_.end())
        return it->second.name;

    return "Unknown room (ID:" + std::to_string(currentRoomIndex_) + ")";
}

bool RoomManager::changeRoom(int newRoomIndex,
                             const std::string& entryDirection,
                             sf::RectangleShape& joueur)
{
    // If already in that room, nothing to do.
    if (currentRoomIndex_ == newRoomIndex) return true;

    // Validate target room exists.
    auto it = rooms_.find(newRoomIndex);
    if (it == rooms_.end())
    {
        std::cerr << "Error: Target room " << newRoomIndex << " not found." << std::endl;
        return false;
    }

    // Switch current room.
    currentRoomIndex_ = newRoomIndex;

    // Player size used to place player fully inside screen.
    float playerW = joueur.getSize().x;
    float playerH = joueur.getSize().y;

    float halfW = playerW * 0.5f;
    float halfH = playerH * 0.5f;

    // Reposition player based on entry direction so it appears to come from that door.
    // Small offset avoids spawning inside the wall/edge.
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
        // Fallback: center the player if direction is unknown.
        joueur.setPosition(screenW * 0.5f - halfW, screenH * 0.5f - halfH);
    }

    return true;
}

} // namespace Modele
