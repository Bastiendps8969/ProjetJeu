#include "HomePage.h"
#include "CreditsWindow.h"
#include "ScoreWindow.h"
#include "LevelPage.h"
#include <numeric> // accumulate
#include <cmath>
#include <iostream>

namespace Vue
{
    HomePage::HomePage(std::function<std::vector<int>()> getScores, const std::string& backgroundPath)
        : getScoresCb(std::move(getScores))
    {
        // load system font if possible
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontLoaded = true;

            // Title - "CHERUB: Hades Operation"
            titleText.setFont(font);
            titleText.setString("CHERUB");
            titleText.setCharacterSize(72);
            titleText.setFillColor(sf::Color(255, 80, 80)); // neon red
            titleText.setStyle(sf::Text::Bold);

            // shadow for title
            titleShadow = titleText;
            titleShadow.setFillColor(sf::Color(0,0,0,160));

            // Subtitle - "Operation Hades"
            inputText.setFont(font);
            inputText.setString("Operation Hades");
            inputText.setCharacterSize(28);
            inputText.setFillColor(sf::Color(200, 100, 100)); // slightly darker red
            inputText.setStyle(sf::Text::Bold);

            // Hide input box (visual box, not the text)
            inputBox.setSize({0.f, 0.f});

            // ========== NEON RED STYLE BUTTONS ==========

            // PLAY Button - matching image style
            playButton.setSize({380.f, 65.f});
            playButton.setFillColor(sf::Color(180, 20, 20)); // deep red
            playButton.setOutlineColor(sf::Color(255, 100, 100)); // bright red outline (neon glow)
            playButton.setOutlineThickness(0.f);
            playLabel.setFont(font);
            playLabel.setString("PLAY");
            playLabel.setCharacterSize(32);
            playLabel.setFillColor(sf::Color(255, 100, 100)); // neon red text
            playLabel.setStyle(sf::Text::Bold);

            // SCORES Button - neon red style
            scoreButton.setSize({380.f, 65.f});
            scoreButton.setFillColor(sf::Color(180, 20, 20)); // deep red
            scoreButton.setOutlineColor(sf::Color(255, 100, 100)); // bright red outline
            scoreButton.setOutlineThickness(0.f);
            scoreLabel.setFont(font);
            scoreLabel.setString("SCORES");
            scoreLabel.setCharacterSize(32);
            scoreLabel.setFillColor(sf::Color(180, 80, 80)); // slightly darker red text
            scoreLabel.setStyle(sf::Text::Bold);

            // CREDITS Button - neon red style
            creditsButton.setSize({380.f, 65.f});
            creditsButton.setFillColor(sf::Color(180, 20, 20)); // deep red
            creditsButton.setOutlineColor(sf::Color(255, 100, 100)); // bright red outline
            creditsButton.setOutlineThickness(0.f);
            creditsLabel.setFont(font);
            creditsLabel.setString("CREDITS");
            creditsLabel.setCharacterSize(32);
            creditsLabel.setFillColor(sf::Color(180, 80, 80)); // slightly darker red text
            creditsLabel.setStyle(sf::Text::Bold);

            // SOUNDS Button - neon red style
            soundsButton.setSize({380.f, 65.f});
            soundsButton.setFillColor(sf::Color(180, 20, 20)); // deep red
            soundsButton.setOutlineColor(sf::Color(255, 100, 100)); // bright red outline
            soundsButton.setOutlineThickness(0.f);
            soundsLabel.setFont(font);
            soundsLabel.setCharacterSize(32);
            soundsLabel.setFillColor(sf::Color(180, 80, 80)); // slightly darker red text
            soundsLabel.setStyle(sf::Text::Bold);
            soundsLabel.setString(soundsOn ? "SOUNDS : ON" : "SOUNDS : OFF");
        }

        // Try to load the CHERUB image from a few likely locations
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

    // Helper pour dessiner un bouton "jeu" — style cohérent, sans ombre, avec shine et glow
    static void drawStyledButton(sf::RenderWindow& window, sf::RectangleShape button, sf::Text label, bool hovered)
    {
        // base du bouton
        sf::RectangleShape base = button;
        sf::Color baseColor = hovered ? sf::Color(230, 60, 60) : sf::Color(170, 30, 30);
        base.setFillColor(baseColor);
        base.setOutlineColor(hovered ? sf::Color(255, 120, 80) : sf::Color(200, 80, 60));
        base.setOutlineThickness(hovered ? 4.f : 2.f);
        window.draw(base);

        // glow au survol
        if (hovered)
        {
            sf::RectangleShape glow = button;
            glow.setFillColor(sf::Color::Transparent);
            glow.setOutlineColor(sf::Color(255, 160, 110, 200));
            glow.setOutlineThickness(6.f);
            window.draw(glow);
        }

        // texte centré
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
        sf::FloatRect lb = label.getLocalBounds();
        label.setPosition(
            button.getPosition().x + (button.getSize().x - lb.width) / 2.f - lb.left,
            button.getPosition().y + (button.getSize().y - lb.height) / 2.f - lb.top
        );
    }

    void HomePage::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
    {
        if (!active) return;

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
            if (event.key.code == sf::Keyboard::BackSpace && !playerName.empty() && inputFocused)
            {
                playerName.pop_back();
                inputText.setString(playerName);
            }
            else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return)
            {
                active = false; // start game (keep old behavior when Enter pressed)
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2i mp = sf::Mouse::getPosition(fenetre);
            sf::Vector2f world = fenetre.mapPixelToCoords(mp);

            if (playButton.getGlobalBounds().contains(world))
            {
                // Open level selector (instead of starting immediately)
                openLevelPage(fenetre);
                return;
            }

            if (scoreButton.getGlobalBounds().contains(world))
            {
                // open separate score window
                openScoreWindow();
                return;
            }

            if (creditsButton.getGlobalBounds().contains(world))
            {
                showCredits = !showCredits;
                return;
            }

            if (soundsButton.getGlobalBounds().contains(world))
            {
                soundsOn = !soundsOn;
                soundsLabel.setString(soundsOn ? "SOUNDS : ON" : "SOUNDS : OFF");
                return;
            }

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
        // compute centered column layout based on current window size
        sf::Vector2u win = fenetre.getSize();
        float centerX = static_cast<float>(win.x) * 0.5f;

        // background clear
        fenetre.clear(sf::Color(18,18,28)); // deep dark background

        // draw cherub background (fullscreen)
        if (cherubLoaded)
        {
            const sf::Texture& t = cherubTexture;
            float texW = static_cast<float>(t.getSize().x);
            float texH = static_cast<float>(t.getSize().y);

            // calculate scale to cover entire screen (maintain aspect ratio)
            float scaleX = static_cast<float>(win.x) / texW;
            float scaleY = static_cast<float>(win.y) / texH;
            float scale = std::max(scaleX, scaleY); // use largest scale to cover entire screen

            cherubSprite.setScale(scale, scale);

            // position at top-left corner
            cherubSprite.setOrigin(0.f, 0.f);
            cherubSprite.setPosition(0.f, 0.f);

            // semi-transparent overlay to dim the background
            sf::Color c = cherubSprite.getColor();
            c.a = 180; // more transparent to see UI clearly
            cherubSprite.setColor(c);

            fenetre.draw(cherubSprite);
        }

        // Position TITLE on the LEFT side (like image) - lower than before
        float titleX = static_cast<float>(win.x) * 0.15f;
        float titleY = static_cast<float>(win.y) * 0.35f;

        titleText.setPosition(titleX, titleY);
        titleShadow.setPosition(titleX + 3.f, titleY + 3.f);

        // Position subtitle below title
        sf::FloatRect titleBounds = titleText.getLocalBounds();
        inputText.setPosition(titleX, titleY + titleBounds.height + 10.f);

        // Position buttons on the RIGHT SIDE of the screen (matching the image)
        float buttonX = centerX + 120.f; // right side of center
        float startY = static_cast<float>(win.y) * 0.25f;
        float buttonGap = 85.f;

        // PLAY button
        playButton.setPosition(buttonX, startY);
        centerLabel(playLabel, playButton);
        startY += buttonGap;

        // SCORES button
        scoreButton.setPosition(buttonX, startY);
        centerLabel(scoreLabel, scoreButton);
        startY += buttonGap;

        // CREDITS button
        creditsButton.setPosition(buttonX, startY);
        centerLabel(creditsLabel, creditsButton);
        startY += buttonGap;

        // SOUNDS button
        soundsButton.setPosition(buttonX, startY);
        centerLabel(soundsLabel, soundsButton);

        if (fontLoaded)
        {
            // Get mouse position for hover effects
            sf::Vector2i mousePixel = sf::Mouse::getPosition(fenetre);
            sf::Vector2f mousePos = fenetre.mapPixelToCoords(mousePixel);

            bool hoveredPlay = playButton.getGlobalBounds().contains(mousePos);
            bool hoveredScore = scoreButton.getGlobalBounds().contains(mousePos);
            bool hoveredCredits = creditsButton.getGlobalBounds().contains(mousePos);
            bool hoveredSounds = soundsButton.getGlobalBounds().contains(mousePos);

            // Draw all four buttons using the same styled helper
            drawStyledButton(fenetre, playButton, playLabel, hoveredPlay);
            drawStyledButton(fenetre, scoreButton, scoreLabel, hoveredScore);
            drawStyledButton(fenetre, creditsButton, creditsLabel, hoveredCredits);
            drawStyledButton(fenetre, soundsButton, soundsLabel, hoveredSounds);

            // Draw title with shadow
            fenetre.draw(titleShadow);
            fenetre.draw(titleText);

            // Draw subtitle
            fenetre.draw(inputText);
        }

        // credits overlay
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

        fenetre.display();
    }

    void HomePage::openScoreWindow()
    {
        ScoreWindow scoreWindow(getScoresCb);

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
        CreditsWindow creditsWindow;

        // If CreditsWindow provides its own window loop, use it.
        // Fallback to a simple loop if CreditsWindow exposes update()/draw().
        while (creditsWindow.isWindowOpen() && creditsWindow.isActive())
        {
            creditsWindow.update();
            creditsWindow.draw();
        }
    }

    // Open the LevelPage selector in a fullscreen window and store the selection.
    void HomePage::openLevelPage(sf::RenderWindow& parent)
    {
        LevelPage selector;
        LevelPage::Selection sel = selector.run(); // blocks until closed

        if (sel.valid)
        {
            selectedChapter = sel.chapterIndex;
            selectedLevel = sel.levelIndex;

            // If the user selected a level, close the home page to start the game.
            // The controller can later query getSelectedChapter/getSelectedLevel()
            active = false;
        }
        else
        {
            // user cancelled or closed; do nothing, return to home menu
        }
    }
} // namespace Vue