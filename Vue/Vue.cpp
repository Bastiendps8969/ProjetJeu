#include "Vue.h"

#include <cmath>
#include <memory> // pour std::make_unique

#include "Modele.h"
#include "HomePage.h"
#include "Player.h"

namespace Vue
{
    Vue::Vue(Modele::Modele& modele)
    : modele(modele)
    {
        // charge police pour les textes de jeu (collision / détection)
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontCharge = true;

            collisionText.setFont(font);
            collisionText.setString("Collision detectee !");
            collisionText.setCharacterSize(24);
            collisionText.setFillColor(sf::Color::Red);
            collisionText.setStyle(sf::Text::Bold);
            collisionText.setPosition(10.f, 10.f);

            // Texte pour détection joueur (sous le message de collision)
            joueurDetecteText.setFont(font);
            joueurDetecteText.setString("Joueur detecte !");
            joueurDetecteText.setCharacterSize(22);
            joueurDetecteText.setFillColor(sf::Color::Yellow);
            joueurDetecteText.setStyle(sf::Text::Bold);
            joueurDetecteText.setPosition(10.f, 40.f);
        }
    }

    void Vue::dessiner(sf::RenderWindow& fenetre)
    {
        // NE PAS appeler fenetre.clear() ni fenetre.display() ici.
        // Se contenter de dessiner les éléments sur la fenêtre fournie.

        // Dessine le champ de vision de l'obstacle (couleur claire, transparente)
        const auto& obstacles = modele.getObstacles(); // cache la référence
        if (!obstacles.empty())
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
            cone.setFillColor(sf::Color(173, 216, 230, 100));
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
    }
}
