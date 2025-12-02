#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <string>
#include <memory>

// NÉCESSAIRE: L'import spécifié par l'utilisateur
#include "../cmake-build-debug/json.hpp"

namespace Modele
{
    // Définition simplifiée d'un obstacle pour le JSON
    struct ObstacleDefinition
    {
        std::string type;
        sf::Vector2f position;
        sf::Vector2f size;
    };

    // Définition d'une porte et de la pièce cible
    struct Door
    {
        std::string direction;
        int targetRoomIndex;
        sf::FloatRect bounds;
        // Utilise unique_ptr pour la forme visuelle de la porte
        std::unique_ptr<sf::Shape> visualShape;
    };

    // Définition de la pièce
    struct Room
    {
        std::string name;
        std::vector<ObstacleDefinition> obstacleDefs;
        std::vector<Door> doors;
        // Utilise unique_ptr pour les formes d'obstacles physiques (murs, etc.)
        std::vector<std::unique_ptr<sf::Shape>> obstacleShapes;
    };

    class Modele
    {
    private:
        // Joueur
        sf::RectangleShape joueur;
        // Sol en tuiles
        sf::Texture floorTexture;
        std::vector<std::vector<int>> floorMatrix; // 2D matrix: 1 = floor tile (Floor5.png)
        int tileSize = 64; // taille en pixels d'une tuile (modifiable)

        // Textures pour murs (8 images Wall1_1 .. Wall1_8)
        std::vector<sf::Texture> wallTextures;

        // --- Membres d'IA et de collision (pour le premier obstacle) ---
        sf::Vector2f obstacleVitesse;
        std::vector<sf::Vector2f> pointsPatrouille;
        int pointCibleIndex;
        float vitessePatrouille;
        bool collisionDetectee; // <--- Ce membre était celui qui manquait
        bool joueurDetecte;     // <--- Ce membre était celui qui manquait

        // --- Membres de la carte/pièce ---
        std::map<int, Room> rooms_; // <--- Ce membre était celui qui manquait
        int currentRoomIndex_;      // <--- Ce membre était celui qui manquait
        float screenW, screenH;

        // Méthodes privées
        bool loadRoomsFromJson(const std::string& filename);
        void initializeRoomShapes(Room& room);

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

        // Retourne les obstacles physiques (qui bloquent)
        const std::vector<std::unique_ptr<sf::Shape>>& getObstacleShapes() const;
        const std::vector<Door>& getCurrentRoomDoors() const;
        std::string getCurrentRoomName() const;

        // NOUVEAU: Getters pour les dimensions de l'écran (maintenant dans la classe)
        float getScreenW() const { return screenW; }
        float getScreenH() const { return screenH; }

        // Méthode pour mettre à jour la position de l'obstacle (logique d'IA)
        void mettreAJourObstacles(); // <--- Cette déclaration était manquante

        // Accesseurs pour l'indicateur de collision
        void setCollisionDetectee(bool v) { collisionDetectee = v; } // <--- Corrigé
        bool isCollisionDetectee() const { return collisionDetectee; } // <--- Corrigé

        // Accesseurs pour détection joueur (champ de vision)
        void setJoueurDetecte(bool v) { joueurDetecte = v; }
        bool isJoueurDetecte() const { return joueurDetecte; } // <--- Corrigé

        // Exposer centre et direction avant d'un obstacle (index par défaut 0)
        sf::Vector2f getObstacleCenter(size_t idx = 0) const; // <--- Corrigé
        sf::Vector2f getObstacleForward(size_t idx = 0) const; // <--- Corrigé

        // Changement de pièce
        bool changeRoom(int newRoomIndex, const std::string& entryDirection);
    };
}