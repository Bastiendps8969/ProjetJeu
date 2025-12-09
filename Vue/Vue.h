#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

#include "HomePage.h"   // <-- necessary so std::unique_ptr<HomePage> sees a complete type

namespace Modele { class Modele; }

namespace Vue
{
    // class HomePage; // forward no longer required

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

        // --- Home page moved to HomePage class ---
        std::unique_ptr<HomePage> home;

    public:
        // Constructeur
        Vue(Modele::Modele& modele);

        // Méthode de dessin
        void dessiner(sf::RenderWindow& fenetre);

        // Gestion des évènements (texte, clics pour le menu)
        void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);

        bool isMenuActive() const;
        void setMenuActive(bool v);
        const std::string& getPlayerName() const;
    };
}