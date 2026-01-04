
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
    std::string name;        // Level name (e.g., "Tutorial")
    std::string description; // Level description (e.g., "Test level")
    int countdown;           // Countdown / timer value (present but not exposed in this snippet)

    // Life counter (default 3 at start).
    int lives = 3;

public:
    // Constructs a level with a name and a description.
    // Strings are passed by value to allow move semantics from the caller.
    //
    // WHY pass std::string by value here:
    // - enables efficient move when the caller provides temporaries (e.g., Level("A","B"))
    // - still works with lvalues (caller keeps its own copy; Level stores its own moved/copied data)
    // - implementation can unconditionally std::move into members to avoid extra copies
    Level(std::string n, std::string d);

    // Destructor (no special resource management required).
    //
    // WHY default-like destructor:
    // - Level owns only value-type members (strings and ints); RAII handles cleanup automatically
    ~Level();

    // Returns the number of remaining lives.
    //
    // WHY const + return by value:
    // - int is cheap to copy
    // - const guarantees read-only access
    int getLives() const { return lives; }

    // Sets lives, clamping to at least 0.
    //
    // WHY clamp with std::max(0, l):
    // - prevents invalid negative state
    // - keeps invariant "lives >= 0"
    // WHY int by value:
    // - primitive type; trivial copy
    void setLives(int l) { lives = std::max(0, l); }

    // Decreases lives by the given amount, clamping to at least 0.
    //
    // WHY clamp after subtraction:
    // - ensures lives never goes below 0 even if amount is large
    // - simplifies game-over checks
    void loseLives(int amount) { lives = std::max(0, lives - amount); }

    // Returns true when the player has no lives left.
    //
    // WHY const:
    // - pure query; does not modify object state
    bool isGameOver() const { return lives <= 0; }
};

#endif // TESTCOLLISION_LEVEL_H
