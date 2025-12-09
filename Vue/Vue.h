#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <SFML/System/Clock.hpp>

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

        bool isMenuActive() const { return menuActif; }
        void setMenuActive(bool v) { menuActif = v; }
        const std::string& getPlayerName() const { return playerName; }
    };
}