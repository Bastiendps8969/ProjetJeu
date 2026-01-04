
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include "../Modele/ScoreCalculator.h"

namespace Vue
{
    // ScoreWindow: an UI screen to display either:
    // 1) A legacy list of scores per level (plus total), or
    // 2) A detailed breakdown for a completed level (ScoreDetails).
    //
    // The window is typically used in a modal loop:
    // while(window.isOpen() && scoreWindow.isActive()) { ... scoreWindow.draw(window); }
    class ScoreWindow
    {
    private:
        // Modal state: while active == true, the screen should remain displayed.
        bool active = true;

        // UI resources.
        sf::Font font;
        bool fontLoaded = false;

        // Main texts.
        sf::Text titleText;
        sf::Text totalScoreText;

        // Reused container for score lines.
        // In legacy mode: one line per level ("Tutorial", "Level i").
        // In detailed mode: breakdown lines (time score, objectives, malus...).
        std::vector<sf::Text> levelScores;

        // Detailed score breakdown texts (kept as members so their font/color stay valid).
        sf::Text timeScoreText;
        sf::Text primaryScoreText;
        sf::Text secondaryScoreText;
        sf::Text detectionCountText;
        sf::Text detectionMalusText;

        // Back button UI elements.
        sf::RectangleShape backButton;
        sf::Text backButtonLabel;

        // Callbacks used to fetch data without coupling the window to the model.
        // This is a simple form of dependency injection via std::function.
        //
        // WHY std::function callbacks:
        // - decouples the UI screen from the game model (no direct dependency on Modele instance)
        // - allows caller to provide data dynamically (fresh values each time it's called)
        // - supports two "modes" (legacy vector<int> or detailed ScoreDetails)
        std::function<std::vector<int>()> getScoresCb;
        std::function<Modele::ScoreDetails()> getDetailsCb;

        // Initialize legacy score list UI.
        //
        // WHY const reference:
        // - avoids copying the score vector
        // - function only reads scores to build sf::Text objects
        void initializeTexts(const std::vector<int>& scores);

        // Initialize detailed score breakdown UI.
        //
        // WHY const reference:
        // - avoids copying ScoreDetails
        void initializeDetailedTexts(const Modele::ScoreDetails& details);

        // Bounding rectangle for the "BACK" button.
        // It is updated each draw() and reused in handleEvent() for click hit-testing.
        sf::FloatRect backButtonRect;

    public:
        // Legacy constructor for backward compatibility:
        // takes a callback returning a vector of per-level scores.
        //
        // WHY callback passed by value then moved:
        // - allows passing lambdas/functors easily
        // - std::function is movable; moving avoids an extra copy
        ScoreWindow(std::function<std::vector<int>()> getScores);

        // New constructor for level completion:
        // takes a callback returning a ScoreDetails structure (breakdown).
        //
        // WHY callback:
        // - keeps ScoreWindow independent from the model
        // - caller chooses how to compute score breakdown (e.g., based on current mission)
        ScoreWindow(std::function<Modele::ScoreDetails()> getDetails);

        bool isActive() const { return active; }
        void setActive(bool v) { active = v; }

        // Handle keyboard/mouse/window events (Escape/Return/click BACK to close).
        //
        // WHY const sf::Event&:
        // - avoids copying the event object
        // - event is read-only for this handler
        void handleEvent(const sf::Event& event);

        // Render the score window UI.
        //
        // WHY sf::RenderWindow&:
        // - drawing must affect the real window (RenderWindow is non-copyable)
        void draw(sf::RenderWindow& fenetre);
    };
}
