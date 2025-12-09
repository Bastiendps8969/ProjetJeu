#include "Vue.h"
#include <cmath>
#include "Modele.h"
#include <iostream>
#include <fstream>
#include "../cmake-build-debug/json.hpp"
using json = nlohmann::json;

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

        // --- Charger dialogues depuis dialogue.json (nouveau format: { "events": { "start_game": [...] } }) ---
        {
            std::ifstream f("dialogue.json");
            if (f.is_open())
            {
                try {
                    json j; f >> j;
                    if (j.is_object())
                    {
                        // Nouveau format : events.start_game
                        if (j.contains("events") && j["events"].is_object())
                        {
                            const auto& events = j["events"];
                            if (events.contains("start_game") && events["start_game"].is_array())
                            {
                                for (const auto& el : events["start_game"])
                                {
                                    if (el.is_string()) dialogues.emplace_back(el.get<std::string>());
                                    else if (el.is_object() && el.contains("text") && el["text"].is_string())
                                        dialogues.emplace_back(el["text"].get<std::string>());
                                }
                            }
                        }
                        // Compatibilité : ancien format top-level "dialogs"
                        else if (j.contains("dialogs") && j["dialogs"].is_array())
                        {
                            for (const auto& el : j["dialogs"])
                            {
                                if (el.is_string()) dialogues.emplace_back(el.get<std::string>());
                                else if (el.is_object() && el.contains("text") && el["text"].is_string())
                                    dialogues.emplace_back(el["text"].get<std::string>());
                            }
                        }
                        dialoguesLoaded = !dialogues.empty();
                    }
                } catch (const std::exception& e) {
                } catch (...) {
                }
            } else {
            }

            // Fallback : si le JSON n'a pas été chargé, proposer des dialogues par défaut
            if (!dialoguesLoaded)
            {
                dialogues = {
                    "Dialogue par défault : Bienvenue, aventurier. Appuyez sur une touche ou cliquez pour avancer.",
                    "Cette demeure cache bien des secrets. Explorez prudemment.",
                    "Utilisez ZSQD pour vous déplacer. Bonne chance !"
                };
                dialoguesLoaded = true;
            }
 
             // Préparer la boîte de dialogue (position/forme/texte)
             if (fontCharge)
             {
                 sf::VideoMode dm = sf::VideoMode::getDesktopMode();
                 float W = static_cast<float>(dm.width);
                 float H = static_cast<float>(dm.height);
 
                 dialogBox.setSize(sf::Vector2f(W * 0.92f, H * 0.18f));
                 dialogBox.setFillColor(sf::Color(0, 0, 0, 200)); // fond sombre, semi-transparent
                 dialogBox.setOutlineColor(sf::Color(255,255,255,40));
                 dialogBox.setOutlineThickness(2.f);
                 dialogBox.setPosition(W * 0.04f, H - dialogBox.getSize().y - 30.f);
 
                 dialogText.setFont(font);
                 dialogText.setCharacterSize(24);
                 dialogText.setFillColor(sf::Color::White);
                 dialogText.setStyle(sf::Text::Regular);
                 dialogText.setPosition(dialogBox.getPosition().x + 20.f, dialogBox.getPosition().y + 12.f);
                 dialogText.setString("");
             }
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
            // Lancer le premier dialogue automatiquement une seule fois après sortie du menu
            // uniquement si la sortie du menu a été effectuée via le bouton JOUER
            if (dialoguesLoaded && !dialogStarted && startedByPlayButton)
            {
                dialogStarted = true;
                dialogActive = true;
                currentDialogueIndex = 0;
                if (!dialogues.empty()) {
                    dialogText.setString(dialogues[0]);
                    dialogClock.restart();
                } else {
                    dialogActive = false;
                }
            }
 
             // --- Dessiner le Jeu ---
 
             // 0) Dessiner la grille de tuiles du sol (matrice)
             {
                 const auto& matrix = modele.getFloorMatrix();
                 const sf::Texture& tex = modele.getFloorTexture();
                 const auto& wallTexs = modele.getWallTextures();
                 int tileSize = modele.getTileSize();
                 if (!matrix.empty())
                 {
                     sf::Sprite tileSprite;
                     // Floor sprite
                     bool hasFloor = (tex.getSize().x > 0);
                     if (hasFloor) tileSprite.setTexture(tex);
                     // Wall sprite (we'll setTexture per tile)
                     for (size_t r = 0; r < matrix.size(); ++r)
                     {
                         for (size_t c = 0; c < matrix[r].size(); ++c)
                         {
                             int val = matrix[r][c];
                             if (val == 1 && hasFloor)
                             {
                                 float sx = static_cast<float>(tileSize) / static_cast<float>(tex.getSize().x);
                                 float sy = static_cast<float>(tileSize) / static_cast<float>(tex.getSize().y);
                                 tileSprite.setScale(sx, sy);
                                 tileSprite.setPosition(static_cast<float>(c * tileSize), static_cast<float>(r * tileSize));
                                 fenetre.draw(tileSprite);
                             }
                             else if (val >= 11 && val <= 18)
                             {
                                 int wi = val - 11;
                                 if (wi >= 0 && static_cast<size_t>(wi) < wallTexs.size() && wallTexs[wi].getSize().x > 0)
                                 {
                                     sf::Sprite w;
                                     w.setTexture(wallTexs[wi]);
                                     float sx = static_cast<float>(tileSize) / static_cast<float>(wallTexs[wi].getSize().x);
                                     float sy = static_cast<float>(tileSize) / static_cast<float>(wallTexs[wi].getSize().y);
                                     w.setScale(sx, sy);
                                     w.setPosition(static_cast<float>(c * tileSize), static_cast<float>(r * tileSize));
                                     fenetre.draw(w);
                                 }
                             }
                         }
                     }
                 }
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
            // Dessin du sprite animé si disponible, sinon fallback au rectangle
            const sf::Sprite& ps = modele.getPlayerSprite();
            if (ps.getTexture() && ps.getTexture()->getSize().x > 0)
            {
                // Le sprite interne a déjà été synchronisé (scale + position)
                fenetre.draw(ps);
            }
            else
            {
                fenetre.draw(modele.getJoueur());
            }

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

            // 5bis. Dessiner la boîte de dialogue en bas si active
            if (dialogActive && fontCharge)
            {
                fenetre.draw(dialogBox);
                fenetre.draw(dialogText);

                // auto-advance si le temps est écoulé
                if (dialogClock.getElapsedTime().asSeconds() >= dialogDisplayDuration)
                {
                    advanceDialogue();
                }
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
                            // Indiquer que la sortie du menu provient du clic sur JOUER
                            startedByPlayButton = true;
                            // Lancer immédiatement le premier dialogue si on en a
                            if (dialoguesLoaded && !dialogues.empty())
                            {
                                dialogStarted = true;
                                dialogActive = true;
                                currentDialogueIndex = 0;
                                // repositionner le texte au cas où
                                dialogText.setPosition(dialogBox.getPosition().x + 20.f, dialogBox.getPosition().y + 12.f);
                                dialogText.setString(dialogues[0]);
                                dialogClock.restart();
                            } else {
                            }
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
        else
        {
            // si boîte de dialogue active, avancer au clic ou à la touche
            if (dialogActive)
            {
                if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::KeyPressed || event.type == sf::Event::TouchBegan)
                {
                    advanceDialogue();
                    return; // consommer l'évènement pour éviter actions simultanées
                }
            }
        }
    }

    // Implémentation de l'avance de dialogue
    void Vue::advanceDialogue()
    {
        if (!dialoguesLoaded) return;
        currentDialogueIndex++;
        if (currentDialogueIndex >= dialogues.size())
        {
            dialogActive = false; // fini
        }
        else
        {
            dialogText.setString(dialogues[currentDialogueIndex]);
            dialogClock.restart();
        }
    }

}