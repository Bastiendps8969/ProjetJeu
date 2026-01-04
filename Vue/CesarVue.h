
//
// Created by bertr on 14-12-25.
//
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <vector>
#include "Objective.h"

class CesarVue
{
private:
    // Non-owning pointer to an Objective.
    // WHY pointer (and not reference):
    // - Objective may be optional (nullptr means "no objective attached")
    // - CesarVue must be able to re-attach to another Objective at runtime (setObjective)
    // - Ownership is external (model/controller), CesarVue only observes/modifies it
    Objective* objective = nullptr;

    // UI resources owned by CesarVue (composition).
    // WHY store sf::Font as a member:
    // - sf::Text stores a pointer to a font; the font must outlive all texts using it
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
    // Accept a pointer to the objective so the original can be modified.
    // WHY pointer:
    // - CesarVue wants to modify the same Objective instance as the rest of the game
    // - no copy of Objective is made (avoids duplication/desync)
    CesarVue(Objective* objective);

    ~CesarVue();

    void setObjective(Objective* o);

    Objective* getObjective() const;

    // Render the view.
    // WHY RenderWindow&:
    // - drawing mutates the window render target
    void draw(sf::RenderWindow& window);

    // Handle input events.
    // WHY event by const reference:
    // - read-only and avoids copying SFML event objects
    // WHY window by non-const reference:
    // - can call window.close() and map pixel coordinates
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);

    bool shouldWindowClose() const { return shouldClose; }
};
