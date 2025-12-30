#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
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
        // Fenêtre
        sf::RenderWindow fenetre;

        // (level movement moved to ControllerLevel)

        // Modèle et vue
        Modele::Modele& modele;
        Vue::Vue& vue;

        // Level controller (contains per-level logic)
        std::unique_ptr<ControllerLevel> niveauController;

        // Pause menu (modal during gameplay)
        std::unique_ptr<Vue::PauseMenu> pauseMenu;

        // Stored player scores (appended when a level is exited)
        std::vector<int> playerScores;

        // --- NOUVEAU: Affichage du menu d'accueil ---
        void afficherMenuAccueil();

    public:
        // Constructeur
        Controleur(Modele::Modele& modele, Vue::Vue& vue);

        // Boucle principale
        void gererBoucle();
    };
}