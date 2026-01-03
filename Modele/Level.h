
#ifndef TESTCOLLISION_LEVEL_H
#define TESTCOLLISION_LEVEL_H

#include <string>
#include <algorithm>

// Level is a simple data class representing a game level/session.
// It stores:
// - a name and description (metadata)
// - a countdown value (currently not used by the shown API)
// - a life counter (lives), used to determine game-over state
//
// Design notes:
// - lives is clamped to a minimum of 0 to prevent negative values.

class Level {
private:
    std::string name;         // Level name (e.g., "Tutorial")
    std::string description;  // Level description (e.g., "Test level")

    int countdown;            // Countdown / timer value (present but not exposed in this snippet)

    // Life counter (default 3 at start).
    int lives = 3;

public:
    // Constructs a level with a name and a description.
    // Strings are passed by value to allow move semantics from the caller.
    Level(std::string n, std::string d);

    // Destructor (no special resource management required).
    ~Level();

    // Returns the number of remaining lives.
    int getLives() const { return lives; }

    // Sets lives, clamping to at least 0.
    void setLives(int l) { lives = std::max(0, l); }

    // Decreases lives by the given amount, clamping to at least 0.
    void loseLives(int amount) { lives = std::max(0, lives - amount); }

    // Returns true when the player has no lives left.
    bool isGameOver() const { return lives <= 0; }
};

#endif // TESTCOLLISION_LEVEL_H
