// Simple modal confirmation dialog with Yes/No buttons
#pragma once
#include <SFML/Graphics.hpp>
#include <string>

namespace Vue {

    class ConfirmationDialog {
    public:
        // Create a modal dialog displaying a message and two actions: Yes / No.
        explicit ConfirmationDialog(const std::string& message);

        // Handle input events:
        // - Escape -> cancel (confirmed=false)
        // - Mouse click on Yes/No -> sets confirmed accordingly and closes the dialog (active=false)
        void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);

        // Draw a semi-transparent overlay and the centered dialog box.
        void draw(sf::RenderWindow& fenetre);

        // Modal flag: while true, the dialog remains displayed.
        bool isActive() const { return active; }

        // Result flag: true if the user clicked "Yes", false otherwise.
        bool isConfirmed() const { return confirmed; }

    private:
        // UI resources.
        sf::Font font;
        bool fontLoaded = false;

        // Message content.
        std::string message;

        // Modal state and result.
        bool active = true;
        bool confirmed = false;
        int selectedIndex = 0; // 0 = Yes, 1 = No
        // UI elements: message text and two buttons + their labels.
        sf::Text messageText;

        sf::RectangleShape yesButton;
        sf::Text yesLabel;

        sf::RectangleShape noButton;
        sf::Text noLabel;

        // Compute/refresh layout positions (centered) based on current window size.
        void initUI(sf::RenderWindow& fenetre);

        // Utility: center a text label inside a rectangle button using SFML bounds.
        void centerLabel(sf::Text& label, const sf::RectangleShape& button);
    };

} // namespace Vue
