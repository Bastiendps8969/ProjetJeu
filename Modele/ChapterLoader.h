
#pragma once

#include <string>
#include <vector>

namespace Modele
{
    // LevelInfo represents a single playable level entry loaded from JSON.
    // It is a pure data container (no runtime gameplay logic).
    struct LevelInfo
    {
        std::string id;           // JSON key (often "0", "1", "2", ...)
        std::string name;         // Display name of the level
        std::string description;  // Short description used in UI
        std::string picture;      // Path to a preview image (UI thumbnail)
        int neededScore;          // Minimum score required to unlock this level
        std::string levelData;    // Path to the level data file (e.g., rooms/level JSON)
    };

    // ChapterInfo represents a chapter (story arc) containing multiple levels.
    // Levels may be loaded on demand (lazy loading) using levelsFile.
    struct ChapterInfo
    {
        std::string id;           // JSON key (often "0", "1", "2", ...)
        std::string name;         // Display name of the chapter
        std::string description;  // Chapter description (UI text)
        std::string picture;      // Path to a chapter preview image
        std::string levelsFile;   // Path to the JSON file describing levels of this chapter
        std::vector<LevelInfo> levels; // Loaded levels (optional/lazy)
    };

    // ChapterLoader is a small utility responsible for loading chapter/level metadata from JSON files.
    // Design notes:
    // - All methods are static: no instance/state required.
    // - Returns vectors by value (RVO/move makes this efficient in modern C++).
    class ChapterLoader
    {
    public:
        // Loads all chapters from a chapters.json-like file.
        static std::vector<ChapterInfo> loadChapters(const std::string& chaptersJsonPath);

        // Loads all levels from a levels.json-like file.
        static std::vector<LevelInfo> loadLevels(const std::string& levelsJsonPath);

        // Loads the chapter at the given index and also loads its levels from chapter.levelsFile.
        // If index is invalid, returns a default-constructed ChapterInfo.
        static ChapterInfo loadChapterWithLevels(const std::string& chaptersJsonPath, int chapterIndex);
    };
}
