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

// RoomManager.cpp
// ----------------
// Responsable du chargement des définitions de pièces depuis un fichier JSON
// et de la fourniture d'informations simples à la couche modèle/jeu.
//
// Format JSON attendu (extrait):
// {
//   "0": {
//     "name": "Entrée",
//     "doors": { "up": 1, "right": 2 },
//     "enemies": [
//       { "x": 400, "y": 300, "type": "camera", "facing": "left", "visionRange": 400, "visionAngle": 70, "texture": "cam_tex" }
//     ]
//   }
// }
//


RoomManager::RoomManager(float w, float h)
    : screenW(w), screenH(h)
{}

// Constructeur
// - `w` / `h`: taille de la fenêtre (ou surface de jeu) ; utilisée pour positionner
//   les formes visuelles simples des portes et pour calculer les positions lors
//   des changements de pièce.


bool RoomManager::loadRoomsFromJson(const std::string& filename)
{
    rooms_.clear();
    std::ifstream i(filename);
    if (!i.is_open())
    {
        // Erreur si le fichier de configuration n'est pas accessible
        std::cerr << "Erreur: Impossible d'ouvrir le fichier JSON " << filename << ". Vérifiez qu'il est dans le dossier de l'exécutable." << std::endl;
        return false;
    }

    // Parse JSON and populate rooms_ map
    json j;
    try
    {
        i >> j;
        // Each top-level key is a room id (string that we convert to int)
        for (auto it = j.begin(); it != j.end(); ++it)
        {
            int roomId = std::stoi(it.key());
            json roomJson = it.value();
            Room newRoom;

            // Room name (champ obligatoire)
            // Utiliser at() pour lever une exception si le champ manque,
            // afin de repérer rapidement les erreurs de configuration.
            newRoom.name = roomJson.at("name").get<std::string>();

            // Doors: map direction -> target index
            if (roomJson.contains("doors"))
            {
                for (auto doorIt = roomJson.at("doors").begin(); doorIt != roomJson.at("doors").end(); ++doorIt)
                {
                    Door door;
                    door.direction = doorIt.key();
                    door.targetRoomIndex = doorIt.value().get<int>();
                    // Stocke la définition minimale d'une porte (direction et index cible)
                    newRoom.doors.emplace_back(std::move(door));
                }
            }

            // Obstacles: rectangles définis dans le JSON
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

            // Optional: external map file for the room
            if (roomJson.contains("mapFile")) {
                try {
                    newRoom.mapFile = roomJson.at("mapFile").get<std::string>();
                } catch (...) {}
            }

            // Optional: room-level dialogue reference (sequence id in dialogues.json)
            if (roomJson.contains("dialogueRef")) {
                try {
                    newRoom.dialogueRef = roomJson.at("dialogueRef").get<std::string>();
                } catch (...) { newRoom.dialogueRef = std::string(); }
            }

            // Objectives: parse JSON and construct Objective descriptors
            if (roomJson.contains("objectives")) {
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
                    // Optional Cesar fields (present in ProjetJeu format)
                    objective.setCesar(objJson.value("cesar", false));
                    objective.setCode(objJson.value("code", std::string("")));
                    objective.setchangeValue(objJson.value("changeValue", 0));
                    newRoom.objectives.emplace_back(std::move(objective));
                }
            }

            // Enemies: liste de descriptions `EnemyDefinition` (données pures provenant du JSON)
            // Chaque `EnemyDefinition` est consommé plus tard par Modele pour créer
            // les instances runtime (clonage de prototypes, application d'échelles, etc.).
            if (roomJson.contains("enemies"))
            {
                for (const auto& enemyJson : roomJson.at("enemies"))
                {
                    EnemyDefinition ed;
                    // Position obligatoire
                    ed.position.x = enemyJson.at("x").get<float>();
                    ed.position.y = enemyJson.at("y").get<float>();
                    // Optional fields with defaults
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

                    // Texture et type : les valeurs par défaut permettent de ne pas
                    // répéter la configuration pour chaque ennemi.
                    ed.textureName = enemyJson.value("texture", "ennemy_textures1");
                    std::string type = enemyJson.value("type", "");

                    // Traitement spécifique selon le type d'ennemi.
                    // - camera: détecteur statique avec un cône de vision orienté
                    // - laser: détecteur par faisceau (ligne) orienté
                    if (type == "camera") {
                        ed.isCamera = true;
                        ed.facing = enemyJson.value("facing", "left");
                        // Les caméras sont immobiles par défaut
                        ed.speed = 0.f;
                        ed.patrolPoints.clear();
                    }
                    if (type == "laser") {
                        ed.isLaser = true;
                        ed.facing = enemyJson.value("facing", "right");
                        ed.speed = 0.f;
                        ed.patrolPoints.clear();
                        // Longueur du laser en pixels (peut être convertie ensuite)
                        ed.laserLength = enemyJson.value("laserLength", 600.f);
                    }

                    // Paramètres de vision: peuvent être présents dans le JSON pour
                    // ajuster la portée et l'angle par ennemi. Les valeurs par défaut
                    // dépendent du type (caméra vs ennemi mobile générique).
                    ed.visionRange = enemyJson.value("visionRange", ed.isCamera ? 400.f : 300.f);
                    ed.visionAngle = enemyJson.value("visionAngle", ed.isCamera ? 70.f : 60.f);

                    // Ajoute la définition à la pièce; l'instanciation concrète
                    // sera faite plus tard (Modele::reloadEnemiesForCurrentRoom).
                    newRoom.enemyDefs.push_back(ed);
                }
            }

            // Store the parsed room
            rooms_[roomId] = std::move(newRoom);
        }

        // Crée des formes visuelles simples pour les portes (utile pour le debug
        // et pour les collisions de base). Les formes sont calculées ici en
        // fonction de la taille de l'écran connue du RoomManager.
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
    // Create obstacle shapes from definitions (rectangles for now)
    room.obstacleShapes.clear();
    // Cache textures to avoid reloading the same file multiple times
    static std::unordered_map<std::string, std::shared_ptr<sf::Texture>> texCache;
    namespace fs = std::filesystem;
    for (const auto& def : room.obstacleDefs)
    {
        if (def.type == "rect") {
            auto rect = std::make_unique<sf::RectangleShape>(def.size);
            rect->setPosition(def.position);
            // Draw a visible green outline for debugging / visibility
            rect->setOutlineColor(sf::Color::Green);
            rect->setOutlineThickness(2.f);
            // If a texture name is provided, try to load and apply it
            if (!def.textureName.empty()) {
                std::string rel = std::string("Asset/Decoration/") + def.textureName;
                std::vector<std::string> candidates;
                candidates.push_back(rel);
                candidates.push_back("./" + rel);
                candidates.push_back("../" + rel);
                candidates.push_back("../../" + rel);
                // also try build output relative paths (cmake-build-debug)
                candidates.push_back(std::string("cmake-build-debug/") + rel);
                candidates.push_back(std::string("./cmake-build-debug/") + rel);

                std::shared_ptr<sf::Texture> tex;
                std::string usedPath;
                // Try to find a candidate that exists
                for (const auto& c : candidates) {
                    fs::path p = c;
                    bool exists = false;
                    try { exists = fs::exists(p); } catch(...) { exists = false; }
                    std::cout << "[RoomManager] Trying texture path: '" << c << "' (exists=" << (exists?"yes":"no") << ")" << std::endl;
                    if (exists) {
                        usedPath = c;
                        break;
                    }
                }

                // if none exists, still try the original rel path last
                if (usedPath.empty()) usedPath = rel;

                auto it = texCache.find(usedPath);
                if (it != texCache.end()) {
                    tex = it->second;
                } else {
                    tex = std::make_shared<sf::Texture>();
                    std::cout << "[RoomManager] Loading texture from: " << usedPath << std::endl;
                    if (tex->loadFromFile(usedPath)) {
                        tex->setSmooth(true);
                        texCache[usedPath] = tex;
                        std::cout << "[RoomManager] Texture loaded OK: " << usedPath << std::endl;
                    } else {
                        std::cerr << "[RoomManager] Failed to load obstacle texture: " << usedPath << std::endl;
                        tex.reset();
                    }
                }

                if (tex) {
                    rect->setTexture(tex.get());
                } else {
                    rect->setFillColor(sf::Color::Red);
                }
            } else {
                rect->setFillColor(sf::Color::Red);
            }
            room.obstacleShapes.emplace_back(std::move(rect));
        }
    }


    // For each door in the room, create a simple RectangleShape that
    // visually represents the door on the corresponding edge of the screen.
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

        // Définition visuelle et calcul des bounds utilisés par le jeu
        // pour détecter les entrées/sorties de pièce.
        if (door.visualShape) {
            // If targetRoomIndex < 0, this is an exit door -> highlight with a distinct color
            if (door.targetRoomIndex < 0) {
                door.visualShape->setFillColor(sf::Color(255, 140, 0, 200)); // orange-ish, more opaque
            } else {
                door.visualShape->setFillColor(sf::Color(0, 150, 255, 128)); // default bluish translucent
            }
            door.bounds = door.visualShape->getGlobalBounds();
        }
    }
}

const std::vector<Door>& RoomManager::getCurrentRoomDoors() const
{
    static const std::vector<Door> emptyDoors;
    auto it = rooms_.find(currentRoomIndex_);
    if (it != rooms_.end())
    {
        // Retourne la liste des portes de la pièce courante. On retourne une
        // référence vers le vecteur interne; le caller ne doit pas modifier
        // ce vecteur directement.
        return it->second.doors;
    }
    return emptyDoors;
}

const std::vector<EnemyDefinition>& RoomManager::getCurrentRoomEnemies() const
{
    static const std::vector<EnemyDefinition> empty;
    auto it = rooms_.find(currentRoomIndex_);
    if (it != rooms_.end())
        // Renvoie les définitions d'ennemis pour la pièce courante. Ces
        // définitions doivent être utilisées pour instancier les ennemis
        // (clonage de prototypes) dans la couche modèle runtime.
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
    {
        // Nom lisible de la pièce utilisée dans l'UI et les logs
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

    // Calcule la position du joueur en fonction de sa taille pour
    // éviter qu'il soit partiellement hors-écran lors du teleporte
    float playerW = joueur.getSize().x;
    float playerH = joueur.getSize().y;
    float halfW = playerW * 0.5f;
    float halfH = playerH * 0.5f;

    // Positionne le joueur selon la direction d'entrée pour simuler
    // une arrivée par la porte correspondante. Les décalages fixes
    // (20.f) évitent que le joueur soit collé contre la bordure.
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
        // Position centrale par défaut si la direction n'est pas reconnue
        joueur.setPosition(screenW * 0.5f - halfW, screenH * 0.5f - halfH);
    }

    return true;
}

}
