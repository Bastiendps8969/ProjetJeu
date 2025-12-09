#include "Controleur.h"
#include "DialogueManager.h"
#include <SFML/Window.hpp>
#include "Modele.h"
#include "Vue.h"
#include <cmath>    // Nécessaire pour std::sqrt
#include <iostream> // Ajouté pour std::cout

namespace Controleur
{
    // DÉFINITION : Constructeur
    Controleur::Controleur(Modele::Modele& modele, Vue::Vue& vue)
        : modele(modele), vue(vue),
          // Ouvre en plein écran sur la résolution du bureau
          fenetre(sf::VideoMode::getDesktopMode(), "Déplacement du personnage", sf::Style::Fullscreen),
          mouvement(0.f, 0.f)
    {
        // limiter le framerate pour éviter saccades et surchauffe
        fenetre.setFramerateLimit(6000);
        // ou fenetre.setVerticalSyncEnabled(true);
    }

    // Affiche le menu d'accueil
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

        fenetre.setFramerateLimit(6000);
    }

    // DÉFINITION : Boucle principale
    void Controleur::gererBoucle()
    {
        Vue::DialogueManager dialogueManager;

        // FPS / debug helpers
        sf::Clock fpsTimer;
        int fpsFrames = 0;
        bool debug_skip_obstacles = false;

        //  Starting dialogue
        dialogueManager.startDialogueSequence("welcome");

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

            // Lancer le dialogue UNE SEULE FOIS si collision ET flag pas encore activé
            if (modele.isCollisionDetectee() && !modele.hasDialogueTriggered())
            {
                dialogueManager.startDialogueSequence("agent_detected");
                modele.setDialogueTriggered(true);
            }



            // Réinitialiser le flag quand le dialogue est terminé
            // if (!dialogueManager.isDialogueActive() && modele.hasDialogueTriggered())
            // {
            //     modele.resetDialogueTriggered();
            // }

            // Geler le gameplay si un dialogue est actif
            if (!dialogueManager.isDialogueActive())
            {
                // Le jeu continue normalement
                gererEntree();
                mettreAJour();
                modele.mettreAJourObstacles();
                verifierPorte(); // Nouvelle méthode pour vérifier le changement de pièce
            }
            // Si dialogue actif : ne pas appeler gererEntree() ni mettreAJour()
            // => le joueur et obstacles restent immobiles

            if (!debug_skip_obstacles)
                vue.dessiner(fenetre);
            else
                fenetre.draw(modele.getJoueur());

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
    // Gestion des entrées clavier : flèches !
    void Controleur::gererEntree()
    {
        // Réinitialisation du mouvement pour garantir que le mouvement du rectangle
        // soit basé uniquement sur les touches actuellement pressées -> x = 0 et y = 0
        // Quand le rectangle arrive à la position de déplacement, on le fait s'arrêter
        // TEST si on ne met pas cette ligne --> il va bouger jusqu'aux limites permises
        mouvement = sf::Vector2f(0.f, 0.f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            mouvement.x -= 0.5f;

        //OU if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        //{
        //    mouvement.x = -0.125f;
        //    mouvement.y = 0.f;
        //}


        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            mouvement.x = 0.5f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            mouvement.y -= 0.5f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            mouvement.y = 0.5f;
    }
    // Mise à jour de la position du joueur et gestion des collisions
    void Controleur::mettreAJour()
    {
        // Mise à jour de la position des obstacles
        modele.mettreAJourObstacles();

        // Calcul de la nouvelle position du joueur : position actuelle + mouvement x ou y
        sf::Vector2f nouvellePosition = modele.getJoueur().getPosition() + mouvement;

        // Déclare le flag collision ici pour l'utiliser et le transmettre au modèle
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

        // **CORRECTION ICI : getObstacles() -> getObstacleShapes()**
        for (const auto& obsPtr : modele.getObstacleShapes())
        {
            if (modele.getJoueur().getGlobalBounds().intersects(obsPtr->getGlobalBounds()))
            {
                // Collision sur X. Annuler le mouvement X et replacer le joueur
                sf::FloatRect obstacleBounds = obsPtr->getGlobalBounds();
                if (deplacement.x > 0) // Mouvement vers la droite
                    modele.getJoueur().setPosition(obstacleBounds.left - playerW, modele.getJoueur().getPosition().y);
                else // Mouvement vers la gauche
                    modele.getJoueur().setPosition(obstacleBounds.left + obstacleBounds.width, modele.getJoueur().getPosition().y);

                collision = true;
                break;
            }
        }

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

        // **CORRECTION ICI : getObstacles() -> getObstacleShapes()**
        for (const auto& obsPtr : modele.getObstacleShapes())
        {
            if (modele.getJoueur().getGlobalBounds().intersects(obsPtr->getGlobalBounds()))
            {
                // Collision sur Y. Annuler le mouvement Y.
                sf::FloatRect obstacleBounds = obsPtr->getGlobalBounds();
                if (deplacement.y > 0) // Mouvement vers le bas
                    modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, obstacleBounds.top - playerH);
                else // Mouvement vers le haut
                    modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, obstacleBounds.top + obstacleBounds.height);

                collision = true;
                break;
            }
        }

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