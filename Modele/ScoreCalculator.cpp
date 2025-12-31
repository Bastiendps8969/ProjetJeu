#include "ScoreCalculator.h"

namespace Modele
{
    ScoreDetails ScoreCalculator::calculateScore(
        const std::vector<Objective>& objectives,
        int secondsRemaining,
        int numberOfDetections
    )
    {
        ScoreDetails details;
        details.secondsRemaining = secondsRemaining;
        details.numberOfDetections = numberOfDetections;
        
        // Compter les objectifs complétés
        details.primaryObjectivesCompleted = countCompletedPrimaryObjectives(objectives);
        details.secondaryObjectivesCompleted = countCompletedSecondaryObjectives(objectives);
        
        // Calculer chaque composante du score
        details.timeScore = 50 * secondsRemaining;
        details.primaryScore = 10000 * details.primaryObjectivesCompleted;
        details.secondaryScore = 5000 * details.secondaryObjectivesCompleted;
        details.detectionMalus = -2000 * details.numberOfDetections;

        // Calculer le score total (inclut la pénalité de détection)
        details.totalScore = details.timeScore + details.primaryScore + details.secondaryScore + details.detectionMalus;
        
        return details;
    }


    int ScoreCalculator::countCompletedPrimaryObjectives(
        const std::vector<Objective>& objectives
    )
    {
        int count = 0;
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
        bool hasPrimary = false;
        for (const auto& obj : objectives)
        {
            if (obj.isPrimary()) {
                hasPrimary = true;
                if (!obj.isAccomplished()) return false;
            }
        }
        // If no primary objectives present, treat as completed
        return true;
    }
}
