#include "ScoreWindow.h"
#include <numeric>
#include <sstream>
#include <algorithm>

namespace Vue
{
    // petit helper pour dessiner un bouton stylé cohérent avec le reste
    static void drawStyledButton(sf::RenderWindow& window, const sf::RectangleShape& button, sf::Text label, bool hovered)
    {
        sf::RectangleShape base = button;
        sf::Color baseColor = hovered ? sf::Color(230, 60, 60) : sf::Color(170, 30, 30);
        base.setFillColor(baseColor);
        base.setOutlineColor(hovered ? sf::Color(255, 120, 80) : sf::Color(200, 80, 60));
        base.setOutlineThickness(hovered ? 4.f : 2.f);
        window.draw(base);

        // shine effect (subtile)
        sf::RectangleShape shine({button.getSize().x * 0.92f, button.getSize().y * 0.28f});
        shine.setOrigin(shine.getSize().x * 0.5f, 0.f);
        shine.setPosition(button.getPosition().x, button.getPosition().y - button.getSize().y * 0.5f + (button.getSize().y * 0.14f));
        shine.setFillColor(hovered ? sf::Color(255, 180, 140, 110) : sf::Color(255, 150, 120, 70));
        window.draw(shine);

        // glow
        if (hovered)
        {
            sf::RectangleShape glow = button;
            glow.setFillColor(sf::Color::Transparent);
            glow.setOutlineColor(sf::Color(255, 160, 110, 200));
            glow.setOutlineThickness(6.f);
            window.draw(glow);
        }

        // draw centered label
        sf::FloatRect lb = label.getLocalBounds();
        label.setPosition(
            button.getPosition().x + (button.getSize().x - lb.width) / 2.f - lb.left,
            button.getPosition().y + (button.getSize().y - lb.height) / 2.f - lb.top
        );
        label.setFillColor(hovered ? sf::Color(255, 250, 240) : sf::Color(255, 220, 200));
        window.draw(label);
    }

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

    ScoreWindow::ScoreWindow(std::function<Modele::ScoreDetails()> getDetails)
        : getDetailsCb(std::move(getDetails))
    {
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontLoaded = true;
            Modele::ScoreDetails details = getDetailsCb();
            initializeDetailedTexts(details);
        }
    }

    void ScoreWindow::initializeDetailedTexts(const Modele::ScoreDetails& details)
    {
        // Titre
        titleText.setFont(font);
        titleText.setString("NIVEAU COMPLETE!");
        titleText.setCharacterSize(48);
        titleText.setFillColor(sf::Color(220, 30, 30));
        titleText.setStyle(sf::Text::Bold);

        // Score total (principale)
        totalScoreText.setFont(font);
        totalScoreText.setString("Score Total: " + std::to_string(details.totalScore));
        totalScoreText.setCharacterSize(32);
        totalScoreText.setFillColor(sf::Color(255, 200, 140));

        // Clear previous scores
        levelScores.clear();

        // Time remaining score
        timeScoreText.setFont(font);
        std::ostringstream str1;
        str1 << "Remaining time: " << details.secondsRemaining << " s x 50 = " << details.timeScore;
        timeScoreText.setString(str1.str());
        timeScoreText.setCharacterSize(22);
        timeScoreText.setFillColor(sf::Color(100, 200, 255));
        levelScores.push_back(timeScoreText);

        // Primary objectives score
        primaryScoreText.setFont(font);
        std::ostringstream str2;
        str2 << "Primary objectives: " << details.primaryObjectivesCompleted << " x 10000 = " << details.primaryScore;
        primaryScoreText.setString(str2.str());
        primaryScoreText.setCharacterSize(22);
        primaryScoreText.setFillColor(sf::Color(150, 255, 150));
        levelScores.push_back(primaryScoreText);

        // Secondary objectives score
        secondaryScoreText.setFont(font);
        std::ostringstream str3;
        str3 << "Secondary objectives: " << details.secondaryObjectivesCompleted << " x 5000 = " << details.secondaryScore;
        secondaryScoreText.setString(str3.str());
        secondaryScoreText.setCharacterSize(22);
        secondaryScoreText.setFillColor(sf::Color(255, 200, 100));
        levelScores.push_back(secondaryScoreText);

        detectionMalusText.setFont(font);
        std::ostringstream str4;
        str4 << "Detection malus: " << details.numberOfDetections << " x -2000 = " << details.detectionMalus;
        detectionMalusText.setString(str4.str());
        detectionMalusText.setCharacterSize(22);
        detectionMalusText.setFillColor(sf::Color(220, 140, 140));
        levelScores.push_back(detectionMalusText);
    }

    void ScoreWindow::initializeTexts(const std::vector<int>& scores)
    {
        // Titre
        titleText.setFont(font);
        titleText.setString("SCORES");
        titleText.setCharacterSize(48);
        titleText.setFillColor(sf::Color(220, 30, 30));
        titleText.setStyle(sf::Text::Bold);

        // Score total
        int totalScore = std::accumulate(scores.begin(), scores.end(), 0);
        totalScoreText.setFont(font);
        totalScoreText.setString("Score Total: " + std::to_string(totalScore));
        totalScoreText.setCharacterSize(28);
        totalScoreText.setFillColor(sf::Color(255, 200, 140));

        // Scores par niveau (les textes seront positionnés lors du draw)
        levelScores.clear();
        for (size_t i = 0; i < scores.size(); ++i)
        {
            sf::Text levelText;
            levelText.setFont(font);

            std::ostringstream str;
            if (i == 0)
                str << "Tutorial: " << scores[i];
            else
                str << "Level " << i << ": " << scores[i];

            levelText.setString(str.str());
            levelText.setCharacterSize(20);
            levelText.setFillColor(sf::Color(200, 200, 200));
            levelScores.push_back(levelText);
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

        // handle mouse click on back button
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            // use the backButtonRect set by the last draw()
            if (backButtonRect.contains(static_cast<float>(event.mouseButton.x),
                                        static_cast<float>(event.mouseButton.y)))
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
        // Layout
        sf::Vector2u win = fenetre.getSize();
        fenetre.clear(sf::Color(18, 18, 18)); // dark background matching other menus

        if (!fontLoaded)
        {
            fenetre.display();
            return;
        }

        // Title & positions
        float leftX = 40.f;
        float topY = 30.f;
        titleText.setPosition(leftX, topY);
        fenetre.draw(titleText);

        totalScoreText.setPosition(leftX, topY + 70.f);
        fenetre.draw(totalScoreText);

        // Draw score cards
        float cardX = leftX;
        float cardStartY = topY + 120.f;
        float cardW = std::min(760.f, static_cast<float>(win.x) - 120.f);
        float cardH = 48.f;
        float gap = 14.f;

        for (size_t i = 0; i < levelScores.size(); ++i)
        {
            float y = cardStartY + i * (cardH + gap);

            // background card
            sf::RectangleShape card({cardW, cardH});
            card.setPosition(cardX, y);
            card.setFillColor(sf::Color(28, 28, 28)); // slightly lighter than background
            card.setOutlineColor(sf::Color(90, 20, 20));
            card.setOutlineThickness(2.f);
            fenetre.draw(card);

            // small accent bar on the left
            sf::RectangleShape accent({8.f, cardH});
            accent.setPosition(cardX + 6.f, y + 0.f);
            accent.setFillColor(sf::Color(200, 30, 30));
            fenetre.draw(accent);

            // draw the text
            sf::Text text = levelScores[i];
            text.setPosition(cardX + 26.f, y + 8.f);
            fenetre.draw(text);
        }

        // If no scores, show placeholder
        if (levelScores.empty())
        {
            sf::Text empty;
            empty.setFont(font);
            empty.setString("Aucun score disponible.");
            empty.setCharacterSize(20);
            empty.setFillColor(sf::Color(200, 200, 200));
            empty.setPosition(leftX, cardStartY);
            fenetre.draw(empty);
        }

        // Bottom-right BACK button
        sf::Vector2f btnSize(200.f, 60.f);
        sf::Vector2f btnPos(static_cast<float>(win.x) - btnSize.x - 20.f, static_cast<float>(win.y) - btnSize.y - 20.f);
        sf::RectangleShape backBtn(btnSize);
        backBtn.setPosition(btnPos);

        sf::Text backLabel;
        backLabel.setFont(font);
        backLabel.setString("BACK");
        backLabel.setCharacterSize(26);
        backLabel.setFillColor(sf::Color::White);

        // hover detection
        sf::Vector2i mousePixel = sf::Mouse::getPosition(fenetre);
        sf::Vector2f mousePos = fenetre.mapPixelToCoords(mousePixel);
        bool hovered = backBtn.getGlobalBounds().contains(mousePos);

        // Draw only base (no shadow/shine)
        backBtn.setFillColor(hovered ? sf::Color(230, 60, 60) : sf::Color(170, 30, 30));
        backBtn.setOutlineColor(hovered ? sf::Color(255, 120, 80) : sf::Color(200, 80, 60));
        backBtn.setOutlineThickness(hovered ? 4.f : 2.f);
        fenetre.draw(backBtn);

        // Center and draw label
        sf::FloatRect lb = backLabel.getLocalBounds();
        backLabel.setPosition(
            backBtn.getPosition().x + (backBtn.getSize().x - lb.width) / 2.f - lb.left,
            backBtn.getPosition().y + (backBtn.getSize().y - lb.height) / 2.f - lb.top
        );
        fenetre.draw(backLabel);

        // Update back button rectangle for event handling
        backButtonRect = backBtn.getGlobalBounds();

        fenetre.display();
    }
}
