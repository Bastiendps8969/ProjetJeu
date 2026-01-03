
//
// Pause menu - shows objectives and 3 actions: Resume, Exit level, Exit game
//
#pragma once

#include <SFML/Graphics.hpp>
#include "../Modele/Modele.h"

namespace Vue {

class PauseMenu {
public:
    // Strongly-typed options to avoid "magic integers".
    enum class Option { Resume, ExitLevel, ExitGame };

    // The menu reads objective state from the model (aggregation-like dependency).
    // A pointer is stored internally to allow a nullable dependency if needed.
    PauseMenu(Modele::Modele& modele);

    // Handle user input events (keyboard navigation + mouse hover/click).
    void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);

    // Render the pause overlay, objectives list, title, and buttons.
    void draw(sf::RenderWindow& fenetre);

    // Modal flag: while active, the menu stays on screen.
    bool isActive() const { return active; }
    void setActive(bool v) { active = v; }

    // Selected option (read by the controller once the menu closes).
    Option getSelectedOption() const { return selectedOption; }

private:
    // Pointer to the model (nullable). The model is owned elsewhere.
    Modele::Modele* modelePtr = nullptr;

    // Modal state flag.
    bool active = false;

    // Current selection (driven by keyboard or mouse hover).
    Option selectedOption = Option::Resume;

    // Font resources.
    sf::Font font;
    bool fontLoaded = false;

    // Vertical spacing between buttons (note: stored as int but initialized with float literal).
    int spaceBetweenButton = 160.f;

    // UI elements
    sf::Text titleText;
    // Shadow for title to match other menus visually
    sf::Text titleShadow;

    // Buttons and their labels
    sf::RectangleShape resumeButton;
    sf::Text resumeLabel;

    sf::RectangleShape exitLevelButton;
    sf::Text exitLevelLabel;

    sf::RectangleShape exitGameButton;
    sf::Text exitGameLabel;

    // Color palette
    const sf::Color buttonColor = sf::Color(60, 60, 60);
    const sf::Color selectedColor = sf::Color(120, 120, 120);
    const sf::Color textColor = sf::Color::White;

    // Build/refresh layout positions based on current window size.
    void initUI(sf::RenderWindow& fenetre);

    // Update base button fill colors according to selectedOption.
    void updateButtonColors();

    // Utility: center a text label within a rectangle button.
    void centerLabel(sf::Text& label, const sf::RectangleShape& button);
};

} // namespace Vue
