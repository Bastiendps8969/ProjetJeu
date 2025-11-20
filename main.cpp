#include <iostream>

#include "Modele/Modele.h"
#include "Vue/Vue.h"
#include "Controleur.h"

int main()
{
    Modele::Modele modele;
    Vue::Vue vue(modele);
    Controleur::Controleur controleur(modele, vue);

    // Lance la boucle principale (ouvre la fenêtre SFML)
    controleur.gererBoucle();

    std::cout << "Hello World!\n";
    std::cout << modele.getPlayer().getPlayerTotalScore();
    return 0;
}