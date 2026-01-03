
#pragma once
//
// LevelPage.h
//
#ifndef TESTCOLLISION_LEVELPAGE_H
#define TESTCOLLISION_LEVELPAGE_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include "../Modele/ChapterLoader.h"

namespace Vue
{
    /**
     * LevelPage: chapter and level selection screen.
     * - run() opens a fullscreen window and returns the user's selection (blocking / modal).
     * - Displays a vertical chapter list, then a horizontal level carousel
     *   with a right-side preview panel (title, description, needed score, image).
     */
    class LevelPage
    {
    public:
        // Returned selection object for the caller (controller/menu).
        struct Selection {
            int chapterIndex = -1;
            int levelIndex = -1;
            bool valid = false;
            std::string levelData; // path to level JSON (rooms) if available
        };

        LevelPage() = default;

        // Optional callback returning current player scores (used to determine level locks).
        LevelPage(std::function<std::vector<int>()> getScoresCb);

        // Update/replace the optional callback.
        void setGetScoresCb(std::function<std::vector<int>()> cb);

        // Opens a fullscreen window and returns the selection (blocking).
        Selection run();

    private:
        // Internal rendering helpers.
        // NOTE: drawFrame() is declared here but not defined in the provided .cpp (may be legacy).
        void drawFrame(sf::RenderWindow& window, const std::vector<Modele::ChapterInfo>& chapters,
                       int chapterIdx, int levelIdx, const sf::Sprite& backgroundSprite, bool bgLoaded);

        // Draw chapter selection screen (vertical list).
        void drawChapterSelection(sf::RenderWindow& window,
                                  const std::vector<Modele::ChapterInfo>& chapters,
                                  int chapterIdx,
                                  const sf::Sprite& backgroundSprite, bool bgLoaded);

        // Draw level selection screen (carousel + right preview panel).
        void drawLevelSelection(sf::RenderWindow& window,
                                const std::vector<Modele::ChapterInfo>& chapters,
                                int chapterIdx, int levelIdx,
                                const sf::Sprite& backgroundSprite, bool bgLoaded);

        // Optional callback provided by caller to query current player scores.
        // Used to decide whether a level is unlocked based on neededScore.
        std::function<std::vector<int>()> getScoresCb;
    };
}

#endif //TESTCOLLISION_LEVELPAGE_H
