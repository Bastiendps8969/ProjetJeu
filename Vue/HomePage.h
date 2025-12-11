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
    class HomePage
    {
    private:
        bool active = true;
        bool inputFocused = false;
        std::string playerName;

        sf::Font font;
        bool fontLoaded = false;

        sf::Text titleText;
        sf::Text titleShadow;

        sf::RectangleShape inputBox;
        sf::Text inputText;
        sf::RectangleShape playButton;
        sf::Text playLabel;

        // New buttons
        sf::RectangleShape scoreButton;
        sf::Text scoreLabel;
        sf::RectangleShape creditsButton;
        sf::Text creditsLabel;
        sf::RectangleShape soundsButton;
        sf::Text soundsLabel;

        bool soundsOn = true;
        bool showCredits = false;

        // cherub image
        sf::Texture cherubTexture;
        sf::Sprite cherubSprite;
        bool cherubLoaded = false;

        // animation
        sf::Clock animClock;

        // callback to obtain player scores (vector<int>)
        std::function<std::vector<int>()> getScoresCb;

        void centerLabel(sf::Text& label, const sf::RectangleShape& button);

        // open a separate window showing total and per-level scores
        void openScoreWindow();
        void openCreditsWindow();

        // open level selector and store selection
        void openLevelPage(sf::RenderWindow& parent);

        // selected indices (chapter, level) after the level selector returns
        int selectedChapter = -1;
        int selectedLevel = -1;

    public:
        // accepts a callback used to fetch current player scores
        HomePage(std::function<std::vector<int>()> getScores, const std::string& backgroundPath = "");

        bool isActive() const { return active; }
        void setActive(bool v) { active = v; }

        const std::string& getPlayerName() const { return playerName; }

        // After the menu is closed, the controller can query which level was chosen
        int getSelectedChapter() const { return selectedChapter; }
        int getSelectedLevel() const { return selectedLevel; }

        void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);
        void draw(sf::RenderWindow& fenetre);
    };
}

#endif //TESTCOLLISION_HOMEPAGE_H