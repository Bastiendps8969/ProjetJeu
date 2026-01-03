
#include "ScoreCalculator.h"

namespace Modele
{
    ScoreDetails ScoreCalculator::calculateScore(
        const std::vector<Objective>& objectives,
        int secondsRemaining,
        int numberOfDetections
    )
    {
        // Create and populate the result structure.
        ScoreDetails details;

        // Store raw inputs for reporting/debug/UI usage.
        details.secondsRemaining = secondsRemaining;
        details.numberOfDetections = numberOfDetections;

        // Count completed objectives by type.
        details.primaryObjectivesCompleted = countCompletedPrimaryObjectives(objectives);
        details.secondaryObjectivesCompleted = countCompletedSecondaryObjectives(objectives);

        // Compute each score component.
        // Time reward: more remaining time => higher score.
        details.timeScore = 50 * secondsRemaining;

        // Main mission reward.
        details.primaryScore = 10000 * details.primaryObjectivesCompleted;

        // Optional/bonus reward.
        details.secondaryScore = 5000 * details.secondaryObjectivesCompleted;

        // Detection penalty: each detection reduces the score.
        details.detectionMalus = -2000 * details.numberOfDetections;

        // Compute total score (malus is negative, so it can be added).
        details.totalScore =
            details.timeScore +
            details.primaryScore +
            details.secondaryScore +
            details.detectionMalus;

        return details;
    }

    int ScoreCalculator::countCompletedPrimaryObjectives(
        const std::vector<Objective>& objectives
    )
    {
        int count = 0;

        // Count objectives that are both primary AND accomplished.
        for (const auto& obj : objectives)
        {
            if (obj.isPrimary() && obj.isAccomplished())
            {
                count++;
            }
        }

        return count;
    }

    int ScoreCalculator::countCompletedSecondaryObjectives(
        const std::vector<Objective>& objectives
    )
    {
        int count = 0;

        // Secondary objectives are those that are NOT primary.
        // Count those that are accomplished.
        for (const auto& obj : objectives)
        {
            if (!obj.isPrimary() && obj.isAccomplished())
            {
                count++;
            }
        }

        return count;
    }

    bool ScoreCalculator::areAllPrimaryObjectivesCompleted(
        const std::vector<Objective>& objectives
    )
    {
        // Track whether the level actually contains any primary objectives.
        bool hasPrimary = false;

        // If any primary objective is not accomplished, return false immediately.
        for (const auto& obj : objectives)
        {
            if (obj.isPrimary())
            {
                hasPrimary = true;
                if (!obj.isAccomplished())
                    return false;
            }
        }

        // If no primary objectives are present, this implementation treats the condition as satisfied.
        (void)hasPrimary; // kept for clarity; indicates an intentional design choice
        return true;
    }
}
