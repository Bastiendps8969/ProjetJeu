
#pragma once

#include <SFML/Graphics.hpp>
#include <string>

// Forward declaration to reduce header dependencies.
// The View only needs a reference to the model type here.
namespace Modele { class Modele; }

namespace Vue
{
    // View layer (MVC): responsible for rendering the current game state.
    // It reads data from the model and draws map/doors/obstacles/objectives/player/enemies + HUD texts.
    class Vue
    {
    private:
        // Aggregation: the view does not own the model.
        // Using a reference avoids copies and guarantees non-null dependency.
        Modele::Modele& modele;

        // Font and texts used for on-screen debug/HUD messages.
        sf::Font font;
        sf::Text collisionText;
        bool fontCharge = false;

        // Text displayed when the player is detected by enemies.
        sf::Text joueurDetecteText;

        // Text displaying the remaining lives (updated each frame).
        sf::Text livesText;

    public:
        // Constructor: receives the model by reference (aggregation).
        // Also loads the HUD font and initializes text objects.
        Vue(Modele::Modele& modele);

        // Main render function: draws the entire scene (map, entities, UI texts).
        void dessiner(sf::RenderWindow& fenetre);

        // Event handling entry point for the view.
        // Currently unused (menus/dialogues are handled elsewhere).
        void handleEvent(const sf::Event& event, sf::RenderWindow& fenetre);
    };
}
