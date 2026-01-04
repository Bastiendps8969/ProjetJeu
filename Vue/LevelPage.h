
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
        // WHY this is a nested type:
        // - it is conceptually "part of" LevelPage API (composition in UML terms)
        // - keeps the selection payload close to the screen that produces it
        struct Selection
        {
            int chapterIndex = -1;
            int levelIndex = -1;
            bool valid = false;
            std::string levelData; // path to level JSON (rooms) if available
        };

        LevelPage() = default;

        // Optional callback returning current player scores (used to determine level locks).
        // WHY passed by value (std::function):
        // - callers can provide lambdas, std::bind, functors easily
        // - passing by value allows move into the member to avoid extra copies
        LevelPage(std::function<std::vector<int>()> getScoresCb);

        // Update/replace the optional callback.
        // WHY pass by value + move again:
        // - same rationale as constructor (flexible call site + efficient storage)
        void setGetScoresCb(std::function<std::vector<int>()> cb);

        // Opens a fullscreen window and returns the selection (blocking).
        // WHY return by value:
        // - Selection is a small POD-like payload
        // - enables NRVO / move semantics, clean ownership
        Selection run();

    private:
        // Internal rendering helpers.

        // NOTE: drawFrame() is declared here but not defined in the provided .cpp (may be legacy).
        // Parameters:
        // - RenderWindow& is non-const reference because drawing mutates the window (clear/draw/display)
        // - chapters passed as const reference to avoid copying potentially large vectors
        // - backgroundSprite passed as const reference to avoid copying SFML sprite data
        void drawFrame(sf::RenderWindow& window, const std::vector<Modele::ChapterInfo>& chapters,
                       int chapterIdx, int levelIdx, const sf::Sprite& backgroundSprite, bool bgLoaded);

        // Draw chapter selection screen (vertical list).
        // WHY const refs:
        // - prevents expensive copies (chapters, sprite)
        // - enforces read-only intent for inputs
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
        //
        // WHY this is an aggregation-like dependency:
        // - the logic (scores retrieval) is owned by the caller (controller/model/etc.)
        // - LevelPage stores a callable "handle" to invoke it later
        // - LevelPage does not own the score system; it only owns the std::function wrapper
        std::function<std::vector<int>()> getScoresCb;
    };
}

#endif // TESTCOLLISION_LEVELPAGE_H
