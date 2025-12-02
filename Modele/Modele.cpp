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
        joueur.setSize(sf::Vector2f(boxSize, boxSize));
        joueur.setFillColor(sf::Color::Green);

        // --- Chargement de la texture de sol (Floor5.png) ---
        // On tente quelques chemins relatifs usuels. Adaptez si nécessaire.
        bool loaded = false;
        const std::vector<std::string> tryPaths = {
            "cmake-build-debug/Asset/Floor/Floor5.png",
            "Asset/Floor/Floor5.png",
            "Floor5.png"
        };
        for (const auto& p : tryPaths) {
            if (floorTexture.loadFromFile(p)) { loaded = true; break; }
        }
        if (!loaded) {
            std::cerr << "Avertissement: impossible de charger Floor5.png. Vérifiez le chemin.\n";
        }

        // --- Chargement des textures de murs (Wall1_1 .. Wall1_8) ---
        // Hypothèse d'association (si différent, réordonnez tryPathsWall) :
        // Wall1_1 = coin supérieur gauche
        // Wall1_5 = mur supérieur (horizontal)
        // Wall1_6 = coin supérieur droit
        // Wall1_7 = mur gauche (vertical)  <-- hypothèse (le user a indiqué "mur droite" deux fois)
        // Wall1_2 = mur droit (vertical)
        // Wall1_3 = coin inférieur gauche
        // Wall1_4 = mur inférieur (horizontal)
        // Wall1_8 = coin inférieur droit
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
            if (!wallTextures[i].loadFromFile(tryPathsWall[i]))
            {
                // Essayer chemins alternatifs courts si besoin
                std::string shortPath = "Asset/Wall/Wall1_" + std::to_string(i+1) + ".png";
                if (!wallTextures[i].loadFromFile(shortPath))
                {
                    std::cerr << "Avertissement: impossible de charger " << tryPathsWall[i] << " ni " << shortPath << "\n";
                }
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
}