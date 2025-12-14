#include "Controleur.h"
#include <SFML/Window.hpp>
#include "Modele.h"
#include "Vue.h"
#include "HomePage.h"
#include "DialogueManager.h"
#include "CesarVue.h"
#include <cmath>
#include <iostream>

#include "ControllerLevel.h"

namespace Controleur
{
    // Constructeur
    Controleur::Controleur(Modele::Modele& modele, Vue::Vue& vue)
        : modele(modele), vue(vue),
          fenetre(sf::VideoMode::getDesktopMode(), "Déplacement du personnage", sf::Style::Fullscreen)
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
        bool cesrDialogueClosed = false;  // Track si dialogue César terminé pour ouvrir CesarVue

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

            // Lancer le dialogue UNE SEULE FOIS si le joueur est détecté par un ennemi
            if (modele.isJoueurDetecte() && !agentDialogueLaunched && !dialogueManager.isDialogueActive())
            {
                dialogueManager.startDialogueSequence("agent_detected");
                agentDialogueLaunched = true;
            }
            if (!modele.isJoueurDetecte())
            {
                agentDialogueLaunched = false;
            }

            // Si dialogue objectif César terminé, ouvrir la fenêtre CesarVue
            bool shouldOpen = niveauController->shouldOpenCesarWindow();
            bool dialogueNotActive = !dialogueManager.isDialogueActive();
            bool notClosedYet = !cesrDialogueClosed;
            
            if (shouldOpen && dialogueNotActive && notClosedYet)
            {
                cesrDialogueClosed = true;
                // Ouvrir la fenêtre César
                CesarVue cesarVue(niveauController->getCesarObjective());
                
                // Boucle de gestion de la fenêtre César
                while (fenetre.isOpen() && !cesarVue.shouldWindowClose()) {
                    sf::Event cesarEvent;
                    while (fenetre.pollEvent(cesarEvent)) {
                        if (cesarEvent.type == sf::Event::Closed) {
                            fenetre.close();
                        }
                        cesarVue.handleEvent(cesarEvent, fenetre);
                    }
                    
                    fenetre.clear(sf::Color::Black);
                    cesarVue.draw(fenetre);
                    fenetre.display();
                }
                
                niveauController->resetCesarWindowFlag();
                cesrDialogueClosed = false;
            }

            // Geler le gameplay si un dialogue est actif
            if (!dialogueManager.isDialogueActive())
            {
                niveauController->handleInput();
                niveauController->update();
                modele.mettreAJourObstacles();
                modele.updateEnemies(); // update enemy logic + animations (from sav)
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

    // NOTE: door-checking, input handling and update now live in ControllerLevel.
    // The old free-standing implementations were removed during the merge
    // to avoid duplicate definitions (ControllerLevel handles level logic).
} // FIN du namespace Controleur