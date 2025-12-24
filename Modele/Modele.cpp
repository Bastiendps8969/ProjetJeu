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

    // Room loading is handled by RoomManager now.


    // Constructeur : toutes les variables membres sont maintenant initialisées
    Modele::Modele()
    : collisionDetectee(false)
    {
        // Détermine la résolution du bureau
        sf::VideoMode dm = sf::VideoMode::getDesktopMode();
        float screenW = static_cast<float>(dm.width);
        float screenH = static_cast<float>(dm.height);

        roomManager = std::make_unique<RoomManager>(screenW, screenH);
        // map manager for tile/floor handling
        mapManager = std::make_unique<MapManager>();

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
                // assign the texture to the sprite immediately
                playerSprite.setTexture(playerTexture);
                playerSprite.setTextureRect(computePlayerTextureRect());
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

        // Default asset search paths for floors and walls
        const std::vector<std::string> tryFloorPaths = {
            "cmake-build-debug/Asset/Floor/floor_01.png",
            "Asset/Floor/floor_01.png",
            "floor_01.png"
        };
        const std::vector<std::string> tryFloor02Paths = {
            "cmake-build-debug/Asset/Floor/floor_02.png",
            "Asset/Floor/floor_02.png",
            "floor_02.png"
        };
        const std::vector<std::string> tryWallPaths = {
            "cmake-build-debug/Asset/Wall/Wall1_2.png",
            "cmake-build-debug/Asset/Wall/Wall1_3.png",
            "cmake-build-debug/Asset/Wall/Wall1_4.png",
            "cmake-build-debug/Asset/Wall/Wall1_8.png"
        };

        if (mapManager) mapManager->loadDefaults(tryFloorPaths, tryFloor02Paths, tryWallPaths);

        // Wall/floor tile codes moved to MapManager (use Modele::MapManager::TILE_...)

        // floor tile registrations (floor_01 -> 22, floor_02 -> 21) are handled
        // by MapManager::loadDefaults called above.
        // NOTE: Par défaut, nous n'assignons plus de codes de murs (11..18)
        // automatiquement à la matrice de sol. Les murs doivent être
        // explicitement définis par les données de niveau (JSON) ou par
        // une logique dédiée. On laisse donc `floorMatrix` tel quel.

        // Chargement des pièces via RoomManager
        if (roomManager->loadRoomsFromJson("Asset/levels/tutorial/tutorial_1.json") && roomManager->getRooms().count(0))
        {
            roomManager->changeRoom(0, "", joueur);
            // positionner le joueur au centre de l'écran
            joueur.setPosition(roomManager->getScreenW() * 0.5f - boxSize * 0.5f, roomManager->getScreenH() * 0.5f - boxSize * 0.5f);
            // sync sprite position/scale with the rectangle
            syncPlayerSprite();

            // If the room references an external map file, load it into the MapManager
            auto& rooms = roomManager->getRooms();
            auto it = rooms.find(0);
            if (it != rooms.end() && !it->second.mapFile.empty() && mapManager) {
                if (!mapManager->loadMapFromFile(it->second.mapFile)) {
                    std::cerr << "Warning: failed to load map file '" << it->second.mapFile << "' for room 0" << std::endl;
                }
            }

            reloadEnemiesForCurrentRoom();
        }
        else
        {
            std::cerr << "Échec du chargement de la carte. Pièce 0 non valide." << std::endl;
            roomManager->changeRoom(-1, "", joueur);
            joueur.setPosition(roomManager->getScreenW() * 0.5f - boxSize * 0.5f, roomManager->getScreenH() * 0.5f - boxSize * 0.5f);
            // sync sprite position if texture is loaded
            syncPlayerSprite();
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
            // If camera textureName not provided, default to camera_<facing>
            if (type == "camera" && e->textureName.empty()) {
                std::string facingDefault = ed.facing.empty() ? "left" : ed.facing;
                e->textureName = std::string("camera_") + facingDefault;
            }

            // Load texture depending on enemy type. Cameras use their own asset folder
            if (type == "camera") {
                const std::vector<std::string> tryPathsCam = {
                    "cmake-build-debug/Asset/camera/" + e->textureName + ".png",
                    "Asset/camera/" + e->textureName + ".png",
                    "camera/" + e->textureName + ".png",
                    e->textureName + ".png"
                };
                bool camLoaded = false;
                std::cout << "[DEBUG] Chargement texture camera: name='" << e->textureName << "'\n";
                for (const auto& p : tryPathsCam) {
                    std::cout << "[DEBUG]  Trying camera texture path: " << p << std::endl;
                    if (e->texture.loadFromFile(p)) { e->sprite.setTexture(e->texture); camLoaded = true; std::cout << "[DEBUG]   -> Loaded camera texture: " << p << std::endl; break; }
                    else std::cout << "[DEBUG]   -> Failed: " << p << std::endl;
                }
                if (!camLoaded) {
                    std::cerr << "[DEBUG] Avertissement: impossible de charger texture camera pour '" << e->textureName << "'\n";
                } else {
                    // Camera-specific settings: assume static image (no sprite-sheet animation)
                    e->frameCount = 1;
                    e->idleFrameCount = 1;
                    e->frameIndex = 0;
                    e->isMoving = false;

                    sf::Vector2u ts = e->texture.getSize();
                    if (ts.x > 0 && ts.y > 0) {
                        // Use the real texture size for camera sprite rect and origin
                        e->sprite.setTextureRect(sf::IntRect(0, 0, static_cast<int>(ts.x), static_cast<int>(ts.y)));
                        e->sprite.setOrigin(static_cast<float>(ts.x) / 2.f, static_cast<float>(ts.y) / 2.f);
                        e->sprite.setScale(1.f, 1.f);
                        // Keep tileSize in case other logic depends on it, set to min dimension
                        e->tileSize = static_cast<int>(std::min(ts.x, ts.y));
                    } else {
                        e->sprite.setOrigin(e->tileSize / 2.f, e->tileSize / 2.f);
                        e->sprite.setScale(2.5f, 2.5f);
                    }
                    // Apply rotation based on facing (if CameraEnemy was configured)
                    CameraEnemy* cc = dynamic_cast<CameraEnemy*>(e.get());
                    if (cc) {
                        if (cc->facing == "left") e->sprite.setRotation(180.f);
                        else if (cc->facing == "up") e->sprite.setRotation(-90.f);
                        else if (cc->facing == "down") e->sprite.setRotation(90.f);
                        else e->sprite.setRotation(0.f);
                    }
                }
            }
            else {
                // load texture if present (other enemies use Human spritesheets / tiles)
                const std::vector<std::string> tryPathsTex = {
                    "cmake-build-debug/Asset/Human/" + e->textureName + ".png",
                    "Asset/Human/" + e->textureName + ".png",
                    "Human/" + e->textureName + ".png",
                    e->textureName + ".png"
                };
                std::cout << "[DEBUG1] Chargement texture ennemi: type='" << type << "' name='" << e->textureName << "'\n";
                for (const auto& p : tryPathsTex) {
                    std::cout << "[DEBUG1]  Trying enemy texture path: " << p << std::endl;
                    if (e->texture.loadFromFile(p)) { e->sprite.setTexture(e->texture); std::cout << "[DEBUG]   -> Loaded enemy texture: " << p << std::endl; break; }
                    else std::cout << "[DEBUG1]   -> Failed: " << p << std::endl;
                }
                e->sprite.setOrigin(e->tileSize / 2.f, e->tileSize / 2.f);
                e->sprite.setScale(2.5f, 2.5f);
            }

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

    const std::vector<std::unique_ptr<sf::Shape>>& Modele::getObstacleShapes() const
    {
        static const std::vector<std::unique_ptr<sf::Shape>> empty;
        if (!roomManager) return empty;
        auto& rooms = roomManager->getRooms();
        int idx = roomManager->getCurrentRoomIndex();
        auto it = rooms.find(idx);
        if (it != rooms.end()) return it->second.obstacleShapes;
        return empty;
    }

    const std::vector<Door>& Modele::getCurrentRoomDoors() const
    {
        return roomManager->getCurrentRoomDoors();
    }

    std::string Modele::getCurrentRoomName() const
    {
        return roomManager->getCurrentRoomName();
    }

    const std::vector<Objective>& Modele::getCurrentRoomObjectives() const
    {
        static const std::vector<Objective> empty;
        if (!roomManager) return empty;
        auto& rooms = roomManager->getRooms();
        int idx = roomManager->getCurrentRoomIndex();
        auto it = rooms.find(idx);
        if (it != rooms.end()) return it->second.objectives;
        return empty;
    }

    const std::vector<std::unique_ptr<Enemy>>& Modele::getEnemies() const
    {
        return enemies;
    }

    sf::Vector2f Modele::getObstacleCenter(size_t idx) const
    {
        return agent ? agent->getObstacleCenter(idx) : sf::Vector2f();
    }

    sf::Vector2f Modele::getObstacleForward(size_t idx) const
    {
        return agent ? agent->getObstacleForward(idx) : sf::Vector2f();
    }

    float Modele::getScreenW() const { return roomManager ? roomManager->getScreenW() : 0.f; }
    float Modele::getScreenH() const { return roomManager ? roomManager->getScreenH() : 0.f; }

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
    void Modele::setJoueurDetecte(bool v) { joueurDetecte = v; }
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
        if (ok) {
            // Charger la map uniquement si elle existe
            auto& rooms = roomManager->getRooms();
            auto it = rooms.find(newRoomIndex);
            if (it != rooms.end() && !it->second.mapFile.empty()) {
                if (!mapManager->loadMapFromFile(it->second.mapFile)) {
                    std::cerr << "Warning: failed to load map file '"
                              << it->second.mapFile << "' for room "
                              << newRoomIndex << std::endl;
                }
            } else {
                // Aucune mapFile -> on efface la carte
                mapManager->clearMap();
            }

            reloadEnemiesForCurrentRoom();
        }
        return ok;

    }

    // Objective contact accessors
    void Modele::setObjectiveContact(const Objective &obj) {
        objectiveContact = obj;
    }

    void Modele::setObjectiveContactDetectee(const bool b) {
        objectiveContactDetectee = b;
    }

    Objective Modele::getObjectiveContact() const {
        return objectiveContact;
    }

    bool Modele::getObjectiveContactDetectee() const {
        return objectiveContactDetectee;
    }

    bool Modele::setTileTexture(int id, const std::string& path)
    {
        if (!mapManager) return false;
        return mapManager->setTileTexture(id, path);
    }

    const sf::Texture* Modele::getTileTexture(int id) const
    {
        if (!mapManager) return nullptr;
        return mapManager->getTileTexture(id);
    }

    const std::vector<std::vector<int>>& Modele::getFloorMatrix() const
    {
        static const std::vector<std::vector<int>> empty;
        if (!mapManager) return empty;
        return mapManager->getFloorMatrix();
    }

    const sf::Texture& Modele::getFloorTexture() const
    {
        static sf::Texture dummy;
        if (!mapManager) return dummy;
        return mapManager->getFloorTexture();
    }

    void Modele::setFloorMatrix(const std::vector<std::vector<int>>& m)
    {
        if (mapManager) mapManager->setFloorMatrix(m);
    }

    int Modele::getTileSize() const
    {
        return mapManager ? mapManager->getTileSize() : 0;
    }

    const std::vector<sf::Texture>& Modele::getWallTextures() const
    {
        static const std::vector<sf::Texture> empty;
        if (!mapManager) return empty;
        return mapManager->getWallTextures();
    }
}