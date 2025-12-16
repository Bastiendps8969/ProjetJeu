#pragma once

#include <string>
#include <vector>

namespace Modele
{
    // Représente un niveau
    struct LevelInfo {
        std::string id;
        std::string name;
        std::string description;
        std::string picture;       // chemin vers l'image du niveau
        int neededScore;
        std::string levelData;     // chemin vers rooms.json du niveau
    };

    // Représente un chapitre (histoire)
    struct ChapterInfo {
        std::string id;
        std::string name;
        std::string description;
        std::string picture;       // chemin vers l'image du chapitre
        std::string levelsFile;    // chemin vers le fichier JSON des niveaux
        std::vector<LevelInfo> levels; // levels chargés à la demande
    };

    // Utilitaire pour charger les chapitres et niveaux depuis JSON
    class ChapterLoader
    {
    public:
        // Charge la liste de tous les chapitres depuis chapters.json
        static std::vector<ChapterInfo> loadChapters(const std::string& chaptersJsonPath);

        // Charge les niveaux d'un chapitre spécifique
        static std::vector<LevelInfo> loadLevels(const std::string& levelsJsonPath);

        // Charge un chapitre avec ses niveaux
        static ChapterInfo loadChapterWithLevels(const std::string& chaptersJsonPath, int chapterIndex);
    };
}