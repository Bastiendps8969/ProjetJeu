#pragma once
//
// Created by bertr on 25-11-25.
//

#ifndef TESTCOLLISION_HOMEPAGE_H
#define TESTCOLLISION_HOMEPAGE_H


#include <SFML/Graphics.hpp>
#include <string>
#include <functional>
#include <vector>

namespace Vue
{
    /**
     * HomePage : menu principal du jeu
     * - Affiche titre, boutons (Play, Scores, Credits, Sounds)
     * - Permet la saisie du nom du joueur
     * - Possède un callback pour récupérer les scores (affichage dans ScoreWindow)
     *
     * Le constructeur accepte un callback pour obtenir les scores et un chemin optionnel
     * pour l'image de fond.
     */
    class HomePage
    {
    private:
        bool active = true;            ///< Bool : la page est-elle active (affichée) ?
        bool inputFocused = false;     ///< Focus d'entrée pour la saisie du nom du joueur
        std::string playerName;        ///< Nom saisi du joueur (max 32 caractères)

        // Police et éléments graphiques
        sf::Font font;
        bool fontLoaded = false;
        sf::Text titleText;
        sf::Text titleShadow;
        sf::RectangleShape inputBox;
        sf::Text inputText;
        sf::RectangleShape playButton;
        sf::Text playLabel;

        // Boutons supplémentaires (Scores, Credits, Sounds)
        sf::RectangleShape scoreButton;
        sf::Text scoreLabel;
        sf::RectangleShape creditsButton;
        sf::Text creditsLabel;
        sf::RectangleShape soundsButton;
        sf::Text soundsLabel;

        bool soundsOn = true;
        bool showCredits = false;

        // Arrière-plan "Cherub" (semi-transparent si présent)
        sf::Texture cherubTexture;
        sf::Sprite cherubSprite;
        bool cherubLoaded = false;

        // Horloge basique d'animation (utilisée si nécessaire)
        sf::Clock animClock;

        // Callback pour obtenir les scores actuels du joueur
        std::function<std::vector<int>()> getScoresCb;

        // UTIL : centrer un texte à l'intérieur d'un bouton
        void centerLabel(sf::Text& label, const sf::RectangleShape& button);

        // Ouvre une fenêtre popup pour afficher les scores
        void openScoreWindow();

        // Ouvre l'écran des crédits
        void openCreditsWindow();

        // Lance le sélecteur de niveau (LevelPage) et enregistre la sélection
        void openLevelPage(sf::RenderWindow& parent);

        // Indices sélectionnés après la fermeture du sélecteur (chapter, level)
        int selectedChapter = -1;
        int selectedLevel = -1;

    public:
        /**
         * Constructeur
         * @param getScores Callback (fonction) retournant un std::vector<int> représentant les scores
         * @param backgroundPath Chemin optionnel vers l'image de fond (des chemins de secours sont testés dans le constructeur)
         */
        HomePage(std::function<std::vector<int>()> getScores, const std::string& backgroundPath = "");

        bool isActive() const { return active; }
        void setActive(bool v) { active = v; }

        const std::string& getPlayerName() const { return playerName; }

        // Après la fermeture du LevelPage, obtenir les indices choisis
        int getSelectedChapter() const { return selectedChapter; }
        int getSelectedLevel() const { return selectedLevel; }

        // Gestion des événements et rendu
        void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);
        void draw(sf::RenderWindow& fenetre);
    };
}

#endif //TESTCOLLISION_HOMEPAGE_H