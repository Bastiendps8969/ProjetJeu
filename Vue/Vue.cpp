
#include "Vue.h"
#include <cmath>
#include "Modele.h"
#include <iostream>
#include <string>

namespace Vue {

// ================================
// Constructeur
// ================================
Vue::Vue(Modele::Modele& modele)
    : modele(modele)
{
    if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
    {
        fontCharge = true;
        std::cout << "[DEBUG] Police chargee : C:\\Windows\\Fonts\\arial.ttf" << std::endl;

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

        livesText.setFont(font);
        livesText.setString("Lives: 3");
        livesText.setCharacterSize(28);
        livesText.setFillColor(sf::Color::White);
        livesText.setStyle(sf::Text::Bold);
        livesText.setPosition(10.f, 70.f);
    }
    else if (font.loadFromFile("arial.ttf"))
    {
        fontCharge = true;
        std::cout << "[DEBUG] Police chargee : arial.ttf (local)" << std::endl;
    }
    else
    {
        std::cerr << "[DEBUG] Erreur: Impossible de charger la police (arial.ttf)." << std::endl;
    }
}

// ============================================================================
// Helper : applique un viewport “letterbox” pour conserver le ratio de la map
// ============================================================================
static void applyLetterboxView(sf::RenderWindow& window, sf::View& view, float worldW, float worldH)
{
    sf::Vector2u win = window.getSize();
    if (win.x == 0 || win.y == 0 || worldW <= 0.f || worldH <= 0.f) return;

    float windowRatio = static_cast<float>(win.x) / static_cast<float>(win.y);
    float viewRatio   = worldW / worldH;

    float sizeX = 1.f, sizeY = 1.f;
    float posX  = 0.f, posY  = 0.f;

    // Si la fenêtre est "plus large" que la map : bandes sur les côtés
    if (windowRatio > viewRatio)
    {
        sizeX = viewRatio / windowRatio;
        posX  = (1.f - sizeX) * 0.5f;
    }
    // Sinon : bandes en haut/bas
    else
    {
        sizeY = windowRatio / viewRatio;
        posY  = (1.f - sizeY) * 0.5f;
    }

    view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
}

// ================================
// Méthode de dessin principale
// ================================
void Vue::dessiner(sf::RenderWindow& fenetre)
{
    if (modele.getFloorMatrix().empty())
        return;

    // ================================
    // 1) Calcul des dimensions monde de la carte
    // ================================
    const auto& matrix = modele.getFloorMatrix();
    const sf::Texture& floorTex = modele.getFloorTexture();
    const auto& wallTexs = modele.getWallTextures();
    int tileSize = modele.getTileSize();

    size_t rows = matrix.size();
    size_t cols = 0;
    for (const auto& row : matrix)
        if (row.size() > cols) cols = row.size();

    float mapWidth  = static_cast<float>(cols * tileSize);
    float mapHeight = static_cast<float>(rows * tileSize);

    // ================================
    // 2) Mise en place d'une View “monde”
    // ================================
    sf::View worldView(sf::FloatRect(0.f, 0.f, mapWidth, mapHeight));
    worldView.setCenter(mapWidth * 0.5f, mapHeight * 0.5f);
    applyLetterboxView(fenetre, worldView, mapWidth, mapHeight);

    fenetre.setView(worldView);

    // ================================
    // 3) Dessin de la carte (tuiles)
    // ================================
    sf::Sprite tileSprite;
    bool hasFloor = (floorTex.getSize().x > 0);
    if (hasFloor)
        tileSprite.setTexture(floorTex);

    for (size_t r = 0; r < rows; ++r)
    {
        for (size_t c = 0; c < cols; ++c)
        {
            if (c >= matrix[r].size()) break;

            int val = matrix[r][c];

            float drawX = static_cast<float>(c * tileSize);
            float drawY = static_cast<float>(r * tileSize);

            // Mur (val 11..18)
            if (val >= 11 && val <= 18)
            {
                int wi = val - 11;
                if (wi >= 0 && static_cast<size_t>(wi) < wallTexs.size() && wallTexs[wi].getSize().x > 0)
                {
                    sf::Sprite w;
                    w.setTexture(wallTexs[wi]);
                    float sx = static_cast<float>(tileSize) / static_cast<float>(wallTexs[wi].getSize().x);
                    float sy = static_cast<float>(tileSize) / static_cast<float>(wallTexs[wi].getSize().y);
                    w.setScale(sx, sy);
                    w.setPosition(drawX, drawY);
                    fenetre.draw(w);
                }
            }
            else if (val == 1)
            {
                // Ne rien dessiner : floor_01 est désormais tile id 22
            }
            else if (val >= 1)
            {
                const sf::Texture* t = modele.getTileTexture(val);
                if (t && t->getSize().x > 0)
                {
                    sf::Sprite s;
                    s.setTexture(*t);
                    float sx = static_cast<float>(tileSize) / static_cast<float>(t->getSize().x);
                    float sy = static_cast<float>(tileSize) / static_cast<float>(t->getSize().y);
                    s.setScale(sx, sy);
                    s.setPosition(drawX, drawY);
                    fenetre.draw(s);
                }
                else if (hasFloor)
                {
                    sf::Sprite s;
                    s.setTexture(floorTex);
                    float sx = static_cast<float>(tileSize) / static_cast<float>(floorTex.getSize().x);
                    float sy = static_cast<float>(tileSize) / static_cast<float>(floorTex.getSize().y);
                    s.setScale(sx, sy);
                    s.setPosition(drawX, drawY);
                    fenetre.draw(s);
                }
            }
        }
    }

    // ================================
    // 4) Portes (en coordonnées monde)
    // ================================
    for (const auto& door : modele.getCurrentRoomDoors())
    {
        if (door.visualShape)
            fenetre.draw(*door.visualShape);
    }

    // ================================
    // 5) Obstacles (en coordonnées monde)
    // ================================
    for (const auto& obsPtr : modele.getObstacleShapes())
    {
        if (!obsPtr) continue;
        fenetre.draw(*obsPtr);
    }

    // ================================
    // 6) Objectifs (en coordonnées monde)
    //    -> On force le scale du sprite selon hitboxSize pour être stable
    // ================================
    auto& currentObjectives = modele.getCurrentRoomObjectives();

    // Logs debug (si tu veux garder)
    if (!currentObjectives.empty())
    {
        std::cout << "[Vue] Current room has " << currentObjectives.size() << " objectives:\n";
        for (const auto& o : currentObjectives)
        {
            const sf::Sprite& s = o.getSprite();
            const sf::Texture* tptr = s.getTexture();
            std::cout << " - '" << o.getTitle() << "' pos=("
                      << o.getHitboxPosition().x << "," << o.getHitboxPosition().y
                      << ") size=(" << o.getHitboxSize().x << "," << o.getHitboxSize().y
                      << ") texPtr=" << reinterpret_cast<const void*>(tptr)
                      << " dialog='" << o.getDialogueRef()
                      << "' cesar=" << o.isCesar() << std::endl;
        }
    }

    for (const auto& objective : currentObjectives)
    {
        sf::Vector2f hp = objective.getHitboxPosition();
        sf::Vector2f hs = objective.getHitboxSize();

        sf::Sprite spr = objective.getSprite();
        const sf::Texture* tptr = spr.getTexture();

        if (tptr && tptr->getSize().x > 0)
        {
            // Position monde
            spr.setPosition(hp);

            // Scale pour correspondre à w/h du JSON (hitbox)
            float sx = hs.x / static_cast<float>(tptr->getSize().x);
            float sy = hs.y / static_cast<float>(tptr->getSize().y);
            spr.setScale(sx, sy);

            fenetre.draw(spr);
        }
        else
        {
            // Hitbox debug en monde
            fenetre.draw(objective.getHitbox());
        }
    }

    // ================================
    // 7) Joueur (en coordonnées monde)
    // ================================
    const sf::Sprite& ps = modele.getPlayerSprite();

    if (ps.getTexture() && ps.getTexture()->getSize().x > 0)
    {
        // Important : on suppose que le sprite joueur est déjà en coordonnées monde
        fenetre.draw(ps);
    }
    else
    {
        fenetre.draw(modele.getJoueur());
    }

    // ================================
    // 8) Ennemis + vision (en coordonnées monde)
    // ================================
    for (const auto& enemy : modele.getEnemies())
    {
        sf::Vector2f center  = enemy->position;
        sf::Vector2f forward = enemy->direction;
        float range          = enemy->visionRange;
        float angleDeg       = enemy->visionAngle;

        // Laser (monde)
        if (enemy->isLaser)
        {
            sf::Vector2f end = center + forward * enemy->laserLength;

            sf::VertexArray laser(sf::Lines, 2);
            laser[0].position = center;
            laser[0].color    = sf::Color::Red;
            laser[1].position = end;
            laser[1].color    = sf::Color::Red;
            fenetre.draw(laser);

            // Bande semi-transparente (monde)
            sf::RectangleShape band;
            float bandWidth = 40.f;
            band.setSize(sf::Vector2f(enemy->laserLength, bandWidth));
            band.setOrigin(0.f, bandWidth / 2.f);
            band.setPosition(center);
            float angle = std::atan2(forward.y, forward.x) * 180.f / 3.14159265f;
            band.setRotation(angle);
            band.setFillColor(sf::Color(255, 0, 0, 40));
            fenetre.draw(band);
        }
        else
        {
            // Cone (monde)
            const float pi = 3.14159265f;
            float halfRad = (angleDeg * 0.5f) * pi / 180.f;

            auto rotate = [](const sf::Vector2f& v, float rad) -> sf::Vector2f
            {
                float c = std::cos(rad), s = std::sin(rad);
                return sf::Vector2f(c * v.x - s * v.y, s * v.x + c * v.y);
            };

            sf::ConvexShape cone;
            int arcPoints = 30;
            cone.setPointCount(1 + arcPoints);
            cone.setPoint(0, center);

            for (int i = 0; i < arcPoints; ++i)
            {
                float t = -halfRad + (i / float(arcPoints - 1)) * (2.f * halfRad);
                sf::Vector2f dir = rotate(forward, t);
                sf::Vector2f worldPt = center + dir * range;
                cone.setPoint(i + 1, worldPt);
            }

            if (enemy->isCamera)
                cone.setFillColor(sf::Color(0, 255, 255, 80));
            else
                cone.setFillColor(sf::Color(255, 200, 0, 80));

            cone.setOutlineThickness(0.f);
            fenetre.draw(cone);
        }

        // Sprite ennemi (monde)
        if (enemy->texture.getSize().x > 0)
        {
            enemy->sprite.setPosition(enemy->position);
            fenetre.draw(enemy->sprite);
        }
        else
        {
            sf::CircleShape enemyShape(30.f);
            enemyShape.setOrigin(30.f, 30.f);
            enemyShape.setPosition(enemy->position);

            if (enemy->isCamera)
                enemyShape.setFillColor(sf::Color::Cyan);
            else if (enemy->isLaser)
                enemyShape.setFillColor(sf::Color::Red);
            else
                enemyShape.setFillColor(sf::Color::Red);

            fenetre.draw(enemyShape);
        }
    }

    // ================================
    // 9) HUD : textes en coordonnées écran
    // ================================
    fenetre.setView(fenetre.getDefaultView());

    if (fontCharge && modele.isCollisionDetectee())
        fenetre.draw(collisionText);

    if (fontCharge && modele.isJoueurDetecte())
        fenetre.draw(joueurDetecteText);

    if (fontCharge)
    {
        livesText.setString("Lives: " + std::to_string(modele.getLives()));
        fenetre.draw(livesText);
    }
}

// ================================
// Gestion des événements
// ================================
void Vue::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
{
    // plus de gestion de menu/dialogue ici
}

} // namespace Vue