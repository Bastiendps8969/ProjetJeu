
#include "ControllerLevel.h"
#include <cmath>
#include <iostream>
#include <SFML/Window.hpp>
#include "../Vue/DialogueManager.h"
#include "../Vue/ConfirmationDialog.h"
#include "../Vue/ScoreWindow.h"
#include <sstream>

// timer members
#include <SFML/System/Clock.hpp>

namespace Controleur {

ControllerLevel::ControllerLevel(Modele::Modele& modele, Vue::Vue& vue, sf::RenderWindow& fenetre)
 : modele(modele), vue(vue), fenetre(fenetre), mouvement(0.f, 0.f)
{
    // Start (or restart) the level timer at construction.
    levelTimerClock.restart();

    // Load HUD font.
    // NOTE: This hardcoded path is Windows-specific.
    hudFontLoaded = hudFont.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");
    if (hudFontLoaded) {
        uiText.setFont(hudFont);
        uiText.setCharacterSize(28);
        uiText.setFillColor(sf::Color::White);
    }
}

// --- Cesar window support ---
bool ControllerLevel::shouldOpenCesarWindow() const {
    return openCesarWindow;
}

Objective* ControllerLevel::getCesarObjective() const {
    return cesarObjective;
}

void ControllerLevel::resetCesarWindowFlag() {
    // One-shot consumption: reset flag and pointer after the upper layer used them.
    openCesarWindow = false;
    cesarObjective = nullptr;
}

void ControllerLevel::handleInput()
{
    // Build a raw direction vector from keyboard state.
    // (Not normalized yet; normalization is done in update().)
    mouvement = sf::Vector2f(0.f, 0.f);

    // ZQSD mapping (French keyboard habits).
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
        mouvement.y -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        mouvement.y += 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
        mouvement.x -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        mouvement.x += 1.0f;
}

void ControllerLevel::update()
{
    const float VITESSE_JOUEUR = 5.0f;

    // Convert input direction to displacement.
    sf::Vector2f deplacement = mouvement;

    // Normalize so diagonal movement isn't faster.
    float longueur = std::sqrt(deplacement.x * deplacement.x + deplacement.y * deplacement.y);
    if (longueur > 0.001f)
    {
        deplacement.x = (deplacement.x / longueur) * VITESSE_JOUEUR;
        deplacement.y = (deplacement.y / longueur) * VITESSE_JOUEUR;
    }

    bool collision = false;

    // Cache frequently used values for clarity and to avoid repeated calls.
    float playerW = modele.getJoueur().getSize().x;
    float playerH = modele.getJoueur().getSize().y;
    float screenW = modele.getScreenW();
    float screenH = modele.getScreenH();

    // ------------------------------------------------------------
    // 1) Move on X axis and resolve collisions with obstacles (AABB).
    // Axis-separated collision resolution is simple and stable.
    // ------------------------------------------------------------
    modele.getJoueur().move(deplacement.x, 0.f);
    sf::FloatRect joueurBounds = modele.getJoueur().getGlobalBounds();

    for (const auto& obsPtr : modele.getObstacleShapes())
    {
        if (modele.getJoueur().getGlobalBounds().intersects(obsPtr->getGlobalBounds()))
        {
            sf::FloatRect obstacleBounds = obsPtr->getGlobalBounds();

            // Push player out of the obstacle depending on direction of travel.
            if (deplacement.x > 0)
                modele.getJoueur().setPosition(obstacleBounds.left - playerW, modele.getJoueur().getPosition().y);
            else
                modele.getJoueur().setPosition(obstacleBounds.left + obstacleBounds.width, modele.getJoueur().getPosition().y);

            collision = true;
            break;
        }
    }

    // Clamp to screen bounds on X.
    joueurBounds = modele.getJoueur().getGlobalBounds();
    if (joueurBounds.left < 0) {
        modele.getJoueur().setPosition(0.f, modele.getJoueur().getPosition().y);
        collision = true;
    } else if (joueurBounds.left + playerW > screenW) {
        modele.getJoueur().setPosition(screenW - playerW, modele.getJoueur().getPosition().y);
        collision = true;
    }

    // ------------------------------------------------------------
    // 2) Move on Y axis and resolve collisions with obstacles (AABB).
    // ------------------------------------------------------------
    modele.getJoueur().move(0.f, deplacement.y);
    joueurBounds = modele.getJoueur().getGlobalBounds();

    for (const auto& obsPtr : modele.getObstacleShapes())
    {
        if (modele.getJoueur().getGlobalBounds().intersects(obsPtr->getGlobalBounds()))
        {
            sf::FloatRect obstacleBounds = obsPtr->getGlobalBounds();

            // Push player out of the obstacle depending on direction of travel.
            if (deplacement.y > 0)
                modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, obstacleBounds.top - playerH);
            else
                modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, obstacleBounds.top + obstacleBounds.height);

            collision = true;
            break;
        }
    }

    // ------------------------------------------------------------
    // 3) Collision with objectives:
    // If player touches objective hitbox, we mark contact in the model
    // so another stage (processCollisions) can handle dialogues & rewards.
    // ------------------------------------------------------------
    for (auto& objectiveRef : modele.getCurrentRoomObjectives()) {
        if (modele.getJoueur().getGlobalBounds().intersects(objectiveRef.getHitbox().getGlobalBounds())) {
            // NOTE: These moves look unusual: they move the player again using deplacement,
            // likely intended to adjust resolution ordering; not changed here.
            modele.getJoueur().move(deplacement.x, 0.f);
            modele.getJoueur().move(deplacement.y, 0.f);

            const sf::FloatRect objectiveBounds = objectiveRef.getHitbox().getGlobalBounds();

            // Store contact state in the model (objective pointer used because it's optional and mutable).
            modele.setObjectiveContactDetectee(true);
            modele.setObjectiveContact(&objectiveRef);

            std::cout << "[ControllerLevel] Collision with objective: '" << objectiveRef.getTitle()
                      << "' cesar=" << objectiveRef.isCesar()
                      << " dialogueRef=" << objectiveRef.getDialogueRef() << std::endl;
            std::cout << "deplacement x :" << deplacement.x << std::endl;
            std::cout << "deplacement y :" << deplacement.y << std::endl;

            // Basic push-out resolution depending on movement direction.
            // From the left
            if (deplacement.x > 0) {
                modele.getJoueur().setPosition(objectiveBounds.left - playerW, modele.getJoueur().getPosition().y);
                std::cout << "1" << std::endl;
            }
            // From the right
            else if (deplacement.x < 0) {
                modele.getJoueur().setPosition(objectiveBounds.left + objectiveBounds.width, modele.getJoueur().getPosition().y);
                std::cout << "2" << std::endl;
            }
            // From the top
            if (deplacement.y > 0) {
                modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, objectiveBounds.top - playerH);
                std::cout << "3" << std::endl;
            }
            // From the bottom
            else if (deplacement.y < 0) {
                modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, objectiveBounds.top + objectiveBounds.height);
                std::cout << "4" << std::endl;
            }

            collision = true;
            break;
        }
    }

    // Clamp to screen bounds on Y.
    joueurBounds = modele.getJoueur().getGlobalBounds();
    if (joueurBounds.top < 0) {
        modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, 0.f);
        collision = true;
    } else if (joueurBounds.top + playerH > screenH) {
        modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, screenH - playerH);
        collision = true;
    }

    // If we are NOT colliding with an objective, we store the collision flag.
    // (Objective contact has its own dedicated flag in the model.)
    if (!modele.getObjectiveContactDetectee()) {
        modele.setCollisionDetectee(collision);
    }

    // Determine if the player is moving (used for animation and direction).
    bool isMoving = (std::abs(deplacement.x) > 0.001f || std::abs(deplacement.y) > 0.001f);

    // Update facing direction based on dominant axis of movement.
    if (isMoving)
    {
        if (std::abs(deplacement.x) > std::abs(deplacement.y))
        {
            if (deplacement.x > 0) modele.setPlayerDirection(4);
            else modele.setPlayerDirection(2);
        }
        else
        {
            if (deplacement.y < 0) modele.setPlayerDirection(1);
            else modele.setPlayerDirection(3);
        }
    }

    // Delegate animation updates to the model.
    modele.updatePlayerAnimation(isMoving);
    modele.syncPlayerSprite();
}

int ControllerLevel::getRemainingSeconds() const
{
    // Compute elapsed time while accounting for pauses:
    // elapsed = (clock time) - (total paused time).
    double elapsedSec;
    double current = levelTimerClock.getElapsedTime().asSeconds();

    if (timerPaused) {
        // If paused, elapsed is measured up to the moment we paused.
        elapsedSec = pauseStartSeconds - pausedAccumulated;
    } else {
        elapsedSec = current - pausedAccumulated;
    }

    int rem = static_cast<int>(levelTimerStartSeconds - static_cast<int>(std::floor(elapsedSec)));
    return rem > 0 ? rem : 0;
}

void ControllerLevel::resetLevelTimer()
{
    // Reset timer and pause bookkeeping.
    levelTimerClock.restart();
    timerPaused = false;
    pauseStartSeconds = 0.0;
    pausedAccumulated = 0.0;
}

void ControllerLevel::drawUI(sf::RenderWindow& fenetre)
{
    // Draw timer HUD at top-right.
    if (!hudFontLoaded) return;

    std::ostringstream string;

    // Timer display (seconds remaining).
    string << getRemainingSeconds() << "s";
    uiText.setString(string.str());

    // Position top-right with margin.
    sf::FloatRect tb = uiText.getLocalBounds();
    float x = fenetre.getSize().x - tb.width - 20.f;
    float y = 20.f;
    uiText.setPosition(x - tb.left, y - tb.top);

    fenetre.draw(uiText);
}

void ControllerLevel::setTimerPaused(bool p)
{
    // Ignore if state doesn't change.
    if (p == timerPaused) return;

    double current = levelTimerClock.getElapsedTime().asSeconds();

    if (p) {
        // Pausing now: remember when the pause started.
        pauseStartSeconds = current;
        timerPaused = true;
    } else {
        // Resuming: accumulate pause duration so elapsed time excludes it.
        pausedAccumulated += (current - pauseStartSeconds);
        pauseStartSeconds = 0.0;
        timerPaused = false;
    }
}

Modele::ScoreDetails ControllerLevel::getScoreDetails() const
{
    // Score computation is delegated to ScoreCalculator (separation of concerns).
    const std::vector<Objective>& objectives = modele.getAllLevelObjectives();
    int remainingSeconds = getRemainingSeconds();

    return Modele::ScoreCalculator::calculateScore(objectives, remainingSeconds, modele.getDetectionCount());
}

bool ControllerLevel::areAllPrimaryObjectivesCompleted() const
{
    // Delegation to ScoreCalculator (keeps controller thin).
    const std::vector<Objective>& objectives = modele.getAllLevelObjectives();
    return Modele::ScoreCalculator::areAllPrimaryObjectivesCompleted(objectives);
}

void ControllerLevel::checkDoors()
{
    sf::FloatRect joueurBounds = modele.getJoueur().getGlobalBounds();
    const float playerW = modele.getJoueur().getSize().x;
    const float playerH = modele.getJoueur().getSize().y;

    // Iterate doors of the current room and handle first intersection.
    for (const auto& door : modele.getCurrentRoomDoors())
    {
        sf::FloatRect doorBounds;

        // Door may have a visual shape; otherwise use the raw bounds.
        if (door.visualShape)
            doorBounds = door.visualShape->getGlobalBounds();
        else
            doorBounds = door.bounds;

        if (joueurBounds.intersects(doorBounds))
        {
            // Compute opposite entry direction when changing rooms.
            std::string opposite;
            if (door.direction == "up") opposite = "down";
            else if (door.direction == "down") opposite = "up";
            else if (door.direction == "left") opposite = "right";
            else if (door.direction == "right") opposite = "left";
            else opposite = door.direction;

            // targetRoomIndex < 0 is used as a sentinel meaning "exit level".
            if (door.targetRoomIndex < 0)
            {
                // Ask confirmation to quit the level (modal UI loop).
                Vue::ConfirmationDialog confirm("Exit level?\nYour progress will be save if you've completed\nall the primary objectives.");
                while (fenetre.isOpen() && confirm.isActive()) {
                    sf::Event ce;
                    while (fenetre.pollEvent(ce)) {
                        if (ce.type == sf::Event::Closed) fenetre.close();
                        confirm.handleEvent(ce, fenetre);
                    }

                    fenetre.clear(sf::Color::Black);
                    vue.dessiner(fenetre);
                    confirm.draw(fenetre);
                    fenetre.display();
                }

                if (!fenetre.isOpen()) break;

                if (confirm.isConfirmed()) {
                    // If confirmed, we optionally show the score window (only if primary objectives are done).
                    if (areAllPrimaryObjectivesCompleted() && Modele::ScoreCalculator::areAllPrimaryObjectivesCompleted(modele.getAllLevelObjectives()))
                    {
                        // Show score screen (modal). Lambda provides dynamic score computation.
                        Vue::ScoreWindow scoreWindow([this]() -> Modele::ScoreDetails {
                           return this->getScoreDetails();
                       });

                        while (fenetre.isOpen() && scoreWindow.isActive())
                        {
                            sf::Event se;
                            while (fenetre.pollEvent(se))
                            {
                                if (se.type == sf::Event::Closed) fenetre.close();
                                scoreWindow.handleEvent(se);
                            }
                            scoreWindow.draw(fenetre);
                        }

                        if (!fenetre.isOpen()) break;

                        // Update model's score table in memory using mission index (not room index).
                        // try/catch used defensively to avoid crashing on unexpected errors.
                        try {
                            Modele::ScoreDetails details = this->getScoreDetails();
                            int missionIdx = modele.getCurrentMissionIndex();
                            modele.setPlayerScore(missionIdx, details.totalScore);
                        } catch (...) {
                            // ignore any unexpected errors during score update
                        }
                    }

                    // Signal to outer game loop that the level should exit.
                    exitRequestedFlag = true;
                } else {
                    // Canceled -> do not exit level.
                    // Nudge player away from the door to avoid immediate re-trigger.
                    int x = modele.getJoueur().getPosition().x;
                    int newY = modele.getJoueur().getPosition().y - 5;
                    modele.getJoueur().setPosition(x, newY);
                }
            }
            else if (modele.changeRoom(door.targetRoomIndex, opposite))
            {
                std::cout << "Changement de piece vers ID " << door.targetRoomIndex
                          << " (entree: " << opposite << ").\n";
            }
            else
            {
                std::cout << "Echec du changement de piece vers ID " << door.targetRoomIndex << ".\n";
            }

            // Handle only one door per frame.
            break;
        }
    }
}

void ControllerLevel::processCollisions(Vue::DialogueManager& dialogueManager)
{
    // ------------------------------------------------------------
    // Objective collision handling (dialogues + accomplishment).
    // The model sets a flag + pointer; controller resolves consequences here.
    // ------------------------------------------------------------
    if (modele.getObjectiveContactDetectee()) {
        Objective* contactObj = modele.getObjectiveContact();
        if (!contactObj) return;

        std::cout << "[ControllerLevel] Objective contact detected: " << contactObj->getTitle() << std::endl;
        std::cout << "[ControllerLevel] isCesar() = " << contactObj->isCesar() << std::endl;
        std::cout << "[ControllerLevel] Opening dialog: " << contactObj->getDialogueRef() << std::endl;

        if (contactObj->isAccomplished()) {
            // If already done, play a generic "accomplished" message.
            dialogueManager.startDialogueSequence("accomplished_objective");
        } else {
            // Otherwise, play objective-specific dialogue.
            dialogueManager.startDialogueSequence(contactObj->getDialogueRef());

            // If it's a "Cesar" objective, we do not mark it accomplished here.
            // Instead we raise a flag so the upper controller can open a special window
            // after the dialogue ends.
            if (contactObj->isCesar()) {
                std::cout << "[ControllerLevel] Detected Cesar objective: " << contactObj->getTitle() << std::endl;
                cesarObjective = contactObj;
                openCesarWindow = true; // Flag for upper controller (consumed after dialogue)
            }
            else {
                // Standard objective: mark as accomplished immediately.
                contactObj->setAccomplished(true);
            }
        }

        // Consume contact detection flag (one-shot).
        modele.setObjectiveContactDetectee(false);
    }
    // ------------------------------------------------------------
    // Player detection handling (life loss + detection counter).
    // Guarded so it triggers once per continuous detection event.
    // ------------------------------------------------------------
    else if (modele.isJoueurDetecte())
    {
        // Do not start the dialogue here (would be called every frame and restart it).
        // Just set the model flag; the top-level `Controleur` will start the dialogue
        // once using its `agentDialogueLaunched` guard.

        // Apply life loss only once per detection (not every frame).
        if (!playerWasDetectedLastFrame)
        {
            // Determine the enemy type that detected the player to apply proper life cost.
            bool isHuman = false;

            // Inspect enemies and find the one currently detecting the player.
            const auto& enemies = modele.getEnemies();
            for (const auto& enemy : enemies)
            {
                if (enemy && enemy->joueurDetecte)
                {
                    // GenericEnemy is the "human" type.
                    // Detect by checking isCamera / isLaser flags.
                    isHuman = !enemy->isCamera && !enemy->isLaser;
                    break;
                }
            }

            loseLivesByDetection(isHuman);

            // Count this detection for scoring.
            modele.incrementDetectionCount();

            // Mark that we already applied the cost for the current detection streak.
            playerWasDetectedLastFrame = true;
        }

        // Keep model's detected flag true while detection persists.
        modele.setJoueurDetecte(true);
    }
    else
    {
        // Reset guard when player is no longer detected.
        playerWasDetectedLastFrame = false;
    }
}

int ControllerLevel::getLives() const
{
    // Controller delegates life state to the model.
    return modele.getLives();
}

void ControllerLevel::loseLivesByDetection(bool isHuman)
{
    // Lose 3 lives if human, 1 otherwise (camera or laser).
    int livesLost = isHuman ? 3 : 1;
    modele.loseLives(livesLost);

    std::cout << "[ControllerLevel] Lives lost: " << livesLost
              << ". Remaining lives: " << modele.getLives() << std::endl;
}

bool ControllerLevel::isGameOver() const
{
    // Controller delegates game-over logic to the model.
    return modele.isGameOver();
}

} // namespace Controleur
