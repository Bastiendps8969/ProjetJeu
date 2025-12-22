#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include "../Modele/ScoreCalculator.h"

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

        // Detailed score breakdown texts
        sf::Text timeScoreText;
        sf::Text primaryScoreText;
        sf::Text secondaryScoreText;
        sf::Text detectionCountText;
        sf::Text detectionMalusText;

        sf::RectangleShape backButton;
        sf::Text backButtonLabel;

        std::function<std::vector<int>()> getScoresCb;
        std::function<Modele::ScoreDetails()> getDetailsCb;

        void initializeTexts(const std::vector<int>& scores);
        void initializeDetailedTexts(const Modele::ScoreDetails& details);

        sf::FloatRect backButtonRect; // Added member variable for back button rectangle

    public:
        // Legacy constructor for backward compatibility
        ScoreWindow(std::function<std::vector<int>()> getScores);

        // New constructor for level completion with detailed scoring
        ScoreWindow(std::function<Modele::ScoreDetails()> getDetails);

        bool isActive() const { return active; }
        void setActive(bool v) { active = v; }

        void handleEvent(const sf::Event& event);
        void draw(sf::RenderWindow& fenetre);
    };
}