#pragma once
//
// Created by bertr on 25-11-25.
//

#ifndef TESTCOLLISION_LEVELPAGE_H
#define TESTCOLLISION_LEVELPAGE_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace Vue
{
    class LevelPage
    {
    public:
        struct Selection {
            int chapterIndex = -1;
            int levelIndex = -1;
            bool valid = false;
        };

        LevelPage() = default;

        // Ouvre une fenêtre fullscreen et renvoie la sélection (bloquant)
        Selection run();
    private:
        // internal helpers
        void drawFrame(sf::RenderWindow& window, const std::vector<std::pair<std::string, std::vector<std::string>>>& chapters,
                       int chapterIdx, int levelIdx, const sf::Sprite& backgroundSprite, bool bgLoaded);

        void drawChapterSelection(sf::RenderWindow& window,
                                  const std::vector<std::pair<std::string, std::vector<std::string>>>& chapters,
                                  int chapterIdx,
                                  const sf::Sprite& backgroundSprite, bool bgLoaded);

        void drawLevelSelection(sf::RenderWindow& window,
                                const std::vector<std::pair<std::string, std::vector<std::string>>>& chapters,
                                int chapterIdx, int levelIdx,
                                const sf::Sprite& backgroundSprite, bool bgLoaded);
    };
}

#endif //TESTCOLLISION_LEVELPAGE_H