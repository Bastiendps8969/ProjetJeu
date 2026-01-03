
#include "Player.h"

// Helper constant for the number of missions.
// Keeping this in the .cpp avoids exposing it globally in the header.
static constexpr int kMissionCount = 12;

Player::Player()
{
    // Default player name.
    setPlayerName("James Adams");

    // Initialize all mission scores to 0.
    // IMPORTANT: We must iterate over the score array size (12),
    // not over the player name length (which would be incorrect and unsafe).
    for (int i = 0; i < kMissionCount; ++i)
    {
        playerScore[i] = 0;
    }
}

Player::Player(const std::string& name)
{
    // Set provided name (if non-empty).
    setPlayerName(name);

    // Initialize all mission scores to 0.
    for (int i = 0; i < kMissionCount; ++i)
    {
        playerScore[i] = 0;
    }
}

void Player::setPlayerName(const std::string& name)
{
    // Only update if non-empty to prevent accidental blank names.
    if (!name.empty())
    {
        playerName = name;
    }
}

void Player::setPlayerScore(int missionIndex, int newScore)
{
    // Defensive bounds check: missionIndex must be within [0, 11].
    if (missionIndex < 0 || missionIndex >= kMissionCount)
    {
        return; // invalid mission index => ignore
    }

    // High score rule: only keep the best score for each mission.
    if (newScore > playerScore[missionIndex])
    {
        playerScore[missionIndex] = newScore;
    }
}

std::string Player::getPlayerName()
{
    // Return by value (copy). For large strings, returning const reference could be faster.
    return playerName;
}

int Player::getPlayerScore(int missionIndex) const
{
    // Defensive bounds check. If invalid, return 0 as a safe default.
    if (missionIndex < 0 || missionIndex >= kMissionCount)
    {
        return 0;
    }

    return playerScore[missionIndex];
}

int Player::getPlayerTotalScore() const
{
    int totalScore = 0;

    // Sum all mission scores.
    for (int i = 0; i < kMissionCount; ++i)
    {
        totalScore += playerScore[i];
    }

    return totalScore;
}
