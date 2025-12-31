#pragma once

#include <vector>
#include "Objective.h"

namespace Modele
{
    struct ScoreDetails
    {
        int secondsRemaining = 0;
        int primaryObjectivesCompleted = 0;
        int secondaryObjectivesCompleted = 0;
        int numberOfDetections = 0;
        
        int timeScore = 0;      // 500 * secondsRemaining
        int primaryScore = 0;   // 10000 * primaryObjectivesCompleted
        int secondaryScore = 0; // 5000 * secondaryObjectivesCompleted
        int detectionMalus = 0;
        int totalScore = 0;     // sum of all
    };

    class ScoreCalculator
    {
    public:
        // Calcule les détails du score basé sur les objectifs et le temps restant
        static ScoreDetails calculateScore(
            const std::vector<Objective>& objectives,
            int secondsRemaining,
            int numberOfDetections
        );

        // Compte les objectifs primaires complétés
        static int countCompletedPrimaryObjectives(
            const std::vector<Objective>& objectives
        );

        // Compte les objectifs secondaires complétés
        static int countCompletedSecondaryObjectives(
            const std::vector<Objective>& objectives
        );
        // Vérifie si tous les objectifs primaires sont complétés
        static bool areAllPrimaryObjectivesCompleted(
            const std::vector<Objective>& objectives
        );
    };
}
