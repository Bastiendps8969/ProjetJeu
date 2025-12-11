#pragma once
#include <SFML/Graphics.hpp>

namespace Vue
{
    class SplashPage
    {
    private:
        bool active = true;

        sf::Texture backgroundTexture;
        sf::Sprite backgroundSprite;
        bool backgroundLoaded = false;

        sf::Font font;
        sf::Text startText;

        sf::Clock blinkClock;

        std::vector<std::string> defaultPaths();

    public:
        SplashPage();

        bool isActive() const { return active; }

        void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);
        void draw(sf::RenderWindow& fenetre);
    };
}