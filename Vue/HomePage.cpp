
// HomePage.cpp - implementation of HomePage
// Handles main menu rendering and user interactions.
// Uses a "Cherub" background image if available, and a callback to retrieve scores.

#include "HomePage.h"
#include "CreditsWindow.h"
#include "ScoreWindow.h"
#include "LevelPage.h"
#include <numeric> // std::accumulate
#include <cmath>
#include <iostream>

namespace Vue
{
    /**
     * HomePage constructor
     * - Attempts to load a system font and initializes UI components (title, buttons, etc.)
     * - Attempts to load a background image from multiple fallback paths
     * @param getScores callback to retrieve scores (used by ScoreWindow / LevelPage locks)
     * @param backgroundPath optional background image path
     */
    HomePage::HomePage(std::function<std::vector<int>()> getScores, const std::string& backgroundPath)
        : getScoresCb(std::move(getScores))
    {
        // Load a system font (Arial) if possible (Windows-specific path).
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontLoaded = true;

            // Title - "CHERUB"
            titleText.setFont(font);
            titleText.setString("CHERUB");
            titleText.setCharacterSize(72);
            titleText.setFillColor(sf::Color(255, 80, 80)); // neon red
            titleText.setStyle(sf::Text::Bold);

            // Title shadow for depth/contrast.
            titleShadow = titleText;
            titleShadow.setFillColor(sf::Color(0,0,0,160));

            // Subtitle (reuses inputText here as a "subtitle" label).
            inputText.setFont(font);
            inputText.setString("Operation Hades");
            inputText.setCharacterSize(28);
            inputText.setFillColor(sf::Color(200, 100, 100)); // lighter red
            inputText.setStyle(sf::Text::Bold);

            // Input box (visual not really used here; size is set to 0).
            // It mainly acts as a hit-test area for focusing input (currently impossible since size is 0).
            inputBox.setSize({0.f, 0.f});

            // ========== NEON RED BUTTON STYLE ==========
            // PLAY button
            playButton.setSize({380.f, 65.f});
            playButton.setFillColor(sf::Color(180, 20, 20)); // deep red
            playButton.setOutlineColor(sf::Color(255, 100, 100)); // bright red outline (neon glow)
            playButton.setOutlineThickness(0.f);

            playLabel.setFont(font);
            playLabel.setString("PLAY");
            playLabel.setCharacterSize(32);
            playLabel.setFillColor(sf::Color(255, 100, 100)); // neon red text
            playLabel.setStyle(sf::Text::Bold);

            // SCORES button - neon red style
            scoreButton.setSize({380.f, 65.f});
            scoreButton.setFillColor(sf::Color(180, 20, 20)); // deep red
            scoreButton.setOutlineColor(sf::Color(255, 100, 100)); // bright red outline
            scoreButton.setOutlineThickness(0.f);

            scoreLabel.setFont(font);
            scoreLabel.setString("SCORES");
            scoreLabel.setCharacterSize(32);
            scoreLabel.setFillColor(sf::Color(180, 80, 80)); // slightly darker red text
            scoreLabel.setStyle(sf::Text::Bold);

            // CREDITS button - neon red style
            creditsButton.setSize({380.f, 65.f});
            creditsButton.setFillColor(sf::Color(180, 20, 20)); // deep red
            creditsButton.setOutlineColor(sf::Color(255, 100, 100)); // bright red outline
            creditsButton.setOutlineThickness(0.f);

            creditsLabel.setFont(font);
            creditsLabel.setString("CREDITS");
            creditsLabel.setCharacterSize(32);
            creditsLabel.setFillColor(sf::Color(180, 80, 80)); // slightly darker red text
            creditsLabel.setStyle(sf::Text::Bold);


        }

        // Try multiple probable paths for the Cherub background image.
        std::vector<std::string> tryPaths;
        if (!backgroundPath.empty())
            tryPaths.push_back(backgroundPath);

        tryPaths.push_back("Asset/Menu/CherubMenuJeu.png");
        tryPaths.push_back("Asset/Menu/CherubMenu.png");
        tryPaths.push_back("CherubMenuJeu.png");
        tryPaths.push_back("CherubMenu.png");
        tryPaths.push_back("cmake-build-debug/Asset/Menu/CherubMenuJeu.png");
        tryPaths.push_back("cmake-build-debug/Asset/Menu/CherubMenu.png");

        for (const auto& p : tryPaths)
        {
            if (cherubTexture.loadFromFile(p))
            {
                cherubLoaded = true;
                cherubSprite.setTexture(cherubTexture);
                cherubSprite.setColor(sf::Color(255,255,255,200)); // semi-transparent
                break;
            }
        }
    }

    // Utility to draw a stylized button consistently (hover outline/glow).
    static void drawStyledButton(sf::RenderWindow& window, sf::RectangleShape button, sf::Text label, bool hovered)
    {
        // Base button
        sf::RectangleShape base = button;
        sf::Color baseColor = hovered ? sf::Color(230, 60, 60) : sf::Color(170, 30, 30);
        base.setFillColor(baseColor);
        base.setOutlineColor(hovered ? sf::Color(255, 120, 80) : sf::Color(200, 80, 60));
        base.setOutlineThickness(hovered ? 4.f : 2.f);
        window.draw(base);

        // Hover glow
        if (hovered)
        {
            sf::RectangleShape glow = button;
            glow.setFillColor(sf::Color::Transparent);
            glow.setOutlineColor(sf::Color(255, 160, 110, 200));
            glow.setOutlineThickness(6.f);
            window.draw(glow);
        }

        // Center the label inside the button.
        sf::FloatRect lb = label.getLocalBounds();
        label.setPosition(
            button.getPosition().x + (button.getSize().x - lb.width) / 2.f - lb.left,
            button.getPosition().y + (button.getSize().y - lb.height) / 2.f - lb.top
        );
        label.setFillColor(hovered ? sf::Color(255, 250, 240) : sf::Color(255, 220, 200));
        window.draw(label);
    }

    void HomePage::centerLabel(sf::Text& label, const sf::RectangleShape& button)
    {
        // Center label within a rectangle by using its local bounds (accounts for glyph offset).
        sf::FloatRect lb = label.getLocalBounds();
        label.setPosition(
            button.getPosition().x + (button.getSize().x - lb.width) / 2.f - lb.left,
            button.getPosition().y + (button.getSize().y - lb.height) / 2.f - lb.top
        );
    }

    void HomePage::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
    {
        if (!active) return;

        // Forward events to credits window if it is active (modal behavior).
        if (creditsWindow && creditsWindow->isActive())
        {
            creditsWindow->handleEvent(event);
            return;
        }

        // Text input for player name (ASCII printable range) when input is focused.
        if (event.type == sf::Event::TextEntered)
        {
            if (event.text.unicode >= 32 && event.text.unicode < 127 && playerName.size() < 32 && inputFocused)
            {
                playerName.push_back(static_cast<char>(event.text.unicode));
                inputText.setString(playerName);
            }
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            // Backspace deletes last character when focused.
            if (event.key.code == sf::Keyboard::BackSpace && !playerName.empty() && inputFocused)
            {
                // Edit name when input is focused
                playerName.pop_back();
                inputText.setString(playerName);
            }
            // Enter/Return closes HomePage (legacy behavior: "start game").
            else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return)
            {
                // If the name input is focused, Enter starts the game (legacy behavior)
                if (inputFocused)
                {
                    active = false; // start the game
                    return;
                }

                // Otherwise, activate the currently selected menu item
                if (selectedIndex == 0)
                {
                    openLevelPage(fenetre);
                    return;
                }
                else if (selectedIndex == 1)
                {
                    openScoreWindow();
                    return;
                }
                else if (selectedIndex == 2)
                {
                    if (!creditsWindow)
                    {
                        creditsWindow = std::make_unique<CreditsWindow>();
                    }
                    else
                    {
                        creditsWindow->setActive(true);
                    }
                    return;
                }
            }
            else if (!inputFocused)
            {
                // Navigate menu with Up/Down (wrap around three entries)
                if (event.key.code == sf::Keyboard::Up)
                {
                    selectedIndex = (selectedIndex + 2) % 3; // wrap-around for 3 items
                }
                else if (event.key.code == sf::Keyboard::Down)
                {
                    selectedIndex = (selectedIndex + 1) % 3;
                }
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            // Convert mouse pixel position to world coords (SFML view aware).
            sf::Vector2i mp = sf::Mouse::getPosition(fenetre);
            sf::Vector2f world = fenetre.mapPixelToCoords(mp);

            // PLAY -> open level selector (LevelPage) instead of starting immediately.
            if (playButton.getGlobalBounds().contains(world))
            {
                // Open the level selector (instead of starting immediately)
                openLevelPage(fenetre);
                return;
            }

            // SCORES -> open score window.
            if (scoreButton.getGlobalBounds().contains(world))
            {
                // Open the scores window
                openScoreWindow();
                return;
            }

            // CREDITS -> open/create credits window.
            if (creditsButton.getGlobalBounds().contains(world))
            {
                // Create or display the credits window
                if (!creditsWindow)
                {
                    creditsWindow = std::make_unique<CreditsWindow>();
                }
                else
                {
                    creditsWindow->setActive(true);
                }
                return;
            }


            // Name input focus.
            if (inputBox.getGlobalBounds().contains(world))
            {
                inputFocused = true;
            }
            else
            {
                inputFocused = false;
            }
        }
    }

    void HomePage::draw(sf::RenderWindow& fenetre)
    {
        // Layout depends on current window size.
        sf::Vector2u win = fenetre.getSize();
        float centerX = static_cast<float>(win.x) * 0.5f;

        // Clear background.
        fenetre.clear(sf::Color(18,18,28));

        // Draw Cherub background if loaded (scaled to cover, tinted for readability).
        if (cherubLoaded)
        {
            const sf::Texture& t = cherubTexture;
            float texW = static_cast<float>(t.getSize().x);
            float texH = static_cast<float>(t.getSize().y);

            // Scale to cover full screen (keeps aspect ratio).
            float scaleX = static_cast<float>(win.x) / texW;
            float scaleY = static_cast<float>(win.y) / texH;
            float scale = std::max(scaleX, scaleY);

            cherubSprite.setScale(scale, scale);

            // Position at the top left
            cherubSprite.setOrigin(0.f, 0.f);
            cherubSprite.setPosition(0.f, 0.f);

            // Semi-transparent overlay to make UI readable.
            sf::Color c = cherubSprite.getColor();
            c.a = 180; // more transparent so that the UI is more readable
            cherubSprite.setColor(c);

            fenetre.draw(cherubSprite);
        }

        // Position the TITLE on the left (as in the image) — slightly lower
        float titleX = static_cast<float>(win.x) * 0.15f;
        float titleY = static_cast<float>(win.y) * 0.35f;

        titleText.setPosition(titleX, titleY);
        titleShadow.setPosition(titleX + 3.f, titleY + 3.f);

        // Subtitle placed below title.
        sf::FloatRect titleBounds = titleText.getLocalBounds();
        inputText.setPosition(titleX, titleY + titleBounds.height + 10.f);

        // Position the buttons on the right side of the screen (to match the image)
        float buttonX = centerX + 120.f; // right side of the center
        float startY = static_cast<float>(win.y) * 0.25f;
        float buttonGap = 85.f;

        // PLAY
        playButton.setPosition(buttonX, startY);
        centerLabel(playLabel, playButton);
        startY += buttonGap;

        // SCORES
        scoreButton.setPosition(buttonX, startY);
        centerLabel(scoreLabel, scoreButton);
        startY += buttonGap;

        // CREDITS
        creditsButton.setPosition(buttonX, startY);
        centerLabel(creditsLabel, creditsButton);
        startY += buttonGap;



        if (fontLoaded)
        {
            // Hover effects using mouse position.
            sf::Vector2i mousePixel = sf::Mouse::getPosition(fenetre);
            sf::Vector2f mousePos = fenetre.mapPixelToCoords(mousePixel);

            bool hoveredPlay = playButton.getGlobalBounds().contains(mousePos);
            bool hoveredScore = scoreButton.getGlobalBounds().contains(mousePos);
            bool hoveredCredits = creditsButton.getGlobalBounds().contains(mousePos);

            // If mouse hovers over a button, sync the keyboard selection
            if (hoveredPlay) selectedIndex = 0;
            else if (hoveredScore) selectedIndex = 1;
            else if (hoveredCredits) selectedIndex = 2;

            // Combine mouse hover with keyboard selection for visual feedback
            hoveredPlay = hoveredPlay || (selectedIndex == 0);
            hoveredScore = hoveredScore || (selectedIndex == 1);
            hoveredCredits = hoveredCredits || (selectedIndex == 2);

            // Draw the three buttons using the same stylish utility function
            drawStyledButton(fenetre, playButton, playLabel, hoveredPlay);
            drawStyledButton(fenetre, scoreButton, scoreLabel, hoveredScore);
            drawStyledButton(fenetre, creditsButton, creditsLabel, hoveredCredits);

            // Draw title shadow + title, then subtitle.
            fenetre.draw(titleShadow);
            fenetre.draw(titleText);
            fenetre.draw(inputText);
        }

        // Overlay of credits
        if (showCredits)
        {
            sf::RectangleShape overlay({(float)win.x * 0.6f, (float)win.y * 0.35f});
            overlay.setFillColor(sf::Color(0,0,0,200));
            overlay.setPosition(centerX - overlay.getSize().x/2.f, centerX*0.15f);
            fenetre.draw(overlay);

            sf::Text txt;
            txt.setFont(font);
            txt.setString("CREDITS\nDeveloper: ...\nGraphics: ...");
            txt.setCharacterSize(18);
            txt.setFillColor(sf::Color::White);
            txt.setPosition(overlay.getPosition() + sf::Vector2f(20.f,20.f));
            fenetre.draw(txt);
        }

        // Afficher la fenêtre de crédits si active
        if (creditsWindow && creditsWindow->isActive())
        {
            creditsWindow->draw(fenetre);
        }

        fenetre.display();
    }

    void HomePage::openScoreWindow()
    {
        // Create the score screen using the injected callback.
        ScoreWindow scoreWindow(getScoresCb);

        // Open a fullscreen "Scores" window (modal).
        sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Scores", sf::Style::Fullscreen);

        while (window.isOpen() && scoreWindow.isActive())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                scoreWindow.handleEvent(event);
                if (event.type == sf::Event::Closed)
                    window.close();
            }
            scoreWindow.draw(window);
        }
    }

    void HomePage::openCreditsWindow()
    {
        // Stub: credits are now handled as a modal overlay via CreditsWindow and handleEvent/draw.
        CreditsWindow creditsWindow;
        // Display credits as a modal on the home page.
        // Parent window is the game window passed to handleEvent.
    }

    // Opens LevelPage selector in fullscreen and stores the selection.
    void HomePage::openLevelPage(sf::RenderWindow& parent)
    {
        LevelPage selector(this->getScoresCb);

        // Blocks until LevelPage closes and returns a Selection.
        LevelPage::Selection sel = selector.run();

        if (sel.valid)
        {
            selectedChapter = sel.chapterIndex;
            selectedLevel = sel.levelIndex;
            selectedLevelData = sel.levelData;

            // If user selected a level, close the HomePage so gameplay can start.
            active = false;
        }
        else
        {
            // User canceled or closed the window -> return to home page (do nothing).
        }
    }
} // namespace Vue
