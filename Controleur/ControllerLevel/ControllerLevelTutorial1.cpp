

#include "ControllerLevelTutorial1.h"
#include <iostream>

ControllerLevelTutorial1::ControllerLevelTutorial1(Modele::Modele &modele, Vue::Vue &vue, sf::RenderWindow &fenetre) :
    // Delegate construction to the base ControllerLevel:
    // we reuse its initialization (timer, HUD font, internal flags, etc.).
    Controleur::ControllerLevel(modele, vue, fenetre)
{
    // Debug trace: confirms that the tutorial controller instance was created.
    std::cout << "CONTROLLER LEVEL TUTORIAL 1 CREATED" << std::endl;
}

ControllerLevelTutorial1::~ControllerLevelTutorial1()
{
    // Debug trace: confirms destruction (useful to verify RAII / unique_ptr reset).
    std::cout << "ControllerLevel Tuto1::~ControllerLevelTuto1" << std::endl;
}
