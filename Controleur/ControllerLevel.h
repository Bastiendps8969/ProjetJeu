
#pragma once

#include <SFML/Graphics.hpp>
#include "Modele.h"
#include "Vue.h"
#include "ScoreCalculator.h"

// Forward declaration to avoid including the full header here.
// This reduces compile-time coupling between Controller and Dialogue system.
namespace Vue { class DialogueManager; }

namespace Controleur {

class ControllerLevel {
public:
    // The controller does NOT own the model, view, or window.
    // They are provided from outside and must outlive this controller (aggregation).
    ControllerLevel(Modele::Modele& modele, Vue::Vue& vue, sf::RenderWindow& fenetre);

    // Read user input (keyboard state) and build the intended movement vector.
    void handleInput();

    // Update level logic: movement normalization, collisions, animation sync, etc.
    void update();

    // Check and handle doors / room transitions (including exit flow).
    void checkDoors();

    // Process consequences related to collisions:
    // - objective contact -> start dialogues / set accomplishment
    // - detection -> life loss and detection counters
    void processCollisions(Vue::DialogueManager &dialogueManager);

    // Flag consumed by a higher-level controller to open a special "Cesar" window
    // after an objective dialogue has finished.
    bool shouldOpenCesarWindow() const;
    Objective* getCesarObjective() const;
    void resetCesarWindowFlag();

    // Used by the top-level controller / game loop to know if the level wants to exit.
    bool isExitRequested() const { return exitRequestedFlag; }

    // Score API (delegates to ScoreCalculator).
    Modele::ScoreDetails getScoreDetails() const;
    bool areAllPrimaryObjectivesCompleted() const;

    // Level timer API (countdown).
    int getRemainingSeconds() const;
    void resetLevelTimer();
    void drawUI(sf::RenderWindow& fenetre);

    // Pause control for the level timer (public so upper controller can toggle it).
    void setTimerPaused(bool p);
    bool isTimerPaused() const { return timerPaused; }

    // Life system API (delegates to the model).
    int getLives() const;
    void loseLivesByDetection(bool isHuman);
    bool isGameOver() const;

private:
    // Aggregation: references enforce "non-null" dependencies and avoid copies.
    // Lifetime must be managed by the caller.
    Modele::Modele& modele;
    Vue::Vue& vue;
    sf::RenderWindow& fenetre;

    // Current movement direction computed from input (not yet scaled by speed).
    sf::Vector2f mouvement;

    // --- Cesar window support ---
    // openCesarWindow is a one-shot flag consumed by higher layers.
    bool openCesarWindow = false;

    // Pointer used because the Cesar objective is optional (nullptr means "none").
    // The pointed Objective is owned/stored by the model; controller only references it.
    Objective* cesarObjective = nullptr;

    // Set when the player confirmed leaving the level (e.g. via exit door).
    bool exitRequestedFlag = false;

    // Detection guard: ensures life loss happens only once per continuous detection event.
    bool playerWasDetectedLastFrame = false;

    // Present but not used in the .cpp (could be intended for dialogue spam prevention).
    bool agentDialogueStartedForCurrentDetection = false; // prevent re-triggering agent dialog while still detected

    // Level timer (countdown start in seconds).
    const int levelTimerStartSeconds = 300; // 5 minutes

    // Timer internal data.
    sf::Clock levelTimerClock;

    // HUD text resources.
    sf::Font hudFont;
    bool hudFontLoaded = false;
    sf::Text uiText;

    // Pause support for the timer:
    // - pauseStartSeconds: clock time when pause began
    // - pausedAccumulated: total seconds spent paused
    bool timerPaused = false;
    double pauseStartSeconds = 0.0;
    double pausedAccumulated = 0.0; // total seconds paused
};

} // namespace Controleur
