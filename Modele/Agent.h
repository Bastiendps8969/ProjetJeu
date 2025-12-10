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
    bool joueurDetecte = false; // Ajout

    EnemyAgent(const sf::Vector2f& pos, const std::vector<sf::Vector2f>& patrol, float spd)
        : position(pos), patrolPoints(patrol), speed(spd)
    {
        if (!patrolPoints.empty())
            direction = normalize(patrolPoints[0] - position);
    }

    void update()
    {
        if (patrolPoints.empty()) return;
        sf::Vector2f target = patrolPoints[patrolIndex];
        sf::Vector2f toTarget = target - position;
        float dist = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
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
    }

    void detectPlayer(const sf::RectangleShape& joueur)
    {
        // Détection simple dans le cône de vision
        sf::FloatRect joueurBounds = joueur.getGlobalBounds();
        sf::Vector2f joueurCenter(joueurBounds.left + joueurBounds.width * 0.5f,
                                  joueurBounds.top + joueurBounds.height * 0.5f);

        float range = 300.f;
        float angleDeg = 60.f;
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
