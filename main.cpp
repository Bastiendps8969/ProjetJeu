

#include "Controleur.h"
#include "Modele.h"
#include "Vue.h"

int main()
{
    Modele::Modele modele;
    Vue::Vue vue(modele);
    Controleur::Controleur controleur(modele, vue);

    // Lance la boucle principale (ouvre la fenêtre SFML)
    controleur.gererBoucle();

    return 0;
}
