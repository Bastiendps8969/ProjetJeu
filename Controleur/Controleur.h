#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "Modele.h"
#include "Vue.h"
#include "DialogueManager.h"
#include "ControllerLevel.h"

namespace Controleur
{
    class Controleur
    {
    private:
        // Fenêtre
        sf::RenderWindow fenetre;
        // Vue SFML pour gérer le redimensionnement et mise à l'échelle
        sf::View mainView;

        // (level movement moved to ControllerLevel)

        // Modèle et vue
        Modele::Modele& modele;
        Vue::Vue& vue;

        // Level controller (contains per-level logic)
        std::unique_ptr<ControllerLevel> niveauController;

        // --- NOUVEAU: Affichage du menu d'accueil ---
        void afficherMenuAccueil();

    public:
        // Constructeur
        Controleur(Modele::Modele& modele, Vue::Vue& vue);

        // Boucle principale
        void gererBoucle();
    };
}