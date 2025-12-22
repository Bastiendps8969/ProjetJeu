#pragma once
//
// LevelPage.h
//

#ifndef TESTCOLLISION_LEVELPAGE_H
#define TESTCOLLISION_LEVELPAGE_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "../Modele/ChapterLoader.h"

namespace Vue
{
    /**
     * LevelPage : écran de sélection des chapitres et des niveaux
     * - run() ouvre une fenêtre plein écran et retourne la sélection (bloquant)
     * - Gère l'affichage d'une liste de chapitres ainsi qu'un carrousel de niveaux
     *   avec un panneau de prévisualisation sur la droite.
     */
    class LevelPage
    {
    public:
        struct Selection {
            int chapterIndex = -1;
            int levelIndex = -1;
            bool valid = false;
        };

        LevelPage() = default;

        // Ouvre une fenêtre plein écran et renvoie la sélection (bloquant)
        Selection run();

    private:
        // Fonctions utilitaires internes pour le rendu
        void drawFrame(sf::RenderWindow& window, const std::vector<Modele::ChapterInfo>& chapters,
                       int chapterIdx, int levelIdx, const sf::Sprite& backgroundSprite, bool bgLoaded);

        // Dessine l'écran de sélection de chapitre (liste verticale)
        void drawChapterSelection(sf::RenderWindow& window,
                                  const std::vector<Modele::ChapterInfo>& chapters,
                                  int chapterIdx,
                                  const sf::Sprite& backgroundSprite, bool bgLoaded);

        // Dessine l'écran de sélection de niveau (carrousel + panneau droit de prévisualisation)
        void drawLevelSelection(sf::RenderWindow& window,
                                const std::vector<Modele::ChapterInfo>& chapters,
                                int chapterIdx, int levelIdx,
                                const sf::Sprite& backgroundSprite, bool bgLoaded);
    };
}

#endif //TESTCOLLISION_LEVELPAGE_H
