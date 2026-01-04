#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

namespace Vue {

/**
 * GameOverPage
 * ----------------
 * Dedicated full-screen page shown when the player loses all lives.
 * - Attempts to load a single high-quality final image (Game_OverFinal.png).
 * - Draws the image scaled to cover the window while preserving quality when possible.
 * - Shows a small blinking prompt that asks the player to press Enter/Space to return.
 *
 * This class is intentionally lightweight and presentation-focused: the image fills the
 * screen, a small prompt blinks, and a simple fallback is drawn if the asset is missing.
 */
class GameOverPage {
private:
    bool active = true;

    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    bool bgLoaded = false;

    // If false, the image will not be upscaled beyond its native resolution
    // Default: allow upscaling so the image fills the screen
    bool allowUpscale = true;

    // Simple text elements for fallback and prompt
    sf::Font font;
    sf::Text titleText;
    sf::Text promptText;
    // Clock used to animate blinking of the prompt, same behavior as SplashPage
    sf::Clock blinkClock;

    // Path of the asset actually loaded (empty if none)
    std::string selectedPath;

    // Minimal candidate list (tries a small set of filenames)
    std::vector<std::string> candidatePaths();

public:
    GameOverPage();

    bool isActive() const { return active; }
    void setActive(bool v) { active = v; }

    // Control whether upscaling is permitted (default: true so image fills screen)
    void setAllowUpscale(bool v) { allowUpscale = v; }
    bool getAllowUpscale() const { return allowUpscale; }

    void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);
    void draw(sf::RenderWindow& fenetre);
};

} // namespace Vue