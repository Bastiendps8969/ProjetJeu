// Simple modal confirmation dialog with Yes/No buttons
#pragma once
#include <SFML/Graphics.hpp>
#include <string>

namespace Vue {
class ConfirmationDialog {
public:
    ConfirmationDialog(const std::string& message);
    void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);
    void draw(sf::RenderWindow& fenetre);
    bool isActive() const { return active; }
    bool isConfirmed() const { return confirmed; }
private:
    sf::Font font;
    bool fontLoaded = false;
    std::string message;
    bool active = true;
    bool confirmed = false;
    int selectedIndex = 0; // 0 = Yes, 1 = No

    sf::Text messageText;
    sf::RectangleShape yesButton;
    sf::Text yesLabel;
    sf::RectangleShape noButton;
    sf::Text noLabel;

    void initUI(sf::RenderWindow& fenetre);
    void centerLabel(sf::Text& label, const sf::RectangleShape& button);
};
} // namespace Vue
