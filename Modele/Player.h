
#ifndef TESTCOLLISION_PLAYER_H
#define TESTCOLLISION_PLAYER_H

#include <string>

// Player is a lightweight data class that stores:
// - a player name
// - an array of mission scores (best score per mission)
//
// Design notes:
// - Scores are stored as a fixed-size array of 12 missions.
// - setPlayerScore implements a "high score" policy: only improves a mission score.
// - This class does not manage resources, so a custom destructor is not required.

class Player {
private:
    std::string playerName;

    // Mission score slots:
    // Index 0: tutorial
    // Index 1: test mission
    // Index 2..11: main missions (total 12 entries)
    int playerScore[12]{};

public:
    Player();
    Player(const std::string& name);

    // Rule of 0: no manual cleanup needed (std::string cleans itself).
    ~Player() = default;

    // Sets player name if non-empty.
    void setPlayerName(const std::string& name);

    // Updates the score for the given mission if the new score is higher.
    void setPlayerScore(int missionIndex, int newScore);

    // Returns the current player name.
    // Note: could be const std::string& to avoid a copy, but kept as in original style.
    std::string getPlayerName();

    // Returns the score for a specific mission.
    int getPlayerScore(int missionIndex) const;

    // Returns the sum of all mission scores.
    int getPlayerTotalScore() const;
};

#endif // TESTCOLLISION_PLAYER_H
