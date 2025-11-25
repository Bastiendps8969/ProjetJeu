#include "Controleur.h"
#include <SFML/Window.hpp>

namespace Controleur
{
    // Constructeur
    Controleur::Controleur(Modele::Modele& modele, Vue::Vue& vue)
        : modele(modele), vue(vue),
          // Ouvre en plein écran sur la résolution du bureau
          fenetre(sf::VideoMode::getDesktopMode(), "Déplacement du personnage", sf::Style::Fullscreen),
          mouvement(0.f, 0.f)
    {
        fenetre.setFramerateLimit(6000);
    }

    // Boucle principale
    void Controleur::gererBoucle()
    {
        // Tant que la fenêtre est ouverte
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

                // Toujours transmettre les événements à la vue (pour le menu / input texte)
                vue.handleEvent(evenement, fenetre);
            }

            // Si le menu est actif, on n'applique pas la logique du jeu (mouvement / collisions)
            if (!vue.isMenuActive())
            {
                // Gestion des entrées clavier
                gererEntree();
                // Mise à jour de la logique du jeu
                mettreAJour();
            }

            // Dessin de la vue (menu ou scène selon l'état de la vue)
            vue.dessiner(fenetre);
        }
    }

    // Gestion des entrées clavier : flèches
    void Controleur::gererEntree()
    {
        // Réinitialisation du mouvement
        mouvement = sf::Vector2f(0.f, 0.f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            mouvement.x -= 0.5f;
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

        // Calcul de la nouvelle position du joueur : position actuelle + mouvement
        sf::Vector2f nouvellePosition = modele.getJoueur().getPosition() + mouvement;

        bool collision = false;

        // Vérification que la nouvelle position est dans les limites de la fenêtre
        sf::Vector2u winSize = fenetre.getSize();
        float winW = static_cast<float>(winSize.x);
        float winH = static_cast<float>(winSize.y);
        if (nouvellePosition.x >= 0 && nouvellePosition.x + modele.getJoueur().getSize().x <= winW &&
            nouvellePosition.y >= 0 && nouvellePosition.y + modele.getJoueur().getSize().y <= winH)
        {
            sf::FloatRect joueurBounds = modele.getJoueur().getGlobalBounds();
            sf::FloatRect nouveauJoueurBounds = joueurBounds;
            nouveauJoueurBounds.left += mouvement.x;
            nouveauJoueurBounds.top += mouvement.y;

            for (const auto& obs : modele.getObstacles())
            {
                if (nouveauJoueurBounds.intersects(obs->getGlobalBounds()))
                {
                    collision = true;
                    break;
                }
            }

            if (!collision)
            {
                modele.getJoueur().setPosition(nouvellePosition);
            }
            else
            {
                // Ajustements post-collision
                for (const auto& obs : modele.getObstacles())
                {
                    sf::FloatRect obstacleBounds = obs->getGlobalBounds();
                    if (joueurBounds.intersects(obstacleBounds))
                    {
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
            collision = false;
        }

        // Transmet l'état de collision au modèle (ne pas écraser une collision déjà vraie)
        modele.setCollisionDetectee(collision || modele.isCollisionDetectee());
    }
}