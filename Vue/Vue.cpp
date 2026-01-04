
#include "Vue.h"
#include <cmath>
#include "Modele.h"
#include <iostream>
#include <string>

namespace Vue {

    // Constructor
    Vue::Vue(Modele::Modele& modele)
        : modele(modele)
    {
        // Try to load a font from a Windows system path first (Windows-specific),
        // then fallback to a local "arial.ttf" if present.
        //
        // WHY try multiple paths:
        // - supports different run directories (IDE, build output, distribution)
        // - avoids hard failure if Windows font path is unavailable
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontCharge = true;
            std::cout << "[DEBUG] Police chargée : C:\\Windows\\Fonts\\arial.ttf" << std::endl;

            // Collision HUD text
            collisionText.setFont(font);
            collisionText.setString("Collision detected !");
            collisionText.setCharacterSize(24);
            collisionText.setFillColor(sf::Color::Red);
            collisionText.setStyle(sf::Text::Bold);
            collisionText.setPosition(10.f, 10.f);

            // Player detected HUD text
            playerDetecteText.setFont(font);
            playerDetecteText.setString("Player detected !");
            playerDetecteText.setCharacterSize(22);
            playerDetecteText.setFillColor(sf::Color::Yellow);
            playerDetecteText.setStyle(sf::Text::Bold);
            playerDetecteText.setPosition(10.f, 40.f);

            // Lives HUD text (string updated each frame)
            livesText.setFont(font);
            livesText.setString("Lives: 3");
            livesText.setCharacterSize(28);

            // White because obstacles are red too (better contrast).
            livesText.setFillColor(sf::Color::White);
            livesText.setStyle(sf::Text::Bold);
            livesText.setPosition(10.f, 70.f);
        }
        else if (font.loadFromFile("arial.ttf"))
        {
            fontCharge = true;
            std::cout << "[DEBUG] Font loaded : arial.ttf (local)" << std::endl;

            // WHY still consider initializing texts here (note):
            // - current code loads the font but does not re-initialize the sf::Text fields in this branch
            // - left unchanged to preserve original behavior/comments
        }
        else
        {
            std::cerr << "[DEBUG] Error : Failed to load font (arial.ttf)." << std::endl;
        }
    }

    // Main render function
    void Vue::draw(sf::RenderWindow& fenetre)
    {
        // If no floor matrix, there is nothing to render for the map.
        //
        // WHY early return:
        // - avoids unnecessary work when map is not loaded / matrix is empty
        if (modele.getFloorMatrix().empty())
            return;

        // ================================
        // 1) Draw the map (tile grid)
        // ================================
        const auto& matrix = modele.getFloorMatrix();
        const sf::Texture& floorTex = modele.getFloorTexture();
        const auto& wallTexs = modele.getWallTextures();
        int tileSize = modele.getTileSize();

        // WHY use const references here:
        // - avoids copying large matrices/vectors and sf::Texture objects
        // - renderer only needs read-only access

        if (!matrix.empty())
        {
            // Determine rows and max columns (matrix may have ragged rows).
            size_t rows = matrix.size();
            size_t cols = 0;
            for (const auto& row : matrix) {
                if (row.size() > cols) cols = row.size();
            }

            // Compute map size in pixels (logical tile grid size).
            float mapWidth = static_cast<float>(cols * tileSize);
            float mapHeight = static_cast<float>(rows * tileSize);

            // Window size for scaling to fit the entire map on screen.
            sf::Vector2u winSize = fenetre.getSize();

            // Compute per-axis scale factors so the whole map fits in the window.
            // NOTE: This may stretch if scaleX != scaleY (aspect ratio not preserved).
            float scaleX = static_cast<float>(winSize.x) / mapWidth;
            float scaleY = static_cast<float>(winSize.y) / mapHeight;

            // Offsets are currently 0 (no centering / camera offset applied).
            float offsetX = 0.f;
            float offsetY = 0.f;

            // Reusable sprite for drawing floor fallback.
            sf::Sprite tileSprite;

            bool hasFloor = (floorTex.getSize().x > 0);
            if (hasFloor) tileSprite.setTexture(floorTex);

            // Draw each tile based on its ID.
            for (size_t r = 0; r < rows; ++r)
            {
                if (r >= matrix.size()) break;

                for (size_t c = 0; c < cols; ++c)
                {
                    if (c >= matrix[r].size()) break;

                    int val = matrix[r][c];

                    // Compute draw position in screen space (after scaling).
                    float drawX = offsetX + static_cast<float>(c * tileSize) * scaleX;
                    float drawY = offsetY + static_cast<float>(r * tileSize) * scaleY;

                    // Wall tiles are encoded as values 11..18 (mapped to wallTexs[0..7]).
                    if (val >= 11 && val <= 18)
                    {
                        int wi = val - 11;
                        if (wi >= 0 && static_cast<size_t>(wi) < wallTexs.size()) {
                            if (wallTexs[wi].getSize().x > 0) {
                                sf::Sprite w;
                                w.setTexture(wallTexs[wi]);

                                // Scale texture to tileSize, then apply global map scaling.
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
                        // Do not draw for tile value 1:
                        // comment indicates floor_01 moved to tile id 22.
                    }
                    else if (val >= 1)
                    {
                        // For general tiles, ask the model for the texture associated with this ID.
                        const sf::Texture* t = modele.getTileTexture(val);

                        // WHY model returns a pointer:
                        // - texture may not exist for an id => nullptr signals "missing"
                        // - avoids copying sf::Texture
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
                            // Fallback: if specific tile texture is missing, draw generic floor texture.
                            sf::Sprite s;
                            s.setTexture(floorTex);

                            float sx = (static_cast<float>(tileSize) / static_cast<float>(floorTex.getSize().x)) * scaleX;
                            float sy = (static_cast<float>(tileSize) / static_cast<float>(floorTex.getSize().y)) * scaleY;
                            s.setScale(sx, sy);
                            s.setPosition(drawX, drawY);
                            fenetre.draw(s);

                            // WHY fallback to floor texture:
                            // - ensures the map remains visible even if some tile ids are not registered
                        }
                    }
                }
            }
        }

        // ================================
        // 2) Draw doors
        // ================================
        for (const auto& door : modele.getCurrentRoomDoors())
        {
            // Doors have an optional visual shape for rendering.
            if (door.visualShape)
            {
                // WHY door.visualShape is checked:
                // - visualShape is a unique_ptr owned by the room/door
                // - it can be null if shapes haven't been initialized or door has no shape
                fenetre.draw(*door.visualShape);
            }
        }

        // ================================
        // 3) Draw physical obstacles (decor/solid)
        // ================================
        for (const auto& obsPtr : modele.getObstacleShapes()) {
            if (!obsPtr) continue;

            // Debug: log if the obstacle has a texture pointer or not.
            const sf::Texture* t = obsPtr->getTexture();
            if (t) {
                std::cout << "[Vue] Obstacle has texture ptr=" << reinterpret_cast<const void*>(t)
                          << " size=(" << t->getSize().x << "," << t->getSize().y << ")" << std::endl;
            } else {
                std::cout << "[Vue] Obstacle has no texture; drawing fill" << std::endl;
            }

            // The obstacle is drawn as-is (likely a RectangleShape or Sprite-like shape).
            //
            // WHY obsPtr is a unique_ptr owned elsewhere:
            // - model/room owns obstacles; view only renders them (non-owning usage)
            fenetre.draw(*obsPtr);
        }

        // ================================
        // 4) Draw objectives
        // ================================
        auto& currentObjectives = modele.getCurrentRoomObjectives();

        // WHY currentObjectives obtained by reference:
        // - model stores objectives; view reads them without copying the vector
        // - objectives contain SFML resources; copying them would be expensive and may be unsafe
        if (!currentObjectives.empty()) {
            // Debug: print objective list and key properties (position, size, texture pointer, etc.).
            std::cout << "[Vue] Current room has " << currentObjectives.size() << " objectives:\n";
            for (const auto& o : currentObjectives) {
                const sf::Sprite& s = o.getSprite();
                const sf::Texture* tptr = s.getTexture();
                std::cout << " - '" << o.getTitle() << "' pos=(" << o.getHitboxPosition().x << "," << o.getHitboxPosition().y
                          << ") size=(" << o.getHitboxSize().x << "," << o.getHitboxSize().y << ") texPtr="
                          << reinterpret_cast<const void*>(tptr) << " dialog='" << o.getDialogueRef()
                          << "' cesar=" << o.isCesar() << std::endl;
            }
        }

        for (const auto& objective : currentObjectives) {
            // Debug note : logs were added to diagnose why the sprite
            // did not show its texture. If texture is invalid, fallback to drawing hitbox.
            sf::Vector2f hp = objective.getHitboxPosition();
            sf::Vector2f hs = objective.getHitboxSize();

            // Draw the sprite if valid.
            // NOTE: A copy of the sprite is made here (sf::Sprite is lightweight).
            //
            // WHY copy sprite here:
            // - keeps Objective encapsulation (Objective currently returns sf::Sprite by value)
            // - allows drawing without retaining references to internal SFML state
            sf::Sprite spr = objective.getSprite();

            const sf::Texture* tptr = spr.getTexture();
            if (tptr && tptr->getSize().x > 0)
            {
                fenetre.draw(spr);
            }
            else
            {
                std::cout << "[DEBUG] Objective: sprite has no texture, drawing hitbox" << std::endl;
                fenetre.draw(objective.getHitbox()); // visual fallback
            }
        }

        // ================================
        // 5) Draw player (sprite if available, else fallback rectangle)
        // ================================
        const sf::Sprite& ps = modele.getPlayerSprite();
        if (ps.getTexture() && ps.getTexture()->getSize().x > 0)
        {
            fenetre.draw(ps);
        }
        else
        {
            fenetre.draw(modele.getJoueur());
        }

        // ================================
        // 6) Draw enemies and their vision representation (cone/laser)
        // ================================
        for (const auto& enemy : modele.getEnemies())
        {
            // enemy is a unique_ptr<Enemy> owned by the model;
            // view uses it as a non-owning observer.
            sf::Vector2f center = enemy->position;
            sf::Vector2f forward = enemy->direction;
            float range = enemy->visionRange;
            float angleDeg = enemy->visionAngle;

            // Laser enemy: draw a red line + an optional semi-transparent detection band.
            if (enemy->isLaser) {
                sf::Vector2f end = center + forward * enemy->laserLength;

                sf::VertexArray laser(sf::Lines, 2);
                laser[0].position = center;
                laser[0].color = sf::Color::Red;
                laser[1].position = end;
                laser[1].color = sf::Color::Red;
                fenetre.draw(laser);

                sf::RectangleShape band;
                float bandWidth = 40.f;
                band.setSize(sf::Vector2f(enemy->laserLength, bandWidth));
                band.setOrigin(0, bandWidth/2.f);
                band.setPosition(center);

                float angle = std::atan2(forward.y, forward.x) * 180.f / 3.14159265f;
                band.setRotation(angle);
                band.setFillColor(sf::Color(255,0,0,40));
                fenetre.draw(band);
            }
            else
            {
                // Vision cone: approximate the arc with multiple points.
                const float pi = 3.14159265f;
                float halfRad = (angleDeg * 0.5f) * pi / 180.f;

                // Local lambda for rotating a vector by an angle in radians.
                //
                // WHY lambda here:
                // - small math helper scoped to this render block
                // - avoids polluting the class interface with a tiny utility
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

                // Color-coding: cameras cyan, others orange.
                if (enemy->isCamera)
                    cone.setFillColor(sf::Color(0, 255, 255, 80));
                else
                    cone.setFillColor(sf::Color(255, 200, 0, 80));

                cone.setOutlineColor(sf::Color(255, 200, 0, 120));
                cone.setOutlineThickness(0.f);
                fenetre.draw(cone);
            }

            // Draw enemy sprite if texture is loaded, else fallback circle.
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

        // ================================
        // 7) Status texts (collision/detected) + lives counter
        // ================================
        if (fontCharge && modele.isCollisionDetectee())
        {
            fenetre.draw(collisionText);
        }

        if (fontCharge && modele.isPlayerDetecte())
        {
            fenetre.draw(playerDetecteText);
        }

        // Lives display is updated each frame.
        if (fontCharge)
        {
            livesText.setString("Lives: " + std::to_string(modele.getLives()));
            fenetre.draw(livesText);
        }
    }

    // Event handling (currently not used)
    void Vue::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
    {
        // No menu/dialogue handling here anymore.
        //
        // WHY keep this method:
        // - keeps a consistent "View interface" even if currently unused
        // - future UI interactions could be added here
        (void)event;
        (void)fenetre;
    }

}

