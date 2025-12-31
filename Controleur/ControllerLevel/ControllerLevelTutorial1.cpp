//
// Created by bertr on 13-12-25.
//

#include "ControllerLevelTutorial1.h"

#include <iostream>

ControllerLevelTutorial1::ControllerLevelTutorial1(Modele::Modele &modele, Vue::Vue &vue, sf::RenderWindow &fenetre) :
    Controleur::ControllerLevel(modele, vue, fenetre) {

    std::cout << "CONTROLLER LEVEL TUTORIAL 1 CREATED" << std::endl;
}

ControllerLevelTutorial1::~ControllerLevelTutorial1() {
    std::cout << "ControllerLevel Tuto1::~ControllerLevelTuto1" << std::endl;
}


