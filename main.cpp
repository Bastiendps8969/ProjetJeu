#include "Controleur.h"
#include "Modele.h"
#include "Vue.h"

int main()
{
    Modele::Modele modele;
    Vue::Vue vue(modele);
    Controleur::Controleur controleur(modele, vue);

    controleur.gererBoucle();

    return 0;
}
