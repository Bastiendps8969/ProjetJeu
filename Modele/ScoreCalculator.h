
#pragma once

#include <vector>
#include "Objective.h"

namespace Modele
{
    // Holds a detailed breakdown of the computed score.
    // Rationale: a structured result is useful for UI display, debugging,
    // and post-level summary screens (rather than returning only one integer).
    struct ScoreDetails
    {
        int secondsRemaining = 0;                 // Remaining time (seconds)
        int primaryObjectivesCompleted = 0;       // Number of completed primary objectives
        int secondaryObjectivesCompleted = 0;     // Number of completed secondary objectives
        int numberOfDetections = 0;               // Number of times the player was detected

        int timeScore = 0;                        // Implementation uses: 50 * secondsRemaining
        int primaryScore = 0;                     // 10000 * primaryObjectivesCompleted
        int secondaryScore = 0;                   // 5000 * secondaryObjectivesCompleted
        int detectionMalus = 0;                   // -2000 * numberOfDetections

        int totalScore = 0;                       // Sum of all components (including malus)
    };

    class ScoreCalculator
    {
    public:
        // Computes the full score breakdown.
        // Parameters:
        // - objectives: all objectives for the level (primary + secondary)
        // - secondsRemaining: remaining time when the level ends
        // - numberOfDetections: number of detections (applies a penalty)
        static ScoreDetails calculateScore(
            const std::vector<Objective>& objectives,
            int secondsRemaining,
            int numberOfDetections
        );

        // Counts how many PRIMARY objectives are accomplished.
        static int countCompletedPrimaryObjectives(
            const std::vector<Objective>& objectives
        );

        // Counts how many SECONDARY objectives are accomplished.
        static int countCompletedSecondaryObjectives(
            const std::vector<Objective>& objectives
        );

        // Returns true if all primary objectives are accomplished.
        // Design choice: if no primary objectives exist, this function returns true.
        static bool areAllPrimaryObjectivesCompleted(
            const std::vector<Objective>& objectives
        );
    };
}
