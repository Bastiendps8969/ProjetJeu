//
// Created by bertr on 14-12-25.
//

#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>

#include "Objective.h"

class CesarVue {
private:
    Objective* objective = nullptr;

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
    // Accept a pointer to the objective so the original can be modified
    CesarVue(Objective* objective);

    ~CesarVue();

    void setObjective(Objective* o);
    Objective* getObjective() const;

    void draw(sf::RenderWindow& window);

    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    
    bool shouldWindowClose() const { return shouldClose; }
};
