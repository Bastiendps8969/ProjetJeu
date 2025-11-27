#include "Controleur.h"
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
        fenetre.setFramerateLimit(60);
    }

    // DÉFINITION : Boucle principale
    void Controleur::gererBoucle()
    {
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

                vue.handleEvent(evenement, fenetre);
            }

            if (!vue.isMenuActive())
            {
                gererEntree();
                mettreAJour();
                modele.mettreAJourObstacles();
                verifierPorte(); // Nouvelle méthode pour vérifier le changement de pièce
            }

            vue.dessiner(fenetre);
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
    } // FIN de la fonction mettreAJour()
} // FIN du namespace Controleur