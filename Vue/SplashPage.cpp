
#include "SplashPage.h"
#include <iostream>
#include <cmath>

namespace Vue
{
    SplashPage::SplashPage()
    {
        // Load a font (fallback to local Arial if Windows path fails).
        // NOTE: "C:\\Windows\\Fonts\\arial.ttf" is Windows-specific.
        //
        // WHY try Windows path then local path:
        // - supports running on Windows without shipping a font file
        // - supports running from project directory where "arial.ttf" may exist
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf") == false)
            font.loadFromFile("arial.ttf");

        // Configure the start label.
        startText.setFont(font);
        startText.setString("START GAME");
        startText.setCharacterSize(36);
        startText.setFillColor(sf::Color::White);
        startText.setStyle(sf::Text::Bold);

        // Try a few likely paths for the background image (CherubMenu.png).
        //
        // WHY iterate over candidates:
        // - working directory may differ (IDE vs build folder)
        // - first successful path wins; avoids hardcoding a single fragile location
        for (const auto& p : defaultPaths())
        {
            if (backgroundTexture.loadFromFile(p))
            {
                backgroundLoaded = true;
                backgroundSprite.setTexture(backgroundTexture);
                break;
            }
        }

        // If not found, keep a black background and warn in logs.
        if (!backgroundLoaded)
        {
            std::cerr << "[SplashPage] Warning: background image not found (CherubMenu.png)\n";
        }
    }

    // Returns the list of candidate locations for the splash background image.
    // This helps when the working directory differs (IDE vs build folder).
    std::vector<std::string> SplashPage::defaultPaths()
    {
        // WHY return by value:
        // - returns a fresh list of candidates (small vector)
        // - modern C++ will optimize with RVO/move
        return {
            "Asset/Menu/CherubMenu.png",
            "CherubMenu.png",
            "cmake-build-debug/Asset/Menu/CherubMenu.png"
        };
    }

    void SplashPage::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
    {
        // If already inactive, ignore events.
        if (!active) return;

        // WHY active guard:
        // - prevents handling input twice after the splash has been dismissed
        if (event.type == sf::Event::KeyPressed)
        {
            // Proceed on Enter/Return/Space.
            if (event.key.code == sf::Keyboard::Enter
                || event.key.code == sf::Keyboard::Return
                || event.key.code == sf::Keyboard::Space)
            {
                active = false; // one-shot transition to the next screen
            }
            // Quit on Escape.
            else if (event.key.code == sf::Keyboard::Escape)
            {
                // WHY close the window here:
                // - splash is the first screen; ESC is treated as "quit"
                fenetre.close();
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            // Any left click proceeds (deactivates splash).
            active = false;
        }
        else if (event.type == sf::Event::Closed)
        {
            // Closing the window ends the application.
            fenetre.close();
        }
    }

    void SplashPage::draw(sf::RenderWindow& fenetre)
    {
        sf::Vector2u win = fenetre.getSize();

        // Draw background scaled to cover the entire window.
        // Using max(scaleX, scaleY) ensures full coverage (may crop parts of the image).
        if (backgroundLoaded)
        {
            const sf::Texture& t = backgroundTexture;

            float texW = static_cast<float>(t.getSize().x);
            float texH = static_cast<float>(t.getSize().y);

            float scaleX = static_cast<float>(win.x) / texW;
            float scaleY = static_cast<float>(win.y) / texH;
            float scale = std::max(scaleX, scaleY);

            // WHY use a single "max" scale factor:
            // - guarantees the image covers the full window without borders
            // - trade-off: image may be cropped on one axis
            backgroundSprite.setScale(scale, scale);
            backgroundSprite.setPosition(0.f, 0.f);

            fenetre.clear(sf::Color::Black);
            fenetre.draw(backgroundSprite);
        }
        else
        {
            // Fallback background when texture is missing.
            fenetre.clear(sf::Color::Black);
        }

        // Blinking effect for the "START GAME" text.
        // We alternate between opaque white and semi-transparent white based on time.
        float t = blinkClock.getElapsedTime().asSeconds();
        if (std::fmod(t, 1.0f) < 0.7f)
            startText.setFillColor(sf::Color::White);
        else
            startText.setFillColor(sf::Color(255,255,255,60));

        // WHY blinking via clock + fmod:
        // - time-based animation independent of framerate
        // - simple periodic behavior without storing extra state

        // Center the text horizontally and place it near the bottom (93% height).
        sf::FloatRect tb = startText.getLocalBounds();

        // y had been changed for better visual
        startText.setPosition((float)win.x * 0.5f - tb.width / 2.f - tb.left,
                              (float)win.y * 0.93f - tb.height / 2.f - tb.top);

        fenetre.draw(startText);

        // Present the splash frame.
        //
        // NOTE: display() is called here, meaning SplashPage owns the full frame presentation.
        // This is a "modal screen" pattern where the splash draws and presents itself.
        fenetre.display();
    }
}
