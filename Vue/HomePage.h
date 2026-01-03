
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
#include <memory>
#include "CreditsWindow.h"

namespace Vue
{
    /**
     * HomePage: main menu of the game.
     * - Displays title + buttons (Play, Scores, Credits, Sounds)
     * - Allows player name input (max 32 chars)
     * - Receives a callback to retrieve scores (displayed in ScoreWindow)
     *
     * The constructor accepts a score callback and an optional background image path.
     */
    class HomePage
    {
    private:
        // Modal state: when false, the menu loop can exit.
        bool active = true; ///< Is the page active (displayed) ?

        // Player name input state.
        bool inputFocused = false; ///< Whether name input is focused
        std::string playerName;    ///< Player name (max 32 chars)

        // Font and UI elements.
        sf::Font font;
        bool fontLoaded = false;

        // Title and shadow (for a "neon" style).
        sf::Text titleText;
        sf::Text titleShadow;

        // Input box + text.
        // Note: in the current implementation, inputBox is size (0,0) and mainly used for hit-testing.
        sf::RectangleShape inputBox;
        sf::Text inputText;

        // Main Play button.
        sf::RectangleShape playButton;
        sf::Text playLabel;

        // Additional buttons (Scores, Credits, Sounds).
        sf::RectangleShape scoreButton;
        sf::Text scoreLabel;

        sf::RectangleShape creditsButton;
        sf::Text creditsLabel;

        sf::RectangleShape soundsButton;
        sf::Text soundsLabel;

        // Simple sounds toggle state.
        bool soundsOn = true;

        // Legacy credits overlay flag (separate from CreditsWindow).
        bool showCredits = false;

        // Credits window (modal overlay managed with RAII).
        std::unique_ptr<CreditsWindow> creditsWindow;

        // Cherub background image (semi-transparent if present).
        sf::Texture cherubTexture;
        sf::Sprite cherubSprite;
        bool cherubLoaded = false;

        // Basic animation clock (available if needed).
        sf::Clock animClock;

        // Callback to query current player scores (used by ScoreWindow and LevelPage locks).
        std::function<std::vector<int>()> getScoresCb;

        // Utility: center a label inside a rectangle button.
        void centerLabel(sf::Text& label, const sf::RectangleShape& button);

        // Open a dedicated score window (ScoreWindow).
        void openScoreWindow();

        // Open credits screen (currently stubbed / handled via CreditsWindow pointer).
        void openCreditsWindow();

        // Launch LevelPage selector and store the chosen chapter/level indices.
        void openLevelPage(sf::RenderWindow& parent);

        // Selected indices after closing LevelPage.
        int selectedChapter = -1;
        int selectedLevel = -1;
        std::string selectedLevelData;

    public:
        /**
         * Constructor
         * @param getScores Callback returning a vector of scores
         * @param backgroundPath Optional path to background image (fallback paths are tried)
         */
        HomePage(std::function<std::vector<int>()> getScores, const std::string& backgroundPath = "");

        bool isActive() const { return active; }
        void setActive(bool v) { active = v; }

        const std::string& getPlayerName() const { return playerName; }

        // After closing LevelPage, get chosen indices and optional level JSON path.
        int getSelectedChapter() const { return selectedChapter; }
        int getSelectedLevel() const { return selectedLevel; }
        const std::string& getSelectedLevelData() const { return selectedLevelData; }

        // Input handling and rendering.
        void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);
        void draw(sf::RenderWindow& fenetre);
    };
}

#endif //TESTCOLLISION_HOMEPAGE_H
