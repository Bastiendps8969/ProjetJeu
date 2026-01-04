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
    // Constructor
    Controleur::Controleur(Modele::Modele& modele, Vue::Vue& vue)
    : modele(modele), vue(vue),
      // Create the window in fullscreen using desktop resolution.
      fenetre(sf::VideoMode::getDesktopMode(), "Déplacement du personnage", sf::Style::Fullscreen)
    {
        // Fixed framerate for smoother gameplay and deterministic-ish updates.
        fenetre.setFramerateLimit(60);

        // Create a per-level controller responsible for gameplay logic.
        niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);

        // Pause menu managed by RAII.
        pauseMenu = std::make_unique<Vue::PauseMenu>(modele);
    }

    // Display the home menu (imported from ProjetJeu):
    // - splash screen
    // - main home page (chapter/level selection)
    // - load selected level into the model
    void Controleur::afficherMenuAccueil()
    {
        // Lambda to retrieve player scores from the model (used by HomePage UI).
        auto getScores = [this] -> std::vector<int> {
            return this->modele.getPlayerScores();
        };

        // 1) Show splash screen (modal loop).
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

        // 2) Then show the main menu (HomePage) using a background image.
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

        // After closing main menu, check if user selected a chapter/level.
        int selChapter = homePage.getSelectedChapter();
        int selLevel = homePage.getSelectedLevel();
        if (selChapter >= 0 && selLevel >= 0)
        {
            // If HomePage returned a levelData path, load that level file into the model.
            const std::string& lvlPath = homePage.getSelectedLevelData();
            if (!lvlPath.empty()) {
                if (!modele.loadLevelFromFile(lvlPath)) {
                    std::cerr << "[Controleur] Échec du chargement du niveau depuis '" << lvlPath << "'\n";
                } else {
                    std::cout << "[Controleur] Niveau chargé depuis : " << lvlPath << "\n";
                    // Recreate level controller so it picks up the new model state.
                    niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
                }
            }
            else
            {
                // Fallback legacy mapping (kept for older level definitions).
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
                        // Recreate level controller to ensure a consistent per-level state.
                        niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
                    }
                }
            }
        }

        fenetre.setFramerateLimit(60);
    }

    // Main game loop
    void Controleur::gererBoucle()
    {
        // Show main menu first.
        afficherMenuAccueil();

        // Ensure the level controller exists when entering gameplay.
        if (!niveauController) {
            // Ensure model is reset so any previous progress is cleared.
            modele.reset();
            niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
        }

        // Dialogue manager orchestrates in-game dialogues (modal overlay).
        Vue::DialogueManager dialogueManager;

        int prevRoomIndex = modele.getCurrentRoomIndex();

        // At level start, if current room has a dialogue and it wasn't shown yet, start it once.
        {
            std::string startDialog = modele.getCurrentRoomDialogueRef();
            if (!startDialog.empty() && !modele.isCurrentRoomDialogueShown() && dialogueManager.hasDialogueSequence(startDialog)) {
                // Ensure visuals are updated for the new room before showing dialogue.
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

        // Guards and state flags to avoid re-triggering actions every frame (60 FPS loop).
        bool agentDialogueLaunched = false; // Ajout
        bool cesrDialogueClosed = false; // Track si dialogue César terminé pour ouvrir CesarVue
        bool timeDialogueLaunched = false; // guard to start time-up dialogue once
        bool gameOverPending = false; // attente si un dialogue est actif lors du game over
        bool gameOverRequested = false; // demande différée de retour au menu (exécuter quand aucun dialogue n'est actif)

        // Lambda to centralize Game Over handling (reset + return to menu).
        auto processGameOver = [&] {
            std::cout << "[Controleur] Game Over! No more lives." << std::endl;

            // Destroy current level so replay starts from a fresh state.
            modele.reset();
            niveauController.reset();

            // Go back to main menu.
            afficherMenuAccueil();

            if (!niveauController) {
                modele.reset();
                niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
                timeDialogueLaunched = false;
            }
        };

        // Debug FPS counter.
        sf::Clock fpsTimer;
        int fpsFrames = 0;

        while (fenetre.isOpen())
        {
            // If level controller is missing (e.g., after a reset), skip update/render.
            if (!niveauController)
            {
                fenetre.clear();
                fenetre.display();
                continue;
            }

            // --- Event handling ---
            sf::Event evenement;
            while (fenetre.pollEvent(evenement))
            {
                // Safety: if the level was destroyed (Game Over or Exit), wait for a new one to be created if (!niveauController) { fenetre.clear(); fenetre.display(); continue; }

                if (evenement.type == sf::Event::Closed)
                { fenetre.close(); }

                // Escape opens a modal pause menu with confirmation dialogs for destructive actions.
                if ((evenement.type == sf::Event::KeyPressed)
                    && (evenement.key.code == sf::Keyboard::Escape))
                {
                    // Show pause menu and require confirmation for destructive actions.
                    bool abortOuter = false;

                    while (fenetre.isOpen()) {
                        // Note: A local PauseMenu instance is created here (modal pause flow).
                        Vue::PauseMenu pause(modele);
                        pause.setActive(true);

                        // Pause loop
                        while (fenetre.isOpen() && pause.isActive()) {
                            sf::Event pe;
                            while (fenetre.pollEvent(pe)) {
                                if (pe.type == sf::Event::Closed) fenetre.close();
                                pause.handleEvent(pe, fenetre);
                            }

                            // Draw current game frame behind the pause menu.
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
                            // Confirm exit level (destructive: progress lost).
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
                                // Reset model + destroy current level controller (fresh restart).
                                modele.reset();
                                niveauController.reset();

                                // Return to main menu and recreate controller afterwards.
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
                            // Confirm exit game (destructive).
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

                // Forward events to dialogue manager (dialogue navigation, clicks, etc.).
                dialogueManager.handleEvent(evenement);
            }

            // --- Frame update & render ---
            fenetre.clear(sf::Color::Black);

            // Delegate collision consequences to the level controller (objectives/detections/lives flags).
            niveauController->processCollisions(dialogueManager);

            // Launch "agent_detected" dialogue once when player is detected by an enemy.
            if (modele.isJoueurDetecte() && !agentDialogueLaunched && !dialogueManager.isDialogueActive())
            {
                dialogueManager.startDialogueSequence("agent_detected");
                agentDialogueLaunched = true;
            }
            if (!modele.isJoueurDetecte())
            {
                agentDialogueLaunched = false;
            }

            // Start time-up dialogue once when timer reaches zero.
            if (niveauController && !timeDialogueLaunched && !dialogueManager.isDialogueActive()) {
                if (niveauController->getRemainingSeconds() == 0) {
                    dialogueManager.startDialogueSequence("time_up");
                    timeDialogueLaunched = true;
                }
            }

            // If a "Cesar" objective dialogue ended, open CesarVue window (modal loop).
            bool shouldOpen = niveauController->shouldOpenCesarWindow();
            bool dialogueNotActive = !dialogueManager.isDialogueActive();
            bool notClosedYet = !cesrDialogueClosed;
            if (shouldOpen && dialogueNotActive && notClosedYet)
            {
                cesrDialogueClosed = true;

                // Open Cesar window.
                CesarVue cesarVue(niveauController->getCesarObjective());

                // Cesar window loop.
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

                // Consume Cesar flag in the level controller.
                niveauController->resetCesarWindowFlag();
                cesrDialogueClosed = false;
            }

            // Pause the level timer while dialogues are active (handled by level controller).
            if (!niveauController) {
                // Ensure model is reset so any previous progress is cleared
                modele.reset();
                niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
                timeDialogueLaunched = false;
            }

            // Only run gameplay updates when no dialogue is active.
            if (!dialogueManager.isDialogueActive())
            {
                niveauController->handleInput();
                niveauController->update();
                modele.mettreAJourObstacles();
                modele.updateEnemies(); // update enemy logic + animations (from sav)

                // Check doors and possibly change room.
                int before = modele.getCurrentRoomIndex();
                niveauController->checkDoors();
                int after = modele.getCurrentRoomIndex();

                // If room changed, attempt to start its associated dialogue sequence.
                if (after != before)
                {
                    prevRoomIndex = after;
                    std::string roomDialog = modele.getCurrentRoomDialogueRef();
                    if (!roomDialog.empty() && !modele.isCurrentRoomDialogueShown() && dialogueManager.hasDialogueSequence(roomDialog) && !dialogueManager.isDialogueActive())
                    {
                        // Update visuals once so the enemy/agent positions reflect the new room.
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

            // If the level controller requested exit (via exit door + confirmation), handle it here.
            if (niveauController->isExitRequested())
            {
                modele.reset();
                niveauController.reset();

                // Go back to main menu and recreate a fresh controller afterwards.
                afficherMenuAccueil();
                if (!niveauController) {
                    modele.reset();
                    niveauController = std::make_unique<ControllerLevel>(modele, vue, fenetre);
                    timeDialogueLaunched = false;
                }

                // Skip rest of this frame iteration.
                continue;
            }

            // Check if game over (lives = 0). Defer processing if a dialogue is active.
            if (niveauController->isGameOver())
            {
                gameOverRequested = true;
            }

            // Draw world (view) first.
            vue.dessiner(fenetre);

            // Draw HUD timer from the level controller.
            if (niveauController) {
                niveauController->drawUI(fenetre);
            }

            // Draw dialogue overlay on top.
            dialogueManager.update(fenetre.getSize());
            dialogueManager.draw(fenetre);

            // If game over was pending/requested, handle it once no dialogue is active.
            if ((gameOverPending || gameOverRequested) && !dialogueManager.isDialogueActive())
            {
                gameOverPending = false;
                gameOverRequested = false;
                processGameOver();
                continue; //  leave this frame immediately
            }

            // If the time-up dialogue finished, return to main menu.
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

            // Present final frame.
            fenetre.display();

            // FPS debug print once per second.
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
}
