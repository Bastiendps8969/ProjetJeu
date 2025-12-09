#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <SFML/System/Clock.hpp>
#include <memory>

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

        // --- NOUVEAU: Nom de la pièce ---
        sf::Text roomNameText;

        // --- NOUVEAU: Nom de la pièce ---
        sf::Text roomNameText;

        // --- Home page moved to HomePage class ---
        std::unique_ptr<HomePage> home;

        // Dialogues chargés depuis un JSON
        std::vector<std::string> dialogues;
        size_t currentDialogueIndex = 0;
        bool dialoguesLoaded = false;
        bool dialogActive = false;
        bool dialogStarted = false;
        // Démarre les dialogues uniquement si la sortie du menu s'est faite par le bouton JOUER
        bool startedByPlayButton = false;
        sf::RectangleShape dialogBox;
        sf::Text dialogText;
        float dialogDisplayDuration = 4.0f; // secondes avant auto-avancer
        sf::Clock dialogClock;

        // Avance au dialogue suivant / ferme la boîte si fini
        void advanceDialogue();

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