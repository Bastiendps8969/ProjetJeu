//
// LevelPage.cpp
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
                                         const std::vector<Modele::ChapterInfo>& chapters,
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

            // Nom du chapitre (provenant du JSON)
            sf::Text chapText;
            chapText.setFont(font);
            chapText.setString(chapters[i].name);
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
                                       const std::vector<Modele::ChapterInfo>& chapters,
                                       int chapterIdx, int levelIdx,
                                       const sf::Sprite& backgroundSprite, bool bgLoaded)
    {
        window.clear(sf::Color(18,18,28));
        sf::Vector2u win = window.getSize();
        sf::Font font;
        font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");

        // DEBUG: show chapter and levels info when drawing level selection
        if (chapterIdx >= 0 && chapterIdx < static_cast<int>(chapters.size())) {
            const auto &ch = chapters[chapterIdx];
            std::cerr << "[DEBUG] drawLevelSelection chapterIdx=" << chapterIdx << " name=\"" << ch.name << "\" levels=" << ch.levels.size() << " levelsFile=\"" << ch.levelsFile << "\"" << std::endl;
            for (size_t i = 0; i < ch.levels.size(); ++i) {
                const auto &L = ch.levels[i];
                std::cerr << "[DEBUG]   level " << i << " id=\"" << L.id << "\" name=\"" << L.name << "\" neededScore=" << L.neededScore << " picture=\"" << L.picture << "\"" << std::endl;
            }
        } else {
            std::cerr << "[DEBUG] drawLevelSelection invalid chapterIdx=" << chapterIdx << " chapters.size=" << chapters.size() << std::endl;
        }

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

        // Récupérer niveaux depuis JSON (vide si non chargé)
        const auto& levels = chapters[chapterIdx].levels;
        const int n = static_cast<int>(levels.size());

        // Si pas de niveaux : afficher message & panneau droit avec description du chapitre
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

        // --- Dessiner la rangée de niveaux en carrousel horizontal (si >=1) ---
        if (n > 0)
        {
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

                // Étiquette du niveau (provenant du JSON)
                sf::Text txt;
                txt.setFont(font);
                txt.setString(levels[i].name);
                txt.setCharacterSize(static_cast<unsigned int>((i == levelIdx) ? std::max(18.f, h*0.16f) : std::max(14.f, h*0.13f)));
                txt.setFillColor(i == levelIdx ? sf::Color(100, 50, 20) : sf::Color(80, 40, 20));
                sf::FloatRect lb = txt.getLocalBounds();
                txt.setPosition(rect.getPosition().x - lb.width / 2.f - lb.left,
                                rect.getPosition().y - lb.height / 2.f - lb.top);
                window.draw(txt);

                // dessiner un petit cadenas si ce n'est pas le premier niveau (comportement inchangé)
                // dessiner un petit cadenas si le niveau nécessite un score (verrouillé)
                if (levels[i].neededScore > 0)
                {
                    sf::CircleShape lockDot(std::max(5.f, std::min(10.f, w * 0.03f)));
                    lockDot.setFillColor(sf::Color(160, 40, 40));
                    lockDot.setPosition(rect.getPosition().x + w * 0.5f - lockDot.getRadius() * 2.f,
                                        rect.getPosition().y - h * 0.5f + 4.f);
                    window.draw(lockDot);
                }
            }
        }
        else
        {
            // Aucun niveau : message informatif
            sf::Text noLevels;
            noLevels.setFont(font);
            noLevels.setString("No levels available");
            noLevels.setCharacterSize(20);
            noLevels.setFillColor(sf::Color(220,220,220));
            noLevels.setPosition(centerX - 80.f, levelsTopY + 30.f);
            window.draw(noLevels);
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
        chapText.setString(chapters[chapterIdx].name);
        chapText.setCharacterSize(static_cast<unsigned int>(std::max(18.f, chapH * 0.14f)));
        chapText.setFillColor(sf::Color(255, 180, 180));
        sf::FloatRect cb = chapText.getLocalBounds();
        chapText.setPosition(chapRect.getPosition().x + (chapW - cb.width) / 2.f - cb.left,
                             chapRect.getPosition().y + (chapH - cb.height) / 2.f - cb.top);
        window.draw(chapText);

        // --- Panneau de prévisualisation à droite (titre, neededScore, description, image) ---
        float rightW = std::min(static_cast<float>(win.x) * 0.28f, 420.f);
        float rightX = static_cast<float>(win.x) - rightW;
        sf::RectangleShape rightRect({rightW, static_cast<float>(win.y)});
        rightRect.setPosition(rightX, 0.f);
        rightRect.setFillColor(sf::Color(18, 18, 18));
        window.draw(rightRect);

        // Titre & neededScore & description
        sf::Text lvlTitle;
        lvlTitle.setFont(font);
        lvlTitle.setCharacterSize(20);
        lvlTitle.setFillColor(sf::Color(220, 30, 30));
        lvlTitle.setStyle(sf::Text::Bold);
        lvlTitle.setPosition(rightX + 20.f, 24.f);

        sf::Text hs;
        hs.setFont(font);
        hs.setCharacterSize(16);
        hs.setFillColor(sf::Color(200, 90, 90));
        hs.setPosition(rightX + 20.f, 60.f);

        float textMaxW = rightW - 40.f;
        unsigned int descCharSize = 14;

        // Si pas de niveaux, afficher infos du chapitre ; sinon infos du niveau sélectionné
        std::string desc;
        if (n == 0)
        {
            const auto& ch = chapters[chapterIdx];
            lvlTitle.setString(ch.name);
            hs.setString("");
            desc = ch.description.empty() ? "Description indisponible." : ch.description;
        }
        else
        {
            const auto& lvl = levels[levelIdx];
            lvlTitle.setString(lvl.name + " - " + chapters[chapterIdx].name);
            hs.setString("Needed score : " + std::to_string(lvl.neededScore));
            desc = lvl.description.empty() ? "Description indisponible." : lvl.description;
        }
        window.draw(lvlTitle);
        window.draw(hs);

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

        // Image (si niveau présent) : utiliser lvl.picture si disponible, sinon essayer quelques chemins fallback
        static std::unordered_map<std::string, sf::Texture> texCache;
        bool haveTexture = false;
        std::string texKey;
        const Modele::LevelInfo* showLvl = (n > 0) ? &levels[levelIdx] : nullptr;
        if (showLvl)
        {
            texKey = !showLvl->picture.empty() ? showLvl->picture : ( !showLvl->id.empty() ? showLvl->id : showLvl->name );
            auto it = texCache.find(texKey);
            if (it == texCache.end())
            {
                std::vector<std::string> tryPaths;
                if (!showLvl->picture.empty()) tryPaths.push_back(showLvl->picture);
                if (!showLvl->id.empty()) tryPaths.push_back(std::string("Asset/levels/") + showLvl->id + ".png");
                tryPaths.push_back(std::string("Asset/Menu/") + showLvl->name + ".png");
                tryPaths.push_back(std::string("cmake-build-debug/Asset/Menu/") + showLvl->name + ".png");
                for (const auto& p : tryPaths)
                {
                    sf::Texture tex;
                    if (tex.loadFromFile(p))
                    {
                        texCache[texKey] = std::move(tex);
                        it = texCache.find(texKey);
                        haveTexture = true;
                        break;
                    }
                }
            }
            else
            {
                haveTexture = true;
            }

            float imgW = std::min(rightW * 0.6f, 280.f);
            float imgH = std::min(static_cast<float>(win.y) * 0.18f, 180.f);
            float imgX = rightX + (rightW - imgW) / 2.f;
            float imgY = static_cast<float>(win.y) * 0.60f;

            if (haveTexture && texCache.find(texKey) != texCache.end())
            {
                const sf::Texture& t = texCache[texKey];
                sf::Sprite spr(t);
                float tw = static_cast<float>(t.getSize().x);
                float th = static_cast<float>(t.getSize().y);
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
        }

        // --- Bouton START dans le panneau droit ---
        float buttonW = rightW - 40.f;
        float buttonH = 54.f;
        float buttonX = rightX + 20.f;
        float buttonY = static_cast<float>(win.y) - buttonH - 40.f;

        sf::RectangleShape startButton({buttonW, buttonH});
        startButton.setPosition(buttonX, buttonY);
        // Niveau déverrouillé si neededScore <= 0
        bool startEnabled = (n > 0 && levels[levelIdx].neededScore <= 0);
        if (startEnabled)
        {
            startButton.setFillColor(sf::Color(230, 60, 60));
            startButton.setOutlineColor(sf::Color(255, 120, 80));
            startButton.setOutlineThickness(3.f);
        }
        else
        {
            startButton.setFillColor(sf::Color(60, 60, 60));
            startButton.setOutlineColor(sf::Color(90, 90, 90));
            startButton.setOutlineThickness(2.f);
        }
        window.draw(startButton);

        sf::Text startLabel;
        startLabel.setFont(font);
        startLabel.setCharacterSize(22);
        startLabel.setStyle(sf::Text::Bold);
        startLabel.setFillColor(startEnabled ? sf::Color(255, 240, 220) : sf::Color(180,180,180));
        startLabel.setString(startEnabled ? "START" : "LOCKED");
        sf::FloatRect slb = startLabel.getLocalBounds();
        startLabel.setPosition(startButton.getPosition().x + (buttonW - slb.width) / 2.f - slb.left,
                               startButton.getPosition().y + (buttonH - slb.height) / 2.f - slb.top);
        window.draw(startLabel);

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
        // Charger les chapitres depuis JSON (plusieurs chemins d'essai)
        std::vector<Modele::ChapterInfo> chapters;
        std::vector<std::string> tryPaths = {
            "Asset/chapters/chapters.json",
            "Asset/chapters.json",
            "cmake-build-debug/Asset/chapters/chapters.json",
            "cmake-build-debug/Asset/chapters.json"
        };
        bool loaded = false;
        for (const auto& p : tryPaths)
        {
            std::cerr << "[DEBUG] try load chapters with: " << p << std::endl;
            auto loadedChapters = Modele::ChapterLoader::loadChapters(p);
            std::cerr << "[DEBUG] -> loadedChapters.size() = " << loadedChapters.size() << std::endl;
            if (!loadedChapters.empty())
            {
                chapters = std::move(loadedChapters);
                loaded = true;
                break;
            }
        }
        if (!loaded)
        {
            // Fallback: create Tutorial chapter using tutorial.json if possible
            Modele::ChapterInfo ch;
            ch.name = "Tutorial";
            ch.description = "Fallback tutorial chapter.";
            ch.levelsFile = "Asset/chapters/tutorial.json";
            std::cerr << "[DEBUG] fallback: attempt to load levels from " << ch.levelsFile << std::endl;
            ch.levels = Modele::ChapterLoader::loadLevels(ch.levelsFile);
            std::cerr << "[DEBUG] fallback loaded levels = " << ch.levels.size() << std::endl;
            chapters.push_back(std::move(ch));
        }

        // DEBUG: afficher ce qui a été chargé
        std::cerr << "[DEBUG] chapters loaded: " << chapters.size() << std::endl;
        for (size_t i = 0; i < chapters.size(); ++i) {
            std::cerr << "[DEBUG] chapter " << i
                      << " id=" << chapters[i].id
                      << " name=\"" << chapters[i].name << "\""
                      << " desc=\"" << chapters[i].description << "\""
                      << " levelsFile=\"" << chapters[i].levelsFile << "\"" << std::endl;
        }

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
                            // lazy-load des niveaux si besoin
                            if (chapters[chapterIdx].levels.empty() && !chapters[chapterIdx].levelsFile.empty())
                            {
                                // Try loading levels with several fallback paths
                                std::vector<std::string> tryLvPaths;
                                tryLvPaths.push_back(chapters[chapterIdx].levelsFile);
                                tryLvPaths.push_back(std::string("cmake-build-debug/") + chapters[chapterIdx].levelsFile);
                                // If path is just a filename, try in Asset/chapters/
                                size_t lastSlash = chapters[chapterIdx].levelsFile.find_last_of("/\\");
                                std::string base = (lastSlash == std::string::npos) ? chapters[chapterIdx].levelsFile : chapters[chapterIdx].levelsFile.substr(lastSlash+1);
                                tryLvPaths.push_back(std::string("Asset/chapters/") + base);
                                tryLvPaths.push_back(std::string("cmake-build-debug/Asset/chapters/") + base);

                                for (const auto &p2 : tryLvPaths)
                                {
                                    std::cerr << "[DEBUG] try load levels with: " << p2 << std::endl;
                                    auto loaded = Modele::ChapterLoader::loadLevels(p2);
                                    if (!loaded.empty())
                                    {
                                        chapters[chapterIdx].levels = std::move(loaded);
                                        std::cerr << "[DEBUG] loaded " << chapters[chapterIdx].levels.size() << " levels from: " << p2 << std::endl;
                                        break;
                                    }
                                }

                                // Fallback heuristic: try known chapter-specific filenames (Tutorial, OH)
                                if (chapters[chapterIdx].levels.empty())
                                {
                                    std::string lname = chapters[chapterIdx].name;
                                    std::transform(lname.begin(), lname.end(), lname.begin(), [](unsigned char c){ return std::tolower(c); });
                                    if (lname.find("tutorial") != std::string::npos)
                                    {
                                        std::string p3 = std::string("Asset/chapters/tutorial.json");
                                        std::cerr << "[DEBUG] fallback try: " << p3 << std::endl;
                                        auto loaded = Modele::ChapterLoader::loadLevels(p3);
                                        if (!loaded.empty()) { chapters[chapterIdx].levels = std::move(loaded); std::cerr << "[DEBUG] loaded " << chapters[chapterIdx].levels.size() << " levels from fallback: " << p3 << std::endl; }
                                    }
                                    else if (lname.find("hades") != std::string::npos || lname.find("operation hades") != std::string::npos || lname.find("oh") != std::string::npos)
                                    {
                                        std::string p3 = std::string("Asset/chapters/OH.json");
                                        std::cerr << "[DEBUG] fallback try: " << p3 << std::endl;
                                        auto loaded = Modele::ChapterLoader::loadLevels(p3);
                                        if (!loaded.empty()) { chapters[chapterIdx].levels = std::move(loaded); std::cerr << "[DEBUG] loaded " << chapters[chapterIdx].levels.size() << " levels from fallback: " << p3 << std::endl; }
                                    }
                                }
                            }
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
                                if (chapters[chapterIdx].levels.empty() && !chapters[chapterIdx].levelsFile.empty())
                                {
                                    std::vector<std::string> tryLvPaths;
                                    tryLvPaths.push_back(chapters[chapterIdx].levelsFile);
                                    tryLvPaths.push_back(std::string("cmake-build-debug/") + chapters[chapterIdx].levelsFile);
                                    size_t lastSlash = chapters[chapterIdx].levelsFile.find_last_of("/\\");
                                    std::string base = (lastSlash == std::string::npos) ? chapters[chapterIdx].levelsFile : chapters[chapterIdx].levelsFile.substr(lastSlash+1);
                                    tryLvPaths.push_back(std::string("Asset/chapters/") + base);
                                    tryLvPaths.push_back(std::string("cmake-build-debug/Asset/chapters/") + base);

                                    for (const auto &p2 : tryLvPaths)
                                    {
                                        std::cerr << "[DEBUG] try load levels with: " << p2 << std::endl;
                                        auto loaded = Modele::ChapterLoader::loadLevels(p2);
                                        if (!loaded.empty())
                                        {
                                            chapters[chapterIdx].levels = std::move(loaded);
                                            std::cerr << "[DEBUG] loaded " << chapters[chapterIdx].levels.size() << " levels from: " << p2 << std::endl;
                                            break;
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
                else
                {
                    // Gestion des entrées pour la sélection de niveau
                    const auto& levelsVec = chapters[chapterIdx].levels;
                    int nlevels = static_cast<int>(levelsVec.size());
                    if (event.type == sf::Event::KeyPressed)
                    {
                        if (event.key.code == sf::Keyboard::Escape)
                        {
                            selectingChapter = true; // Retour à la sélection du chapitre
                        }
                        else if (event.key.code == sf::Keyboard::Right)
                        {
                            if (nlevels > 0) levelIdx = std::min(nlevels - 1, levelIdx + 1);
                        }
                        else if (event.key.code == sf::Keyboard::Left)
                        {
                            if (nlevels > 0) levelIdx = std::max(0, levelIdx - 1);
                        }
                        else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return)
                        {
                            // Autoriser l'Enter si le niveau sélectionné est déverrouillé (neededScore <= 0)
                            if (nlevels > 0 && levelsVec[levelIdx].neededScore <= 0)
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
                                showUnavailableOverlay(window, nlevels == 0 ? "No levels available" : "Niveau indisponible");
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

                        // Vérifier si l'utilisateur a cliqué sur le bouton START du panneau droit
                        float rightW = std::min(static_cast<float>(winSz.x) * 0.28f, 420.f);
                        float rightX = static_cast<float>(winSz.x) - rightW;
                        float buttonW = rightW - 40.f;
                        float buttonH = 54.f;
                        float buttonX = rightX + 20.f;
                        float buttonY = static_cast<float>(winSz.y) - buttonH - 40.f;
                        sf::FloatRect startRect(buttonX, buttonY, buttonW, buttonH);
                            bool startEnabled = (nlevels > 0 && levelsVec[levelIdx].neededScore <= 0);
                        if (startEnabled && startRect.contains(mpos))
                        {
                            Selection sel;
                            sel.chapterIndex = chapterIdx;
                            sel.levelIndex = levelIdx;
                            sel.valid = true;
                            window.close();
                            return sel;
                        }

                        if (nlevels > 0)
                        {
                            float baseW = std::min(300.f, static_cast<float>(winSz.x) * 0.18f);
                            float baseH = std::min(140.f, static_cast<float>(winSz.y) * 0.12f);
                            float selectedScale = 1.35f;
                            float normalScale = 1.0f;
                            float gap = std::max(15.f, baseW * 0.12f);

                            std::vector<float> widths(nlevels), heights(nlevels), centers(nlevels);
                            for (int i = 0; i < nlevels; ++i)
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
                            for (int i = levelIdx + 1; i < nlevels; ++i)
                            {
                                float leftCenter = centers[i - 1];
                                centers[i] = leftCenter + (widths[i - 1] + widths[i]) / 2.f + gap;
                            }

                            bool handled = false;
                            for (int i = 0; i < nlevels; ++i)
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