
#include "ScoreWindow.h"
#include <numeric>
#include <sstream>
#include <algorithm>

namespace Vue
{
    // Small helper to draw a styled button (hover colors, outline, shine, glow).
    // NOTE: In this file, draw() currently draws the BACK button manually,
    // so this helper is not used yet (could be intended for future refactoring).
    static void drawStyledButton(sf::RenderWindow& window, const sf::RectangleShape& button, sf::Text label, bool hovered)
    {
        // WHY label is passed by value:
        // - the function modifies label position and fill color for drawing
        // - passing by value avoids mutating the caller's original sf::Text object
        sf::RectangleShape base = button;

        // Basic palette with hover variation.
        sf::Color baseColor = hovered ? sf::Color(230, 60, 60) : sf::Color(170, 30, 30);
        base.setFillColor(baseColor);
        base.setOutlineColor(hovered ? sf::Color(255, 120, 80) : sf::Color(200, 80, 60));
        base.setOutlineThickness(hovered ? 4.f : 2.f);
        window.draw(base);

        // Shine effect (subtle highlight stripe).
        sf::RectangleShape shine({button.getSize().x * 0.92f, button.getSize().y * 0.28f});
        shine.setOrigin(shine.getSize().x * 0.5f, 0.f);
        shine.setPosition(button.getPosition().x,
                          button.getPosition().y - button.getSize().y * 0.5f + (button.getSize().y * 0.14f));
        shine.setFillColor(hovered ? sf::Color(255, 180, 140, 110) : sf::Color(255, 150, 120, 70));
        window.draw(shine);

        // Glow effect on hover.
        if (hovered)
        {
            sf::RectangleShape glow = button;
            glow.setFillColor(sf::Color::Transparent);
            glow.setOutlineColor(sf::Color(255, 160, 110, 200));
            glow.setOutlineThickness(6.f);
            window.draw(glow);
        }

        // Draw centered label.
        sf::FloatRect lb = label.getLocalBounds();
        label.setPosition(
            button.getPosition().x + (button.getSize().x - lb.width) / 2.f - lb.left,
            button.getPosition().y + (button.getSize().y - lb.height) / 2.f - lb.top
        );
        label.setFillColor(hovered ? sf::Color(255, 250, 240) : sf::Color(255, 220, 200));
        window.draw(label);
    }

    // Legacy constructor: fetches a vector of scores (one per level) and builds UI.
    ScoreWindow::ScoreWindow(std::function<std::vector<int>()> getScores)
        : getScoresCb(std::move(getScores))
    {
        // WHY std::move(getScores):
        // - std::function may hold captured state (lambda captures)
        // - moving avoids duplicating that state and is the idiomatic way to store callbacks

        // Load font (Windows-specific path).
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontLoaded = true;

            // Fetch score data via callback and initialize texts.
            std::vector<int> scores = getScoresCb();

            // WHY scores stored in a local variable:
            // - initializeTexts expects a const reference; local keeps data alive for the call
            initializeTexts(scores);
        }
    }

    // New constructor: fetches detailed breakdown for a completed level.
    ScoreWindow::ScoreWindow(std::function<Modele::ScoreDetails()> getDetails)
        : getDetailsCb(std::move(getDetails))
    {
        // Load font (Windows-specific path).
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontLoaded = true;

            // Fetch score details via callback and initialize breakdown texts.
            Modele::ScoreDetails details = getDetailsCb();

            // WHY details in a local variable:
            // - initializeDetailedTexts takes const ref; local extends lifetime for the call
            initializeDetailedTexts(details);
        }
    }

    void ScoreWindow::initializeDetailedTexts(const Modele::ScoreDetails& details)
    {
        // Title
        titleText.setFont(font);
        titleText.setString("NIVEAU COMPLETE!");
        titleText.setCharacterSize(48);
        titleText.setFillColor(sf::Color(220, 30, 30));
        titleText.setStyle(sf::Text::Bold);

        // Total score (main highlight)
        totalScoreText.setFont(font);
        totalScoreText.setString("Score Total: " + std::to_string(details.totalScore));
        totalScoreText.setCharacterSize(32);
        totalScoreText.setFillColor(sf::Color(255, 200, 140));

        // Clear previous list and rebuild the "cards" list.
        levelScores.clear();

        // Time remaining score line
        timeScoreText.setFont(font);
        std::ostringstream str1;
        str1 << "Remaining time: " << details.secondsRemaining << " s x 50 = " << details.timeScore;
        timeScoreText.setString(str1.str());
        timeScoreText.setCharacterSize(22);
        timeScoreText.setFillColor(sf::Color(100, 200, 255));

        // WHY push member sf::Text into a vector:
        // - ScoreWindow keeps "template" Text members so font/color remain configured
        // - vector holds copies used for layout in draw()
        levelScores.push_back(timeScoreText);

        // Primary objectives score line
        primaryScoreText.setFont(font);
        std::ostringstream str2;
        str2 << "Primary objectives: " << details.primaryObjectivesCompleted << " x 10000 = " << details.primaryScore;
        primaryScoreText.setString(str2.str());
        primaryScoreText.setCharacterSize(22);
        primaryScoreText.setFillColor(sf::Color(150, 255, 150));
        levelScores.push_back(primaryScoreText);

        // Secondary objectives score line
        secondaryScoreText.setFont(font);
        std::ostringstream str3;
        str3 << "Secondary objectives: " << details.secondaryObjectivesCompleted << " x 5000 = " << details.secondaryScore;
        secondaryScoreText.setString(str3.str());
        secondaryScoreText.setCharacterSize(22);
        secondaryScoreText.setFillColor(sf::Color(255, 200, 100));
        levelScores.push_back(secondaryScoreText);

        // Detection malus line
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
        // Title
        titleText.setFont(font);
        titleText.setString("SCORES");
        titleText.setCharacterSize(48);
        titleText.setFillColor(sf::Color(220, 30, 30));
        titleText.setStyle(sf::Text::Bold);

        // Total score is the sum of all level scores.
        int totalScore = std::accumulate(scores.begin(), scores.end(), 0);

        totalScoreText.setFont(font);
        totalScoreText.setString("Score Total: " + std::to_string(totalScore));
        totalScoreText.setCharacterSize(28);
        totalScoreText.setFillColor(sf::Color(255, 200, 140));

        // Build one text line per level (positions are assigned in draw()).
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

            // WHY store sf::Text by value in the vector:
            // - each line is an independent drawable object (position assigned later)
            // - sf::Text is lightweight to copy compared to textures
            levelScores.push_back(levelText);
        }
    }

    void ScoreWindow::handleEvent(const sf::Event& event)
    {
        // Close on keyboard keys (Escape / Return).
        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Escape
                || event.key.code == sf::Keyboard::Return)
            {
                active = false;
            }
        }

        // Close on mouse click on BACK button.
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            // Use the backButtonRect computed in the last draw().
            //
            // WHY reuse backButtonRect:
            // - separates layout computation (draw) from input hit-testing (handleEvent)
            // - avoids recomputing bounds multiple times per frame
            if (backButtonRect.contains(static_cast<float>(event.mouseButton.x),
                                        static_cast<float>(event.mouseButton.y)))
            {
                active = false;
            }
        }

        // Close if the window is closed.
        if (event.type == sf::Event::Closed)
        {
            active = false;
        }
    }

    void ScoreWindow::draw(sf::RenderWindow& fenetre)
    {
        // Layout base values.
        sf::Vector2u win = fenetre.getSize();

        fenetre.clear(sf::Color(18, 18, 18)); // dark background matching other menus

        // If font was not loaded, just display the cleared background.
        if (!fontLoaded)
        {
            fenetre.display();
            return;
        }

        // Title & basic positions.
        float leftX = 40.f;
        float topY = 30.f;
        titleText.setPosition(leftX, topY);
        fenetre.draw(titleText);

        totalScoreText.setPosition(leftX, topY + 70.f);
        fenetre.draw(totalScoreText);

        // Draw score "cards" list.
        float cardX = leftX;
        float cardStartY = topY + 120.f;
        float cardW = std::min(760.f, static_cast<float>(win.x) - 120.f);
        float cardH = 48.f;
        float gap = 14.f;

        for (size_t i = 0; i < levelScores.size(); ++i)
        {
            float y = cardStartY + i * (cardH + gap);

            // Card background.
            sf::RectangleShape card({cardW, cardH});
            card.setPosition(cardX, y);
            card.setFillColor(sf::Color(28, 28, 28)); // slightly lighter than background
            card.setOutlineColor(sf::Color(90, 20, 20));
            card.setOutlineThickness(2.f);
            fenetre.draw(card);

            // Small accent bar on the left.
            sf::RectangleShape accent({8.f, cardH});
            accent.setPosition(cardX + 6.f, y + 0.f);
            accent.setFillColor(sf::Color(200, 30, 30));
            fenetre.draw(accent);

            // Draw the score line text.
            sf::Text text = levelScores[i];
            text.setPosition(cardX + 26.f, y + 8.f);
            fenetre.draw(text);

            // WHY copy levelScores[i] into a local sf::Text:
            // - allows per-draw positioning without mutating the stored template text
            // - keeps vector entries reusable across frames
        }

        // If no scores exist, show a placeholder message.
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

        // Bottom-right BACK button.
        sf::Vector2f btnSize(200.f, 60.f);
        sf::Vector2f btnPos(static_cast<float>(win.x) - btnSize.x - 20.f,
                            static_cast<float>(win.y) - btnSize.y - 20.f);

        sf::RectangleShape backBtn(btnSize);
        backBtn.setPosition(btnPos);

        sf::Text backLabel;
        backLabel.setFont(font);
        backLabel.setString("BACK");
        backLabel.setCharacterSize(26);
        backLabel.setFillColor(sf::Color::White);

        // Hover detection using mouse position in window coordinates.
        sf::Vector2i mousePixel = sf::Mouse::getPosition(fenetre);
        sf::Vector2f mousePos = fenetre.mapPixelToCoords(mousePixel);
        bool hovered = backBtn.getGlobalBounds().contains(mousePos);

        // Draw button base (hover changes color and outline thickness).
        backBtn.setFillColor(hovered ? sf::Color(230, 60, 60) : sf::Color(170, 30, 30));
        backBtn.setOutlineColor(hovered ? sf::Color(255, 120, 80) : sf::Color(200, 80, 60));
        backBtn.setOutlineThickness(hovered ? 4.f : 2.f);
        fenetre.draw(backBtn);

        // Center and draw the label on the button.
        sf::FloatRect lb = backLabel.getLocalBounds();
        backLabel.setPosition(
            backBtn.getPosition().x + (backBtn.getSize().x - lb.width) / 2.f - lb.left,
            backBtn.getPosition().y + (backBtn.getSize().y - lb.height) / 2.f - lb.top
        );
        fenetre.draw(backLabel);

        // Update back button rectangle for click hit-testing in handleEvent().
        backButtonRect = backBtn.getGlobalBounds();

        // Present frame.
        fenetre.display();
    }
}
