#include "Controleur.h"
#include <SFML/Window.hpp>
#include "Modele.h"
#include "Vue.h"
#include "HomePage.h"
#include "DialogueManager.h"
#include "SplashPage.h"
#include "ChapterLoader.h"
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
                {
                    // Open pause menu during gameplay (do not exit app immediately)
                    if (!dialogueManager.isDialogueActive()) {
                        if (pauseMenu) pauseMenu->setActive(true);

                        // Modal loop for pause menu
                        while (fenetre.isOpen() && pauseMenu && pauseMenu->isActive()) {
                            sf::Event pe;
                            while (fenetre.pollEvent(pe)) {
                                if (pe.type == sf::Event::Closed) { fenetre.close(); break; }
                                // Forward events to pause menu
                                pauseMenu->handleEvent(pe, fenetre);
                            }

                            // Draw current game frame underneath
                            fenetre.clear(sf::Color::Black);
                            vue.dessiner(fenetre);
                            pauseMenu->draw(fenetre);
                            fenetre.display();
                        }

                        // After closing pause menu, check selection
                        if (pauseMenu && !pauseMenu->isActive()) {
                            auto sel = pauseMenu->getSelectedOption();
                            if (sel == Vue::PauseMenu::Option::ExitLevel) {
                                // Return to main menu
                                afficherMenuAccueil();
                            } else if (sel == Vue::PauseMenu::Option::ExitGame) {
                                fenetre.close();
                                break;
                            }
                        }
                    } else {
                        // If a dialogue is active we ignore Escape here
                    }
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