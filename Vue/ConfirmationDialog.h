
// Simple modal confirmation dialog with Yes/No buttons
#pragma once

#include <SFML/Graphics.hpp>
#include <string>

namespace Vue
{
    class ConfirmationDialog
    {
    public:
        // Create a modal dialog displaying a message and two actions: Yes / No.
        // WHY message by const reference:
        // - avoids copying the caller string on entry
        // - we still store our own copy internally for lifetime safety
        explicit ConfirmationDialog(const std::string& message);

        // Handle input events:
        // - Escape -> cancel (confirmed=false)
        // - Mouse click on Yes/No -> sets confirmed accordingly and closes the dialog (active=false)
        //
        // WHY event by const reference:
        // - read-only, avoids copying
        // WHY window by non-const reference:
        // - coordinate mapping and size queries depend on the live window state
        void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);

        // Draw a semi-transparent overlay and the centered dialog box.
        // WHY window by non-const reference:
        // - drawing mutates the render target
        void draw(sf::RenderWindow& fenetre);

        // Modal flag: while true, the dialog remains displayed.
        bool isActive() const { return active; }

        // Result flag: true if the user clicked "Yes", false otherwise.
        bool isConfirmed() const { return confirmed; }

    private:
        // UI resources.
        // WHY sf::Font as a member:
        // - sf::Text keeps a reference to a font; the font must outlive all sf::Text objects
        sf::Font font;
        bool fontLoaded = false;

        // Message content.
        std::string message;

        // Modal state and result.
        bool active = true;
        bool confirmed = false;

        // Keyboard selection: 0 = Yes, 1 = No
        int selectedIndex = 0;

        // UI elements: message text and two buttons + their labels.
        sf::Text messageText;
        sf::RectangleShape yesButton;
        sf::Text yesLabel;
        sf::RectangleShape noButton;
        sf::Text noLabel;

        // Compute/refresh layout positions (centered) based on current window size.
        // WHY window passed by reference:
        // - size depends on actual window (fullscreen, resized, etc.)
        void initUI(sf::RenderWindow& fenetre);

        // Utility: center a text label inside a rectangle button using SFML bounds.
        // WHY label by non-const reference:
        // - we modify its position
        // WHY button by const reference:
        // - read-only and avoids copying shapes
        void centerLabel(sf::Text& label, const sf::RectangleShape& button);
    };
} // namespace Vue
