#pragma once
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

        // callback to obtain player scores (vector<int>)
        std::function<std::vector<int>()> getScoresCb;

        void centerLabel(sf::Text& label, const sf::RectangleShape& button);

        // open a separate window showing total and per-level scores
        void openScoreWindow();
        void openCreditsWindow();

    public:
        // accepts a callback used to fetch current player scores
        HomePage(std::function<std::vector<int>()> getScores);

        bool isActive() const { return active; }
        void setActive(bool v) { active = v; }

        const std::string& getPlayerName() const { return playerName; }

        void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);
        void draw(sf::RenderWindow& fenetre);
    };
}