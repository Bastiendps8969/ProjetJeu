//
// Created by bertr on 14-12-25.
//

#ifndef TESTCOLLISION_CESARVUE_H
#define TESTCOLLISION_CESARVUE_H
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>

#include "Objective.h"


class CesarVue {
private:
    Objective objective;

    sf::Font font;
    sf::Text text;

    sf::RectangleShape inputBox;
    sf::Text inputText;
    
    sf::RectangleShape exitButton;
    sf::Text exitButtonText;
    
    bool shouldClose = false;
    bool isValidated = false;
    std::string validationMessage;

public:
    CesarVue(const Objective &objective);

    ~CesarVue();

    void setObjective(const Objective &o);
    Objective getObjective() const;

    void draw(sf::RenderWindow& window);

    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    
    bool shouldWindowClose() const { return shouldClose; }
};


#endif //TESTCOLLISION_CESARVUE_H