#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

namespace Modele
{
    class Modele
    {
    private:
        // Joueur
        sf::RectangleShape joueur;

        // Vecteur d'obstacles de type pointeur
        std::vector<sf::Shape*> obstacles;

        // Vitesse de déplacement de l'obstacle
        sf::Vector2f obstacleVitesse;

        // Points de patrouille
        std::vector<sf::Vector2f> pointsPatrouille;
        int pointCibleIndex;
        float vitessePatrouille;

        // Indicateur de collision
        bool collisionDetectee;

        // Indicateur détection joueur par le champ de vision de l'obstacle
        bool joueurDetecte;

    public:
        // Constructeur
        Modele();

        // Destructeur car liste de pointeurs à gérer
        ~Modele();

        // Getters
        sf::RectangleShape& getJoueur() { return joueur; }
        const std::vector<sf::Shape*>& getObstacles() const { return obstacles; }

        // Méthode pour mettre à jour la position de l'obstacle
        void mettreAJourObstacles();

        // Accesseurs pour l'indicateur de collision
        void setCollisionDetectee(bool v) { collisionDetectee = v; }
        bool isCollisionDetectee() const { return collisionDetectee; }

        // Accesseurs pour détection joueur (champ de vision)
        void setJoueurDetecte(bool v) { joueurDetecte = v; }
        bool isJoueurDetecte() const { return joueurDetecte; }

        // Exposer centre et direction avant d'un obstacle (index par défaut 0)
        sf::Vector2f getObstacleCenter(size_t idx = 0) const;
        sf::Vector2f getObstacleForward(size_t idx = 0) const;
        // FOV doublé
        float getFovRange() const { return 440.f; }
        float getFovAngleDeg() const { return 60.f; }
    };
}
