
#include "SplashPage.h"
#include <iostream>
#include <cmath>

namespace Vue
{
    SplashPage::SplashPage()
    {
        // Load a font (fallback to local Arial if Windows path fails).
        // NOTE: "C:\\Windows\\Fonts\\arial.ttf" is Windows-specific.
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf") == false)
            font.loadFromFile("arial.ttf");

        // Configure the start label.
        startText.setFont(font);
        startText.setString("START GAME");
        startText.setCharacterSize(36);
        startText.setFillColor(sf::Color::White);
        startText.setStyle(sf::Text::Bold);

        // Try a few likely paths for the background image (CherubMenu.png).
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

        if (event.type == sf::Event::KeyPressed)
        {
            // Proceed on Enter/Return/Space.
            if (event.key.code == sf::Keyboard::Enter ||
                event.key.code == sf::Keyboard::Return ||
                event.key.code == sf::Keyboard::Space)
            {
                active = false;
            }
            // Quit on Escape.
            else if (event.key.code == sf::Keyboard::Escape)
            {
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

        // Center the text horizontally and place it near the bottom (93% height).
        sf::FloatRect tb = startText.getLocalBounds();
        // y had been changed for better visual
        startText.setPosition((float)win.x * 0.5f - tb.width / 2.f - tb.left,
                              (float)win.y * 0.93f - tb.height / 2.f - tb.top);

        fenetre.draw(startText);

        // Present the splash frame.
        fenetre.display();
    }
}
