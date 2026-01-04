
#pragma once

#include <vector>
#include "Objective.h"

namespace Modele
{
    // ScoreDetails aggregates multiple score components into a single returned object.
    // WHY: returning a structured breakdown is more useful than returning only one integer:
    // - UI can display each component (time, objectives, malus)
    // - debugging/telemetry becomes straightforward
    // - future extensions (new components) don't break the function signature as often
    struct ScoreDetails
    {
        int secondsRemaining = 0;
        int primaryObjectivesCompleted = 0;
        int secondaryObjectivesCompleted = 0;
        int numberOfDetections = 0;

        int timeScore = 0;
        int primaryScore = 0;
        int secondaryScore = 0;
        int detectionMalus = 0;
        int totalScore = 0;
    };

    class ScoreCalculator
    {
    public:
        /*
            Passing semantics rationale (WHY these choices):

            - objectives: const std::vector<Objective>&
              WHY:
              1) Efficiency: a std::vector can be large; passing by const reference avoids copying
                 the whole container.
              2) Intent: const clearly states "this function will not modify the caller's data".
              3) Ownership: passing by reference expresses "the caller keeps ownership / lifetime".

            - secondsRemaining, numberOfDetections: passed by value (int)
              WHY:
              1) ints are tiny and cheap to copy; passing by value is simple and efficient.
              2) value semantics make it explicit that the function works on a local copy of the number
                 (no aliasing concerns).

            - return: ScoreDetails by value
              WHY:
              1) Value-return is natural for a computed result object.
        */
        static ScoreDetails calculateScore(
            const std::vector<Objective>& objectives,
            int secondsRemaining,
            int numberOfDetections
        );

        // objectives passed by const reference for the same reasons:
        // avoid copying, keep read-only contract, caller retains ownership.
        static int countCompletedPrimaryObjectives(
            const std::vector<Objective>& objectives
        );

        // objectives passed by const reference: efficient + explicit "read-only" API.
        static int countCompletedSecondaryObjectives(
            const std::vector<Objective>& objectives
        );

        // objectives passed by const reference: efficient traversal, no modification allowed.
        static bool areAllPrimaryObjectivesCompleted(
            const std::vector<Objective>& objectives
        );
    };
}
