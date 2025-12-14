// Agent.cpp
// Implémentation de la classe Agent (logique d'obstacle/détection simple).
// Ce fichier contient des routines utilitaires utilisées par Modele
// pour calculer le centre et l'orientation d'un obstacle, et
// pour déterminer si le joueur est dans le champ de vision d'un
// obstacle fixe/animé.

#include "Agent.h"
#include <cmath>

namespace Modele {

// Constructeur
// - `shapes` : pointeur vers le vecteur de formes (géré par RoomManager)
// - `patrouillePoints` : points éventuels utilisés pour orienter l'obstacle
Agent::Agent(std::vector<std::unique_ptr<sf::Shape>>* shapes, const std::vector<sf::Vector2f>& patrouillePoints)
    : obstacleShapes(shapes), pointsPatrouille(patrouillePoints)
{}

// mettreAJour
// Calcule si le joueur est détecté par le premier obstacle de `obstacleShapes`.
// Implémentation : champ de vision fixe (angle + portée) centré sur le "forward"
// obtenu via `getObstacleForward(0)`.
void Agent::mettreAJour(const sf::RectangleShape& joueur)
{
    if (!obstacleShapes || obstacleShapes->empty() || (*obstacleShapes)[0] == nullptr) {
        joueurDetecte = false;
        return;
    }

    // Récupère le centre du joueur (centre de sa hitbox)
    sf::FloatRect joueurBounds = joueur.getGlobalBounds();
    sf::Vector2f joueurCenter = sf::Vector2f(joueurBounds.left + joueurBounds.width * 0.5f,
                                             joueurBounds.top + joueurBounds.height * 0.5f);

    // Paramètres de FOV (codés en dur pour cet agent)
    const float fovAngle = 60.0f;    // angle total du cône en degrés
    const float fovRange = 440.0f;   // portée maximale en pixels

    // Pré-calcul : cos(halfAngle) est utilisé pour la comparaison via produit scalaire
    // car dot(F, Vn) >= cos(halfAngle) équivaut à angle(F, V) <= halfAngle.
    const float halfRad = fovAngle * 0.5f * (3.14159265f / 180.f);
    const float cosHalfFov = std::cos(halfRad);

    // Centre et vecteur 'avant' (unit) de l'obstacle
    sf::Vector2f center = getObstacleCenter(0);
    sf::Vector2f forward = getObstacleForward(0);

    // Vecteur du centre obstacle vers le joueur
    sf::Vector2f toJoueur = joueurCenter - center;
    float distJ = std::sqrt(toJoueur.x * toJoueur.x + toJoueur.y * toJoueur.y);

    // Cas limite : si la distance est quasi nulle, considérer le joueur détecté
    // (évite division par zéro lors de la normalisation).
    if (distJ <= 0.0001f) {
        joueurDetecte = true;
        return;
    }

    // Normalisation et produit scalaire
    sf::Vector2f toJNorm = sf::Vector2f(toJoueur.x / distJ, toJoueur.y / distJ);
    float dot = forward.x * toJNorm.x + forward.y * toJNorm.y;

    // Condition de détection : être dans la portée ET dans l'angle.
    // - `distJ <= fovRange` : le joueur est suffisamment proche.
    // - `dot >= cosHalfFov` : l'angle entre `forward` et `toJoueur` est inférieur au demi-angle.
    joueurDetecte = (distJ <= fovRange && dot >= cosHalfFov);

    // Remarque : ce test est une approximation basée sur l'AABB du joueur (getGlobalBounds).
    // Pour des obstacles qui doivent bloquer la vision (murs), il faudrait effectuer
    // un test de visibilité (raycast) entre `center` et `joueurCenter`, en vérifiant
    // l'intersection avec les shapes opaques de la pièce.
}

// getObstacleCenter
// Retourne le centre (en coordonnées monde) de la forme d'index `idx`.
sf::Vector2f Agent::getObstacleCenter(size_t idx) const
{
    if (!obstacleShapes || idx >= obstacleShapes->size() || (*obstacleShapes)[idx] == nullptr)
        return sf::Vector2f(0.f, 0.f);
    sf::FloatRect b = (*obstacleShapes)[idx]->getGlobalBounds();
    // getGlobalBounds renvoie un AABB (axis-aligned bounding box) en coordonnées monde.
    // Le centre retourné ici est le centre de l'AABB. Pour des formes non-rectangulaires,
    // on pourrait calculer un centroïde géométrique, mais l'AABB est suffisant pour
    // notre logique de détection simple.
    return sf::Vector2f(b.left + b.width * 0.5f, b.top + b.height * 0.5f);
}

// getObstacleForward
// Calcule un vecteur unitaire pointant vers le point de patrouille courant
// (utilisé comme "forward" pour la détection visuelle).
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
        // Retourne un vecteur unitaire pointant vers le prochain point de patrouille.
        // Ce vecteur est utilisé comme référence d'orientation pour le test de FOV.
        return sf::Vector2f(direction.x / distance, direction.y / distance);
    else
        // Si la cible est exactement la position courante, retourner un vecteur par défaut.
        return sf::Vector2f(1.f, 0.f);
}

} // namespace Modele
