#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

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

}
