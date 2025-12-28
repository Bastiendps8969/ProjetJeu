#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>

namespace Vue
{
    class ScoreWindow
    {
    private:
        bool active = true;
        sf::Font font;
        bool fontLoaded = false;

        sf::Text titleText;
        sf::Text totalScoreText;
        std::vector<sf::Text> levelScores;

        std::function<std::vector<int>()> getScoresCb;

        void initializeTexts(const std::vector<int>& scores);

        sf::FloatRect backButtonRect; // Added member variable for back button rectangle

    public:
        ScoreWindow(std::function<std::vector<int>()> getScores);

        bool isActive() const { return active; }
        void setActive(bool v) { active = v; }

        void handleEvent(const sf::Event& event);
        void draw(sf::RenderWindow& fenetre);
    };
}