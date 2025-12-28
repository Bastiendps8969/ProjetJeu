#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

namespace Vue
{
    class CreditsWindow
    {
    private:
        bool active = true;
        sf::RenderWindow window;
        sf::Font font;
        bool fontLoaded = false;

        sf::Text titleText;
        std::vector<sf::Text> creditTexts;

        void initializeTexts();

    public:
        CreditsWindow();

        bool isActive() const { return active; }
        void setActive(bool v) { active = v; }

        bool isWindowOpen() const { return window.isOpen(); }

        void handleEvent(const sf::Event& event);
        void draw();
        void update();
    };
}