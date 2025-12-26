#include "Vue.h"
#include <cmath>
#include "Modele.h"
#include <iostream>

namespace Vue {

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
    // ================================
    // Dessin de la carte (tuiles)
    // ================================

    const auto& matrix = modele.getFloorMatrix();
    const sf::Texture& floorTex = modele.getFloorTexture();
    const auto& wallTexs = modele.getWallTextures();
    int tileSize = modele.getTileSize();

    if (!matrix.empty())
    {
        size_t rows = matrix.size();
        size_t cols = 0;
        for (const auto& row : matrix) {
            if (row.size() > cols) cols = row.size();
            }
        float mapWidth = static_cast<float>(cols * tileSize);
        float mapHeight = static_cast<float>(rows * tileSize);
        sf::Vector2u winSize = fenetre.getSize();

        // Calcul du facteur de zoom pour que toute la carte tienne dans la fenêtre
        float scaleX = static_cast<float>(winSize.x) / mapWidth;
        float scaleY = static_cast<float>(winSize.y) / mapHeight;
        float offsetX = 0.f;
        float offsetY = 0.f;

        sf::Sprite tileSprite;
        bool hasFloor = (floorTex.getSize().x > 0);
        if (hasFloor) tileSprite.setTexture(floorTex);
        for (size_t r = 0; r < rows; ++r)
        {
            if (r >= matrix.size()) break;
            for (size_t c = 0; c < cols; ++c)
            {
                if (c >= matrix[r].size()) break;
                int val = matrix[r][c];
                float drawX = offsetX + static_cast<float>(c * tileSize) * scaleX;
                float drawY = offsetY + static_cast<float>(r * tileSize) * scaleY;
                if (val >= 11 && val <= 18)
                {
                    int wi = val - 11;
                    if (wi >= 0 && static_cast<size_t>(wi) < wallTexs.size()) {
                        if (wallTexs[wi].getSize().x > 0) {
                            sf::Sprite w;
                            w.setTexture(wallTexs[wi]);
                            float sx = (static_cast<float>(tileSize) / static_cast<float>(wallTexs[wi].getSize().x)) * scaleX;
                            float sy = (static_cast<float>(tileSize) / static_cast<float>(wallTexs[wi].getSize().y)) * scaleY;
                            w.setScale(sx, sy);
                            w.setPosition(drawX, drawY);
                            fenetre.draw(w);
                        } else {
                            std::cerr << "[DEBUG] Texture mur wallTexs[" << wi << "] non chargée pour val=" << val << std::endl;
                        }
                    }
                }
                else if (val == 1)
                {
                    // Ne rien dessiner pour la valeur 1 : floor_01 est désormais tile id 22
                }
                else if (val >= 1)
                {
                    const sf::Texture* t = modele.getTileTexture(val);
                    if (t && t->getSize().x > 0)
                    {
                        sf::Sprite s;
                        s.setTexture(*t);
                        float sx = (static_cast<float>(tileSize) / static_cast<float>(t->getSize().x)) * scaleX;
                        float sy = (static_cast<float>(tileSize) / static_cast<float>(t->getSize().y)) * scaleY;
                        s.setScale(sx, sy);
                        s.setPosition(drawX, drawY);
                        fenetre.draw(s);
                    }
                    else if (hasFloor)
                    {
                        sf::Sprite s;
                        s.setTexture(floorTex);
                        float sx = (static_cast<float>(tileSize) / static_cast<float>(floorTex.getSize().x)) * scaleX;
                        float sy = (static_cast<float>(tileSize) / static_cast<float>(floorTex.getSize().y)) * scaleY;
                        s.setScale(sx, sy);
                        s.setPosition(drawX, drawY);
                        fenetre.draw(s);
                    }
                }
            }
        }
    }

        // ================================
        // Dessin des portes
        // ================================
        for (const auto& door : modele.getCurrentRoomDoors())
        {
            if (door.visualShape)
            {
                fenetre.draw(*door.visualShape);
            }
        }

        // 2. Dessiner les obstacles physiques (obstacles/decors)
        for (const auto& obsPtr : modele.getObstacleShapes()) {
            if (!obsPtr) continue;
            const sf::Texture* t = obsPtr->getTexture();
            if (t) {
                std::cout << "[Vue] Obstacle has texture ptr=" << reinterpret_cast<const void*>(t)
                          << " size=(" << t->getSize().x << "," << t->getSize().y << ")" << std::endl;
            } else {
                std::cout << "[Vue] Obstacle has no texture; drawing fill" << std::endl;
            }
            fenetre.draw(*obsPtr);
        }

        for (const auto& objective : modele.getCurrentRoomObjectives()) {
            // Debug : afficher infos utiles sur l'objectif
            // NOTE: ces logs ont été ajoutés temporairement pour diagnostiquer
            // pourquoi le `sf::Sprite` n'affichait pas la texture. Ils
            // montrent la position/tailles et si le sprite référence une
            // texture valide. Une fois le bug corrigé, ces lignes peuvent
            // être supprimées ou entourées par un flag de debug.
            
            sf::Vector2f hp = objective.getHitboxPosition();
            sf::Vector2f hs = objective.getHitboxSize();
            sf::Texture tex = objective.getTexture();

            // Dessine le sprite s’il est valide
            sf::Sprite spr = objective.getSprite();
            const sf::Texture* tptr = spr.getTexture();
            if (tptr && tptr->getSize().x > 0) {
              //  std::cout << "[DEBUG] Objective: drawing sprite (texture ptr=" << reinterpret_cast<const void*>(tptr) << ") scale=(" << spr.getScale().x << "," << spr.getScale().y << ") pos=(" << spr.getPosition().x << "," << spr.getPosition().y << ")" << std::endl;
                fenetre.draw(spr);
            }
            else {
                std::cout << "[DEBUG] Objective: sprite has no texture, drawing hitbox" << std::endl;
                fenetre.draw(objective.getHitbox()); // fallback visuel
            }
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

        // 4. (SUPPRIMER) Dessiner le champ de vision de l'obstacle (ennemi) lié à l'ancien système
        // if (!obstacles.empty()) { ... }

        // --- DESSIN DES ENNEMIS ET DE LEUR CONE DE VISION ---
        for (const auto& enemy : modele.getEnemies())
        {
            sf::Vector2f center = enemy->position;
            sf::Vector2f forward = enemy->direction;
            float range = enemy->visionRange;
            float angleDeg = enemy->visionAngle;

            // Laser
            if (enemy->isLaser) {
                sf::Vector2f end = center + forward * enemy->laserLength;
                sf::VertexArray laser(sf::Lines, 2);
                laser[0].position = center;
                laser[0].color = sf::Color::Red;
                laser[1].position = end;
                laser[1].color = sf::Color::Red;
                fenetre.draw(laser);

                // Optionnel : dessiner une bande de détection
                sf::RectangleShape band;
                float bandWidth = 40.f;
                band.setSize(sf::Vector2f(enemy->laserLength, bandWidth));
                band.setOrigin(0, bandWidth/2.f);
                band.setPosition(center);
                float angle = std::atan2(forward.y, forward.x) * 180.f / 3.14159265f;
                band.setRotation(angle);
                band.setFillColor(sf::Color(255,0,0,40));
                fenetre.draw(band);
            } else {
                const float pi = 3.14159265f;
                float halfRad = (angleDeg * 0.5f) * pi / 180.f;
                auto rotate = [](const sf::Vector2f& v, float rad) -> sf::Vector2f {
                    float c = std::cos(rad), s = std::sin(rad);
                    return sf::Vector2f(c * v.x - s * v.y, s * v.x + c * v.y);
                };

                sf::ConvexShape cone;
                int arcPoints = 30;
                cone.setPointCount(1 + arcPoints);
                cone.setPoint(0, center);
                for (int i = 0; i < arcPoints; ++i) {
                    float t = -halfRad + (i / float(arcPoints - 1)) * (2.f * halfRad);
                    sf::Vector2f dir = rotate(forward, t);
                    cone.setPoint(i + 1, center + dir * range);
                }
                if (enemy->isCamera)
                    cone.setFillColor(sf::Color(0, 255, 255, 80));
                else
                    cone.setFillColor(sf::Color(255, 200, 0, 80));
                cone.setOutlineColor(sf::Color(255, 200, 0, 120));
                cone.setOutlineThickness(0.f);
                fenetre.draw(cone);
            }

            // Dessin du sprite ennemi animé si texture chargée, sinon cercle rouge
            if (enemy->texture.getSize().x > 0) {
                enemy->sprite.setPosition(center);
                fenetre.draw(enemy->sprite);
            } else {
                sf::CircleShape enemyShape(30.f);
                enemyShape.setOrigin(30.f, 30.f);
                enemyShape.setPosition(center);
                if (enemy->isCamera)
                    enemyShape.setFillColor(sf::Color::Cyan);
                else if (enemy->isLaser)
                    enemyShape.setFillColor(sf::Color::Red);
                else
                    enemyShape.setFillColor(sf::Color::Red);
                fenetre.draw(enemyShape);
            }
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