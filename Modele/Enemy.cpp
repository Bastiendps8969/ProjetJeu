// Enemy.cpp
// Implémentations déplacées des classes d'ennemis : GenericEnemy, CameraEnemy, LaserEnemy

#include "Enemy.h"
#include <memory>
#include <cmath>
#include "RoomManager.h"
#include <map>
#include <iostream>

namespace Modele {

// GenericEnemy
void GenericEnemy::update()
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

std::unique_ptr<Enemy> GenericEnemy::clone() const { return std::make_unique<GenericEnemy>(*this); }

void GenericEnemy::detectPlayer(const sf::RectangleShape& joueur)
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

// CameraEnemy
void CameraEnemy::detectPlayer(const sf::RectangleShape& joueur)
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

std::unique_ptr<Enemy> CameraEnemy::clone() const { return std::make_unique<CameraEnemy>(*this); }

// LaserEnemy
void LaserEnemy::detectPlayer(const sf::RectangleShape& joueur)
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

std::unique_ptr<Enemy> LaserEnemy::clone() const { return std::make_unique<LaserEnemy>(*this); }

} // namespace Modele

// Factory implementation
namespace Modele {

std::unique_ptr<Enemy> createEnemyFromDefinition(const EnemyDefinition& ed,
                                                const std::map<std::string, std::unique_ptr<Enemy>>& prototypes,
                                                float scaleW, float scaleH)
{
    std::vector<sf::Vector2f> patrol;
    for (const auto& pt : ed.patrolPoints) patrol.push_back(sf::Vector2f(pt.x * scaleW, pt.y * scaleH));
    sf::Vector2f pos(ed.position.x * scaleW, ed.position.y * scaleH);
    float scaledVisionRange = ed.visionRange * std::sqrt(scaleW * scaleH);
    float scaledLaserLength = ed.laserLength * std::sqrt(scaleW * scaleH);

    std::string type = ed.isLaser ? "laser" : (ed.isCamera ? "camera" : "generic");
    auto it = prototypes.find(type);
    std::unique_ptr<Enemy> e;
    if (it != prototypes.end()) e = it->second->clone();
    else {
        auto itg = prototypes.find("generic");
        if (itg != prototypes.end()) e = itg->second->clone();
        else e = std::make_unique<GenericEnemy>();
    }

    e->position = pos;
    e->textureName = ed.textureName;

    // Texture loading and sprite setup
    if (type == "camera") {
        const std::vector<std::string> tryPathsCam = {
            "cmake-build-debug/Asset/camera/" + e->textureName + ".png",
            "Asset/camera/" + e->textureName + ".png",
            "camera/" + e->textureName + ".png",
            e->textureName + ".png"
        };
        bool camLoaded = false;
        for (const auto& p : tryPathsCam) {
            if (e->texture.loadFromFile(p)) { e->sprite.setTexture(e->texture); camLoaded = true; break; }
        }
        if (camLoaded) {
            e->frameCount = 1;
            e->idleFrameCount = 1;
            e->frameIndex = 0;
            e->isMoving = false;
            sf::Vector2u ts = e->texture.getSize();
            if (ts.x > 0 && ts.y > 0) {
                e->sprite.setTextureRect(sf::IntRect(0,0,static_cast<int>(ts.x), static_cast<int>(ts.y)));
                e->sprite.setOrigin(static_cast<float>(ts.x)/2.f, static_cast<float>(ts.y)/2.f);
                e->sprite.setScale(1.f,1.f);
                e->tileSize = static_cast<int>(std::min(ts.x, ts.y));
            } else {
                e->sprite.setOrigin(e->tileSize/2.f, e->tileSize/2.f);
                e->sprite.setScale(2.5f,2.5f);
            }
            CameraEnemy* cc = dynamic_cast<CameraEnemy*>(e.get());
            if (cc) {
                if (cc->facing == "left") e->sprite.setRotation(180.f);
                else if (cc->facing == "up") e->sprite.setRotation(-90.f);
                else if (cc->facing == "down") e->sprite.setRotation(90.f);
                else e->sprite.setRotation(0.f);
            }
        }
    } else {
        const std::vector<std::string> tryPathsTex = {
            "cmake-build-debug/Asset/Human/" + e->textureName + ".png",
            "Asset/Human/" + e->textureName + ".png",
            "Human/" + e->textureName + ".png",
            e->textureName + ".png"
        };
        for (const auto& p : tryPathsTex) {
            if (e->texture.loadFromFile(p)) { e->sprite.setTexture(e->texture); break; }
        }
        e->sprite.setOrigin(e->tileSize/2.f, e->tileSize/2.f);
        e->sprite.setScale(2.5f, 2.5f);
    }

    if (type == "generic") {
        GenericEnemy* g = dynamic_cast<GenericEnemy*>(e.get());
        if (g) {
            g->patrolPoints = patrol;
            g->speed = ed.speed;
            if (!g->patrolPoints.empty()) g->direction = Enemy::normalize(g->patrolPoints[0] - g->position);
        }
        e->isCamera = false;
        e->isLaser = false;
        e->visionRange = scaledVisionRange;
        e->visionAngle = ed.visionAngle;
    } else if (type == "camera") {
        CameraEnemy* c = dynamic_cast<CameraEnemy*>(e.get());
        if (c) {
            c->facing = ed.facing;
            if (c->facing == "left") c->direction = {-1.f,0.f};
            else if (c->facing == "right") c->direction = {1.f,0.f};
            else if (c->facing == "up") c->direction = {0.f,-1.f};
            else c->direction = {0.f,1.f};
        }
        e->isCamera = true;
        e->isLaser = false;
        e->visionRange = scaledVisionRange;
        e->visionAngle = ed.visionAngle;
    } else if (type == "laser") {
        LaserEnemy* l = dynamic_cast<LaserEnemy*>(e.get());
        if (l) {
            l->facing = ed.facing;
            if (l->facing == "left") l->direction = {-1.f,0.f};
            else if (l->facing == "right") l->direction = {1.f,0.f};
            else if (l->facing == "up") l->direction = {0.f,-1.f};
            else l->direction = {0.f,1.f};
        }
        e->isLaser = true;
        e->isCamera = false;
        e->laserLength = scaledLaserLength;
    }

    return e;
}

} // namespace Modele
