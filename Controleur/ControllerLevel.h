#pragma once

#include <SFML/Graphics.hpp>

#include "Modele.h"
#include "Vue.h"
#include "ScoreCalculator.h"

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
    bool isExitRequested() const { return exitRequestedFlag; }

    // Score API
    Modele::ScoreDetails getScoreDetails() const;
    bool areAllSecondaryObjectivesCompleted() const;

    // Level timer API
    int getRemainingSeconds() const;
    void resetLevelTimer();
    void drawUI(sf::RenderWindow& fenetre);
    // Pause control for the level timer (public so Controleur can toggle it)
    void setTimerPaused(bool p);
    bool isTimerPaused() const { return timerPaused; }

    // Life system API
    int getLives() const;
    void loseLivesByDetection(bool isHuman);
    bool isGameOver() const;


private:
    Modele::Modele& modele;
    Vue::Vue& vue;
    sf::RenderWindow& fenetre;

    // Mouvement courant calculé à partir de l'entrée
    sf::Vector2f mouvement;

    // État pour la fenêtre César
    bool openCesarWindow = false;
    Objective* cesrObjective = nullptr;
    bool exitRequestedFlag = false;
    bool playerWasDetectedLastFrame = false;  // Track if player was detected to apply life loss only once
    // Level timer (countdown in seconds)
    const int levelTimerStartSeconds = 10; // 300

    // Timer internal data
    sf::Clock levelTimerClock;
    sf::Font hudFont;
    bool hudFontLoaded = false;
    sf::Text uiText;
    // Pause support for the timer
    bool timerPaused = false;
    double pauseStartSeconds = 0.0;
    double pausedAccumulated = 0.0; // total seconds paused

    
};

} // namespace Controleur
