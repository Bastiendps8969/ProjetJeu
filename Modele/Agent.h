#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <cmath> // <-- Ajout ici

namespace Modele {

class Agent {
private:
    std::vector<std::unique_ptr<sf::Shape>>* obstacleShapes;
    std::vector<sf::Vector2f> pointsPatrouille;
    int pointCibleIndex = 0;
    float vitessePatrouille = 0.125f;
    sf::Vector2f obstacleVitesse = sf::Vector2f(0.3f, 0.2f);
    bool joueurDetecte = false;

public:
    Agent(std::vector<std::unique_ptr<sf::Shape>>* shapes, const std::vector<sf::Vector2f>& patrouillePoints);

    void mettreAJour(const sf::RectangleShape& joueur);
    sf::Vector2f getObstacleCenter(size_t idx = 0) const;
    sf::Vector2f getObstacleForward(size_t idx = 0) const;
    bool isJoueurDetecte() const { return joueurDetecte; }
    void setJoueurDetecte(bool v) { joueurDetecte = v; }
    void setPointsPatrouille(const std::vector<sf::Vector2f>& pts) { pointsPatrouille = pts; }
    void setPointCibleIndex(int idx) { pointCibleIndex = idx; }
};

class EnemyAgent {
public:
    sf::Vector2f position;
    std::vector<sf::Vector2f> patrolPoints;
    int patrolIndex = 0;
    float speed = 2.0f;
    sf::Vector2f direction = {1.f, 0.f};
    bool joueurDetecte = false;
    std::string textureName;
    sf::Texture texture;
    sf::Sprite sprite;

    // Animation
    int frameCount = 9; // frames par ligne (comme agent)
    int frameIndex = 0;
    int row = 3; // 1=up,2=left,3=down,4=right (par défaut bas)
    float frameDuration = 0.08f;
    float idleFrameDuration = 0.24f;
    int idleFrameCount = 2;
    bool isMoving = false;
    int tileSize = 64;
    sf::Clock animClock;

    // Ajout pour caméra
    bool isCamera = false;
    std::string facing;
    float visionRange = 300.f;
    float visionAngle = 60.f;
    // Ajout pour laser
    bool isLaser = false;
    float laserLength = 600.f;

    EnemyAgent(const sf::Vector2f& pos, const std::vector<sf::Vector2f>& patrol, float spd, const std::string& texName,
               bool camera = false, const std::string& facingDir = "",
               float visionRange_ = 300.f, float visionAngle_ = 60.f,
               bool laser = false, float laserLength_ = 600.f)
        : position(pos), patrolPoints(patrol), speed(spd), textureName(texName),
          isCamera(camera), facing(facingDir), visionRange(visionRange_), visionAngle(visionAngle_),
          isLaser(laser), laserLength(laserLength_)
    {
        if (isCamera || isLaser) {
            if (facing == "left") direction = {-1.f, 0.f};
            else if (facing == "right") direction = {1.f, 0.f};
            else if (facing == "up") direction = {0.f, -1.f};
            else direction = {0.f, 1.f}; // "down" par défaut
        } else if (!patrolPoints.empty()) {
            direction = normalize(patrolPoints[0] - position);
        }

        // Chargement de la texture (chemins possibles)
        const std::vector<std::string> tryPaths = {
            "cmake-build-debug/Asset/Human/" + textureName + ".png",
            "Asset/Human/" + textureName + ".png",
            "Human/" + textureName + ".png",
            textureName + ".png"
        };
        for (const auto& p : tryPaths) {
            if (texture.loadFromFile(p)) {
                sprite.setTexture(texture);
                break;
            }
        }
        sprite.setOrigin(tileSize / 2.f, tileSize / 2.f);
        sprite.setScale(2.5f, 2.5f); // même échelle que l'agent
        animClock.restart();
    }

    void update()
    {
        if (isCamera) {
            isMoving = false;
            // direction déjà fixée
            return;
        }
        if (patrolPoints.empty()) return;
        sf::Vector2f target = patrolPoints[patrolIndex];
        sf::Vector2f toTarget = target - position;
        float dist = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
        isMoving = dist > 1.f;
        if (dist < speed)
        {
            position = target;
            patrolIndex = (patrolIndex + 1) % patrolPoints.size();
            direction = normalize(patrolPoints[patrolIndex] - position);
        }
        else
        {
            direction = normalize(toTarget);
            position += direction * speed;
        }
        // Déterminer la row d'animation selon la direction
        float dx = direction.x, dy = direction.y;
        if (std::abs(dx) > std::abs(dy)) {
            row = (dx > 0) ? 4 : 2; // droite/gauche
        } else {
            row = (dy < 0) ? 1 : 3; // haut/bas
        }
    }

    void updateAnimation()
    {
        if (texture.getSize().x == 0) return;
        int movementFramesCount = std::max(1, frameCount - idleFrameCount);
        int idleCols = std::max(1, idleFrameCount);
        float elapsed = animClock.getElapsedTime().asSeconds();
        float duration = isMoving ? frameDuration : idleFrameDuration;

        if (elapsed >= duration) {
            if (isMoving)
                frameIndex = (frameIndex + 1) % movementFramesCount;
            else
                frameIndex = (frameIndex + 1) % idleCols;
            animClock.restart();
        }
        sprite.setTextureRect(computeTextureRect());
    }

    sf::IntRect computeTextureRect() const
    {
        int frameSize = tileSize;
        int col = 0;
        int rowIndex = std::max(0, row - 1);
        int movementFramesCount = std::max(1, frameCount - idleFrameCount);

        if (isMoving) {
            col = frameIndex % movementFramesCount;
            rowIndex = std::max(0, row - 1);
        } else {
            int idleCols = std::max(1, idleFrameCount);
            col = frameIndex % idleCols;
            rowIndex = std::max(0, row - 1) + 4;
        }
        int frameX = col * frameSize;
        int frameY = rowIndex * frameSize;
        return sf::IntRect(frameX, frameY, frameSize, frameSize);
    }

    void detectPlayer(const sf::RectangleShape& joueur)
    {
        sf::FloatRect joueurBounds = joueur.getGlobalBounds();
        sf::Vector2f joueurCenter(joueurBounds.left + joueurBounds.width * 0.5f,
                                  joueurBounds.top + joueurBounds.height * 0.5f);

        if (isLaser) {
            // Détection laser : si le joueur est sur la ligne du laser et dans la portée
            sf::Vector2f toPlayer = joueurCenter - position;
            float proj = toPlayer.x * direction.x + toPlayer.y * direction.y;
            float dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
            float side = toPlayer.x * (-direction.y) + toPlayer.y * direction.x; // distance latérale
            // Laser = bande étroite (±20px) sur la direction, et dans la portée
            joueurDetecte = (proj > 0 && proj < laserLength && std::abs(side) < 20.f);
            return;
        }

        // Détection simple dans le cône de vision
        float range = visionRange;
        float angleDeg = visionAngle;
        float pi = 3.14159265f;
        float halfRad = (angleDeg * 0.5f) * pi / 180.f;
        float cosHalfFov = std::cos(halfRad);

        sf::Vector2f toPlayer = joueurCenter - position;
        float dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
        if (dist < 1.f) { joueurDetecte = true; return; }
        sf::Vector2f toPlayerNorm = {toPlayer.x / dist, toPlayer.y / dist};
        float dot = direction.x * toPlayerNorm.x + direction.y * toPlayerNorm.y;
        joueurDetecte = (dist <= range && dot >= cosHalfFov);
    }

    static sf::Vector2f normalize(const sf::Vector2f& v)
    {
        float len = std::sqrt(v.x * v.x + v.y * v.y);
        if (len > 0.001f) return {v.x / len, v.y / len};
        return {1.f, 0.f};
    }
};

}
