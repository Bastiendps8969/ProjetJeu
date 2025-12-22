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
        details.timeScore = 500 * secondsRemaining;
        details.primaryScore = 10000 * details.primaryObjectivesCompleted;
        details.secondaryScore = 5000 * details.secondaryObjectivesCompleted;
        details.detectionMalus = -2000 * details.numberOfDetections;

        // Calculer le score total (inclut la pénalité de détection)
        details.totalScore = details.timeScore + details.primaryScore + details.secondaryScore + details.detectionMalus;
        
        return details;
    }

    bool ScoreCalculator::areAllSecondaryObjectivesCompleted(
        const std::vector<Objective>& objectives
    )
    {
        // Vérifier s'il y a au moins un objectif secondaire
        bool hasSecondaryObjective = false;
        
        for (const auto& obj : objectives)
        {
            if (!obj.isPrimary())
            {
                hasSecondaryObjective = true;
                if (!obj.isAccomplished())
                {
                    return false; // Au moins un objectif secondaire non complété
                }
            }
        }
        
        // Si pas d'objectif secondaire, on considère que c'est complété
        return true;
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
