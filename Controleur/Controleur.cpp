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
        // Lambda pour récupérer les scores du joueur depuis le modèle
        auto getScores = [this]() -> std::vector<int> {
            return this->modele.getPlayerScores();
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
            // If LevelPage returned a levelData path via HomePage, load that level file into the model.
            const std::string& lvlPath = homePage.getSelectedLevelData();
            if (!lvlPath.empty()) {
                if (!modele.loadLevelFromFile(lvlPath)) {
                    std::cerr << "[Controleur] Échec du chargement du niveau depuis '" << lvlPath << "'\n";
                } else {
                    std::cout << "[Controleur] Niveau chargé depuis : " << lvlPath << "\n";
                    // recreate level controller so it picks up the new model state
                    niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
                }
            }
            else {
                // Fallback to old simple mapping (kept for legacy levels defined inside the initial tutorial JSON)
                int roomId = -1;
                if (selChapter == 0)
                {
                    if (selLevel == 0) roomId = 0;
                    else if (selLevel == 1) roomId = 1;
                    else if (selLevel == 2) roomId = 2;
                    else roomId = 0;
                }

                if (roomId >= 0)
                {
                    if (!modele.changeRoom(roomId, ""))
                    {
                        std::cerr << "[Controleur] Échec du chargement du niveau (room " << roomId << ")\n";
                    }
                    else
                    {
                        std::cout << "[Controleur] Niveau sélectionné chargé : room " << roomId << "\n";
                        niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
                    }
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
        int prevRoomIndex = modele.getCurrentRoomIndex();
        // At level start, if current room (often 0) has a dialogue and it wasn't shown yet, start it once
        {
            std::string startDialog = modele.getCurrentRoomDialogueRef();
            if (!startDialog.empty() && !modele.isCurrentRoomDialogueShown() && dialogueManager.hasDialogueSequence(startDialog)) {
                // Ensure visuals are updated for the new room before showing dialogue
                modele.syncPlayerSprite();
                modele.mettreAJourObstacles();
                modele.updateEnemies();
                vue.dessiner(fenetre);
                dialogueManager.update(fenetre.getSize());
                dialogueManager.draw(fenetre);
                fenetre.display();

                dialogueManager.startDialogueSequence(startDialog);
                modele.markCurrentRoomDialogueShown();
            }
        }
        bool agentDialogueLaunched = false; // Ajout
        bool cesrDialogueClosed = false;  // Track si dialogue César terminé pour ouvrir CesarVue
        bool timeDialogueLaunched = false; // guard to start time-up dialogue once
        bool gameOverPending = false; // attente si un dialogue est actif lors du game over
        bool gameOverRequested = false; // demande différée de retour au menu (exécuter quand aucun dialogue n'est actif)


        // lambda pour centraliser le traitement du Game Over (reset + retour menu)
        auto processGameOver = [&]() {
            std::cout << "[Controleur] Game Over! No more lives." << std::endl;
            // Destroy current level so replay starts from a fresh state
            modele.reset();
            niveauController.reset();

            // Go back to main menu
            afficherMenuAccueil();
            if (!niveauController) {
                modele.reset();
                niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
                timeDialogueLaunched = false;
            }
        };

        sf::Clock fpsTimer;
        int fpsFrames = 0;

        while (fenetre.isOpen())
        {

            if (!niveauController)
            {
                fenetre.clear();
                fenetre.display();
                continue;
            }

            sf::Event evenement;
            while (fenetre.pollEvent(evenement))
            {
                // Sécurité : si le niveau a été détruit (Game Over ou Exit), on attend qu'un nouveau soit créé if (!niveauController) { fenetre.clear(); fenetre.display(); continue; }
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
                                        timeDialogueLaunched = false;
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

            // Start time-up dialogue once when timer reaches zero
            if (niveauController && !timeDialogueLaunched && !dialogueManager.isDialogueActive()) {
                if (niveauController->getRemainingSeconds() == 0) {
                    //  Launch the dialogue
                    dialogueManager.startDialogueSequence("time_up");
                    timeDialogueLaunched = true;
                }
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

            // Pause the level timer while dialogues are active
            if (!niveauController) {
                // Ensure model is reset so any previous progress is cleared
                modele.reset();
                niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
                timeDialogueLaunched = false;
            }
                if (!dialogueManager.isDialogueActive())
                {
                    niveauController->handleInput();
                    niveauController->update();
                    modele.mettreAJourObstacles();
                    modele.updateEnemies(); // update enemy logic + animations (from sav)
                    // check doors and possibly change room
                    int before = modele.getCurrentRoomIndex();
                    niveauController->checkDoors();
                    int after = modele.getCurrentRoomIndex();

                    // If room changed, attempt to start its associated dialogue sequence
                    if (after != before) {
                        prevRoomIndex = after;
                        std::string roomDialog = modele.getCurrentRoomDialogueRef();
                        if (!roomDialog.empty() && !modele.isCurrentRoomDialogueShown() && dialogueManager.hasDialogueSequence(roomDialog) && !dialogueManager.isDialogueActive()) {
                            // Update visuals once so the enemy/agent positions reflect the new room
                            modele.syncPlayerSprite();
                            modele.mettreAJourObstacles();
                            modele.updateEnemies();
                            vue.dessiner(fenetre);
                            dialogueManager.update(fenetre.getSize());
                            dialogueManager.draw(fenetre);
                            fenetre.display();

                            dialogueManager.startDialogueSequence(roomDialog);
                            modele.markCurrentRoomDialogueShown();
                        }
                    }
                }
                // If the level controller requested exit (via door -> -1 + confirmation), handle it here
                if (niveauController->isExitRequested()) {
                    // Destroy current level so replay starts from a fresh state
                    modele.reset();
                    niveauController.reset();

                    // Go back to main menu and recreate a fresh controller afterwards
                    afficherMenuAccueil();
                    if (!niveauController) {
                        modele.reset();
                        niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
                        timeDialogueLaunched = false;
                    }
                    // skip rest of this frame iteration
                    continue;
                }

                // Check if game over (lives = 0)
                if (niveauController->isGameOver())
                {
                    gameOverRequested = true;
                }


            vue.dessiner(fenetre);
            // draw HUD timer from the level controller
            if (niveauController) {
                niveauController->drawUI(fenetre);
            }

            dialogueManager.update(fenetre.getSize());
            dialogueManager.draw(fenetre);

            // If a game over was pending while a dialogue was active, or a game over
            // was requested, handle it now once no dialogue is active.
            if ((gameOverPending || gameOverRequested) && !dialogueManager.isDialogueActive())
            {
                gameOverPending = false;
                gameOverRequested = false;

                gameOverRequested = false;
                processGameOver();
                continue; // ⚠️ ON SORT DE LA FRAME

            }


            // If the time-up dialogue finished, return to main menu
            if (timeDialogueLaunched && !dialogueManager.isDialogueActive()) {
                // Destroy current level and reset model
                modele.reset();
                niveauController.reset();

                // Show main menu and recreate a fresh controller afterwards
                afficherMenuAccueil();
                if (!niveauController) {
                    modele.reset();
                    niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
                }
                timeDialogueLaunched = false;
                // skip rendering the rest of this frame
                continue;
            }

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