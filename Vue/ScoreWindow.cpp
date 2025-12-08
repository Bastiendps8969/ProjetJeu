#include "ScoreWindow.h"
#include <numeric>
#include <sstream>

namespace Vue
{
    ScoreWindow::ScoreWindow(std::function<std::vector<int>()> getScores)
        : getScoresCb(std::move(getScores))
    {
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontLoaded = true;
            std::vector<int> scores = getScoresCb();
            initializeTexts(scores);
        }
    }

    void ScoreWindow::initializeTexts(const std::vector<int>& scores)
    {
        // Titre
        titleText.setFont(font);
        titleText.setString("SCORES");
        titleText.setCharacterSize(48);
        titleText.setFillColor(sf::Color::White);
        titleText.setStyle(sf::Text::Bold);
        titleText.setPosition(50.f, 30.f);

        // Score total
        int totalScore = std::accumulate(scores.begin(), scores.end(), 0);
        totalScoreText.setFont(font);
        totalScoreText.setString("Score Total: " + std::to_string(totalScore));
        totalScoreText.setCharacterSize(32);
        totalScoreText.setFillColor(sf::Color::Yellow);
        totalScoreText.setPosition(50.f, 100.f);

        // Scores par niveau
        levelScores.clear();
        float yPos = 180.f;
        for (size_t i = 0; i < scores.size(); ++i)
        {
            sf::Text levelText;
            levelText.setFont(font);

            std::ostringstream oss;
            if (i == 0)
                oss << "Tutorial: " << scores[i];
            else if (i == 1)
                oss << "Test Mission: " << scores[i];
            else
                oss << "Mission " << (i - 1) << ": " << scores[i];

            levelText.setString(oss.str());
            levelText.setCharacterSize(24);
            levelText.setFillColor(sf::Color::White);
            levelText.setPosition(50.f, yPos);
            levelScores.push_back(levelText);
            yPos += 40.f;
        }
    }

    void ScoreWindow::handleEvent(const sf::Event& event)
    {
        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Escape ||
                event.key.code == sf::Keyboard::Return)
            {
                active = false;
            }
        }

        if (event.type == sf::Event::Closed)
        {
            active = false;
        }
    }

    void ScoreWindow::draw(sf::RenderWindow& fenetre)
    {
        fenetre.clear(sf::Color::Black);

        if (fontLoaded)
        {
            fenetre.draw(titleText);
            fenetre.draw(totalScoreText);
            for (const auto& levelText : levelScores)
            {
                fenetre.draw(levelText);
            }
        }

        fenetre.display();
    }
}