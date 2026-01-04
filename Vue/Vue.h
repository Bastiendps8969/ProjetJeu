#pragma once
#include <SFML/Graphics.hpp>
#include <string>

namespace Modele { class Modele; }

namespace Vue
{
    class Vue
    {
    private:
        Modele::Modele& modele;

        // Police et texte pour le message de collision
        sf::Font font;
        sf::Text collisionText;
        bool fontCharge = false;

        // Texte pour détection joueur
        sf::Text joueurDetecteText;

        // Texte pour les vies
        sf::Text livesText;

        // Game Over display is handled by `Vue::GameOverPage` (separate component).
        // The view no longer owns Game Over textures or prompt text.

    public:
        // Constructeur
        Vue(Modele::Modele& modele);

        // Méthode de dessin
        void dessiner(sf::RenderWindow& fenetre);

        // Gestion des évènements (texte, clics pour le menu)
        void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);
    };
}