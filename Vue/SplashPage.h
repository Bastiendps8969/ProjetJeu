
#pragma once
#include <SFML/Graphics.hpp>

namespace Vue
{
    /**
     * SplashPage: intro / title screen.
     * - Displays a background image (if available) and a "START GAME" label.
     * - Applies a subtle blinking effect to the label.
     * - Reacts to input (Enter/Space/click/ESC) to either proceed or quit.
     */
    class SplashPage
    {
    private:
        // Whether the splash is still active.
        // While true, the caller typically keeps displaying this screen.
        bool active = true; ///< Is the splash screen active (visible while true)?

        // Rendering resources for the background image.
        sf::Texture backgroundTexture; ///< Background texture (Cherub image)
        sf::Sprite backgroundSprite;   ///< Sprite that uses the background texture
        bool backgroundLoaded = false; ///< True if background texture was successfully loaded

        // Font and "Start" text.
        sf::Font font;
        sf::Text startText;

        // Clock used to drive the blinking animation for the "START GAME" text.
        sf::Clock blinkClock;

        // List of default candidate paths to locate the background image.
        // Implemented as a helper that returns a vector of possible file locations.
        std::vector<std::string> defaultPaths();

        // WHY SFML resources are stored by value (composition):
        // - RAII: resources are released automatically when SplashPage is destroyed
        // - clear lifetime: texture must outlive sprite; keeping both as members ensures that
        // - avoids manual memory management (no new/delete)

    public:
        // Constructor initializes font, text styling, and tries to load a background image.
        SplashPage();

        // Returns whether the splash screen is still active.
        bool isActive() const { return active; }

        // Handles input events:
        // - Enter/Return/Space or left click -> deactivate splash (continue)
        // - Escape or window close -> close the window
        //
        // WHY parameters are references:
        // - event is const ref: read-only and avoids copying event data
        // - fenetre is non-const ref: we need to call fenetre.close() on the real window instance
        void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);

        // Draws the splash screen:
        // - background scaled to cover the window (if loaded)
        // - blinking "START GAME" label near the bottom
        //
        // WHY sf::RenderWindow&:
        // - RenderWindow is not meant to be copied
        // - drawing must affect the actual window provided by the caller
        void draw(sf::RenderWindow& fenetre);
    };
}
