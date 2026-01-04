
#include "ChapterLoader.h"
#include "../cmake-build-debug/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace Modele
{
    // Safely read a string field from a JSON object.
    // If the key is missing or not a string, return defaultValue.
    static std::string safeGetString(const json& j,
                                    const std::string& key,
                                    const std::string& defaultValue = "")
    {
        // WHY const json& and const std::string&:
        // - avoids copying JSON objects and keys
        // - function is read-only and purely extracts data
        if (j.contains(key) && j[key].is_string())
        {
            return j[key].get<std::string>();
        }
        return defaultValue; // returned by value: new string result (safe)
    }

    // Safely read an integer field from a JSON object.
    // Accepts either:
    // - a JSON integer
    // - or a string that can be converted to int via std::stoi
    // Otherwise returns defaultValue.
    static int safeGetInt(const json& j,
                          const std::string& key,
                          int defaultValue = 0)
    {
        // WHY defaultValue by value:
        // - int is trivial, and we return a value result anyway
        if (j.contains(key))
        {
            if (j[key].is_number_integer())
            {
                return j[key].get<int>();
            }
            else if (j[key].is_string())
            {
                try
                {
                    return std::stoi(j[key].get<std::string>());
                }
                catch (...)
                {
                    return defaultValue;
                }
            }
        }
        return defaultValue;
    }

    std::vector<ChapterInfo> ChapterLoader::loadChapters(const std::string& chaptersJsonPath)
    {
        std::vector<ChapterInfo> chapters;

        // Open the chapters JSON file.
        std::ifstream ifs(chaptersJsonPath);

        if (!ifs.is_open())
        {
            std::cerr << "Error: Could not open " << chaptersJsonPath << std::endl;
            return chapters; // return empty vector by value
        }

        try
        {
            json j;
            ifs >> j;

            // Iterate over all chapter entries.
            // The file format appears to be an object with numeric string keys: "0", "1", "2", ...
            for (auto it = j.begin(); it != j.end(); ++it)
            {
                const auto& obj = it.value(); // const ref avoids copying sub-json object

                ChapterInfo chapter;

                // Use the JSON key as chapter id.
                chapter.id = it.key();

                // Extract fields safely.
                chapter.name = safeGetString(obj, "name");
                chapter.description = safeGetString(obj, "description");
                chapter.picture = safeGetString(obj, "picture");

                // "levels" is expected to contain the path to the levels JSON file for this chapter.
                chapter.levelsFile = safeGetString(obj, "levels");

                // Note: chapter.levels is intentionally left empty here (lazy loading).

                // WHY push_back(std::move(chapter)):
                // - ChapterInfo contains multiple std::string and a vector
                // - moving avoids unnecessary copies when inserting into chapters
                chapters.push_back(std::move(chapter));
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error parsing " << chaptersJsonPath << ": " << e.what() << std::endl;
        }

        // WHY return vector by value:
        // - modern C++ optimizes this via RVO/move (efficient)
        return chapters;
    }

    std::vector<LevelInfo> ChapterLoader::loadLevels(const std::string& levelsJsonPath)
    {
        std::vector<LevelInfo> levels;

        // Open the levels JSON file.
        std::ifstream ifs(levelsJsonPath);

        if (!ifs.is_open())
        {
            std::cerr << "Error: Could not open " << levelsJsonPath << std::endl;
            return levels;
        }

        try
        {
            json j;
            ifs >> j;

            // Iterate over all level entries.
            // Similar format: object with numeric string keys.
            for (auto it = j.begin(); it != j.end(); ++it)
            {
                const auto& obj = it.value();

                LevelInfo level;

                // Use the JSON key as level id.
                level.id = it.key();

                // Extract fields safely.
                level.name = safeGetString(obj, "name");
                level.description = safeGetString(obj, "description");
                level.picture = safeGetString(obj, "picture");

                // neededScore is the unlock requirement.
                level.neededScore = safeGetInt(obj, "neededScore");

                // levelData points to the level's main data file (e.g., rooms JSON).
                level.levelData = safeGetString(obj, "levelData");

                // WHY push_back(std::move(level)):
                // - LevelInfo owns strings; moving avoids repeated allocations/copies
                levels.push_back(std::move(level));
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error parsing " << levelsJsonPath << ": " << e.what() << std::endl;
        }

        return levels;
    }

    ChapterInfo ChapterLoader::loadChapterWithLevels(const std::string& chaptersJsonPath, int chapterIndex)
    {
        // Load chapter list first.
        auto chapters = loadChapters(chaptersJsonPath);

        // Validate chapterIndex and load levels for the selected chapter.
        if (chapterIndex >= 0 && chapterIndex < static_cast<int>(chapters.size()))
        {
            ChapterInfo& chapter = chapters[chapterIndex];

            // Load the levels for this chapter using the file path specified in chapters.json.
            chapter.levels = loadLevels(chapter.levelsFile);

            // Return the fully populated chapter (with levels loaded).
            //
            // WHY return by value:
            // - returns a self-contained ChapterInfo object to the caller
            // - modern compilers will move/RVO this efficiently
            return chapter;
        }

        // Invalid index => return empty/default chapter.
        return ChapterInfo{};
    }

} // namespace Modele
