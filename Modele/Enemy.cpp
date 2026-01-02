
// Enemy.cpp
// Implements concrete enemy behavior and a factory for data-driven creation.

#include "Enemy.h"

#include <memory>     // std::unique_ptr, std::make_unique
#include <cmath>      // std::sqrt, std::cos
#include "RoomManager.h"  // likely defines EnemyDefinition
#include <map>        // std::map
#include <iostream>   // debug output

namespace Modele {

// ============================================================
// GenericEnemy::update
// ============================================================
void GenericEnemy::update()
{
    // If there is no patrol route, we cannot move anywhere.
    if (patrolPoints.empty())
        return;

    // Current target point in the patrol route.
    sf::Vector2f target = patrolPoints[patrolIndex];

    // Vector pointing from enemy position to the target.
    sf::Vector2f toTarget = target - position;

    // Distance to the target (Euclidean distance).
    float dist = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

    // isMoving is used by updateAnimation() to pick walk vs idle frames.
    isMoving = dist > 1.f;

    // If close enough, "snap" to target to avoid overshoot/jitter.
    if (dist < speed)
    {
        // Set exact position to target.
        position = target;

        // Move to next patrol point (wrap around).
        patrolIndex = (patrolIndex + 1) % patrolPoints.size();

        // Update direction toward the next segment.
        // Normalize ensures direction length = 1 for stable math.
        direction = normalize(patrolPoints[patrolIndex] - position);
    }
    else
    {
        // Move toward the target.
        direction = normalize(toTarget);          // unit direction to target
        position += direction * speed;            // step forward by speed
    }

    // Determine sprite-sheet row based on direction (4-way facing).
    float dx = direction.x;
    float dy = direction.y;

    // If horizontal component dominates, use left/right rows.
    if (std::abs(dx) > std::abs(dy))
    {
        row = (dx > 0) ? 4 : 2;   // right : left
    }
    else
    {
        row = (dy < 0) ? 1 : 3;   // up : down (note: SFML Y axis usually goes downwards)
    }
}

std::unique_ptr<Enemy> GenericEnemy::clone() const
{
    // Prototype pattern: returns a deep copy of *this
    // unique_ptr => caller becomes the unique owner
    return std::make_unique<GenericEnemy>(*this);
}

// ============================================================
// GenericEnemy::detectPlayer  (CONE OF VISION EXPLAINED)
// ============================================================
void GenericEnemy::detectPlayer(const sf::RectangleShape& joueur)
{
    // --------------------------------------------------------
    // STEP 1: Get player's center (we detect the center point).
    // --------------------------------------------------------

    // Get player's global bounding rectangle (position + size in world coords).
    sf::FloatRect joueurBounds = joueur.getGlobalBounds();

    // Compute center of that rectangle:
    // centerX = left + width/2, centerY = top + height/2
    sf::Vector2f joueurCenter(
        joueurBounds.left + joueurBounds.width * 0.5f,
        joueurBounds.top  + joueurBounds.height * 0.5f
    );

    // --------------------------------------------------------
    // STEP 2: Build "cone parameters" (range and angle).
    // --------------------------------------------------------

    // Range: maximum distance the enemy can see.
    float range = visionRange;

    // Angle (degrees): total opening angle of the cone.
    float angleDeg = visionAngle;

    // --------------------------------------------------------
    // STEP 3: Convert cone angle into a dot-product threshold.
    // --------------------------------------------------------
    //
    // Key idea:
    // - For two normalized vectors A and B, dot(A,B) = cos(theta)
    //   where theta is the angle between them.
    //
    // We want: theta <= (FOV / 2)
    // => cos(theta) >= cos(FOV/2)
    //
    // So instead of computing theta with acos (slow),
    // we compare dot >= cos(FOV/2) (fast).

    float pi = 3.14159265f;

    // Half of the field-of-view in radians.
    // Convert degrees to radians: rad = deg * pi / 180.
    float halfRad = (angleDeg * 0.5f) * pi / 180.f;

    // This is our threshold:
    // if dot >= cosHalfFov => within cone angle
    float cosHalfFov = std::cos(halfRad);

    // --------------------------------------------------------
    // STEP 4: Compute vector from enemy to player + distance.
    // --------------------------------------------------------

    // Vector pointing from enemy position to player's center.
    sf::Vector2f toPlayer = joueurCenter - position;

    // Distance to player (length of toPlayer).
    float dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

    // Edge case: if extremely close, auto-detect to avoid division by 0.
    if (dist < 1.f)
    {
        joueurDetecte = true;
        return;
    }

    // --------------------------------------------------------
    // STEP 5: Normalize "toPlayer" so we can use dot product properly.
    // --------------------------------------------------------

    // Unit vector pointing to player.
    // We divide by dist so length becomes 1.
    sf::Vector2f toPlayerNorm = { toPlayer.x / dist, toPlayer.y / dist };

    // --------------------------------------------------------
    // STEP 6: Dot product between enemy forward direction and direction to player.
    // --------------------------------------------------------
    //
    // direction (enemy forward) should be normalized (unit).
    // toPlayerNorm is unit.
    //
    // dot = cos(theta)
    // - dot = 1   => player exactly in front
    // - dot = 0   => player at 90 degrees (side)
    // - dot < 0   => player behind

    float dot = direction.x * toPlayerNorm.x + direction.y * toPlayerNorm.y;

    // --------------------------------------------------------
    // STEP 7: Final cone test = distance AND angle.
    // --------------------------------------------------------
    //
    // Player is detected if:
    //  1) within distance range
    //  2) within the cone opening angle (dot threshold)

    joueurDetecte = (dist <= range && dot >= cosHalfFov);
}

// ============================================================
// CameraEnemy::detectPlayer  (same cone math)
// ============================================================
void CameraEnemy::detectPlayer(const sf::RectangleShape& joueur)
{
    // Same steps as GenericEnemy: range + dot threshold.
    // Camera does not move, but it still has position and direction.

    sf::FloatRect joueurBounds = joueur.getGlobalBounds();

    sf::Vector2f joueurCenter(
        joueurBounds.left + joueurBounds.width * 0.5f,
        joueurBounds.top  + joueurBounds.height * 0.5f
    );

    float range = visionRange;
    float angleDeg = visionAngle;

    float pi = 3.14159265f;
    float halfRad = (angleDeg * 0.5f) * pi / 180.f;
    float cosHalfFov = std::cos(halfRad);

    sf::Vector2f toPlayer = joueurCenter - position;

    float dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

    if (dist < 1.f)
    {
        joueurDetecte = true;
        return;
    }

    sf::Vector2f toPlayerNorm = { toPlayer.x / dist, toPlayer.y / dist };

    float dot = direction.x * toPlayerNorm.x + direction.y * toPlayerNorm.y;

    joueurDetecte = (dist <= range && dot >= cosHalfFov);
}

std::unique_ptr<Enemy> CameraEnemy::clone() const
{
    // Prototype clone
    return std::make_unique<CameraEnemy>(*this);
}

// ============================================================
// LaserEnemy::detectPlayer  (beam / strip detection)
// ============================================================
void LaserEnemy::detectPlayer(const sf::RectangleShape& joueur)
{
    // Compute player's center.
    sf::FloatRect joueurBounds = joueur.getGlobalBounds();

    sf::Vector2f joueurCenter(
        joueurBounds.left + joueurBounds.width * 0.5f,
        joueurBounds.top  + joueurBounds.height * 0.5f
    );

    // Vector from enemy to player.
    sf::Vector2f toPlayer = joueurCenter - position;

    // proj = projection of toPlayer on the forward direction.
    // If proj > 0 => player is in front of the laser.
    // If proj is large => player is far along the beam.
    float proj = toPlayer.x * direction.x + toPlayer.y * direction.y;

    // side = signed perpendicular distance to the beam axis.
    // We use a perpendicular vector (-dy, dx).
    // If |side| is small => player is close to beam line.
    float side = toPlayer.x * (-direction.y) + toPlayer.y * direction.x;

    // Detect if:
    // - player is in front of enemy (proj > 0)
    // - within laser length (proj < laserLength)
    // - within strip thickness (|side| < 20)
    joueurDetecte = (proj > 0 && proj < laserLength && std::abs(side) < 20.f);
}

std::unique_ptr<Enemy> LaserEnemy::clone() const
{
    // Prototype clone
    return std::make_unique<LaserEnemy>(*this);
}

} // namespace Modele

// ============================================================
// Factory implementation
// ============================================================
namespace Modele {

std::unique_ptr<Enemy> createEnemyFromDefinition(
    const EnemyDefinition& ed,
    const std::map<std::string, std::unique_ptr<Enemy>>& prototypes,
    float scaleW,
    float scaleH
)
{
    // ----------------------------
    // 1) Scale patrol points
    // ----------------------------
    std::vector<sf::Vector2f> patrol;

    // Each point from definition is scaled to match current world/viewport.
    for (const auto& pt : ed.patrolPoints)
    {
        patrol.push_back(sf::Vector2f(pt.x * scaleW, pt.y * scaleH));
    }

    // ----------------------------
    // 2) Scale position
    // ----------------------------
    sf::Vector2f pos(ed.position.x * scaleW, ed.position.y * scaleH);

    // ----------------------------
    // 3) Scale detection distances
    // ----------------------------
    // Use geometric mean of scales as a compromise factor for distances.
    float scaledVisionRange = ed.visionRange * std::sqrt(scaleW * scaleH);
    float scaledLaserLength = ed.laserLength * std::sqrt(scaleW * scaleH);

    // ----------------------------
    // 4) Determine enemy type
    // ----------------------------
    std::string type = ed.isLaser ? "laser" : (ed.isCamera ? "camera" : "generic");

    // ----------------------------
    // 5) Clone from prototypes
    // ----------------------------
    auto it = prototypes.find(type);
    std::unique_ptr<Enemy> e;

    if (it != prototypes.end())
    {
        // Clone the prototype (Prototype pattern).
        e = it->second->clone();
    }
    else
    {
        // Fallback: try generic prototype, else construct a default GenericEnemy.
        auto itg = prototypes.find("generic");
        if (itg != prototypes.end()) e = itg->second->clone();
        else e = std::make_unique<GenericEnemy>();
    }

    // ----------------------------
    // 6) Apply shared fields
    // ----------------------------
    e->position = pos;
    e->textureName = ed.textureName;

    // ----------------------------
    // 7) Load texture and setup sprite
    // ----------------------------
    if (type == "camera")
    {
        // Try multiple paths to support different run directories.
        const std::vector<std::string> tryPathsCam = {
            "cmake-build-debug/Asset/camera/" + e->textureName + ".png",
            "Asset/camera/" + e->textureName + ".png",
            "camera/" + e->textureName + ".png",
            e->textureName + ".png"
        };

        bool camLoaded = false;

        for (const auto& p : tryPathsCam)
        {
            if (e->texture.loadFromFile(p))
            {
                e->sprite.setTexture(e->texture);
                camLoaded = true;
                break;
            }
        }

        if (camLoaded)
        {
            // Cameras are static: disable sprite-sheet animation.
            e->frameCount = 1;
            e->idleFrameCount = 1;
            e->frameIndex = 0;
            e->isMoving = false;

            sf::Vector2u ts = e->texture.getSize();

            if (ts.x > 0 && ts.y > 0)
            {
                e->sprite.setTextureRect(sf::IntRect(0, 0, (int)ts.x, (int)ts.y));
                e->sprite.setOrigin((float)ts.x / 2.f, (float)ts.y / 2.f);
                e->sprite.setScale(1.f, 1.f);

                // Keep tileSize aligned with actual texture size.
                e->tileSize = (int)std::min(ts.x, ts.y);
            }
            else
            {
                // Defensive fallback.
                e->sprite.setOrigin(e->tileSize / 2.f, e->tileSize / 2.f);
                e->sprite.setScale(2.5f, 2.5f);
            }

            // NOTE:
            // At this moment, CameraEnemy::facing may not yet be assigned (it happens later below).
            // Rotation based on cc->facing might therefore default to 0.
            // A robust approach is: rotate using ed.facing here, or move rotation after setting facing.
            CameraEnemy* cc = dynamic_cast<CameraEnemy*>(e.get());
            if (cc)
            {
                if (cc->facing == "left")      e->sprite.setRotation(180.f);
                else if (cc->facing == "up")   e->sprite.setRotation(-90.f);
                else if (cc->facing == "down") e->sprite.setRotation(90.f);
                else                           e->sprite.setRotation(0.f);
            }
        }
    }
    else
    {
        // Human enemies (generic/laser) use sprite-sheets in Human folder.
        const std::vector<std::string> tryPathsTex = {
            "cmake-build-debug/Asset/Human/" + e->textureName + ".png",
            "Asset/Human/" + e->textureName + ".png",
            "Human/" + e->textureName + ".png",
            e->textureName + ".png"
        };

        for (const auto& p : tryPathsTex)
        {
            if (e->texture.loadFromFile(p))
            {
                e->sprite.setTexture(e->texture);
                break;
            }
        }

        // Center origin makes positioning/rotation easier.
        e->sprite.setOrigin(e->tileSize / 2.f, e->tileSize / 2.f);
        e->sprite.setScale(2.5f, 2.5f);
    }

    // ----------------------------
    // 8) Type-specific setup
    // ----------------------------

    if (type == "generic")
    {
        // Access derived fields with RTTI (dynamic_cast).
        GenericEnemy* g = dynamic_cast<GenericEnemy*>(e.get());
        if (g)
        {
            g->patrolPoints = patrol;
            g->speed = ed.speed;

            // Initialize direction toward first patrol point.
            if (!g->patrolPoints.empty())
                g->direction = Enemy::normalize(g->patrolPoints[0] - g->position);
        }

        e->isCamera = false;
        e->isLaser  = false;

        e->visionRange = scaledVisionRange;
        e->visionAngle = ed.visionAngle;
    }
    else if (type == "camera")
    {
        CameraEnemy* c = dynamic_cast<CameraEnemy*>(e.get());
        if (c)
        {
            c->facing = ed.facing;

            // Convert facing to a direction vector (must be normalized).
            if (c->facing == "left")       c->direction = {-1.f, 0.f};
            else if (c->facing == "right") c->direction = { 1.f, 0.f};
            else if (c->facing == "up")    c->direction = { 0.f,-1.f};
            else                           c->direction = { 0.f, 1.f}; // down/default
        }

        e->isCamera = true;
        e->isLaser  = false;

        e->visionRange = scaledVisionRange;
        e->visionAngle = ed.visionAngle;
    }
    else if (type == "laser")
    {
        LaserEnemy* l = dynamic_cast<LaserEnemy*>(e.get());
        if (l)
        {
            l->facing = ed.facing;

            if (l->facing == "left")       l->direction = {-1.f, 0.f};
            else if (l->facing == "right") l->direction = { 1.f, 0.f};
            else if (l->facing == "up")    l->direction = { 0.f,-1.f};
            else                           l->direction = { 0.f, 1.f};
        }

        e->isLaser  = true;
        e->isCamera = false;

        e->laserLength = scaledLaserLength;
    }

    // Return ownership to caller.
    return e;
}

} // namespace Modele
