//
// Created by bertr on 25-11-25.
//

#ifndef TESTCOLLISION_HOMEPAGE_H
#define TESTCOLLISION_HOMEPAGE_H


#include <SFML/Graphics.hpp>
#include <string>

namespace Vue
{
    class HomePage
    {
    private:
        bool active = true;
        std::string playerName;

        sf::Font font;
        bool fontLoaded = false;

        sf::Text titleText;
        sf::RectangleShape inputBox;
        sf::Text inputText;
        sf::RectangleShape playButton;
        sf::Text playLabel;

    public:
        HomePage();

        bool isActive() const { return active; }
        void setActive(bool v) { active = v; }

        const std::string& getPlayerName() const { return playerName; }

        void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);
        void draw(sf::RenderWindow& fenetre);
    };
}

#endif //TESTCOLLISION_HOMEPAGE_H