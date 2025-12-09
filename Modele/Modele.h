#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

namespace Modele
{
    class Modele
    {
    private:
        // Joueur
        sf::RectangleShape joueur;

        // Vecteur d'obstacles de type pointeur
        std::vector<std::unique_ptr<sf::Shape>> obstacles;

        // Vitesse de déplacement de l'obstacle
        sf::Vector2f obstacleVitesse;

        // Points de patrouille
        std::vector<sf::Vector2f> pointsPatrouille;
        int pointCibleIndex;
        float vitessePatrouille;

        // Indicateur de collision
        bool collisionDetectee = false;

        // Indicateur détection joueur par le champ de vision de l'obstacle
        bool joueurDetecte = false;

        // Flag pour tracker si le dialogue a été lancé
        bool dialogueDeclenche = false; // Flag pour empêcher relance du dialogue

    public:
        // Constructeur
        Modele();

        // Destructeur car liste de pointeurs à gérer
        ~Modele();

        // Getters
        // Retour par référence pour éviter copies coûteuses
        const std::vector<std::unique_ptr<sf::Shape>>& getObstacles() const noexcept { return obstacles; }
        // Retour par référence pour modifier la forme sans copier
        sf::RectangleShape& getJoueur() noexcept { return joueur; }

        // Méthode pour mettre à jour la position de l'obstacle
        void mettreAJourObstacles();

        // Accesseurs pour l'indicateur de collision
        void setCollisionDetectee(bool v) { collisionDetectee = v; }
        bool isCollisionDetectee() const { return collisionDetectee; }

        // Accesseurs pour détection joueur (champ de vision)
        void setJoueurDetecte(bool v) { joueurDetecte = v; }
        bool isJoueurDetecte() const { return joueurDetecte; }

        // Vérifie si le dialogue a été déclenché
        bool hasDialogueTriggered() const { return dialogueDeclenche; }
        // Définit l'état du déclenchement du dialogue
        void setDialogueTriggered(bool v) { dialogueDeclenche = v; }
        // Réinitialise l'état du déclenchement du dialogue
        void resetDialogueTriggered() { dialogueDeclenche = false; }

        // Exposer centre et direction avant d'un obstacle (index par défaut 0)
        sf::Vector2f getObstacleCenter(size_t idx = 0) const;
        sf::Vector2f getObstacleForward(size_t idx = 0) const;
        // FOV doublé
        float getFovRange() const { return 440.f; }
        float getFovAngleDeg() const { return 60.f; }
    };
}
