// Agent.h
// Contient la logique simple d'un "agent" utilisé pour la détection
// des obstacles mobiles (ex: obstacles animés / barrières) dans la
// scène. Ceci est distinct de la hiérarchie d'ennemis.

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <cmath> // fonctions math (sqrt, abs)

namespace Modele {

// Classe Agent
// - Gère un ensemble minimal d'obstacles (formes SFML) et un
//   comportement de détection simple vis-à-vis du joueur.
// - Ce n'est pas un ennemi : il sert pour la logique d'obstacles
//   animés et de détection liée aux obstacles.
class Agent {
private:
    // Pointeur vers les formes (obstacles) gérées par RoomManager.
    std::vector<std::unique_ptr<sf::Shape>>* obstacleShapes;

    // Points de patrouille utilisés pour définir une direction de
    // mouvement / orientation pour l'obstacle (si applicable).
    std::vector<sf::Vector2f> pointsPatrouille;
    int pointCibleIndex = 0;

    // Paramètres internes de mouvement du prototype d'obstacle
    float vitessePatrouille = 0.125f;
    sf::Vector2f obstacleVitesse = sf::Vector2f(0.3f, 0.2f);

    // Indicateur si le joueur a été détecté par cet agent
    bool joueurDetecte = false;

public:
    // Constructeur
    // - `shapes` : tableau de formes utilisé pour calculer centre/forward
    // - `patrouillePoints` : points de patrouille (optionnel)
    Agent(std::vector<std::unique_ptr<sf::Shape>>* shapes, const std::vector<sf::Vector2f>& patrouillePoints);

    // NOTE (collisions & détection) :
    // - Les méthodes de cette classe effectuent des tests de visibilité/collision
    //   simples pour des obstacles statiques ou de type "agent".
    // - Les coordonnées utilisées sont celles des formes SFML (coordonnées monde / écran)
    //   retournées par `getGlobalBounds()` et `getPosition()` : il n'y a pas de
    //   transformation vers un espace local supplémentaire.
    // - La détection principale est un FOV (champ de vision) défini par un angle
    //   et une portée. Mathématiquement, pour un obstacle situé en `C` et ayant
    //   un vecteur avant unitaire `F` :
    //     - On calcule le vecteur vers le joueur `V = P - C` et sa longueur `d = |V|`.
    //     - On normalise `Vn = V / d` et on calcule le produit scalaire `dot = F · Vn`.
    //     - Si `d <= range` ET `dot >= cos(halfAngle)` alors le joueur est dans le cône.
    // - Remarques pratiques :
    //     * `getGlobalBounds()` renvoie un rectangle de collision approximatif (AABB).
    //       Pour des formes non-rectangulaires, le centre retourné est le centre de l'AABB.
    //     * Les seuils (ex: distance minimale) protègent contre les divisions par zéro
    //       et garantissent qu'une très courte distance est considérée comme détection.
    //     * Cette implémentation est volontairement simple pour la clarté et les
    //       performances ; si une détection pixel-perfect ou avec obstacles opaques
    //       est nécessaire, il faudra ajouter des tests de visibilité (raycasts).

    // Met à jour l'état interne de détection en fonction de la position du joueur.
    // Utilise un FOV (angle + portée) codé en dur ici pour le premier obstacle.
    void mettreAJour(const sf::RectangleShape& joueur);

    // Renvoie le centre (position) de l'obstacle `idx` (calculé depuis sa forme)
    sf::Vector2f getObstacleCenter(size_t idx = 0) const;

    // Renvoie le vecteur "avant" de l'obstacle (direction vers son point de patrouille cible)
    sf::Vector2f getObstacleForward(size_t idx = 0) const;

    // Accesseurs pour l'indicateur de détection
    bool isJoueurDetecte() const { return joueurDetecte; }
    void setJoueurDetecte(bool v) { joueurDetecte = v; }

    // Modifier les points de patrouille à la volée
    void setPointsPatrouille(const std::vector<sf::Vector2f>& pts) { pointsPatrouille = pts; }
    void setPointCibleIndex(int idx) { pointCibleIndex = idx; }
};

}
