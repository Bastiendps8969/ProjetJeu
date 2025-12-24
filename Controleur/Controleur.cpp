#include "Controleur.h"
#include <SFML/Window.hpp>
#include "Modele.h"
#include "Vue.h"
#include "HomePage.h"
#include "DialogueManager.h"
#include "SplashPage.h"
#include "ChapterLoader.h"
#include "CesarVue.h"
#include "../Vue/PauseMenu.h"
#include "../Vue/ConfirmationDialog.h"
#include <cmath>
#include <iostream>

#include "ControllerLevel.h"
#include "../Vue/PauseMenu.h"

namespace Controleur
{
    // Constructeur
    Controleur::Controleur(Modele::Modele& modele, Vue::Vue& vue)
        : modele(modele), vue(vue),
          fenetre(sf::VideoMode::getDesktopMode(), "Déplacement du personnage", sf::Style::Fullscreen)
    {
        fenetre.setFramerateLimit(60);
        niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
        pauseMenu = std::make_unique<Vue::PauseMenu>(modele);
    }

    // Affiche le menu d'accueil (importé de ProjetJeu)
    void Controleur::afficherMenuAccueil()
    {
        // Lambda pour récupérer les scores du joueur
        auto getScores = [this]() -> std::vector<int> {
            std::vector<int> scores(12, 0); // Placeholder: 12 niveaux avec score 0
            return scores;
        };

        // 1) afficher splash
        Vue::SplashPage splash;
        while (fenetre.isOpen() && splash.isActive())
        {
            sf::Event evenement;
            while (fenetre.pollEvent(evenement))
            {
                if (evenement.type == sf::Event::Closed)
                    fenetre.close();

                if ((evenement.type == sf::Event::KeyPressed)
                    && (evenement.key.code == sf::Keyboard::Escape))
                    fenetre.close();

                splash.handleEvent(evenement, fenetre);
            }
            splash.draw(fenetre);
        }

        if (!fenetre.isOpen()) return;

        // 2) ensuite le menu principal (HomePage) — demander d'utiliser l'image de jeu en fond
        // Passe explicitement le chemin vers CherubMenuJeu.png (essayez Asset/Menu/)
        Vue::HomePage homePage(getScores, "Asset/Menu/CherubMenuJeu.png");

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

        // Après la fermeture du menu principal, regarder si l'utilisateur a choisi un niveau
        int selChapter = homePage.getSelectedChapter();
        int selLevel = homePage.getSelectedLevel();
        if (selChapter >= 0 && selLevel >= 0)
        {
            // Mapping simple chapter->rooms (adapter si tu ajoutes d'autres rooms)
            int roomId = -1;
            if (selChapter == 0)
            {
                // Chapitre "Opération Hades" : Tutoriel -> room 0, Lvl 1 -> room 1, Lvl 2 -> room 2
                if (selLevel == 0) roomId = 0;
                else if (selLevel == 1) roomId = 1;
                else if (selLevel == 2) roomId = 2;
                else roomId = 0; // fallback
            }
            // si tu ajoutes d'autres chapitres, gère ici leur mapping (selChapter == 1 ...)

            if (roomId >= 0)
            {
                if (!modele.changeRoom(roomId, "")) // entryDirection vide => centrage par défaut
                {
                    std::cerr << "[Controleur] Échec du chargement du niveau (room " << roomId << ")\n";
                }
                else
                {
                    std::cout << "[Controleur] Niveau sélectionné chargé : room " << roomId << "\n";
                }
            }
        }

        fenetre.setFramerateLimit(60);
    }

    // DÉFINITION : Boucle principale
    void Controleur::gererBoucle()
    {
        // Show main menu first
        afficherMenuAccueil();

        // Create level controller when entering gameplay (fresh instance)
        if (!niveauController) {
            // Ensure model is reset so any previous progress is cleared
            modele.reset();
            niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
        }

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
                {
                    // Show pause menu and require confirmation for destructive actions.
                    bool abortOuter = false;

                    while (fenetre.isOpen()) {
                        Vue::PauseMenu pause(modele);
                        pause.setActive(true);

                        // Pause loop
                        while (fenetre.isOpen() && pause.isActive()) {
                            sf::Event pe;
                            while (fenetre.pollEvent(pe)) {
                                if (pe.type == sf::Event::Closed) fenetre.close();
                                pause.handleEvent(pe, fenetre);
                            }

                            // Draw current game frame behind the pause menu
                            fenetre.clear(sf::Color::Black);
                            vue.dessiner(fenetre);
                            pause.draw(fenetre);
                            fenetre.display();
                        }

                        if (!fenetre.isOpen()) { abortOuter = true; break; }

                        auto sel = pause.getSelectedOption();
                        if (sel == Vue::PauseMenu::Option::Resume) {
                            break; // resume game
                        }

                        if (sel == Vue::PauseMenu::Option::ExitLevel) {
                            Vue::ConfirmationDialog confirm("Exit level? All your progress will be lost.");

                            // confirmation loop
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

                            if (!fenetre.isOpen()) { abortOuter = true; break; }

                            if (confirm.isConfirmed()) {
                                // Destroy current level so replay starts from a fresh state
                                modele.reset();
                                niveauController.reset();

                                // Go back to main menu and recreate a fresh controller afterwards
                                afficherMenuAccueil();
                                if (!niveauController) {
                                    niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
                                }
                                break; // exit pause handling
                            } else {
                                // user canceled -> reopen pause menu (continue loop)
                                continue;
                            }
                        }

                        if (sel == Vue::PauseMenu::Option::ExitGame) {
                            Vue::ConfirmationDialog confirm("Exit game? All your progress will be lost.");

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

                            if (!fenetre.isOpen()) { abortOuter = true; break; }

                            if (confirm.isConfirmed()) {
                                fenetre.close();
                                abortOuter = true;
                                break;
                            } else {
                                // canceled -> reopen pause menu
                                continue;
                            }
                        }
                    }

                    if (abortOuter) break;
                }

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