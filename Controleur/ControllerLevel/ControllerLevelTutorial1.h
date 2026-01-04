
#ifndef TESTCOLLISION_CONTROLLERLEVELTUTO1_H
#define TESTCOLLISION_CONTROLLERLEVELTUTO1_H

#include "Level.h"
#include "../ControllerLevel.h"

// Tutorial-specific level controller.
// Inherits from the generic ControllerLevel to reuse all common gameplay logic
// (movement, collisions, doors, timer, HUD, lives, scoring, etc.).
//
// This class currently acts as a thin specialization / placeholder:
// it does not add new data or override behavior yet, but it provides a dedicated
// type for "Tutorial 1" so tutorial-specific rules can be introduced later
// without modifying the base controller.
class ControllerLevelTutorial1 : public Controleur::ControllerLevel {
private:
    // No additional state yet.
public:
    // Dependencies are passed by reference (same as base class):
    // - avoids copies of heavy objects,
    // - expresses non-null mandatory dependencies,
    // - the lifetime is managed externally (aggregation).
    ControllerLevelTutorial1(Modele::Modele& modele, Vue::Vue& vue, sf::RenderWindow& fenetre);

    // Destructor prints a debug trace so we can track object lifetime.
    ~ControllerLevelTutorial1();
};

#endif //TESTCOLLISION_CONTROLLERLEVELTUTO1_H
