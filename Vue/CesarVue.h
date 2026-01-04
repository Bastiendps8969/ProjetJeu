//
// Created by bertr on 14-12-25.
//

#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <vector>

#include "Objective.h"

class CesarVue {
private:
    Objective* objective = nullptr;

    sf::Font font;
    sf::Text text;

    // Background PC image (from Asset/cesar/pc_cesar.png)
    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    bool bgLoaded = false;

    sf::RectangleShape inputBox;
    sf::Text inputText;

    // The altered code shown under the title
    sf::Text alteredCodeText;

    // Brute-force helper UI
    bool showBruteResults = false;
    std::vector<sf::Text> bruteLines;
    
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
