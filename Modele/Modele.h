#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include "../cmake-build-debug/json.hpp"
#include "Agent.h"
#include "RoomManager.h"

namespace Modele
{
    class Modele
    {
    private:
        // Joueur
        sf::RectangleShape joueur;
        // Spritesheet du joueur
        sf::Texture playerTexture;
        sf::Sprite playerSprite;
        int playerFrameCount = 9;      // nombre de frames par ligne (total = frames déplacement + frames idle)
        int playerFrameIndex = 0;      // frame courante
        int playerRow = 3;            // 1=up,2=left,3=down,4=right (par défaut bas)
        float playerFrameDuration = 0.08f; // durée par frame en secondes
        // Durée par frame quand le joueur est au repos (idle) -> plus lente
        float playerIdleFrameDuration = 0.24f; // ex: 3x plus lent que playerFrameDuration
        sf::Clock playerClock;
        int playerTileSize = 64;      // taille d'une frame dans la spritesheet

        // Nombre de frames "idle" (au repos) qui suivent les frames de déplacement dans chaque ligne
        int idleFrameCount = 2;
        // Indique si le joueur est actuellement en mouvement ou en idle (utilisé pour choisir la row des idle)
        bool playerIsMoving = false;
        
        // Contrôles pour zoom texture et taille d'affichage du sprite (séparés X/Y)
        float playerTextureZoom = 1.20f;         // >1 => "zoom" sur la texture (recadrage)
        // Réduire ici la taille du sprite affiché (plus petit que le rectangle joueur)
        float playerSpriteDisplayScaleX = 1.0f; // ex. 0.60 => sprite largeur = 60% de la largeur du rectangle joueur
        float playerSpriteDisplayScaleY = 1.0f; // ex. 0.55 => sprite hauteur = 55% de la hauteur du rectangle joueur

        // Calcule la textureRect actuelle (en tenant compte du zoom et du frame index)
        sf::IntRect computePlayerTextureRect() const;
        // Setters pour ajuster à la volée
        void setPlayerTextureZoom(float z) { playerTextureZoom = std::max(0.1f, z); }
        void setPlayerSpriteDisplayScale(float sx, float sy) { playerSpriteDisplayScaleX = sx; playerSpriteDisplayScaleY = sy; }

        // Sol en tuiles
        sf::Texture floorTexture;
        std::vector<std::vector<int>> floorMatrix; // 2D matrix: 1 = floor tile (Floor5.png)
        int tileSize = 64; // taille en pixels d'une tuile (modifiable)

        // Textures pour murs (8 images Wall1_1 .. Wall1_8)
        std::vector<sf::Texture> wallTextures;

        // --- Membres d'IA et de collision (pour le premier obstacle) ---
        std::unique_ptr<Agent> agent;
        bool collisionDetectee;

        // --- Membres de la carte/pièce ---
        std::unique_ptr<RoomManager> roomManager;
        int currentRoomIndex_;      // <--- Ce membre était celui qui manquait
        float screenW, screenH;

        // Constantes de porte
        const float DOOR_SIZE = 120.f;
        const float DOOR_THICKNESS = 100.f;

    public:
        // Constructeur
        Modele();

        // Destructeur (géré par unique_ptr)
        ~Modele() = default;

        // Getters
        sf::RectangleShape& getJoueur() { return joueur; }
        const std::vector<std::vector<int>>& getFloorMatrix() const { return floorMatrix; }
        const sf::Texture& getFloorTexture() const { return floorTexture; }
        int getTileSize() const { return tileSize; }
        const std::vector<sf::Texture>& getWallTextures() const { return wallTextures; }

        // NOUVEAU: Getters pour les dimensions de l'écran (maintenant dans la classe)
        float getScreenW() const { return roomManager->getScreenW(); }
        float getScreenH() const { return roomManager->getScreenH(); }

        // Méthode pour mettre à jour la position de l'obstacle (logique d'IA)
        void mettreAJourObstacles();

        // Accesseurs pour détection joueur (champ de vision)
        bool isJoueurDetecte() const { return agent ? agent->isJoueurDetecte() : false; }
        void setJoueurDetecte(bool v) { if (agent) agent->setJoueurDetecte(v); }

        // Exposer centre et direction avant d'un obstacle (index par défaut 0)
        sf::Vector2f getObstacleCenter(size_t idx = 0) const { return agent ? agent->getObstacleCenter(idx) : sf::Vector2f(); }
        sf::Vector2f getObstacleForward(size_t idx = 0) const { return agent ? agent->getObstacleForward(idx) : sf::Vector2f(); }

        // Player animation API
        void setPlayerDirection(int row); // 1..4
        void updatePlayerAnimation(bool moving); // avancer l'animation si moving
        const sf::Sprite& getPlayerSprite() const { return playerSprite; }

        // Synchronise l'échelle/position du sprite joueur avec le RectangleShape (taille & position)
        void syncPlayerSprite();

        // Accesseurs pour l'indicateur de collision
        void setCollisionDetectee(bool v);
        bool isCollisionDetectee() const;

        // Expose les obstacles physiques (qui bloquent) de la pièce actuelle
        const std::vector<std::unique_ptr<sf::Shape>>& getObstacleShapes() const {
            return roomManager->getObstacleShapes();
        }

        // Expose les portes de la pièce actuelle
        const std::vector<Door>& getCurrentRoomDoors() const {
            return roomManager->getCurrentRoomDoors();
        }

        // Changement de pièce
        bool changeRoom(int newRoomIndex, const std::string& entryDirection) {
            bool ok = roomManager->changeRoom(newRoomIndex, entryDirection, joueur);
            setCollisionDetectee(false);
            setJoueurDetecte(false);
            return ok;
        }
    };
}