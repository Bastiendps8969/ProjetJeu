#include "Vue.h"

#include <cmath>

#include "Modele.h"

namespace Vue
{
    Vue::Vue(Modele::Modele& modele)
    : modele(modele)
    {
        // Tentative de chargement d'une police système (Windows).
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontCharge = true;

            // collision text existing setup
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

            // --- Menu UI setup ---
            titleText.setFont(font);
            titleText.setString("Mon Jeu");
            titleText.setCharacterSize(48);
            titleText.setFillColor(sf::Color::White);
            titleText.setStyle(sf::Text::Bold);
            titleText.setPosition(220.f, 80.f);

            inputBox.setSize({360.f, 40.f});
            inputBox.setFillColor(sf::Color(50,50,50));
            inputBox.setOutlineColor(sf::Color::White);
            inputBox.setOutlineThickness(2.f);
            inputBox.setPosition(220.f, 200.f);

            inputText.setFont(font);
            inputText.setString("");
            inputText.setCharacterSize(20);
            inputText.setFillColor(sf::Color::White);
            inputText.setPosition(inputBox.getPosition() + sf::Vector2f(8.f, 6.f));

            playButton.setSize({160.f, 50.f});
            playButton.setFillColor(sf::Color(70,130,180)); // steelblue
            playButton.setPosition(320.f, 270.f);

            playLabel.setFont(font);
            playLabel.setString("Play");
            playLabel.setCharacterSize(24);
            playLabel.setFillColor(sf::Color::White);
            // center label in button
            sf::FloatRect lb = playLabel.getLocalBounds();
            playLabel.setPosition(
                playButton.getPosition().x + (playButton.getSize().x - lb.width) / 2.f - lb.left,
                playButton.getPosition().y + (playButton.getSize().y - lb.height) / 2.f - lb.top
            );
        }
    }

    void Vue::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
    {
        if (!menuActif) return;

        if (event.type == sf::Event::TextEntered)
        {
            // Accept printable chars, limit length
            if (event.text.unicode >= 32 && event.text.unicode < 127 && playerName.size() < 32)
            {
                playerName.push_back(static_cast<char>(event.text.unicode));
                inputText.setString(playerName);
            }
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::BackSpace && !playerName.empty())
            {
                playerName.pop_back();
                inputText.setString(playerName);
            }
            else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return)
            {
                // Start game with current name
                menuActif = false;
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2i mp = sf::Mouse::getPosition(fenetre);
            sf::Vector2f world = fenetre.mapPixelToCoords(mp);
            if (playButton.getGlobalBounds().contains(world))
            {
                menuActif = false;
            }
            // clicking input box sets focus (not tracked further here)
            if (inputBox.getGlobalBounds().contains(world))
            {
                // optional: could track focus
            }
        }
    }

    void Vue::dessiner(sf::RenderWindow& fenetre)
    {
        // If menu active, draw menu and return
        if (menuActif)
        {
            fenetre.clear(sf::Color(30,30,40)); // dark background

            if (fontCharge)
            {
                fenetre.draw(titleText);
                fenetre.draw(inputBox);
                fenetre.draw(inputText);
                fenetre.draw(playButton);
                fenetre.draw(playLabel);
            }
            fenetre.display();
            return;
        }

        // --- existing game drawing code ---
        // Vide l'écran
        fenetre.clear();

        // Dessine le champ de vision de l'obstacle (couleur claire, transparente)
        if (!modele.getObstacles().empty())
        {
            // Récupère paramètres depuis le modèle
            sf::Vector2f center = modele.getObstacleCenter(0);
            sf::Vector2f forward = modele.getObstacleForward(0);
            float range = modele.getFovRange();
            float angleDeg = modele.getFovAngleDeg();

            // Calcul des deux directions bord gauche/droite du cône
            const float pi = 3.14159265f;
            float halfRad = (angleDeg * 0.5f) * pi / 180.f;
            float c = std::cos(halfRad), s = std::sin(halfRad);
            // rotation +half
            sf::Vector2f leftDir( c * forward.x - s * forward.y,
                                  s * forward.x + c * forward.y );
            // rotation -half
            sf::Vector2f rightDir( c * forward.x + s * forward.y,
                                  -s * forward.x + c * forward.y );

            sf::Vector2f leftPoint  = center + leftDir  * range;
            sf::Vector2f rightPoint = center + rightDir * range;

            sf::ConvexShape cone;
            cone.setPointCount(3);
            cone.setPoint(0, center);
            cone.setPoint(1, leftPoint);
            cone.setPoint(2, rightPoint);
            // Couleur claire + transparence (alpha ~100)
            cone.setFillColor(sf::Color(173, 216, 230, 100)); // light blue, semi-transparent
            cone.setOutlineColor(sf::Color(173, 216, 230, 120));
            cone.setOutlineThickness(0.f);
            fenetre.draw(cone);
        }

        // Dessine le rectangle jaune
        fenetre.draw(modele.getJoueur());

        // Dessine les obstacles du vecteur
        for (const auto& obs : modele.getObstacles())
        {   fenetre.draw(*obs); }

        // Si le modèle indique une collision, affiche le texte (si police disponible)
        if (fontCharge && modele.isCollisionDetectee())
        {
            fenetre.draw(collisionText);
        }

        // Si le modèle indique détection joueur via champ de vision
        if (fontCharge && modele.isJoueurDetecte())
        {
            fenetre.draw(joueurDetecteText);
        }

        fenetre.display();
    }
}
