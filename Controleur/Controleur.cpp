#include "Controleur.h"
#include "DialogueManager.h"
#include <SFML/Window.hpp>
#include <iostream>

namespace Controleur
{
    // Constructeur
    Controleur::Controleur(Modele::Modele& modele, Vue::Vue& vue)
        : modele(modele), vue(vue),
          // Ouvre en plein écran sur la résolution du bureau
          fenetre(sf::VideoMode::getDesktopMode(), "Déplacement du personnage", sf::Style::Fullscreen),
          mouvement(0.f, 0.f)
    {
        // limiter le framerate pour éviter saccades et surchauffe
        fenetre.setFramerateLimit(60);
        // ou fenetre.setVerticalSyncEnabled(true);
    }
    {}

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
    }

    // Boucle principale
    void Controleur::gererBoucle()
    {
        Vue::DialogueManager dialogueManager;

        // FPS / debug helpers
        sf::Clock fpsTimer;
        int fpsFrames = 0;
        bool debug_skip_obstacles = false;

        while (fenetre.isOpen())
        {
            // Gestion des événements
            sf::Event evenement;
            while (fenetre.pollEvent(evenement))
            {
                // Evénement de type "fermeture de fenêtre"
                if (evenement.type == sf::Event::Closed)
                {   fenetre.close();    }

                // Evénement de type "Escape"
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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            mouvement.x = 0.5f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            mouvement.y -= 0.5;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            mouvement.y = 0.5;
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

        // Vérification que la nouvelle position est dans les limites de la fenêtre (adapté au fullscreen)
        sf::Vector2u winSize = fenetre.getSize();
        float winW = static_cast<float>(winSize.x);
        float winH = static_cast<float>(winSize.y);
        if (nouvellePosition.x >= 0 && nouvellePosition.x + modele.getJoueur().getSize().x <= winW &&
            nouvellePosition.y >= 0 && nouvellePosition.y + modele.getJoueur().getSize().y <= winH)
        {
            // Récupération des limites actuelles du joueur
            sf::FloatRect joueurBounds = modele.getJoueur().getGlobalBounds();
            // Calcul des nouvelles limites du joueur après le mouvement
            sf::FloatRect nouveauJoueurBounds = joueurBounds;
            nouveauJoueurBounds.left += mouvement.x;
            nouveauJoueurBounds.top += mouvement.y;

            // Vérification des collisions avec CHAQUE obstacle
            const auto& obstacles = modele.getObstacles(); // cache la référence une fois
            for (const auto& obs : obstacles)
            {
                if (nouveauJoueurBounds.intersects(obs->getGlobalBounds()))
                {    collision = true;
                    break;
                }
            }
            // Si aucune collision n'est détectée, mise à jour de la position du joueur
            if (!collision)
            {   modele.getJoueur().setPosition(nouvellePosition);    }
            else
            {
                // Gestion des collisions avec les obstacles
                for (const auto& obs : obstacles)
                {
                    sf::FloatRect obstacleBounds = obs->getGlobalBounds();
                    if (joueurBounds.intersects(obstacleBounds))
                    {
                        // Ajustement de la position du joueur en fonction de la direction du mouvement
                        if (mouvement.x > 0)
                            modele.getJoueur().setPosition(obstacleBounds.left - joueurBounds.width, modele.getJoueur().getPosition().y);
                        if (mouvement.x < 0)
                            modele.getJoueur().setPosition(obstacleBounds.left + obstacleBounds.width, modele.getJoueur().getPosition().y);
                        if (mouvement.y > 0)
                            modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, obstacleBounds.top - joueurBounds.height);
                        if (mouvement.y < 0)
                            modele.getJoueur().setPosition(modele.getJoueur().getPosition().x, obstacleBounds.top + obstacleBounds.height);
                    }
                }
            }
        }
        else
        {
            // Hors limites de la fenêtre -> pas une collision avec obstacle par défaut
            collision = false;
        }

        // Transmet l'état de collision au modèle (utilisé par la Vue)
        // Ne pas écraser une collision déjà détectée par le modèle (obstacle -> joueur)
        modele.setCollisionDetectee(collision || modele.isCollisionDetectee());
    }
}