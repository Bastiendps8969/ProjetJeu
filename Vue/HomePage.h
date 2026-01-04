
#pragma once
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
        bool active = true;           ///< Whether this page is currently active (displayed)
        bool inputFocused = false;    ///< Focus flag for typing the player name
        std::string playerName;       ///< Typed player name (max 32 chars)

        // Font and graphic elements (owned by HomePage = composition).
        sf::Font font;
        bool fontLoaded = false;

        // Title and shadow (for a "neon" style).
        sf::Text titleText;
        sf::Text titleShadow;

        // Input box + text.
        // NOTE: In current implementation, inputBox is size (0,0) and mainly used for hit-testing.
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

        bool showCredits = false;

        // Credits window owned by HomePage.
        // WHY unique_ptr:
        // - expresses unique ownership (HomePage controls lifetime)
        // - allows "optional" existence (nullptr when not created yet)
        // - automatic destruction (no manual delete)
        std::unique_ptr<CreditsWindow> creditsWindow;

        // Background image (owned by HomePage = composition).
        sf::Texture cherubTexture;
        sf::Sprite cherubSprite;
        bool cherubLoaded = false;

        // Basic animation clock (available if needed).
        sf::Clock animClock;

        // Callback to query current player scores.
        // WHY std::function stored by value:
        // - can hold any callable (lambda, bind, functor)
        // - HomePage does NOT own the score system; it owns only the callable wrapper
        //   (aggregation-like dependency injection)
        std::function<std::vector<int>()> getScoresCb;

        // Utility: center a label inside a rectangle button.
        // WHY label by non-const reference:
        // - we modify its position
        // WHY button by const reference:
        // - read-only and avoids copying shapes
        void centerLabel(sf::Text& label, const sf::RectangleShape& button);

        // Open a dedicated score window (ScoreWindow).
        void openScoreWindow();

        // Open credits screen (currently stubbed / handled via CreditsWindow pointer).
        void openCreditsWindow();

        // Launch LevelPage selector and store the chosen chapter/level indices.
        // WHY parent window by reference:
        // - caller owns the window; we only use it as context (non-owning)
        void openLevelPage(sf::RenderWindow& parent);

        // Selected index for keyboard navigation: 0=Play,1=Scores,2=Credits
        int selectedIndex = 0;

        // Selected indices after closing the selector (chapter, level)
        int selectedChapter = -1;
        int selectedLevel = -1;
        std::string selectedLevelData;

    public:
        /**
         * Constructor
         * @param getScores Callback returning a vector of scores
         * @param backgroundPath Optional path to background image (fallback paths are tried)
         *
         * WHY getScores passed by value:
         * - allows easy passing of temporary lambdas/functors
         * - we can std::move() it into the member to avoid extra copies
         *
         * WHY backgroundPath by const reference:
         * - avoids copying the string when caller already has one
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
        // WHY event by const reference:
        // - event is read-only, avoids copying
        // WHY window by non-const reference:
        // - coordinate mapping and (in draw) rendering mutate window state
        void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);
        void draw(sf::RenderWindow& fenetre);
    };
}

#endif // TESTCOLLISION_HOMEPAGE_H
