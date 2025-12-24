#include "ControllerLevel.h"
#include <cmath>
#include <iostream>
#include <SFML/Window.hpp>
#include "../Vue/DialogueManager.h"

namespace Controleur {

ControllerLevel::ControllerLevel(Modele::Modele& modele, Vue::Vue& vue, sf::RenderWindow& fenetre)
    : modele(modele), vue(vue), fenetre(fenetre), mouvement(0.f, 0.f)
{
}

// --- Ajouts pour la fenêtre Cesar ---
bool ControllerLevel::shouldOpenCesarWindow() const {
    return openCesarWindow;
}

Objective* ControllerLevel::getCesarObjective() {
    return cesarObjective;
}

void ControllerLevel::resetCesarWindowFlag() {
    openCesarWindow = false;
    cesarObjective = nullptr;
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
    for (auto& objectiveRef : modele.getCurrentRoomObjectives()) {
        if (modele.getJoueur().getGlobalBounds().intersects(objectiveRef.getHitbox().getGlobalBounds())) {
            modele.getJoueur().move(deplacement.x, 0.f);
            modele.getJoueur().move(deplacement.y, 0.f);

            const sf::FloatRect objectiveBounds = objectiveRef.getHitbox().getGlobalBounds();
            modele.setObjectiveContactDetectee(true);
            modele.setObjectiveContact(&objectiveRef);

            std::cout << "[ControllerLevel] Collision with objective: '" << objectiveRef.getTitle()
                      << "' cesar=" << objectiveRef.isCesar()
                      << " dialogueRef=" << objectiveRef.getDialogueRef() << std::endl;

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

            if (modele.changeRoom(door.targetRoomIndex, opposite))
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
                std::cout << "[ControllerLevel] Detected Cesar objective: " << contactObj->getTitle() << std::endl;
                cesarObjective = contactObj;
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
