#include "ControllerLevel.h"
#include <cmath>
#include <iostream>
#include <SFML/Window.hpp>

#include "CesarVue.h"
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
    // Start level timer
    levelTimerClock.restart();
    // Load HUD font
    hudFontLoaded = hudFont.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");
    if (hudFontLoaded) {
        uiText.setFont(hudFont);
        uiText.setCharacterSize(28);
        uiText.setFillColor(sf::Color::White);
    }
}

void ControllerLevel::handleInput()
{
    mouvement = sf::Vector2f(0.f, 0.f);

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
    sf::Vector2f deplacement = mouvement;

    float longueur = std::sqrt(deplacement.x * deplacement.x + deplacement.y * deplacement.y);

    if (longueur > 0.001f)
    {
        deplacement.x = (deplacement.x / longueur) * VITESSE_JOUEUR;
        deplacement.y = (deplacement.y / longueur) * VITESSE_JOUEUR;
    }

    bool collision = false;

    float playerW = modele.getJoueur().getSize().x;
    float playerH = modele.getJoueur().getSize().y;

    float screenW = modele.getScreenW();
    float screenH = modele.getScreenH();

    // Déplacer sur X
    modele.getJoueur().move(deplacement.x, 0.f);
    sf::FloatRect joueurBounds = modele.getJoueur().getGlobalBounds();

    for (const auto& obsPtr : modele.getObstacleShapes())
    {
        if (modele.getJoueur().getGlobalBounds().intersects(obsPtr->getGlobalBounds()))
        {
            sf::FloatRect obstacleBounds = obsPtr->getGlobalBounds();
            if (deplacement.x > 0)
                modele.getJoueur().setPosition(obstacleBounds.left - playerW, modele.getJoueur().getPosition().y);
            else
                modele.getJoueur().setPosition(obstacleBounds.left + obstacleBounds.width, modele.getJoueur().getPosition().y);

            collision = true;
            break;
        }
    }

    // Limites X
    joueurBounds = modele.getJoueur().getGlobalBounds();
    if (joueurBounds.left < 0) {
        modele.getJoueur().setPosition(0.f, modele.getJoueur().getPosition().y);
        collision = true;
    } else if (joueurBounds.left + playerW > screenW) {
        modele.getJoueur().setPosition(screenW - playerW, modele.getJoueur().getPosition().y);
        collision = true;
    }

    // Déplacer sur Y
    modele.getJoueur().move(0.f, deplacement.y);
    joueurBounds = modele.getJoueur().getGlobalBounds();

    for (const auto& obsPtr : modele.getObstacleShapes())
    {
        if (modele.getJoueur().getGlobalBounds().intersects(obsPtr->getGlobalBounds()))
        {
            sf::FloatRect obstacleBounds = obsPtr->getGlobalBounds();
            if (deplacement.y > 0)
                modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, obstacleBounds.top - playerH);
            else
                modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, obstacleBounds.top + obstacleBounds.height);

            collision = true;
            break;
        }
    }

    // Collision avec objectifs
    for (auto& objectivePtr : modele.getCurrentRoomObjectives()) {
        if (modele.getJoueur().getGlobalBounds().intersects(objectivePtr.getHitbox().getGlobalBounds())) {
            modele.getJoueur().move(deplacement.x, 0.f);
            modele.getJoueur().move(deplacement.y, 0.f);

            const sf::FloatRect objectiveBounds = objectivePtr.getHitbox().getGlobalBounds();
            modele.setObjectiveContactDetectee(true);
            modele.setObjectiveContact(&objectivePtr);

            std::cout << "deplacement x :" << deplacement.x << std::endl;
            std::cout << "deplacement y :" << deplacement.y << std::endl;

            //  From the left
            if (deplacement.x > 0) {
                modele.getJoueur().setPosition(objectiveBounds.left - playerW, modele.getJoueur().getPosition().y);
                std::cout << "1" << std::endl;
            }
            //  From the right
            else if (deplacement.x < 0) {
                modele.getJoueur().setPosition(objectiveBounds.left + objectiveBounds.width, modele.getJoueur().getPosition().y);
                std::cout << "2" << std::endl;
            }

            //  From the top
            if (deplacement.y > 0) {
                modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, objectiveBounds.top - playerH);
                std::cout << "3" << std::endl;
            }
            //  From the bottom
            else if (deplacement.y < 0) {
                modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, objectiveBounds.top + objectiveBounds.height);
                std::cout << "4" << std::endl;
            }

            collision = true;
            break;
        }
    }

    // Limites Y
    joueurBounds = modele.getJoueur().getGlobalBounds();
    if (joueurBounds.top < 0) {
        modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, 0.f);
        collision = true;
    } else if (joueurBounds.top + playerH > screenH) {
        modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, screenH - playerH);
        collision = true;
    }

    if (!modele.getObjectiveContactDetectee()) {
        modele.setCollisionDetectee(collision);
    }

    bool isMoving = (std::abs(deplacement.x) > 0.001f || std::abs(deplacement.y) > 0.001f);

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

    modele.updatePlayerAnimation(isMoving);
    modele.syncPlayerSprite();
}

int ControllerLevel::getRemainingSeconds() const
{
    double elapsedSec;
    double current = levelTimerClock.getElapsedTime().asSeconds();
    if (timerPaused) {
        // elapsed is up to pause start
        elapsedSec = pauseStartSeconds - pausedAccumulated;
    } else {
        elapsedSec = current - pausedAccumulated;
    }
    int rem = static_cast<int>(levelTimerStartSeconds - static_cast<int>(std::floor(elapsedSec)));
    return rem > 0 ? rem : 0;
}

void ControllerLevel::resetLevelTimer()
{
    levelTimerClock.restart();
    timerPaused = false;
    pauseStartSeconds = 0.0;
    pausedAccumulated = 0.0;
}

void ControllerLevel::drawUI(sf::RenderWindow& fenetre)
{
    if (!hudFontLoaded) return;
    std::ostringstream string;
    //  Timer
    string << getRemainingSeconds() << "s";
    uiText.setString(string.str());

    // position top-right with margin
    sf::FloatRect tb = uiText.getLocalBounds();
    float x = fenetre.getSize().x - tb.width - 20.f;
    float y = 20.f;
    uiText.setPosition(x - tb.left, y - tb.top);
    fenetre.draw(uiText);
}

void ControllerLevel::setTimerPaused(bool p)
{
    if (p == timerPaused) return;
    double current = levelTimerClock.getElapsedTime().asSeconds();
    if (p) {
        // pausing now
        pauseStartSeconds = current;
        timerPaused = true;
    } else {
        // resuming: accumulate pause duration
        pausedAccumulated += (current - pauseStartSeconds);
        pauseStartSeconds = 0.0;
        timerPaused = false;
    }
}

Modele::ScoreDetails ControllerLevel::getScoreDetails() const
{
    const std::vector<Objective>& objectives = modele.getCurrentRoomObjectives();
    int remainingSeconds = getRemainingSeconds();
    return Modele::ScoreCalculator::calculateScore(objectives, remainingSeconds);
}

bool ControllerLevel::areAllSecondaryObjectivesCompleted() const
{
    const std::vector<Objective>& objectives = modele.getCurrentRoomObjectives();
    return Modele::ScoreCalculator::areAllSecondaryObjectivesCompleted(objectives);
}

void ControllerLevel::checkDoors()
{
    sf::FloatRect joueurBounds = modele.getJoueur().getGlobalBounds();
    const float playerW = modele.getJoueur().getSize().x;
    const float playerH = modele.getJoueur().getSize().y;

    for (const auto& door : modele.getCurrentRoomDoors())
    {
        sf::FloatRect doorBounds;
        if (door.visualShape)
            doorBounds = door.visualShape->getGlobalBounds();
        else
            doorBounds = door.bounds;

        if (joueurBounds.intersects(doorBounds))
        {
            std::string opposite;
            if (door.direction == "up")    opposite = "down";
            else if (door.direction == "down") opposite = "up";
            else if (door.direction == "left") opposite = "right";
            else if (door.direction == "right") opposite = "left";
            else opposite = door.direction;

            if (door.targetRoomIndex < 0)
            {
                // Ask confirmation to quit the level
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
                    // Check if all secondary AND primary objectives are completed
                    if (areAllSecondaryObjectivesCompleted() && Modele::ScoreCalculator::areAllPrimaryObjectivesCompleted(modele.getCurrentRoomObjectives()))
                    {
                        // Show score screen
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
                    }
                    
                    exitRequestedFlag = true;
                    
                } else {
                    // canceled -> do not exit level
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

            break;
        }
    }
}

void ControllerLevel::processCollisions(Vue::DialogueManager& dialogueManager)
{
    if (modele.getObjectiveContactDetectee()) {
        Objective* contactObj = modele.getObjectiveContact();
        if (!contactObj) return;
        std::cout << "[ControllerLevel] Objective contact detected: " << contactObj->getTitle() << std::endl;
        std::cout << "[ControllerLevel] isCesar() = " << contactObj->isCesar() << std::endl;
        std::cout << "[ControllerLevel] Opening dialog: " << contactObj->getDialogueRef() << std::endl;

        if (contactObj->isAccomplished()) {
            dialogueManager.startDialogueSequence("accomplished_objective");
        } else {
            dialogueManager.startDialogueSequence(contactObj->getDialogueRef());

            // Si c'est un objectif César, signaler à Controleur d'ouvrir la fenêtre après dialogue
            if (contactObj->isCesar()) {
                cesrObjective = contactObj;
                openCesarWindow = true;  // Flag pour Controleur (sera consommé après fin dialogue)
            }
            else {
                contactObj->setAccomplished(true);
            }
        }

        
        modele.setObjectiveContactDetectee(false);
    }
    else if (modele.isJoueurDetecte())
    {
        // Do not start the dialogue here (would be called every frame and restart it).
        // Just set the model flag; the top-level `Controleur` will start the dialogue
        // once using its `agentDialogueLaunched` guard.
        modele.setJoueurDetecte(true);
    }
}

} // namespace Controleur
