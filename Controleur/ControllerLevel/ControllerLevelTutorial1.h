//
// Created by bertr on 13-12-25.
//

#ifndef TESTCOLLISION_CONTROLLERLEVELTUTO1_H
#define TESTCOLLISION_CONTROLLERLEVELTUTO1_H
#include "Level.h"
#include "../ControllerLevel.h"


class ControllerLevelTutorial1 : public Controleur::ControllerLevel {
private:

public:
    ControllerLevelTutorial1(Modele::Modele& modele, Vue::Vue& vue, sf::RenderWindow& fenetre);

    ~ControllerLevelTutorial1();
};


#endif //TESTCOLLISION_CONTROLLERLEVELTUTO1_H