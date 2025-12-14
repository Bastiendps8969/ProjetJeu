#include "Modele.h"
#include "Agent.h"
#include "Enemy.h"
#include "RoomManager.h"
#include <cmath>
#include <limits>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "Objective.h"

// Alias de namespace pour nlohmann/json
using json = nlohmann::json;

namespace Modele {

    const float DOOR_MARGIN = 20.f;

    // Définition des méthodes privées (maintenant correctement scopees)
    void Modele::initializeRoomShapes(Room& room)
    {
        room.obstacleShapes.clear();

        // 1. Création des obstacles physiques
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

        // 2. Création des zones de porte
        for (auto& door : room.doors)
        {
            // La logique de placement de la porte reste inchangée
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

    // Définition de la méthode privée (maintenant correctement scopee)
    bool Modele::loadRoomsFromJson(const std::string& filename)
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

                if (roomJson.contains("objectives")) {
                    for (const auto& obsJson : roomJson.at("objectives"))
                    {
                        //  Remplacer par un constructeur avec arguments
                        //  pour que ce soit plus simple
                        Objective objective;
                        objective.setTitle(obsJson.at("title").get<std::string>());
                        objective.setDescription(obsJson.at("description").get<std::string>());
                        objective.setTexture(obsJson.at("texture").get<std::string>());
                        objective.setPrimary(obsJson.at("primary").get<bool>());

                        objective.setHitboxPosition(
                            obsJson.at("x").get<float>(),
                            obsJson.at("y").get<float>()
                        );
                        objective.setHitboxSize(
                            obsJson.at("w").get<float>(),
                            obsJson.at("h").get<float>()
                        );

                        objective.getSprite().setTexture(objective.getTexture());
                        objective.getSprite().setPosition(
                            obsJson.at("x").get<float>(),
                            obsJson.at("y").get<float>()
                        );

                        objective.setDialogueFile(obsJson.at("dialogueFile").get<std::string>());
                        objective.setDialogueRef(obsJson.at("dialogueRef").get<std::string>());

                        std::cout << objective.getTitle() << " has been loaded" << std::endl;

                        newRoom.objectives.emplace_back(std::move(objective));
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


    // Constructeur : toutes les variables membres sont maintenant initialisées
    Modele::Modele()
    : collisionDetectee(false)
    {
        // Détermine la résolution du bureau
        sf::VideoMode dm = sf::VideoMode::getDesktopMode();
        float screenW = static_cast<float>(dm.width);
        float screenH = static_cast<float>(dm.height);

        roomManager = std::make_unique<RoomManager>(screenW, screenH);

        // Initialize enemy prototypes (Prototype pattern)
        enemyPrototypes.clear();
        enemyPrototypes["generic"] = std::make_unique<GenericEnemy>();
        enemyPrototypes["camera"] = std::make_unique<CameraEnemy>();
        enemyPrototypes["laser"] = std::make_unique<LaserEnemy>();

        float boxSize = std::max(8.f, std::min(screenW, screenH) * 0.08f);

        // Création du rectangle joueur
        float playerCollisionW = boxSize * playerSpriteDisplayScaleX;
        float playerCollisionH = boxSize * playerSpriteDisplayScaleY;
        const float HITBOX_REDUCTION_FACTOR = 0.8f;
        joueur.setSize(sf::Vector2f(46,130
        ));
        joueur.setFillColor(sf::Color::Blue);


        // --- Chargement de la spritesheet du joueur ---
        const std::vector<std::string> tryPlayerPaths = {
            "cmake-build-debug/Asset/Human/james_adams_textures.png",
            "Asset/Human/james_adams_textures.png",
            "Human/james_adams_textures.png",
            "james_adams_textures.png"
        };
        bool playerLoaded = false;
        for (const auto& p : tryPlayerPaths) {
            if (playerTexture.loadFromFile(p)) {
                playerLoaded = true;
                std::cout << "[DEBUG] Spritesheet joueur chargée : " << p << std::endl;
                break;
            } else {
                std::cout << "[DEBUG] Echec chargement spritesheet joueur : " << p << std::endl;
            }
        }
        if (!playerLoaded) {
            std::cerr << "[DEBUG] Avertissement: impossible de charger la spritesheet joueur (james_adams_textures.png)\n";
        } else {
            playerSprite.setTexture(playerTexture);
            // Initialiser le sprite sur la frame 0 de la row par défaut (playerRow)
            playerFrameIndex = 0;
            playerClock.restart();
            // Définit la zone initiale (en tenant compte du zoom)
            playerSprite.setTextureRect(computePlayerTextureRect());
            // Origine initiale au centre du recadrage (sera mise à jour dans syncPlayerSprite)
            // note: origin en px du recadrage sera défini dans syncPlayerSprite()
            playerSprite.setOrigin(0.f, 0.f);

            // position temporaire centrée sur le rectangle joueur
            sf::Vector2f ppos = joueur.getPosition();
            playerSprite.setPosition(ppos.x + joueur.getSize().x/2.f, ppos.y + joueur.getSize().y/2.f);

            // Mettre à l'échelle le sprite pour remplir la taille du RectangleShape joueur
            syncPlayerSprite();
        }

        // --- Chargement de la texture de sol (Floor5.png) ---
        bool loaded = false;
        const std::vector<std::string> tryPaths = {
            "cmake-build-debug/Asset/Floor/Floor5.png",
            "Asset/Floor/Floor5.png",
            "Floor5.png"
        };
        for (const auto& p : tryPaths) {
            if (floorTexture.loadFromFile(p)) {
                loaded = true;
                std::cout << "[DEBUG] Texture sol chargée : " << p << std::endl;
                break;
            } else {
                std::cout << "[DEBUG] Echec chargement texture sol : " << p << std::endl;
            }
        }
        if (!loaded) {
            std::cerr << "[DEBUG] Avertissement: impossible de charger Floor5.png. Vérifiez le chemin.\n";
        }

        // --- Chargement des textures de murs (Wall1_1 .. Wall1_8) ---
        const std::vector<std::string> tryPathsWall = {
            "cmake-build-debug/Asset/Wall/Wall1_1.png",
            "cmake-build-debug/Asset/Wall/Wall1_5.png",
            "cmake-build-debug/Asset/Wall/Wall1_6.png",
            "cmake-build-debug/Asset/Wall/Wall1_7.png",
            "cmake-build-debug/Asset/Wall/Wall1_2.png",
            "cmake-build-debug/Asset/Wall/Wall1_3.png",
            "cmake-build-debug/Asset/Wall/Wall1_4.png",
            "cmake-build-debug/Asset/Wall/Wall1_8.png"
        };
        wallTextures.clear();
        wallTextures.resize(8);
        for (size_t i = 0; i < tryPathsWall.size(); ++i)
        {
            // Correction : chaque wallTextures[i] doit correspondre à Asset/Wall/Wall1_{i+1}.png
            std::string assetPath = "Asset/Wall/Wall1_" + std::to_string(i+1) + ".png";
            if (!wallTextures[i].loadFromFile(tryPathsWall[i]))
            {
                std::cout << "[DEBUG] Echec chargement mur : " << tryPathsWall[i] << std::endl;
                if (!wallTextures[i].loadFromFile(assetPath))
                {
                    std::cout << "[DEBUG] Echec chargement mur : " << assetPath << std::endl;
                    std::cerr << "[DEBUG] Avertissement: impossible de charger " << tryPathsWall[i] << " ni " << assetPath << "\n";
                }
                else
                {
                    std::cout << "[DEBUG] Texture mur chargée : " << assetPath << std::endl;
                }
            }
            else
            {
                std::cout << "[DEBUG] Texture mur chargée : " << tryPathsWall[i] << std::endl;
            }
        }

        // Recalcul de la matrice du sol / murs (cols, rows calculés plus haut)
        int cols = static_cast<int>(std::ceil(screenW / static_cast<float>(tileSize)));
        int rows = static_cast<int>(std::ceil(screenH / static_cast<float>(tileSize)));
        floorMatrix.assign(rows, std::vector<int>(cols, 1)); // remplir tout l'écran avec la tuile sol (1)

        // Codes pour murs : 11..18 (correspondent aux wallTextures[0..7])
        const int WALL_TL = 11;   // top-left
        const int WALL_TOP = 12;  // top edge
        const int WALL_TR = 13;   // top-right
        const int WALL_LEFT = 14; // left edge
        const int WALL_RIGHT = 15;// right edge
        const int WALL_BL = 16;   // bottom-left
        const int WALL_BOTTOM = 17;// bottom edge
        const int WALL_BR = 18;   // bottom-right


        for (int r = 0; r < rows; ++r)
        {
            for (int c = 0; c < cols; ++c)
            {
                // coins particuliers
                if (r == 0 && c == 0)
                {
                    // coin haut-gauche
                    floorMatrix[r][c] = WALL_TL;
                }
                else if (r == 0 && c == cols - 1)
                {
                    // coin haut-droit
                    floorMatrix[r][c] = WALL_BL;
                }
                else if (r == rows - 1 && c == 0)
                {
                    // coin bas-gauche
                    floorMatrix[r][c] = WALL_TR;
                }
                else if (r == rows - 1 && c == cols - 1)
                {
                    // coin bas-droit
                    floorMatrix[r][c] = WALL_BR;
                }
                else if (r == 0)
                {
                    // bord haut
                    floorMatrix[r][c] = WALL_RIGHT;
                }
                else if (r == rows - 1)
                {
                    // bord bas
                    floorMatrix[r][c] = WALL_LEFT;
                }
                else if (c == 0)
                {
                    // bord gauche
                    floorMatrix[r][c] = WALL_TOP;
                }
                else if (c == cols - 1)
                {
                    // bord droit
                    floorMatrix[r][c] = WALL_BOTTOM;
                }
                // else: leave as 1 (floor)
            }
        }

        // Chargement des pièces
        if (loadRoomsFromJson("Asset/levels/tutorial/tutorial_1.json") && rooms_.count(0))
        {
            roomManager->changeRoom(0, "", joueur);
            joueur.setPosition(screenW * 0.5f - boxSize * 0.5f, screenH * 0.5f - boxSize * 0.5f);
            reloadEnemiesForCurrentRoom();
        }
        else
        {
            std::cerr << "Échec du chargement de la carte. Pièce 0 non valide." << std::endl;
            roomManager->changeRoom(-1, "", joueur);
            joueur.setPosition(screenW * 0.5f - boxSize * 0.5f, screenH * 0.5f - boxSize * 0.5f);
        }

        // Initialisation des points de patrouille (si non chargés par JSON)
        std::vector<sf::Vector2f> patrouillePoints = {
            sf::Vector2f(screenW * 0.125f, screenH * 0.1666667f),
            sf::Vector2f(screenW * 0.75f,  screenH * 0.1666667f),
            sf::Vector2f(screenW * 0.75f,  screenH * 0.6666667f),
            sf::Vector2f(screenW * 0.125f, screenH * 0.6666667f)
        };
        // Supprimer la référence à obstacleShapes (inutile maintenant)
        // agent = std::make_unique<Agent>(&roomManager->getRooms()[roomManager->getCurrentRoomIndex()].obstacleShapes, patrouillePoints);
        agent = nullptr;
    }


    void Modele::reloadEnemiesForCurrentRoom()
    {
        enemies.clear();
        float refW = 2560.f; // résolution de référence (modifiez selon votre design JSON)
        float refH = 1440.f;
        float scaleW = getScreenW() / refW;
        float scaleH = getScreenH() / refH;

        for (const auto& ed : roomManager->getCurrentRoomEnemies())
        {
            std::vector<sf::Vector2f> patrol;
            for (const auto& pt : ed.patrolPoints) {
                patrol.push_back(sf::Vector2f(pt.x * scaleW, pt.y * scaleH));
            }
            sf::Vector2f pos(ed.position.x * scaleW, ed.position.y * scaleH);
            float scaledVisionRange = ed.visionRange * std::sqrt(scaleW * scaleH);
            float scaledLaserLength = ed.laserLength * std::sqrt(scaleW * scaleH);

            // Determine type
            std::string type = ed.isLaser ? "laser" : (ed.isCamera ? "camera" : "generic");
            auto it = enemyPrototypes.find(type);
            std::unique_ptr<Enemy> e;
            if (it != enemyPrototypes.end()) e = it->second->clone();
            else e = enemyPrototypes["generic"]->clone();

            // common setup
            e->position = pos;
            e->textureName = ed.textureName;

            // load texture if present
            const std::vector<std::string> tryPathsTex = {
                "cmake-build-debug/Asset/Human/" + e->textureName + ".png",
                "Asset/Human/" + e->textureName + ".png",
                "Human/" + e->textureName + ".png",
                e->textureName + ".png"
            };
            for (const auto& p : tryPathsTex) {
                if (e->texture.loadFromFile(p)) { e->sprite.setTexture(e->texture); break; }
            }
            e->sprite.setOrigin(e->tileSize / 2.f, e->tileSize / 2.f);
            e->sprite.setScale(2.5f, 2.5f);

            if (type == "generic") {
                GenericEnemy* g = dynamic_cast<GenericEnemy*>(e.get());
                if (g) {
                    g->patrolPoints = patrol;
                    g->speed = ed.speed;
                    if (!g->patrolPoints.empty())
                        g->direction = Enemy::normalize(g->patrolPoints[0] - g->position);
                }
                e->isCamera = false;
                e->isLaser = false;
                e->visionRange = scaledVisionRange;
                e->visionAngle = ed.visionAngle;
            }
            else if (type == "camera") {
                CameraEnemy* c = dynamic_cast<CameraEnemy*>(e.get());
                if (c) {
                    c->facing = ed.facing;
                    // `visionRange` / `visionAngle` sont des membres hérités de la classe de base `Enemy`.
                    // On positionne les valeurs via l'objet `e` ci‑dessous.
                    if (c->facing == "left") c->direction = {-1.f, 0.f};
                    else if (c->facing == "right") c->direction = {1.f, 0.f};
                    else if (c->facing == "up") c->direction = {0.f, -1.f};
                    else c->direction = {0.f, 1.f};
                }
                e->isCamera = true;
                e->isLaser = false;
                e->visionRange = scaledVisionRange;
                e->visionAngle = ed.visionAngle;
            }
            else if (type == "laser") {
                LaserEnemy* l = dynamic_cast<LaserEnemy*>(e.get());
                if (l) {
                    l->facing = ed.facing;
                    // `laserLength` est hérité de `Enemy`; on l'initialise via `e` ci-dessous.
                    if (l->facing == "left") l->direction = {-1.f, 0.f};
                    else if (l->facing == "right") l->direction = {1.f, 0.f};
                    else if (l->facing == "up") l->direction = {0.f, -1.f};
                    else l->direction = {0.f, 1.f};
                }
                e->isLaser = true;
                e->isCamera = false;
                e->laserLength = scaledLaserLength;
            }

            enemies.push_back(std::move(e));
        }
    }

    void Modele::updateEnemies()
    {
        for (auto& e : enemies) {
            e->update();
            e->updateAnimation(); // Ajout : animation frame
            e->detectPlayer(joueur);
        }
    }

    // Mise à jour de la logique d'IA des obstacles (Corrigé pour utiliser la nouvelle structure)
    void Modele::mettreAJourObstacles()
    {
        if (agent)
            agent->mettreAJour(joueur);
    }

    // Calcule la textureRect (recadrée au centre) pour la frame courante selon playerTextureZoom.
    // NOTE: les frames de déplacement sont sur les rows 1..4 ; les frames "idle" (2 frames)
    // sont sur les rows 5..8 (i.e. playerRow + 4), colonnes 0..idleFrameCount-1.
    sf::IntRect Modele::computePlayerTextureRect() const
    {
        int frameSize = playerTileSize;
        int cropSize = static_cast<int>(std::round(frameSize / playerTextureZoom));
        if (cropSize < 1) cropSize = 1;

        int movementFramesCount = std::max(1, playerFrameCount - idleFrameCount);
        int col = 0;
        int rowIndex = std::max(0, playerRow - 1);

        if (playerIsMoving)
        {
            // colonne issue des frames de déplacement (0 .. movementFramesCount-1)
            col = playerFrameIndex % movementFramesCount;
            rowIndex = std::max(0, playerRow - 1); // 0..3
        }
        else
        {
            // idle -> utiliser la ligne idle (playerRow + 4 -> 4..7) et les colonnes 0..idleFrameCount-1
            int idleCols = std::max(1, idleFrameCount);
            col = playerFrameIndex % idleCols;
            rowIndex = std::max(0, playerRow - 1) + 4;
        }

        int frameX = col * frameSize;
        int frameY = rowIndex * frameSize;

        int offsetX = frameX + (frameSize - cropSize) / 2;
        int offsetY = frameY + (frameSize - cropSize) / 2;
        return sf::IntRect(offsetX, offsetY, cropSize, cropSize);
    }

    void Modele::setPlayerDirection(int row)
    {
        if (row < 1) row = 1;
        if (row > 4) row = 4;
        if (playerRow != row) {
            playerRow = row;
            playerFrameIndex = 0;
            playerClock.restart();
            if (playerTexture.getSize().x > 0)
                playerSprite.setTextureRect(computePlayerTextureRect());
        }
    }

    void Modele::updatePlayerAnimation(bool moving)
    {
        if (playerTexture.getSize().x == 0) return; // pas de texture

        int movementFramesCount = std::max(1, playerFrameCount - idleFrameCount);
        int idleCols = std::max(1, idleFrameCount);

        // choisir la durée selon l'état (moving vs idle)
        float frameDuration = moving ? playerFrameDuration : playerIdleFrameDuration;
        float elapsed = playerClock.getElapsedTime().asSeconds();

        // Si changement d'état, réinitialiser l'index de frame
        if (moving != playerIsMoving)
        {
            playerIsMoving = moving;
            playerFrameIndex = 0;
            playerSprite.setTextureRect(computePlayerTextureRect());
            playerClock.restart();
            // continuer pour permettre incrément immédiat si elapsed >= duration
        }

        if (playerIsMoving)
        {
            if (elapsed >= playerFrameDuration)
            {
                playerFrameIndex = (playerFrameIndex + 1) % movementFramesCount;
                playerSprite.setTextureRect(computePlayerTextureRect());
                playerClock.restart();
            }
        }
        else // idle
        {
            if (idleCols <= 0)
            {
                playerFrameIndex = 0;
                playerSprite.setTextureRect(computePlayerTextureRect());
                playerClock.restart();
                return;
            }

            if (elapsed >= playerIdleFrameDuration)
            {
                playerFrameIndex = (playerFrameIndex + 1) % idleCols;
                playerSprite.setTextureRect(computePlayerTextureRect());
                playerClock.restart();
            }
        }
    }

    // Synchronise l'échelle et position du sprite du joueur pour remplir la taille du RectangleShape joueur
    void Modele::syncPlayerSprite()
    {
        if (playerTexture.getSize().x == 0) return;
        // Taille du rectangle joueur (la Hitbox réduite)
        sf::Vector2f size = joueur.getSize();
        // Taille du recadrage courant (crop)
        sf::IntRect rect = computePlayerTextureRect();
        int cropSize = rect.width; // square crop

        // NE PAS adapter l'échelle du sprite à la taille du rectangle joueur
        // Laisser l'image à l'échelle 1:1 (elle peut donc dépasser du rectangle)
        playerSprite.setScale(2.5f, 2.5f);

        // Origine au centre du recadrage (en coordonnées texture avant scale)
        playerSprite.setOrigin(static_cast<float>(cropSize) * 0.5f, static_cast<float>(cropSize) * 0.5f);

        // Positionner le sprite centré sur le RectangleShape (la Hitbox)
        sf::Vector2f pos = joueur.getPosition();
        playerSprite.setPosition(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
    }

    // Helper: sync playerSprite position to rectangle joueur (call this if joueur moved)
    // We'll update sprite position from Controleur after movement.
    void syncPlayerSpritePosition(Modele& m); // forward decl (no-op here)

    // Accesseurs pour collisionDetectee
    void Modele::setCollisionDetectee(bool v) { collisionDetectee = v; }
    bool Modele::isCollisionDetectee() const { return collisionDetectee; }
    bool Modele::isJoueurDetecte() const
    {
        // Retourne vrai si au moins un ennemi détecte le joueur
        // Chaque ennemi calcule sa propre détection (ex: cône, laser). Ici nous
        // agrégons ces résultats : si un ennemi a son flag `joueurDetecte` à true,
        // le joueur est considéré comme détecté globalement.
        // Note : `joueurDetecte` est mis à jour par `Enemy::detectPlayer()` appelé
        // dans `Modele::updateEnemies()` ; la granularité de détection (frame-based)
        // implique que la réactivité dépend du taux de mise à jour.
        for (const auto& e : enemies)
            if (e->joueurDetecte) return true;
        return false;
    }

    bool Modele::changeRoom(int newRoomIndex, const std::string& entryDirection)
    {
        bool ok = roomManager->changeRoom(newRoomIndex, entryDirection, joueur);
        setCollisionDetectee(false);
        setJoueurDetecte(false);
        if (ok) reloadEnemiesForCurrentRoom(); // Ajout ici
        return ok;
    }
}