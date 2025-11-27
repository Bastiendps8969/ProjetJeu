#pragma once
#include <SFML/Graphics.hpp>

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

        // --- NOUVEAU: Nom de la pièce ---
        sf::Text roomNameText;

        // --- Menu de démarrage ---
        bool menuActif = true;
        std::string playerName;
        sf::RectangleShape inputBox;
        sf::Text inputText;
        sf::RectangleShape playButton;
        sf::Text playLabel;
        sf::Text titleText;

    public:
        // Constructeur
        Vue(Modele::Modele& modele);

        // Méthode de dessin
        void dessiner(sf::RenderWindow& fenetre);

        // Gestion des évènements (texte, clics pour le menu)
        void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);

        bool isMenuActive() const { return menuActif; }
        void setMenuActive(bool v) { menuActif = v; }
        const std::string& getPlayerName() const { return playerName; }
    };
}