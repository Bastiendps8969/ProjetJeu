#pragma once

#include <SFML/Graphics.hpp>

#include "Modele.h"
#include "Vue.h"

namespace Vue { class DialogueManager; }

namespace Controleur {

class ControllerLevel {
public:
    ControllerLevel(Modele::Modele& modele, Vue::Vue& vue, sf::RenderWindow& fenetre);

    // Gère l'entrée utilisateur (lecture de l'état des touches)
    void handleInput();

    // Mettre à jour la logique du niveau (physique, collisions, objectifs)
    void update();

    // Vérifier et gérer les portes / transitions de salle
    void checkDoors();

    // Traiter conséquences liées aux collisions (déclencher dialogues, flags)
    void processCollisions(Vue::DialogueManager &dialogueManager);

    // Vérifie si une fenêtre César doit s'ouvrir (après dialogue objectif César)
    bool shouldOpenCesarWindow() const { return openCesarWindow; }
    Objective* getCesarObjective() const { return cesrObjective; }
    void resetCesarWindowFlag() { openCesarWindow = false; }

private:
    Modele::Modele& modele;
    Vue::Vue& vue;
    sf::RenderWindow& fenetre;

    // Mouvement courant calculé à partir de l'entrée
    sf::Vector2f mouvement;

    // État pour la fenêtre César
    bool openCesarWindow = false;
    Objective* cesrObjective = nullptr;
};

} // namespace Controleur
