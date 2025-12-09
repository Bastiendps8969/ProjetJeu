#include "Vue.h"
#include <cmath>
#include "Modele.h"
#include <iostream>

namespace Vue
{
    // DÉFINITION : Constructeur
    Vue::Vue(Modele::Modele& modele)
    : modele(modele)
    {
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontCharge = true;
            std::cout << "[DEBUG] Police chargée : C:\\Windows\\Fonts\\arial.ttf" << std::endl;

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
        }
        else if (font.loadFromFile("arial.ttf"))
        {
            fontCharge = true;
            std::cout << "[DEBUG] Police chargée : arial.ttf (local)" << std::endl;
        }
        else
        {
            std::cerr << "[DEBUG] Erreur: Impossible de charger la police (arial.ttf)." << std::endl;
        }
    }

    // DÉFINITION : Méthode de dessin principale
    void Vue::dessiner(sf::RenderWindow& fenetre)
    {
        // NE PAS appeler fenetre.clear() ni fenetre.display() ici.
        // Se contenter de dessiner les éléments sur la fenêtre fournie.

        // Dessine le champ de vision de l'obstacle (couleur claire, transparente)
        const auto& obstacles = modele.getObstacleShapes();
        if (!obstacles.empty())
        {
            sf::Vector2f center = modele.getObstacleCenter(0);
            sf::Vector2f forward = modele.getObstacleForward(0);
            float range = 440.f;
            float angleDeg = 60.f;

            const float pi = 3.14159265f;
            float halfRad = (angleDeg * 0.5f) * pi / 180.f;
            float c = std::cos(halfRad), s = std::sin(halfRad);
            sf::Vector2f leftDir( c * forward.x - s * forward.y,
                                  s * forward.x + c * forward.y );
            sf::Vector2f rightDir( c * forward.x + s * forward.y,
                                  -s * forward.x + c * forward.y );

            sf::Vector2f leftPoint  = center + leftDir  * range;
            sf::Vector2f rightPoint = center + rightDir * range;

            sf::ConvexShape cone;
            cone.setPointCount(3);
            cone.setPoint(0, center);
            cone.setPoint(1, leftPoint);
            cone.setPoint(2, rightPoint);
            cone.setFillColor(sf::Color(173, 216, 230, 100));
            cone.setOutlineColor(sf::Color(173, 216, 230, 120));
            cone.setOutlineThickness(0.f);
            fenetre.draw(cone);
        }

        // Dessine le rectangle jaune
        fenetre.draw(modele.getJoueur());

        // Dessine les obstacles du vecteur
        for (const auto& obs : modele.getObstacleShapes())
        {   fenetre.draw(*obs); }

        // Dessine la grille de tuiles du sol (matrice)
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
                        if (wi >= 0 && static_cast<size_t>(wi) < wallTexs.size()) {
                            if (wallTexs[wi].getSize().x > 0) {
                                sf::Sprite w;
                                w.setTexture(wallTexs[wi]);
                                float sx = static_cast<float>(tileSize) / static_cast<float>(wallTexs[wi].getSize().x);
                                float sy = static_cast<float>(tileSize) / static_cast<float>(wallTexs[wi].getSize().y);
                                w.setScale(sx, sy);
                                w.setPosition(static_cast<float>(c * tileSize), static_cast<float>(r * tileSize));
                                fenetre.draw(w);
                            } else {
                                std::cerr << "[DEBUG] Texture mur wallTexs[" << wi << "] non chargée pour val=" << val << std::endl;
                            }
                        }
                    }
                }
            }
        }

        // 1. Dessiner les portes (carré bleu)
        for (const auto& door : modele.getCurrentRoomDoors())
        {
            if (door.visualShape)
            {
                fenetre.draw(*door.visualShape);
            }
        }

        // 2. Dessiner les obstacles physiques (ennemis, murs rouges, etc.)
        for (const auto& obsPtr : modele.getObstacleShapes())
        {
            fenetre.draw(*obsPtr);
        }

        // 3. Dessiner le joueur (sprite animé si dispo, sinon rectangle)
        const sf::Sprite& ps = modele.getPlayerSprite();
        if (ps.getTexture() && ps.getTexture()->getSize().x > 0)
        {
            fenetre.draw(ps);
        }
        else
        {
            fenetre.draw(modele.getJoueur());
        }

        // 4. Dessiner le champ de vision de l'obstacle (ennemi)
        if (!obstacles.empty())
        {
            sf::Vector2f center = modele.getObstacleCenter(0);
            sf::Vector2f forward = modele.getObstacleForward(0);
            float range = 440.f;
            float angleDeg = 60.f;

            const float pi = 3.14159265f;
            float halfRad = (angleDeg * 0.5f) * pi / 180.f;
            float c = std::cos(halfRad), s = std::sin(halfRad);
            sf::Vector2f leftDir( c * forward.x - s * forward.y,
                                  s * forward.x + c * forward.y );
            sf::Vector2f rightDir( c * forward.x + s * forward.y,
                                  -s * forward.x + c * forward.y );

            sf::Vector2f leftPoint  = center + leftDir  * range;
            sf::Vector2f rightPoint = center + rightDir * range;

            sf::ConvexShape cone;
            cone.setPointCount(3);
            cone.setPoint(0, center);
            cone.setPoint(1, leftPoint);
            cone.setPoint(2, rightPoint);
            cone.setFillColor(sf::Color(173, 216, 230, 100));
            cone.setOutlineColor(sf::Color(173, 216, 230, 120));
            cone.setOutlineThickness(0.f);
            fenetre.draw(cone);
        }

        // 5. Textes d'état
        if (fontCharge && modele.isCollisionDetectee())
        {
            fenetre.draw(collisionText);
        }
        if (fontCharge && modele.isJoueurDetecte())
        {
            fenetre.draw(joueurDetecteText);
        }
    }

    // DÉFINITION : Gestion des événements (principalement pour le menu)
    void Vue::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
    {
        // plus de gestion de menu/dialogue ici
    }
}