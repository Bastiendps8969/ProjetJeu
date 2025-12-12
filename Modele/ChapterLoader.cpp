#include "ChapterLoader.h"
#include "../cmake-build-debug/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace Modele
{
    static std::string safeGetString(const json& j, const std::string& key, const std::string& defaultValue = "") {
        if (j.contains(key) && j[key].is_string()) {
            return j[key].get<std::string>();
        }
        return defaultValue;
    }

    static int safeGetInt(const json& j, const std::string& key, int defaultValue = 0) {
        if (j.contains(key)) {
            if (j[key].is_number_integer()) {
                return j[key].get<int>();
            } else if (j[key].is_string()) {
                try {
                    return std::stoi(j[key].get<std::string>());
                } catch (...) {
                    return defaultValue;
                }
            }
        }
        return defaultValue;
    }

    std::vector<ChapterInfo> ChapterLoader::loadChapters(const std::string& chaptersJsonPath)
    {
        std::vector<ChapterInfo> chapters;
        
        std::ifstream ifs(chaptersJsonPath);
        if (!ifs.is_open()) {
            std::cerr << "Error: Could not open " << chaptersJsonPath << std::endl;
            return chapters;
        }

        try {
            json j;
            ifs >> j;

            // Itère sur tous les chapitres (clés numériques 0, 1, 2, ...)
            for (auto it = j.begin(); it != j.end(); ++it) {
                const auto& obj = it.value();
                
                ChapterInfo chapter;
                chapter.id = it.key();
                chapter.name = safeGetString(obj, "name");
                chapter.description = safeGetString(obj, "description");
                chapter.picture = safeGetString(obj, "picture");
                chapter.levelsFile = safeGetString(obj, "levels");
                
                chapters.push_back(std::move(chapter));
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing " << chaptersJsonPath << ": " << e.what() << std::endl;
        }

        return chapters;
    }

    std::vector<LevelInfo> ChapterLoader::loadLevels(const std::string& levelsJsonPath)
    {
        std::vector<LevelInfo> levels;

        std::ifstream ifs(levelsJsonPath);
        if (!ifs.is_open()) {
            std::cerr << "Error: Could not open " << levelsJsonPath << std::endl;
            return levels;
        }

        try {
            json j;
            ifs >> j;

            // Itère sur tous les niveaux
            for (auto it = j.begin(); it != j.end(); ++it) {
                const auto& obj = it.value();
                
                LevelInfo level;
                level.id = it.key();
                level.name = safeGetString(obj, "name");
                level.description = safeGetString(obj, "description");
                level.picture = safeGetString(obj, "picture");
                level.neededScore = safeGetInt(obj, "neededScore");
                level.levelData = safeGetString(obj, "levelData");
                
                levels.push_back(std::move(level));
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing " << levelsJsonPath << ": " << e.what() << std::endl;
        }

        return levels;
    }

    ChapterInfo ChapterLoader::loadChapterWithLevels(const std::string& chaptersJsonPath, int chapterIndex)
    {
        auto chapters = loadChapters(chaptersJsonPath);
        if (chapterIndex >= 0 && chapterIndex < static_cast<int>(chapters.size())) {
            ChapterInfo& chapter = chapters[chapterIndex];
            chapter.levels = loadLevels(chapter.levelsFile);
            return chapter;
        }
        return ChapterInfo{};
    }
}