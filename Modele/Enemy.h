
// Enemy.h
// Enemy hierarchy used by the game.
//
// ===============================
// HIGH-LEVEL DESIGN / RATIONALE
// ===============================
//
// 1) Polymorphism (inheritance):
// - The game can store all enemies as Enemy pointers/references and call
// update(), detectPlayer(), updateAnimation() without knowing the concrete type.
//
// 2) Prototype pattern (clone()):
// - In C++, copying via a base type causes slicing (only the base part gets copied).
// - clone() gives a "virtual copy" returning the correct derived type.
// - This is especially useful for data-driven instantiation: keep prototypes and clone them.
//
// 3) Factory function createEnemyFromDefinition(...):
// - Converts external data (EnemyDefinition) into a fully configured Enemy object.
// - Centralizes scaling, asset loading, facing setup, detection params.
//
// 4) Ownership / memory:
// - unique_ptr expresses unique ownership (one owner; RAII; no delete).
//
// 5) SFML composition:
// - Texture and Sprite are stored by value in each enemy for simple lifetime rules.
// (Sprite references the texture => texture must outlive sprite.)
// - Trade-off: can duplicate textures across enemies (could be optimized with a cache later).

#pragma once
#include <SFML/Graphics.hpp> // SFML types: Vector2f, Texture, Sprite, RectangleShape, etc.
#include <vector> // std::vector for patrol points
#include <string> // std::string for textureName / facing
#include <cmath> // std::sqrt, std::cos
#include <algorithm> // std::max, std::min
#include <map> // std::map for prototypes
#include <memory> // std::unique_ptr
#include "Prototype.h" // Base interface that requires clone()

namespace Modele
{
    // ------------------------------------------------------------
    // Base class: Enemy
    // ------------------------------------------------------------
    class Enemy : public Prototype
    {
    public:
        // -------------------------
        // Spatial state
        // -------------------------
        sf::Vector2f position; // World position of the enemy (typically pixels)

        // Facing direction of the enemy.
        // IMPORTANT: Many math tests assume this is normalized (unit length).
        sf::Vector2f direction = {1.f, 0.f}; // Default facing right

        // Whether the player is currently detected by this enemy.
        bool joueurDetecte = false;

        // -------------------------
        // Rendering resources
        // -------------------------
        std::string textureName; // Name/id of texture (used by factory to load file)

        // Texture stored by value:
        // - Guarantees lifetime is tied to enemy instance.
        // - Sprite references this texture; storing by value avoids dangling references.
        //
        // WHY store sf::Texture by value here:
        // - SFML sprites store a pointer to the texture; tying texture lifetime to Enemy avoids dangling
        // - keeps ownership rules simple (no external texture manager required in this implementation)
        sf::Texture texture;

        // Sprite stored by value. It will display a sub-rectangle (sprite-sheet) or full texture.
        //
        // WHY store sf::Sprite by value:
        // - sprite is lightweight and belongs to the enemy instance (composition)
        // - no allocation / no pointer ownership needed
        sf::Sprite sprite;

        // -------------------------
        // Type flags / detection parameters
        // -------------------------
        // Flags allow fast behavior switches without RTTI in hot paths.
        bool isCamera = false; // Camera uses static image (no sprite-sheet animation)
        bool isLaser = false;  // Laser uses beam detection rather than a cone

        // Cone-of-vision parameters (used by GenericEnemy and CameraEnemy):
        float visionRange = 300.f; // Maximum detection distance
        float visionAngle = 60.f;  // Total cone angle in degrees

        // Laser parameter:
        float laserLength = 600.f; // Length of the detection beam in front of the enemy

        // -------------------------
        // Animation parameters (sprite-sheet)
        // -------------------------
        int frameCount = 9;          // Total number of columns for movement+idle frames
        int frameIndex = 0;          // Current frame index inside the active subset
        int row = 3;                 // Sprite-sheet row depending on direction (up/down/left/right)
        float frameDuration = 0.08f; // Seconds per movement frame
        float idleFrameDuration = 0.24f; // Seconds per idle frame
        int idleFrameCount = 2;      // Number of idle frames (subset of columns)
        bool isMoving = false;       // Set by update() logic; consumed by animation
        int tileSize = 64;           // Width/height of one sprite-sheet tile in pixels
        sf::Clock animClock;         // Measures elapsed time for time-based animation

        Enemy() = default;
        virtual ~Enemy() = default;

        // Prototype interface:
        // - Must return a deep copy as the correct dynamic type.
        // - unique_ptr expresses ownership transfer to caller.
        //
        // WHY return std::unique_ptr<Enemy> by value:
        // - cloning creates a new heap object; unique_ptr expresses single-owner transfer safely (RAII)
        // - avoids manual delete and leaks; move-only semantics prevent accidental sharing
        virtual std::unique_ptr<Enemy> clone() const = 0;

        // Per-frame update (movement/AI). Default does nothing; derived types override.
        virtual void update() {}

        // Update sprite animation by selecting the correct texture rectangle.
        // Cameras are excluded because they are static textures (not sprite-sheets).
        virtual void updateAnimation()
        {
            // Cameras are static: no sprite-sheet animation logic should run.
            if (isCamera) return;

            // If texture not loaded, skip (avoids invalid rect computations).
            if (texture.getSize().x == 0) return;

            // Movement frames = total frames minus idle frames.
            // Example: if frameCount=9 and idleFrameCount=2 => movementFramesCount=7
            int movementFramesCount = std::max(1, frameCount - idleFrameCount);

            // Idle frames count (at least 1).
            int idleCols = std::max(1, idleFrameCount);

            // Time since last frame change.
            float elapsed = animClock.getElapsedTime().asSeconds();

            // Choose frame duration depending on state (moving vs idle).
            float duration = isMoving ? frameDuration : idleFrameDuration;

            // If enough time passed, advance frame index.
            if (elapsed >= duration)
            {
                if (isMoving)
                {
                    // Loop over movement subset.
                    frameIndex = (frameIndex + 1) % movementFramesCount;
                }
                else
                {
                    // Loop over idle subset.
                    frameIndex = (frameIndex + 1) % idleCols;
                }
                // Restart timer after switching frame.
                animClock.restart();
            }

            // Apply the computed sub-rectangle to the sprite.
            sprite.setTextureRect(computeTextureRect());
        }

        // Player detection hook.
        // Passed as const reference:
        // - avoids copying SFML objects
        // - guarantees we do not modify the player shape
        //
        // WHY const sf::RectangleShape&:
        // - RectangleShape can be relatively heavy to copy; ref avoids overhead
        // - detection is a read-only query; const enforces this contract
        virtual void detectPlayer(const sf::RectangleShape& joueur) { (void)joueur; }

        // Compute the sprite-sheet rectangle to display.
        // Convention:
        // - Movement frames live on row (row-1)
        // - Idle frames live on row (row-1 + 4)
        sf::IntRect computeTextureRect() const
        {
            int frameSize = tileSize; // One tile is tileSize x tileSize
            int col = 0; // Column index in the sprite-sheet
            int rowIndex = std::max(0, row - 1); // Convert row (1-based) to 0-based safe index

            // How many columns are for movement animation.
            int movementFramesCount = std::max(1, frameCount - idleFrameCount);

            if (isMoving)
            {
                // When moving, pick among the movement frames subset.
                col = frameIndex % movementFramesCount;
                rowIndex = std::max(0, row - 1);
            }
            else
            {
                // When idle, pick among the idle frames subset.
                int idleCols = std::max(1, idleFrameCount);
                col = frameIndex % idleCols;

                // Idle animation rows are offset by +4 in this sprite-sheet layout.
                rowIndex = std::max(0, row - 1) + 4;
            }

            // Convert (col,rowIndex) tile coordinates to pixel coordinates.
            int frameX = col * frameSize;
            int frameY = rowIndex * frameSize;

            // Return rectangle in pixels for SFML to display.
            return sf::IntRect(frameX, frameY, frameSize, frameSize);
        }

        // Normalize a vector:
        // - We want unit vectors to make dot products and projections meaningful.
        static sf::Vector2f normalize(const sf::Vector2f& v)
        {
            // Compute length (Euclidean norm).
            float len = std::sqrt(v.x * v.x + v.y * v.y);

            // If length is not ~0, return v/len.
            if (len > 0.001f)
                return { v.x / len, v.y / len };

            // Fallback: return a valid direction instead of (0,0).
            return { 1.f, 0.f };
        }
    };

    // ------------------------------------------------------------
    // GenericEnemy: patrolling enemy with cone-of-vision
    // ------------------------------------------------------------
    class GenericEnemy : public Enemy
    {
    public:
        // Patrol route points stored inside the enemy (aggregation):
        // each instance has its own route.
        //
        // WHY store patrolPoints by value:
        // - each enemy instance owns its route data
        // - avoids external lifetime issues (no dangling references)
        std::vector<sf::Vector2f> patrolPoints;

        int patrolIndex = 0;  // Which point is currently targeted
        float speed = 2.0f;   // Movement speed per update (units per frame)

        GenericEnemy() = default;
        GenericEnemy(const GenericEnemy& other) = default;

        // Moves along patrol points and updates direction/animation row.
        void update() override;

        // Prototype clone: deep copy of this GenericEnemy.
        std::unique_ptr<Enemy> clone() const override;

        // Cone-of-vision detection.
        void detectPlayer(const sf::RectangleShape& joueur) override;
    };

    // ------------------------------------------------------------
    // CameraEnemy: stationary cone-of-vision
    // ------------------------------------------------------------
    class CameraEnemy : public Enemy
    {
    public:
        // Facing is stored as a string because level data often uses strings (JSON, etc.).
        // The factory converts it into a direction vector.
        //
        // WHY keep facing as std::string:
        // - preserves the original data-driven value from JSON
        // - allows debugging/logging without re-encoding direction vectors
        std::string facing;

        CameraEnemy() = default;
        CameraEnemy(const CameraEnemy& other) = default;

        // Cone-of-vision detection (same math as GenericEnemy, but camera doesn't move).
        void detectPlayer(const sf::RectangleShape& joueur) override;

        // Prototype clone
        std::unique_ptr<Enemy> clone() const override;
    };

    // ------------------------------------------------------------
    // LaserEnemy: stationary beam detection
    // ------------------------------------------------------------
    class LaserEnemy : public Enemy
    {
    public:
        std::string facing;

        LaserEnemy() = default;
        LaserEnemy(const LaserEnemy& other) = default;

        // Beam detection (projection + lateral distance).
        void detectPlayer(const sf::RectangleShape& joueur) override;

        // Prototype clone
        std::unique_ptr<Enemy> clone() const override;
    };

    // Forward declare to avoid include cycles in the header.
    struct EnemyDefinition;

    // Factory function: builds an Enemy instance from data definition.
    // prototypes passed by const reference:
    // - avoids copy (and unique_ptr is non-copyable)
    // - guarantees factory does not mutate prototypes
    //
    // WHY prototypes as const std::map<..., unique_ptr<Enemy>>&:
    // - map contains move-only unique_ptr, so copying is not allowed anyway
    // - const reference signals "read-only lookup + clone", no mutation of prototypes
    std::unique_ptr<Enemy> createEnemyFromDefinition(
        const EnemyDefinition& ed,
        const std::map<std::string, std::unique_ptr<Enemy>>& prototypes,
        float scaleW,
        float scaleH
    );

} // namespace Modele
