// Enemy.h
// Définit la hiérarchie d'ennemis utilisée par le modèle :
// - `Enemy` : classe de base (implémente Prototype)
// - `GenericEnemy` : ennemi patrouilleur classique (cone de vision)
// - `CameraEnemy` : caméra fixe avec cône de vision
// - `LaserEnemy` : laser / rayon droit de détection
//
// Chaque sous-classe implémente `clone()` via le pattern Prototype
// afin de permettre l'instanciation par clonage de prototypes.

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include "Prototype.h"

namespace Modele {

// Classe de base `Enemy` : comporte les champs partagés (position,
// direction, sprite, paramètres de vision/laser) et des méthodes
// virtuelles pour `update`, `updateAnimation`, `detectPlayer`.
// Elle dérive de `Prototype` et impose l'implémentation de
// `clone()` retournant `std::unique_ptr<Prototype>`.
class Enemy : public Prototype {
public:
    sf::Vector2f position;
    sf::Vector2f direction = {1.f, 0.f};
    bool joueurDetecte = false;
    std::string textureName;
    sf::Texture texture;
    sf::Sprite sprite;
    // common type flags / vision settings
    bool isCamera = false;
    bool isLaser = false;
    float visionRange = 300.f;
    float visionAngle = 60.f;
    float laserLength = 600.f;

    // Animation (shared)
    int frameCount = 9;
    int frameIndex = 0;
    int row = 3;
    float frameDuration = 0.08f;
    float idleFrameDuration = 0.24f;
    int idleFrameCount = 2;
    bool isMoving = false;
    int tileSize = 64;
    sf::Clock animClock;

    Enemy() = default;
    virtual ~Enemy() = default;

    // clone (Prototype) : doit être implémenté par chaque sous-classe
    // et retourner une copie encapsulée dans un `std::unique_ptr<Enemy>`.
    virtual std::unique_ptr<Enemy> clone() const = 0;

    virtual void update() {}
    virtual void updateAnimation()
    {
        // Cameras use a single static texture; don't apply sprite-sheet animation.
        if (isCamera) return;
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

    // detectPlayer : logique de détection du joueur. Les sous-classes
    // spécialisées (Generic/Camera/Laser) remplacent cette méthode.
    virtual void detectPlayer(const sf::RectangleShape& joueur) { (void)joueur; }

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

    static sf::Vector2f normalize(const sf::Vector2f& v)
    {
        float len = std::sqrt(v.x * v.x + v.y * v.y);
        if (len > 0.001f) return {v.x / len, v.y / len};
        return {1.f, 0.f};
    }
};

// GenericEnemy
// Ennemi patrouilleur standard. Se déplace entre `patrolPoints` et
// utilise le cône de vision (hérité : `visionRange` + `visionAngle`)
// pour détecter le joueur.
class GenericEnemy : public Enemy {
public:
    std::vector<sf::Vector2f> patrolPoints;
    int patrolIndex = 0;
    float speed = 2.0f;

    GenericEnemy() = default;
    GenericEnemy(const GenericEnemy& other) = default;

    void update() override
    {
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
        float dx = direction.x, dy = direction.y;
        if (std::abs(dx) > std::abs(dy)) {
            row = (dx > 0) ? 4 : 2;
        } else {
            row = (dy < 0) ? 1 : 3;
        }
    }

    // clone : renvoie une copie complète de cet objet.
    std::unique_ptr<Enemy> clone() const override { return std::make_unique<GenericEnemy>(*this); }

    // detectPlayer : implémente la détection par cône pour l'ennemi
    // patrouilleur en utilisant `visionRange` et `visionAngle`.
    void detectPlayer(const sf::RectangleShape& joueur) override
    {
        // Use base Enemy visionRange/visionAngle and current direction
        sf::FloatRect joueurBounds = joueur.getGlobalBounds();
        sf::Vector2f joueurCenter(joueurBounds.left + joueurBounds.width * 0.5f,
                                  joueurBounds.top + joueurBounds.height * 0.5f);

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
};

// Camera enemy (cone)
// CameraEnemy
// Représente une caméra fixe. Possède un champ de vision conique
// (`visionRange`, `visionAngle`) et ne se déplace pas ; sa `direction`
// est fixée selon `facing`.
class CameraEnemy : public Enemy {
public:
    std::string facing;
    // Uses base `visionRange` and `visionAngle` from `Enemy` to avoid duplication.

    CameraEnemy() = default;
    CameraEnemy(const CameraEnemy& other) = default;

    void detectPlayer(const sf::RectangleShape& joueur) override
    {
        sf::FloatRect joueurBounds = joueur.getGlobalBounds();
        sf::Vector2f joueurCenter(joueurBounds.left + joueurBounds.width * 0.5f,
                                  joueurBounds.top + joueurBounds.height * 0.5f);
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

    // clone : copie de la caméra
    std::unique_ptr<Enemy> clone() const override { return std::make_unique<CameraEnemy>(*this); }
};

// Laser enemy (straight beam)
// LaserEnemy
// Représente un rayon / laser droit. La détection se fait si le joueur
// se trouve dans une bande étroite le long de `direction` et à une
// distance inférieure à `laserLength`.
class LaserEnemy : public Enemy {
public:
    std::string facing;
    // Uses base `laserLength` from `Enemy` to avoid duplication.

    LaserEnemy() = default;
    LaserEnemy(const LaserEnemy& other) = default;

    void detectPlayer(const sf::RectangleShape& joueur) override
    {
        sf::FloatRect joueurBounds = joueur.getGlobalBounds();
        sf::Vector2f joueurCenter(joueurBounds.left + joueurBounds.width * 0.5f,
                                  joueurBounds.top + joueurBounds.height * 0.5f);
        sf::Vector2f toPlayer = joueurCenter - position;
        float proj = toPlayer.x * direction.x + toPlayer.y * direction.y;
        float dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
        float side = toPlayer.x * (-direction.y) + toPlayer.y * direction.x;
        joueurDetecte = (proj > 0 && proj < laserLength && std::abs(side) < 20.f);
    }

    // clone : copie du laser
    std::unique_ptr<Enemy> clone() const override { return std::make_unique<LaserEnemy>(*this); }
};

}
