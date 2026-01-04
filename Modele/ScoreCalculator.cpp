
#include "ScoreCalculator.h"

namespace Modele
{
    ScoreDetails ScoreCalculator::calculateScore(
        const std::vector<Objective>& objectives, // const ref: avoid copying a potentially large container
        int secondsRemaining,                     // by value: primitive type, trivial/cheap copy
        int numberOfDetections                    // by value: primitive type, trivial/cheap copy
    )
    {
        // details is a local "aggregated result" object.
        // WHY local value object:
        // - clear lifetime (stack)
        // - easy to fill step-by-step
        // - returned by value as a complete computed result
        ScoreDetails details;

        // Store raw inputs into the aggregated result.
        // WHY: keeping inputs in the result helps reporting/debugging/UI without needing extra parameters later.
        details.secondsRemaining = secondsRemaining;
        details.numberOfDetections = numberOfDetections;

        // These helper functions also take the container by const reference.
        // WHY: consistent API, avoids copies across multiple computations.
        details.primaryObjectivesCompleted = countCompletedPrimaryObjectives(objectives);
        details.secondaryObjectivesCompleted = countCompletedSecondaryObjectives(objectives);

        // Compute each component.
        // WHY: keeping separate components makes scoring rules transparent and easy to tune later.
        details.timeScore = 50 * secondsRemaining;
        details.primaryScore = 10000 * details.primaryObjectivesCompleted;
        details.secondaryScore = 5000 * details.secondaryObjectivesCompleted;

        // Malus stored as a negative number.
        // WHY: it allows totalScore to be computed by a simple sum of all components.
        details.detectionMalus = -2000 * details.numberOfDetections;

        // Aggregate final score.
        details.totalScore =
            details.timeScore +
            details.primaryScore +
            details.secondaryScore +
            details.detectionMalus;

        // Return the aggregated breakdown by value.
        // WHY: the caller gets a complete snapshot of scoring components in one object.
        return details;
    }

    int ScoreCalculator::countCompletedPrimaryObjectives(
        const std::vector<Objective>& objectives // const ref: read-only + avoid container copy
    )
    {
        int count = 0;

        // Range-based loop with const auto&
        // WHY const reference here:
        // - avoids copying each Objective (could be heavier than an int)
        // - ensures we don't modify objectives while iterating
        for (const auto& obj : objectives)
        {
            // Using Objective's public interface (isPrimary / isAccomplished)
            // WHY: encapsulation — ScoreCalculator depends on Objective behavior, not its internal fields.
            if (obj.isPrimary() && obj.isAccomplished())
            {
                count++;
            }
        }

        return count; // return by value: int is cheap and clear
    }

    int ScoreCalculator::countCompletedSecondaryObjectives(
        const std::vector<Objective>& objectives // const ref: read-only + avoid container copy
    )
    {
        int count = 0;

        // const auto& for the same reasons: no per-item copy + read-only traversal.
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
        const std::vector<Objective>& objectives // const ref: read-only traversal, no copy
    )
    {
        // const auto& avoids copying each Objective.
        for (const auto& obj : objectives)
        {
            if (obj.isPrimary())
            {
                // Early return pattern
                // WHY: as soon as one required objective fails, the answer is known (faster on average).
                if (!obj.isAccomplished())
                    return false;
            }
        }

        return true; // return by value: bool is cheap and clear
    }
}
