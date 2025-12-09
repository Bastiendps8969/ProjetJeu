#include "Modele.h"
#include <cmath>
#include <limits>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

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
    : currentRoomIndex_(-1),
      collisionDetectee(false),
      joueurDetecte(false),
      pointCibleIndex(0),
      vitessePatrouille(0.125f),
      obstacleVitesse(0.3f, 0.2f)
    {
        // Détermine la résolution du bureau
        sf::VideoMode dm = sf::VideoMode::getDesktopMode();
        screenW = static_cast<float>(dm.width);
        screenH = static_cast<float>(dm.height);

        float boxSize = std::max(8.f, std::min(screenW, screenH) * 0.08f);

        // Création du rectangle joueur
        float playerCollisionW = boxSize * playerSpriteDisplayScaleX;
        float playerCollisionH = boxSize * playerSpriteDisplayScaleY;
        const float HITBOX_REDUCTION_FACTOR = 0.8f;
        joueur.setSize(sf::Vector2f(
            boxSize * HITBOX_REDUCTION_FACTOR,
            boxSize * HITBOX_REDUCTION_FACTOR
        ));
        joueur.setFillColor(sf::Color::Green);


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

        // Remplissage des bords avec permutation demandée :
        // - bord haut <- ancienne droite
        // - bord gauche <- ancien haut
        // - bord bas <- ancien gauche
        // - bord droit <- ancien bas
        // - coins : échanger coin haut-droit <-> coin bas-gauche
        for (int r = 0; r < rows; ++r)
        {
            for (int c = 0; c < cols; ++c)
            {
                // coins particuliers (on applique l'échange TR <-> BL)
                if (r == 0 && c == 0)
                {
                    // coin haut-gauche : on conserve TL
                    floorMatrix[r][c] = WALL_TL;
                }
                else if (r == 0 && c == cols - 1)
                {
                    // coin haut-droit devient coin bas-gauche (swap demandé)
                    floorMatrix[r][c] = WALL_BL;
                }
                else if (r == rows - 1 && c == 0)
                {
                    // coin bas-gauche devient coin haut-droit (swap demandé)
                    floorMatrix[r][c] = WALL_TR;
                }
                else if (r == rows - 1 && c == cols - 1)
                {
                    // coin bas-droit : on conserve BR
                    floorMatrix[r][c] = WALL_BR;
                }
                else if (r == 0)
                {
                    // bord haut -> prendre l'ancienne droite
                    floorMatrix[r][c] = WALL_RIGHT;
                }
                else if (r == rows - 1)
                {
                    // bord bas -> prendre l'ancienne gauche
                    floorMatrix[r][c] = WALL_LEFT;
                }
                else if (c == 0)
                {
                    // bord gauche -> prendre l'ancienne haut
                    floorMatrix[r][c] = WALL_TOP;
                }
                else if (c == cols - 1)
                {
                    // bord droit -> prendre l'ancienne bas
                    floorMatrix[r][c] = WALL_BOTTOM;
                }
                // else: leave as 1 (floor)
            }
        }

        // Chargement des pièces
        if (loadRoomsFromJson("rooms.json") && rooms_.count(0))
        {
            currentRoomIndex_ = 0;
            joueur.setPosition(screenW * 0.5f - boxSize * 0.5f, screenH * 0.5f - boxSize * 0.5f);
        }
        else
        {
            std::cerr << "Échec du chargement de la carte. Pièce 0 non valide." << std::endl;
            currentRoomIndex_ = -1;
            joueur.setPosition(screenW * 0.5f - boxSize * 0.5f, screenH * 0.5f - boxSize * 0.5f);
        }

        // Initialisation des points de patrouille (si non chargés par JSON)
        pointsPatrouille = {
            sf::Vector2f(screenW * 0.125f, screenH * 0.1666667f),
            sf::Vector2f(screenW * 0.75f,  screenH * 0.1666667f),
            sf::Vector2f(screenW * 0.75f,  screenH * 0.6666667f),
            sf::Vector2f(screenW * 0.125f, screenH * 0.6666667f)
        };
    }


    // Retourne les obstacles physiques (qui bloquent) de la pièce actuelle
    const std::vector<std::unique_ptr<sf::Shape>>& Modele::getObstacleShapes() const
    {
        static const std::vector<std::unique_ptr<sf::Shape>> emptyShapes;
        auto it = rooms_.find(currentRoomIndex_);
        if (it != rooms_.end())
        {
            return it->second.obstacleShapes;
        }
        return emptyShapes;
    }

    // Retourne les définitions de portes de la pièce actuelle (corrigé pour utiliser getObstacleShapes)
    const std::vector<Door>& Modele::getCurrentRoomDoors() const
    {
        static const std::vector<Door> emptyDoors;
        auto it = rooms_.find(currentRoomIndex_);
        if (it != rooms_.end())
        {
            return it->second.doors;
        }
        return emptyDoors;
    }

    // Retourne le nom de la pièce actuelle
    std::string Modele::getCurrentRoomName() const
    {
        auto it = rooms_.find(currentRoomIndex_);
        if (it != rooms_.end())
        {
            return it->second.name;
        }
        return "Pièce inconnue (ID:" + std::to_string(currentRoomIndex_) + ")";
    }

    // Changement de pièce (inchangé)
    bool Modele::changeRoom(int newRoomIndex, const std::string& entryDirection)
    {
        if (currentRoomIndex_ == newRoomIndex) return true;

        auto it = rooms_.find(newRoomIndex);
        if (it == rooms_.end())
        {
            std::cerr << "Erreur: Pièce cible " << newRoomIndex << " introuvable." << std::endl;
            return false;
        }

        currentRoomIndex_ = newRoomIndex;

        // Repositionner le joueur
        float playerW = joueur.getSize().x;
        float playerH = joueur.getSize().y;
        float halfW = playerW * 0.5f;
        float halfH = playerH * 0.5f;

        // Repositionne le joueur à la sortie de la porte opposée
        if (entryDirection == "up")
        {
            joueur.setPosition(screenW / 2.f - halfW, DOOR_THICKNESS + DOOR_MARGIN);
        }
        else if (entryDirection == "down")
        {
            joueur.setPosition(screenW / 2.f - halfW, screenH - DOOR_THICKNESS - DOOR_MARGIN - playerH);
        }
        else if (entryDirection == "left")
        {
            joueur.setPosition(DOOR_THICKNESS + DOOR_MARGIN, screenH / 2.f - halfH);
        }
        else if (entryDirection == "right")
        {
            joueur.setPosition(screenW - DOOR_THICKNESS - DOOR_MARGIN - playerW, screenH / 2.f - halfH);
        }
        else
        {
            joueur.setPosition(screenW * 0.5f - halfW, screenH * 0.5f - halfH);
        }

        setCollisionDetectee(false);
        setJoueurDetecte(false);

        return true;
    }


    // Mise à jour de la logique d'IA des obstacles (Corrigé pour utiliser la nouvelle structure)
    void Modele::mettreAJourObstacles()
    {
        const auto& currentObstacleShapes = getObstacleShapes();
        if (currentObstacleShapes.empty() || currentObstacleShapes[0] == nullptr)
        {
            joueurDetecte = false;
            return;
        }

        // Le reste de la logique de détection reste inchangé (utilise le premier obstacle)
        // ... (Logique de patrouille et de détection non modifiée)
        sf::FloatRect joueurBounds = joueur.getGlobalBounds();
        sf::Vector2f joueurCenter = sf::Vector2f(joueurBounds.left + joueurBounds.width * 0.5f,
                                                 joueurBounds.top + joueurBounds.height * 0.5f);

        const float fovAngle = 60.0f;
        const float fovRange = 440.0f;
        const float cosHalfFov = std::cos(fovAngle * 0.5f * (3.14159265f / 180.f));

        sf::Vector2f center = getObstacleCenter(0);
        sf::Vector2f forward = getObstacleForward(0);

        sf::Vector2f toJoueur = joueurCenter - center;
        float distJ = std::sqrt(toJoueur.x * toJoueur.x + toJoueur.y * toJoueur.y);

        if (distJ <= 0.0f) joueurDetecte = true;
        else {
            sf::Vector2f toJNorm = sf::Vector2f(toJoueur.x / distJ, toJoueur.y / distJ);
            float dot = forward.x * toJNorm.x + forward.y * toJNorm.y;
            joueurDetecte = (distJ <= fovRange && dot >= cosHalfFov);
        }
    }

    // Renvoie le centre (en pixels) de l'obstacle idx
    sf::Vector2f Modele::getObstacleCenter(size_t idx) const
    {
        const auto& currentShapes = getObstacleShapes();
        if (idx >= currentShapes.size() || currentShapes[idx] == nullptr)
            return sf::Vector2f(0.f, 0.f);
        sf::FloatRect b = currentShapes[idx]->getGlobalBounds();
        return sf::Vector2f(b.left + b.width * 0.5f, b.top + b.height * 0.5f);
    }

    // Renvoie la direction normalisée vers la cible actuelle pour l'obstacle idx
    sf::Vector2f Modele::getObstacleForward(size_t idx) const
    {
        const auto& currentShapes = getObstacleShapes();
        if (idx >= currentShapes.size() || currentShapes[idx] == nullptr)
            return sf::Vector2f(1.f, 0.f);

        if (pointsPatrouille.empty()) return sf::Vector2f(1.f, 0.f);

        // Cette partie est simplifiée car les obstacles ne bougent pas encore en fonction de la pièce
        sf::Vector2f pos = currentShapes[idx]->getPosition();
        sf::Vector2f cible = pointsPatrouille[pointCibleIndex];

        sf::Vector2f direction = cible - pos;
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance > 0.0f)
            return sf::Vector2f(direction.x / distance, direction.y / distance);
        else
            return sf::Vector2f(1.f, 0.f);
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

        // Taille affichée souhaitée (maintenant égale à la taille de la Hitbox puisque scale = 1.0f)
        float displayW = size.x * playerSpriteDisplayScaleX;
        float displayH = size.y * playerSpriteDisplayScaleY;

        // Calculer l'échelle nécessaire pour mapper la zone recadrée (cropSize) à la taille affichée
        float sx = displayW / static_cast<float>(cropSize);
        float sy = displayH / static_cast<float>(cropSize);
        playerSprite.setScale(sx, sy);

        // Origine au centre du recadrage (en coordonnées texture avant scale)
        playerSprite.setOrigin(static_cast<float>(cropSize) * 0.5f, static_cast<float>(cropSize) * 0.5f);

        // Positionner le sprite centré sur le RectangleShape (la Hitbox)
        sf::Vector2f pos = joueur.getPosition();
        playerSprite.setPosition(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
    }

    // Helper: sync playerSprite position to rectangle joueur (call this if joueur moved)
    // We'll update sprite position from Controleur after movement.
    void syncPlayerSpritePosition(Modele& m); // forward decl (no-op here)
}