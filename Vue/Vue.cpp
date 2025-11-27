#include "Vue.h"
#include <cmath>
#include "Modele.h"
#include <iostream>

// Définition de PI pour le cône de vision
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Vue
{
    // DÉFINITION : Constructeur
    Vue::Vue(Modele::Modele& modele)
    : modele(modele)
    {
        // ... (Code de chargement de police et initialisation du menu) ...
        // 1. Chargement de la police
        // Le chemin peut varier sur d'autres OS.
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontCharge = true;
        }
        else if (font.loadFromFile("arial.ttf")) // Alternative: si la police est dans le dossier de build
        {
            fontCharge = true;
        }
        else
        {
            std::cerr << "Erreur: Impossible de charger la police (arial.ttf).\n";
        }

        // Si la police est chargée, configurer tous les objets texte
        if (fontCharge)
        {
            // --- Textes d'état du jeu (en haut à gauche) ---
            collisionText.setFont(font);
            collisionText.setString("Collision detectee !");
            collisionText.setCharacterSize(24);
            collisionText.setFillColor(sf::Color::Red);
            collisionText.setStyle(sf::Text::Bold);
            collisionText.setPosition(10.f, 10.f);

            joueurDetecteText.setFont(font);
            joueurDetecteText.setString("Joueur detecte !");
            joueurDetecteText.setCharacterSize(22);
            joueurDetecteText.setFillColor(sf::Color::Yellow);
            joueurDetecteText.setStyle(sf::Text::Bold);
            joueurDetecteText.setPosition(10.f, 40.f);

            // --- Texte Nom de la pièce ---
            roomNameText.setFont(font);
            roomNameText.setCharacterSize(28);
            roomNameText.setFillColor(sf::Color::White);
            roomNameText.setStyle(sf::Text::Bold);
            // La position sera calculée dynamiquement dans dessiner()

            // Définir les coordonnées par rapport à la taille de l'écran pour le centrage
            sf::VideoMode dm = sf::VideoMode::getDesktopMode();
            float W = static_cast<float>(dm.width);
            float H = static_cast<float>(dm.height);

            // Titre
            titleText.setFont(font);
            titleText.setString("Mon Jeu MVC SFML");
            titleText.setCharacterSize(64);
            titleText.setFillColor(sf::Color::White);
            titleText.setStyle(sf::Text::Bold);
            titleText.setOrigin(titleText.getLocalBounds().left + titleText.getLocalBounds().width / 2.0f,
                                titleText.getLocalBounds().top + titleText.getLocalBounds().height / 2.0f);
            titleText.setPosition(W / 2.0f, H / 4.0f);

            // Champ de texte
            inputBox.setSize(sf::Vector2f(300.f, 50.f));
            inputBox.setFillColor(sf::Color(50, 50, 50));
            inputBox.setOutlineColor(sf::Color::White);
            inputBox.setOutlineThickness(2.f);
            inputBox.setOrigin(inputBox.getSize().x / 2.0f, inputBox.getSize().y / 2.0f);
            inputBox.setPosition(W / 2.0f, H / 2.0f);

            inputText.setFont(font);
            inputText.setString("Entrez votre nom...");
            inputText.setCharacterSize(24);
            inputText.setFillColor(sf::Color::White);
            inputText.setOrigin(0.f, 0.f);
            inputText.setPosition(inputBox.getPosition().x - inputBox.getSize().x / 2.0f + 10.f,
                                  inputBox.getPosition().y - inputText.getLocalBounds().height / 2.0f - 10.f);

            // Bouton de jeu
            playButton.setSize(sf::Vector2f(200.f, 60.f));
            playButton.setFillColor(sf::Color(0, 150, 0)); // Vert foncé
            playButton.setOrigin(playButton.getSize().x / 2.0f, playButton.getSize().y / 2.0f);
            playButton.setPosition(W / 2.0f, H * 0.75f);

            playLabel.setFont(font);
            playLabel.setString("JOUER");
            playLabel.setCharacterSize(30);
            playLabel.setFillColor(sf::Color::White);
            playLabel.setStyle(sf::Text::Bold);
            playLabel.setOrigin(playLabel.getLocalBounds().left + playLabel.getLocalBounds().width / 2.0f,
                                playLabel.getLocalBounds().top + playLabel.getLocalBounds().height / 2.0f);
            playLabel.setPosition(playButton.getPosition());
        }
    }

    // DÉFINITION : Méthode de dessin principale
    void Vue::dessiner(sf::RenderWindow& fenetre)
    {
        fenetre.clear(sf::Color(30, 30, 30)); // Fond gris foncé

        if (menuActif)
        {
            // --- Dessiner le Menu de Démarrage ---
            fenetre.draw(titleText);
            fenetre.draw(inputBox);
            fenetre.draw(inputText);
            fenetre.draw(playButton);
            fenetre.draw(playLabel);
        }
        else
        {
            // --- Dessiner le Jeu ---

            // Affichage du nom de la pièce (positionné en haut au centre)
            if (fontCharge)
            {
                roomNameText.setString(modele.getCurrentRoomName());
                roomNameText.setOrigin(roomNameText.getLocalBounds().left + roomNameText.getLocalBounds().width / 2.0f,
                                       roomNameText.getLocalBounds().top + roomNameText.getLocalBounds().height / 2.0f);
                roomNameText.setPosition(modele.getScreenW() / 2.0f, 10.f);
                fenetre.draw(roomNameText);
            }

            // 1. Dessine le champ de vision de l'obstacle
            // **CORRECTION ICI : getObstacles() -> getObstacleShapes()**
            if (modele.getObstacleShapes().size() > 0)
            {
                const float fovAngle = 90.f;
                const float fovRange = 300.f;

                sf::Vector2f center = modele.getObstacleCenter(0);
                sf::Vector2f forward = modele.getObstacleForward(0);

                const float halfFov = fovAngle / 2.f;
                float angle = halfFov * (M_PI / 180.f);
                float c = std::cos(angle);
                float s = std::sin(angle);

                sf::Vector2f leftDir(c * forward.x - s * forward.y,
                                     s * forward.x + c * forward.y);
                sf::Vector2f rightDir(c * forward.x + s * forward.y,
                                     -s * forward.x + c * forward.y);

                sf::Vector2f leftPoint  = center + leftDir  * fovRange;
                sf::Vector2f rightPoint = center + rightDir * fovRange;

                sf::ConvexShape cone;
                cone.setPointCount(3);
                cone.setPoint(0, center);
                cone.setPoint(1, leftPoint);
                cone.setPoint(2, rightPoint);
                cone.setFillColor(sf::Color(173, 216, 230, 100));
                cone.setOutlineThickness(0.f);
                fenetre.draw(cone);
            }

            // 2. Dessine les obstacles physiques (murs, etc.)
            // **CORRECTION ICI : getObstacles() -> getObstacleShapes()**
            for (const auto& obsPtr : modele.getObstacleShapes())
            {   fenetre.draw(*obsPtr); }

            // 3. Dessine le rectangle joueur
            fenetre.draw(modele.getJoueur());

            // 4. Dessine les portes (visuel)
            for (const auto& door : modele.getCurrentRoomDoors())
            {
                if (door.visualShape)
                {
                    fenetre.draw(*door.visualShape);
                }
            }

            // 5. Afficher les textes d'état
            if (fontCharge && modele.isCollisionDetectee())
            {
                fenetre.draw(collisionText);
            }
            if (fontCharge && modele.isJoueurDetecte())
            {
                 fenetre.draw(joueurDetecteText);
            }
        }

        fenetre.display(); // Affichage final
    }

    // DÉFINITION : Gestion des événements (principalement pour le menu)
    void Vue::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
    {
        if (menuActif)
        {
            // Gestion du clic de souris sur le bouton "JOUER"
            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2f mousePos = fenetre.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});

                    if (playButton.getGlobalBounds().contains(mousePos))
                    {
                        if (!playerName.empty())
                        {
                            menuActif = false;
                        } else {
                            // Afficher un message d'erreur si le nom est vide
                            titleText.setString("Entrez un nom pour jouer!");
                            titleText.setFillColor(sf::Color::Red);
                            titleText.setOrigin(titleText.getLocalBounds().left + titleText.getLocalBounds().width / 2.0f,
                                                titleText.getLocalBounds().top + titleText.getLocalBounds().height / 2.0f);
                        }
                    }
                }
            }

            // Gestion de l'entrée de texte pour le champ de nom
            if (event.type == sf::Event::TextEntered)
            {
                if (event.text.unicode < 128)
                {
                    if (event.text.unicode == '\b')
                    {
                        if (!playerName.empty())
                        {
                            playerName.pop_back();
                        }
                    }
                    else if (event.text.unicode == '\r' || event.text.unicode == '\n')
                    {
                        if (!playerName.empty())
                        {
                            menuActif = false;
                        }
                    }
                    else if (playerName.length() < 15 && event.text.unicode != ' ')
                    {
                        playerName += static_cast<char>(event.text.unicode);
                    }

                    // Mise à jour de l'affichage du nom
                    if (playerName.empty())
                    {
                        inputText.setString("Entrez votre nom...");
                    } else {
                        inputText.setString(playerName);
                    }

                    // Remettre le titre à la normale
                    titleText.setString("Mon Jeu MVC SFML");
                    titleText.setFillColor(sf::Color::White);
                    titleText.setOrigin(titleText.getLocalBounds().left + titleText.getLocalBounds().width / 2.0f,
                                        titleText.getLocalBounds().top + titleText.getLocalBounds().height / 2.0f);
                }
            }
        }
    }
}