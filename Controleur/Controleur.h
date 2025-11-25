#pragma once

#include <SFML/Graphics.hpp>
#include "Modele.h"
#include "Vue.h"

namespace Controleur
{
    class Controleur
    {
    private:
        // Fenêtre
        sf::RenderWindow fenetre;

        // Mouvement du rectangle jaune
        sf::Vector2f mouvement;

        // Modèle et vue
        Modele::Modele& modele;
        Vue::Vue& vue;

        // private car lancer uniquement qu'à partir de gererBoucle()
        void gererEntree();
        void mettreAJour();

    public:
        // Constructeur
        Controleur(Modele::Modele& modele, Vue::Vue& vue);

        // Boucle principale
        void gererBoucle();
    };
}
