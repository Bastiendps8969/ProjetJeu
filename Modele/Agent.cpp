#include "Agent.h"
#include <cmath>

namespace Modele {

Agent::Agent(std::vector<std::unique_ptr<sf::Shape>>* shapes, const std::vector<sf::Vector2f>& patrouillePoints)
    : obstacleShapes(shapes), pointsPatrouille(patrouillePoints)
{}

void Agent::mettreAJour(const sf::RectangleShape& joueur)
{
    if (!obstacleShapes || obstacleShapes->empty() || (*obstacleShapes)[0] == nullptr) {
        joueurDetecte = false;
        return;
    }

    sf::FloatRect joueurBounds = joueur.getGlobalBounds();
    sf::Vector2f joueurCenter = sf::Vector2f(joueurBounds.left + joueurBounds.width * 0.5f,
                                             joueurBounds.top + joueurBounds.height * 0.5f);

    const float fovAngle = 60.0f;
    const float fovRange = 440.0f;
    const float cosHalfFov = std::cos(fovAngle * 0.5f * (3.14159265f / 180.f));

    sf::Vector2f center = getObstacleCenter(0);
    sf::Vector2f forward = getObstacleForward(0);

    sf::Vector2f toJoueur = joueurCenter - center;
    float distJ = std::sqrt(toJoueur.x * toJoueur.x + toJoueur.y * toJoueur.y);

    if (distJ <= 0.0f) joueurDetecte = true;
    else {
        sf::Vector2f toJNorm = sf::Vector2f(toJoueur.x / distJ, toJoueur.y / distJ);
        float dot = forward.x * toJNorm.x + forward.y * toJNorm.y;
        joueurDetecte = (distJ <= fovRange && dot >= cosHalfFov);
    }
}

sf::Vector2f Agent::getObstacleCenter(size_t idx) const
{
    if (!obstacleShapes || idx >= obstacleShapes->size() || (*obstacleShapes)[idx] == nullptr)
        return sf::Vector2f(0.f, 0.f);
    sf::FloatRect b = (*obstacleShapes)[idx]->getGlobalBounds();
    return sf::Vector2f(b.left + b.width * 0.5f, b.top + b.height * 0.5f);
}

sf::Vector2f Agent::getObstacleForward(size_t idx) const
{
    if (!obstacleShapes || idx >= obstacleShapes->size() || (*obstacleShapes)[idx] == nullptr)
        return sf::Vector2f(1.f, 0.f);

    if (pointsPatrouille.empty()) return sf::Vector2f(1.f, 0.f);

    sf::Vector2f pos = (*obstacleShapes)[idx]->getPosition();
    sf::Vector2f cible = pointsPatrouille[pointCibleIndex];

    sf::Vector2f direction = cible - pos;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (distance > 0.0f)
        return sf::Vector2f(direction.x / distance, direction.y / distance);
    else
        return sf::Vector2f(1.f, 0.f);
}

}
