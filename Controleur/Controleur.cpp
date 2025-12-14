#include "Controleur.h"
#include <SFML/Window.hpp>
#include "Modele.h"
#include "Vue.h"
#include "HomePage.h"
#include "DialogueManager.h"
#include <cmath>
#include <iostream>

#include "ControllerLevel.h"

namespace Controleur
{
    // Constructeur
    Controleur::Controleur(Modele::Modele& modele, Vue::Vue& vue)
        : modele(modele), vue(vue),
          fenetre(sf::VideoMode::getDesktopMode(), "Déplacement du personnage", sf::Style::Fullscreen),
          mouvement(0.f, 0.f)
    {
        fenetre.setFramerateLimit(60);
        niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
    }

    // Affiche le menu d'accueil (importé de ProjetJeu)
    void Controleur::afficherMenuAccueil()
    {
        // Lambda pour récupérer les scores du joueur
        auto getScores = [this]() -> std::vector<int> {
            std::vector<int> scores(12, 0); // Placeholder: 12 niveaux avec score 0
            return scores;
        };

        Vue::HomePage homePage(getScores);

        while (fenetre.isOpen() && homePage.isActive())
        {
            sf::Event evenement;
            while (fenetre.pollEvent(evenement))
            {
                if (evenement.type == sf::Event::Closed)
                    fenetre.close();

                if ((evenement.type == sf::Event::KeyPressed)
                    && (evenement.key.code == sf::Keyboard::Escape))
                    fenetre.close();

                homePage.handleEvent(evenement, fenetre);
            }

            homePage.draw(fenetre);
        }

        fenetre.setFramerateLimit(60);
    }

    // DÉFINITION : Boucle principale
    void Controleur::gererBoucle()
    {
        afficherMenuAccueil();

        Vue::DialogueManager dialogueManager;
        bool agentDialogueLaunched = false; // Ajout

        sf::Clock fpsTimer;
        int fpsFrames = 0;

        while (fenetre.isOpen())
        {
            sf::Event evenement;
            while (fenetre.pollEvent(evenement))
            {
                if (evenement.type == sf::Event::Closed)
                {   fenetre.close();    }

                if ((evenement.type == sf::Event::KeyPressed)
                    && (evenement.key.code == sf::Keyboard::Escape))
                {   fenetre.close();    }

                dialogueManager.handleEvent(evenement);
            }

            fenetre.clear(sf::Color::Black);

            // Déléguer au controller de niveau la prise en charge des collisions
            niveauController->processCollisions(dialogueManager);

            // Geler le gameplay si un dialogue est actif
            if (!dialogueManager.isDialogueActive())
            {
                niveauController->handleInput();
                niveauController->update();
                modele.mettreAJourObstacles();
                niveauController->checkDoors();
            }

            vue.dessiner(fenetre);

            dialogueManager.update(fenetre.getSize());
            dialogueManager.draw(fenetre);

            fenetre.display();

            ++fpsFrames;
            if (fpsTimer.getElapsedTime().asSeconds() >= 1.0f)
            {
                std::cout << "FPS: " << fpsFrames << std::endl;
                fpsFrames = 0;
                fpsTimer.restart();
            }
        }
    }

    // DÉFINITION : Vérification des portes (implémentation de base)
    void Controleur::verifierPorte()
    {
        sf::FloatRect joueurBounds = modele.getJoueur().getGlobalBounds();
        const float playerW = modele.getJoueur().getSize().x;
        const float playerH = modele.getJoueur().getSize().y;

        for (const auto& door : modele.getCurrentRoomDoors())
        {
            // Utiliser la forme visuelle si elle existe, sinon fallback sur door.bounds
            sf::FloatRect doorBounds;
            if (door.visualShape)
                doorBounds = door.visualShape->getGlobalBounds();
            else
                doorBounds = door.bounds;

            if (joueurBounds.intersects(doorBounds))
            {
                // Calculer la direction opposée pour positionner correctement le joueur
                std::string opposite;
                if (door.direction == "up")    opposite = "down";
                else if (door.direction == "down") opposite = "up";
                else if (door.direction == "left") opposite = "right";
                else if (door.direction == "right") opposite = "left";
                else opposite = door.direction; // fallback

                // Appel du modèle pour changer de pièce
                if (modele.changeRoom(door.targetRoomIndex, opposite))
                {
                    std::cout << "Changement de piece vers ID " << door.targetRoomIndex
                              << " (entree: " << opposite << ").\n";
                }
                else
                {
                    std::cout << "Echec du changement de piece vers ID " << door.targetRoomIndex << ".\n";
                }

                // On sort après un changement pour éviter plusieurs transitions simultanées
                break;
            }
        }
    }

    // DÉFINITION : Gestion de l'entrée utilisateur
    void Controleur::gererEntree()
    {
        mouvement = sf::Vector2f(0.f, 0.f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
        { mouvement.y -= 1.0f; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        { mouvement.y += 1.0f; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
        { mouvement.x -= 1.0f; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        { mouvement.x += 1.0f; }
    }

    // DÉFINITION : Mise à jour du modèle (Physique et collisions)
    void Controleur::mettreAJour()
    {
        const float VITESSE_JOUEUR = 5.0f;
        sf::Vector2f deplacement = mouvement;

        float longueur = std::sqrt(deplacement.x * deplacement.x + deplacement.y * deplacement.y);

        if (longueur > 0.001f)
        {
            // Normalisation pour vitesse constante
            deplacement.x = (deplacement.x / longueur) * VITESSE_JOUEUR;
            deplacement.y = (deplacement.y / longueur) * VITESSE_JOUEUR;
        }

        bool collision = false;

        float playerW = modele.getJoueur().getSize().x;
        float playerH = modele.getJoueur().getSize().y;

        // Utilisation des dimensions de l'écran du Modèle
        float screenW = modele.getScreenW();
        float screenH = modele.getScreenH();


        // ===================================
        // A) Tenter le déplacement sur X
        // ===================================
        modele.getJoueur().move(deplacement.x, 0.f);
        sf::FloatRect joueurBounds = modele.getJoueur().getGlobalBounds();

        // Vérification des limites de la carte sur X (Clamping)
        joueurBounds = modele.getJoueur().getGlobalBounds();
        if (joueurBounds.left < 0) {
            modele.getJoueur().setPosition(0.f, modele.getJoueur().getPosition().y);
            collision = true;
        } else if (joueurBounds.left + playerW > screenW) {
            modele.getJoueur().setPosition(screenW - playerW, modele.getJoueur().getPosition().y);
            collision = true;
        }


        // ===================================
        // B) Tenter le déplacement sur Y
        // ===================================
        modele.getJoueur().move(0.f, deplacement.y);
        joueurBounds = modele.getJoueur().getGlobalBounds(); // Mettre à jour la bounding box

        // Vérification des limites de la carte sur Y (Clamping)
        joueurBounds = modele.getJoueur().getGlobalBounds();
        if (joueurBounds.top < 0) {
            modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, 0.f);
            collision = true;
        } else if (joueurBounds.top + playerH > screenH) {
            modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, screenH - playerH);
            collision = true;
        } // FIN de la gestion des limites Y

        // Transmet l'état de collision au modèle
        modele.setCollisionDetectee(collision);

        // Détermine si le joueur bouge (après normalisation)
        bool isMoving = (std::abs(deplacement.x) > 0.001f || std::abs(deplacement.y) > 0.001f);

        // Définir la direction d'animation selon le vecteur de déplacement
        if (isMoving)
        {
            // Prioriser l'axe dominant
            if (std::abs(deplacement.x) > std::abs(deplacement.y))
            {
                if (deplacement.x > 0) modele.setPlayerDirection(4); // 4 = right
                else modele.setPlayerDirection(2); // 2 = left
            }
            else
            {
                if (deplacement.y < 0) modele.setPlayerDirection(1); // 1 = up (Y négatif vers le haut)
                else modele.setPlayerDirection(3); // 3 = down
            }
        }

        // Mettre à jour l'animation du joueur (fait avancer la frame si moving)
        modele.updatePlayerAnimation(isMoving);

        // Synchroniser le sprite interne du modèle (échelle + position)
        modele.syncPlayerSprite();
    } // FIN de la fonction mettreAJour()
} // FIN du namespace Controleur