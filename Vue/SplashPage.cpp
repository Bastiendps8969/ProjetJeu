#include "SplashPage.h"
#include <iostream>
#include <cmath>

namespace Vue
{
    SplashPage::SplashPage()
    {
        // Load a font (fall back to arial)
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf") == false)
            font.loadFromFile("arial.ttf");

        startText.setFont(font);
        startText.setString("START GAME");
        startText.setCharacterSize(36);
        startText.setFillColor(sf::Color::White);
        startText.setStyle(sf::Text::Bold);

        // Try a few likely paths for CherubMenu.png
        for (const auto& p : defaultPaths())
        {
            if (backgroundTexture.loadFromFile(p))
            {
                backgroundLoaded = true;
                backgroundSprite.setTexture(backgroundTexture);
                break;
            }
        }

        if (!backgroundLoaded)
        {
            std::cerr << "[SplashPage] Warning: background image not found (CherubMenu.png)\n";
        }
    }

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
        if (!active) return;

        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Enter ||
                event.key.code == sf::Keyboard::Return ||
                event.key.code == sf::Keyboard::Space)
            {
                active = false;
            }
            else if (event.key.code == sf::Keyboard::Escape)
            {
                fenetre.close();
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            // Any click proceeds
            active = false;
        }
        else if (event.type == sf::Event::Closed)
        {
            fenetre.close();
        }
    }

    void SplashPage::draw(sf::RenderWindow& fenetre)
    {
        sf::Vector2u win = fenetre.getSize();

        // draw background scaled to cover window
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
            fenetre.clear(sf::Color::Black);
        }

        // draw start text centered with a simple blink
        float t = blinkClock.getElapsedTime().asSeconds();
        if (std::fmod(t, 1.0f) < 0.7f)
            startText.setFillColor(sf::Color::White);
        else
            startText.setFillColor(sf::Color(255,255,255,60));

        sf::FloatRect tb = startText.getLocalBounds();
        startText.setPosition((float)win.x * 0.5f - tb.width / 2.f - tb.left,
                              (float)win.y * 0.78f - tb.height / 2.f - tb.top);

        fenetre.draw(startText);

        fenetre.display();
    }
}