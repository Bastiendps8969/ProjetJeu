//
// Created by bertr on 25-11-25.
//

#include "LevelPage.h"
#include "ChapterLoader.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <sstream>

namespace Vue
{
    // Dessiner l'écran de sélection des chapitres
    void LevelPage::drawChapterSelection(sf::RenderWindow& window,
                                         const std::vector<std::pair<std::string, std::vector<std::string>>>& chapters,
                                         int chapterIdx,
                                         const sf::Sprite& backgroundSprite, bool bgLoaded)
    {
        window.clear(sf::Color(18,18,28));
        sf::Vector2u win = window.getSize();
        sf::Font font;
        font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");

        // Dessiner l'arrière-plan s'il est chargé
        if (bgLoaded)
        {
            const sf::Texture* bgTex = backgroundSprite.getTexture();
            if (bgTex)
            {
                float texW = static_cast<float>(bgTex->getSize().x);
                float texH = static_cast<float>(bgTex->getSize().y);
                float scaleX = static_cast<float>(win.x) / texW;
                float scaleY = static_cast<float>(win.y) / texH;
                float scale = std::max(scaleX, scaleY);

                sf::Sprite bgSprite = backgroundSprite;
                bgSprite.setScale(scale, scale);
                bgSprite.setPosition(0.f, 0.f);
                sf::Color c = bgSprite.getColor();
                c.a = 180;
                bgSprite.setColor(c);
                window.draw(bgSprite);
            }
        }

        // Titre
        sf::Text titleText;
        titleText.setFont(font);
        titleText.setString("SELECT CHAPTER");
        titleText.setCharacterSize(48);
        titleText.setFillColor(sf::Color(220, 30, 30));
        titleText.setStyle(sf::Text::Bold);
        sf::FloatRect tb = titleText.getLocalBounds();
        titleText.setPosition(static_cast<float>(win.x) * 0.5f - tb.width / 2.f - tb.left,
                             static_cast<float>(win.y) * 0.15f);
        window.draw(titleText);

        // Dessiner les chapitres sous forme de gros blocs
        float centerX = static_cast<float>(win.x) * 0.5f;
        float startY = static_cast<float>(win.y) * 0.35f;
        float chapW = std::min(500.f, static_cast<float>(win.x) * 0.6f);
        float chapH = 100.f;
        float gap = 40.f;

        for (size_t i = 0; i < chapters.size(); ++i)
        {
            float chapX = centerX - chapW / 2.f;
            float chapY = startY + i * (chapH + gap);

            sf::RectangleShape chapBlock({chapW, chapH});
            chapBlock.setPosition(chapX, chapY);

            if (static_cast<int>(i) == chapterIdx)
            {
                // Sélectionné : rouge vif
                chapBlock.setFillColor(sf::Color(230, 60, 60));
                chapBlock.setOutlineColor(sf::Color(255, 120, 80));
                chapBlock.setOutlineThickness(4.f);

                // Effet de brillance
                sf::RectangleShape shine({chapW * 0.9f, chapH * 0.28f});
                shine.setPosition(chapX + chapW * 0.05f, chapY + 8.f);
                shine.setFillColor(sf::Color(255, 180, 140, 110));
                window.draw(shine);
            }
            else
            {
                // Normal : rouge plus foncé
                chapBlock.setFillColor(sf::Color(170, 30, 30));
                chapBlock.setOutlineColor(sf::Color(130, 50, 50));
                chapBlock.setOutlineThickness(2.f);

                // Brillance subtile
                sf::RectangleShape shine({chapW * 0.85f, chapH * 0.22f});
                shine.setPosition(chapX + chapW * 0.075f, chapY + 10.f);
                shine.setFillColor(sf::Color(255, 150, 120, 60));
                window.draw(shine);
            }

            window.draw(chapBlock);

            // Nom du chapitre
            sf::Text chapText;
            chapText.setFont(font);
            chapText.setString(chapters[i].first);
            chapText.setCharacterSize(32);
            chapText.setFillColor(sf::Color(255, 240, 220));
            chapText.setStyle(sf::Text::Bold);
            sf::FloatRect ctb = chapText.getLocalBounds();
            chapText.setPosition(chapX + (chapW - ctb.width) / 2.f - ctb.left,
                                chapY + (chapH - ctb.height) / 2.f - ctb.top);
            window.draw(chapText);
        }

        window.display();
    }

    // Dessiner l'écran de sélection de niveaux avec panneau droit
    void LevelPage::drawLevelSelection(sf::RenderWindow& window,
                                       const std::vector<std::pair<std::string, std::vector<std::string>>>& chapters,
                                       int chapterIdx, int levelIdx,
                                       const sf::Sprite& backgroundSprite, bool bgLoaded)
    {
        window.clear(sf::Color(18,18,28));
        sf::Vector2u win = window.getSize();
        sf::Font font;
        font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");

        // Dessiner l'arrière-plan s'il est chargé
        if (bgLoaded)
        {
            const sf::Texture* bgTex = backgroundSprite.getTexture();
            if (bgTex)
            {
                float texW = static_cast<float>(bgTex->getSize().x);
                float texH = static_cast<float>(bgTex->getSize().y);
                float scaleX = static_cast<float>(win.x) / texW;
                float scaleY = static_cast<float>(win.y) / texH;
                float scale = std::max(scaleX, scaleY);

                sf::Sprite bgSprite = backgroundSprite;
                bgSprite.setScale(scale, scale);
                bgSprite.setPosition(0.f, 0.f);
                sf::Color c = bgSprite.getColor();
                c.a = 180;
                bgSprite.setColor(c);
                window.draw(bgSprite);
            }
        }

        // Paramètres de mise en page
        const float centerX = static_cast<float>(win.x) * 0.35f; // niveau centré à gauche (pour faire place au panel)
        const float levelsTopY = static_cast<float>(win.y) * 0.12f;
        const float chapterBottomMargin = 60.f;

        // --- Dessiner la rangée de niveaux en carrousel horizontal (zone supérieure gauche) ---
        const auto& levels = chapters[chapterIdx].second;
        const int n = static_cast<int>(levels.size());

        // Tailles de base
        float baseW = std::min(300.f, static_cast<float>(win.x) * 0.18f);
        float baseH = std::min(140.f, static_cast<float>(win.y) * 0.12f);
        float selectedScale = 1.35f;
        float normalScale = 1.0f;
        float gap = std::max(15.f, baseW * 0.12f);

        // Calculer largeurs/hauteurs redimensionnées
        std::vector<float> widths(n), heights(n);
        for (int i = 0; i < n; ++i)
        {
            float scale = (i == levelIdx) ? selectedScale : normalScale;
            widths[i] = baseW * scale;
            heights[i] = baseH * scale;
        }

        // Calculer les centres
        std::vector<float> centers(n, 0.f);
        centers[levelIdx] = centerX;

        // à gauche
        for (int i = levelIdx - 1; i >= 0; --i)
        {
            float rightCenter = centers[i + 1];
            float wLeft = widths[i];
            float wRight = widths[i + 1];
            centers[i] = rightCenter - (wLeft + wRight) / 2.f - gap;
        }
        // à droite
        for (int i = levelIdx + 1; i < n; ++i)
        {
            float leftCenter = centers[i - 1];
            float wLeft = widths[i - 1];
            float wRight = widths[i];
            centers[i] = leftCenter + (wLeft + wRight) / 2.f + gap;
        }

        // Dessiner les niveaux
        for (int i = 0; i < n; ++i)
        {
            float w = widths[i];
            float h = heights[i];
            sf::RectangleShape rect({w, h});
            rect.setOrigin(w/2.f, h/2.f);
            rect.setPosition(centers[i], levelsTopY + h * 0.5f);

            if (i == levelIdx)
            {
                rect.setFillColor(sf::Color(230, 60, 60));
                rect.setOutlineThickness(4.f);
                rect.setOutlineColor(sf::Color(255, 120, 80));

                sf::RectangleShape shine({w * 0.9f, h * 0.28f});
                shine.setOrigin(shine.getSize().x / 2.f, 0.f);
                shine.setPosition(rect.getPosition().x, rect.getPosition().y - h * 0.25f);
                shine.setFillColor(sf::Color(255, 180, 140, 110));
                window.draw(shine);
            }
            else
            {
                rect.setFillColor(sf::Color(170, 30, 30));
                rect.setOutlineThickness(2.f);
                rect.setOutlineColor(sf::Color(130, 50, 50));

                sf::RectangleShape shine({w * 0.85f, h * 0.22f});
                shine.setOrigin(shine.getSize().x / 2.f, 0.f);
                shine.setPosition(rect.getPosition().x, rect.getPosition().y - h * 0.3f);
                shine.setFillColor(sf::Color(255, 150, 120, 60));
                window.draw(shine);
            }
            window.draw(rect);

            // Étiquette du niveau
            sf::Text txt;
            txt.setFont(font);
            txt.setString(levels[i]);
            txt.setCharacterSize(static_cast<unsigned int>((i == levelIdx) ? std::max(18.f, h*0.16f) : std::max(14.f, h*0.13f)));
            txt.setFillColor(i == levelIdx ? sf::Color(100, 50, 20) : sf::Color(80, 40, 20));
            sf::FloatRect lb = txt.getLocalBounds();
            txt.setPosition(rect.getPosition().x - lb.width / 2.f - lb.left,
                            rect.getPosition().y - lb.height / 2.f - lb.top);
            window.draw(txt);

            // dessiner un petit cadenas si ce n'est pas le premier niveau
            if (i != 0)
            {
                sf::CircleShape lockDot(std::max(5.f, std::min(10.f, w * 0.03f)));
                lockDot.setFillColor(sf::Color(160, 40, 40));
                lockDot.setPosition(rect.getPosition().x + w * 0.5f - lockDot.getRadius() * 2.f,
                                    rect.getPosition().y - h * 0.5f + 4.f);
                window.draw(lockDot);
            }
        }

        // --- Dessiner le bloc du chapitre en bas-gauche ---
        float chapW = std::min(static_cast<float>(win.x) * 0.55f, static_cast<float>(win.x) - 160.f);
        float chapH = std::min(static_cast<float>(win.y) * 0.18f, 260.f);
        sf::RectangleShape chapRect({chapW, chapH});
        float chapX = centerX - chapW / 2.f;
        float chapY = static_cast<float>(win.y) - chapH - chapterBottomMargin;
        chapRect.setPosition(chapX, chapY);
        chapRect.setFillColor(sf::Color(160, 30, 30));
        window.draw(chapRect);

        sf::Text chapText;
        chapText.setFont(font);
        chapText.setString(chapters[chapterIdx].first);
        chapText.setCharacterSize(static_cast<unsigned int>(std::max(18.f, chapH * 0.14f)));
        chapText.setFillColor(sf::Color(255, 180, 180));
        sf::FloatRect cb = chapText.getLocalBounds();
        chapText.setPosition(chapRect.getPosition().x + (chapW - cb.width) / 2.f - cb.left,
                             chapRect.getPosition().y + (chapH - cb.height) / 2.f - cb.top);
        window.draw(chapText);

        // --- Panneau de prévisualisation à droite (titre, highscore, description, image) ---
        float rightW = std::min(static_cast<float>(win.x) * 0.28f, 420.f);
        float rightX = static_cast<float>(win.x) - rightW;
        sf::RectangleShape rightRect({rightW, static_cast<float>(win.y)});
        rightRect.setPosition(rightX, 0.f);
        rightRect.setFillColor(sf::Color(18, 18, 18));
        window.draw(rightRect);

        // Données du niveau courant
        std::string levelName = levels[levelIdx];

        // Titre (accent rouge)
        sf::Text lvlTitle;
        lvlTitle.setFont(font);
        lvlTitle.setString(levelName + " - OH");
        lvlTitle.setCharacterSize(20);
        lvlTitle.setFillColor(sf::Color(220, 30, 30));
        lvlTitle.setStyle(sf::Text::Bold);
        lvlTitle.setPosition(rightX + 20.f, 24.f);
        window.draw(lvlTitle);

        // Espace réservé HighScore (rouge clair)
        sf::Text hs;
        hs.setFont(font);
        hs.setString("HighScore : Score");
        hs.setCharacterSize(16);
        hs.setFillColor(sf::Color(200, 90, 90));
        hs.setPosition(rightX + 20.f, 60.f);
        window.draw(hs);

        // Map des descriptions
        static std::unordered_map<std::string, std::string> descriptions = {
            {"Tutoriel", "Un tutoriel pour l'Operation Hades."},
            {"Lvl 1", "Premier niveau : BastiLove cherche Bertri."},
            {"Lvl 2", "Deuxieme niveau : BastiLove trouve Bertri."},
            {"Lvl 3", "Troisieme niveau : BastiLove et BertriLove."}
        };

        std::string desc = descriptions.count(levelName) ? descriptions[levelName] : "Description indisponible.";

        // Wrap simple des lignes pour la description
        auto wrapToLines = [&](const std::string& text, float maxWidth, unsigned int charSize) {
            std::vector<std::string> lines;
            std::istringstream iss(text);
            std::string word;
            std::string line;
            while (iss >> word)
            {
                std::string test = line.empty() ? word : line + " " + word;
                sf::Text t;
                t.setFont(font);
                t.setString(test);
                t.setCharacterSize(charSize);
                if (t.getLocalBounds().width > maxWidth && !line.empty())
                {
                    lines.push_back(line);
                    line = word;
                }
                else
                {
                    line = test;
                }
            }
            if (!line.empty()) lines.push_back(line);
            return lines;
        };

        float textMaxW = rightW - 40.f;
        unsigned int descCharSize = 14;
        auto lines = wrapToLines(desc, textMaxW, descCharSize);
        float startY = 105.f;
        for (size_t i = 0; i < lines.size(); ++i)
        {
            sf::Text dt;
            dt.setFont(font);
            dt.setString(lines[i]);
            dt.setCharacterSize(descCharSize);
            dt.setFillColor(sf::Color(220, 220, 220));
            dt.setPosition(rightX + 20.f, startY + i * (descCharSize + 4));
            window.draw(dt);
        }

        // Zone du cadre de l'image
        float imgW = std::min(rightW * 0.6f, 280.f);
        float imgH = std::min(static_cast<float>(win.y) * 0.18f, 180.f);
        float imgX = rightX + (rightW - imgW) / 2.f;
        float imgY = static_cast<float>(win.y) * 0.60f;

        // Cache de textures
        static std::unordered_map<std::string, sf::Texture> texCache;
        auto it = texCache.find(levelName);
        bool haveTexture = false;
        if (it == texCache.end())
        {
            std::vector<std::string> tryPaths = {
                std::string("Asset/Menu/")+levelName+".png",
                std::string("Asset/Menu/")+levelName+"Small.png",
                std::string("cmake-build-debug/Asset/Menu/")+levelName+".png",
                std::string("cmake-build-debug/Asset/Menu/")+levelName+"Small.png"
            };
            for (const auto& p : tryPaths)
            {
                sf::Texture tex;
                if (tex.loadFromFile(p))
                {
                    texCache[levelName] = std::move(tex);
                    it = texCache.find(levelName);
                    haveTexture = true;
                    break;
                }
            }
        }
        else
        {
            haveTexture = true;
        }

        if (haveTexture && it != texCache.end())
        {
            sf::Sprite spr(it->second);
            float tw = static_cast<float>(it->second.getSize().x);
            float th = static_cast<float>(it->second.getSize().y);
            float scale = std::min(imgW / tw, imgH / th);
            spr.setScale(scale, scale);
            float sw = tw * scale;
            float sh = th * scale;
            spr.setPosition(imgX + (imgW - sw) / 2.f, imgY + (imgH - sh) / 2.f);

            sf::RectangleShape frame({imgW, imgH});
            frame.setPosition(imgX, imgY);
            frame.setFillColor(sf::Color::Transparent);
            frame.setOutlineColor(sf::Color(200, 30, 30));
            frame.setOutlineThickness(4.f);
            window.draw(frame);
            window.draw(spr);
        }
        else
        {
            sf::RectangleShape imgBox({imgW, imgH});
            imgBox.setPosition(imgX, imgY);
            imgBox.setFillColor(sf::Color(30, 30, 30));
            imgBox.setOutlineColor(sf::Color(200, 30, 30));
            imgBox.setOutlineThickness(4.f);
            window.draw(imgBox);

            sf::Text placeholder;
            placeholder.setFont(font);
            placeholder.setString("Image Lvl");
            placeholder.setCharacterSize(18);
            placeholder.setFillColor(sf::Color(200, 200, 200));
            sf::FloatRect pb = placeholder.getLocalBounds();
            placeholder.setPosition(imgBox.getPosition().x + (imgW - pb.width) / 2.f - pb.left,
                                    imgBox.getPosition().y + (imgH - pb.height) / 2.f - pb.top);
            window.draw(placeholder);
        }

        window.display();
    }

    // Utilitaire : affiche un overlay temporaire "Niveau indisponible"
    static void showUnavailableOverlay(sf::RenderWindow& window, const std::string& msg)
    {
        sf::Font font;
        font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");
        sf::Clock clock;
        const float duration = 1.2f;
        while (window.isOpen() && clock.getElapsedTime().asSeconds() < duration)
        {
            sf::Event evt;
            while (window.pollEvent(evt))
            {
                if (evt.type == sf::Event::Closed) { window.close(); return; }
                if (evt.type == sf::Event::KeyPressed || evt.type == sf::Event::MouseButtonPressed)
                {
                    return;
                }
            }

            sf::Vector2u winSz = window.getSize();
            sf::RectangleShape overlayBg({winSz.x * 0.5f, winSz.y * 0.12f});
            overlayBg.setFillColor(sf::Color(0, 0, 0, 200));
            overlayBg.setOutlineColor(sf::Color(180, 50, 50));
            overlayBg.setOutlineThickness(3.f);
            overlayBg.setPosition((winSz.x - overlayBg.getSize().x) / 2.f, (winSz.y - overlayBg.getSize().y) * 0.45f);

            sf::Text t;
            t.setFont(font);
            t.setString(msg);
            t.setCharacterSize(28);
            t.setFillColor(sf::Color(255, 200, 200));
            sf::FloatRect tb = t.getLocalBounds();
            t.setPosition(overlayBg.getPosition().x + (overlayBg.getSize().x - tb.width) / 2.f - tb.left,
                          overlayBg.getPosition().y + (overlayBg.getSize().y - tb.height) / 2.f - tb.top);

            window.draw(overlayBg);
            window.draw(t);
            window.display();

            sf::sleep(sf::milliseconds(16));
        }
    }

    LevelPage::Selection LevelPage::run()
    {
        // Chapitres & niveaux
        std::vector<std::pair<std::string, std::vector<std::string>>> chapters;
        chapters.push_back({"Operation Hades", {"Tutoriel", "Lvl 1", "Lvl 2", "Lvl 3"}});

        int chapterIdx = 0;
        int levelIdx = 0;
        bool selectingChapter = true; // État pour déterminer quel écran afficher

        sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Select Level", sf::Style::Fullscreen);
        window.setFramerateLimit(60);

        // Charger l'image d'arrière-plan
        sf::Texture bgTexture;
        sf::Sprite bgSprite;
        bool bgLoaded = false;
        
        std::vector<std::string> tryBgPaths = {
            "Asset/Menu/CherubMenuJeu.png",
            "Asset/Menu/CherubMenu.png",
            "CherubMenuJeu.png",
            "CherubMenu.png",
            "cmake-build-debug/Asset/Menu/CherubMenuJeu.png",
            "cmake-build-debug/Asset/Menu/CherubMenu.png"
        };
        
        for (const auto& p : tryBgPaths)
        {
            if (bgTexture.loadFromFile(p))
            {
                bgLoaded = true;
                bgSprite.setTexture(bgTexture);
                break;
            }
        }

        while (window.isOpen())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                    return LevelPage::Selection();
                }

                if (selectingChapter)
                {
                    // Gestion des entrées pour la sélection du chapitre
                    if (event.type == sf::Event::KeyPressed)
                    {
                        if (event.key.code == sf::Keyboard::Escape)
                        {
                            window.close();
                            return LevelPage::Selection();
                        }
                        else if (event.key.code == sf::Keyboard::Up)
                        {
                            chapterIdx = std::max(0, chapterIdx - 1);
                        }
                        else if (event.key.code == sf::Keyboard::Down)
                        {
                            chapterIdx = std::min((int)chapters.size() - 1, chapterIdx + 1);
                        }
                        else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return)
                        {
                            selectingChapter = false; // Passer à la sélection de niveau
                            levelIdx = 0;
                        }
                    }
                    else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                    {
                        sf::Vector2i mp = sf::Mouse::getPosition(window);
                        sf::Vector2f mpos = window.mapPixelToCoords(mp);

                        float centerX = static_cast<float>(window.getSize().x) * 0.5f;
                        float startY = static_cast<float>(window.getSize().y) * 0.35f;
                        float chapW = std::min(500.f, static_cast<float>(window.getSize().x) * 0.6f);
                        float chapH = 100.f;
                        float gap = 40.f;

                        for (size_t i = 0; i < chapters.size(); ++i)
                        {
                            float chapX = centerX - chapW / 2.f;
                            float chapY = startY + i * (chapH + gap);

                            sf::FloatRect rect(chapX, chapY, chapW, chapH);
                            if (rect.contains(mpos))
                            {
                                chapterIdx = static_cast<int>(i);
                                selectingChapter = false;
                                levelIdx = 0;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    // Gestion des entrées pour la sélection de niveau
                    if (event.type == sf::Event::KeyPressed)
                    {
                        if (event.key.code == sf::Keyboard::Escape)
                        {
                            selectingChapter = true; // Retour à la sélection du chapitre
                        }
                        else if (event.key.code == sf::Keyboard::Right)
                        {
                            levelIdx = std::min((int)chapters[chapterIdx].second.size() - 1, levelIdx + 1);
                        }
                        else if (event.key.code == sf::Keyboard::Left)
                        {
                            levelIdx = std::max(0, levelIdx - 1);
                        }
                        else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return)
                        {
                            if (levelIdx == 0)
                            {
                                Selection sel;
                                sel.chapterIndex = chapterIdx;
                                sel.levelIndex = levelIdx;
                                sel.valid = true;
                                window.close();
                                return sel;
                            }
                            else
                            {
                                showUnavailableOverlay(window, "Niveau indisponible");
                            }
                        }
                    }
                    else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                    {
                        sf::Vector2i mp = sf::Mouse::getPosition(window);
                        sf::Vector2f mpos = window.mapPixelToCoords(mp);

                        sf::Vector2u winSz = window.getSize();
                        const float centerX = static_cast<float>(winSz.x) * 0.35f;
                        const float levelsTopY = static_cast<float>(winSz.y) * 0.12f;

                        float baseW = std::min(300.f, static_cast<float>(winSz.x) * 0.18f);
                        float baseH = std::min(140.f, static_cast<float>(winSz.y) * 0.12f);
                        float selectedScale = 1.35f;
                        float normalScale = 1.0f;
                        float gap = std::max(15.f, baseW * 0.12f);

                        const auto& levelsVec = chapters[chapterIdx].second;
                        int n = static_cast<int>(levelsVec.size());
                        std::vector<float> widths(n), heights(n), centers(n);
                        for (int i = 0; i < n; ++i)
                        {
                            float scale = (i == levelIdx) ? selectedScale : normalScale;
                            widths[i] = baseW * scale;
                            heights[i] = baseH * scale;
                        }
                        centers[levelIdx] = centerX;
                        for (int i = levelIdx - 1; i >= 0; --i)
                        {
                            float rightCenter = centers[i + 1];
                            centers[i] = rightCenter - (widths[i] + widths[i + 1]) / 2.f - gap;
                        }
                        for (int i = levelIdx + 1; i < n; ++i)
                        {
                            float leftCenter = centers[i - 1];
                            centers[i] = leftCenter + (widths[i - 1] + widths[i]) / 2.f + gap;
                        }

                        bool handled = false;
                        for (int i = 0; i < n; ++i)
                        {
                            float w = widths[i];
                            float h = heights[i];
                            sf::FloatRect r(centers[i] - w/2.f, levelsTopY, w, h);
                            if (r.contains(mpos))
                            {
                                if (i != levelIdx)
                                {
                                    levelIdx = i;
                                }
                                else
                                {
                                    if (i == 0)
                                    {
                                        Selection sel;
                                        sel.chapterIndex = chapterIdx;
                                        sel.levelIndex = levelIdx;
                                        sel.valid = true;
                                        window.close();
                                        return sel;
                                    }
                                    else
                                    {
                                        showUnavailableOverlay(window, "Niveau indisponible");
                                    }
                                }
                                handled = true;
                                break;
                            }
                        }
                        (void)handled;
                    }
                }
            }

            // Rendu de l'écran approprié
            if (selectingChapter)
            {
                drawChapterSelection(window, chapters, chapterIdx, bgSprite, bgLoaded);
            }
            else
            {
                drawLevelSelection(window, chapters, chapterIdx, levelIdx, bgSprite, bgLoaded);
            }
        }

        return LevelPage::Selection();
    }
}