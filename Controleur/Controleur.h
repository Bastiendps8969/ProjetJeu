
#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "Modele.h"
#include "Vue.h"
#include "../Vue/PauseMenu.h"
#include "DialogueManager.h"
#include "ControllerLevel.h"

namespace Controleur
{
    class Controleur
    {
    private:
        // Main game window.
        // Owned by this controller (lifetime managed here).
        sf::RenderWindow fenetre;

        // (level movement moved to ControllerLevel)

        // Model and view are NOT owned here:
        // we keep references (aggregation) to avoid copies and ensure non-null dependencies.
        Modele::Modele& modele;
        Vue::Vue& vue;

        // Per-level controller holding gameplay logic (movement/collisions/doors/timer/lives).
        // unique_ptr expresses sole ownership + RAII cleanup on reset().
        std::unique_ptr<ControllerLevel> niveauController;

        // Pause menu (stored here, though the .cpp also creates a local PauseMenu instance
        // during the Escape handling loop).
        std::unique_ptr<Vue::PauseMenu> pauseMenu;

        // --- NEW: Main menu / home screen display (splash + home page) ---
        void afficherMenuAccueil();

    public:
        // Constructor: receives model/view by reference (aggregation).
        Controleur(Modele::Modele& modele, Vue::Vue& vue);

        // Main loop: handles events, pause/menu flow, dialogue orchestration,
        // delegates gameplay to ControllerLevel, and renders the frame.
        void gererBoucle();
    };
}
