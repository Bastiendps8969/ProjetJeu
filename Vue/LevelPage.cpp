
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

namespace
{
    // Helper: safe access to scores vector (returns 0 if out of range).
    // WHY scores passed by const reference:
    // - avoids copying the vector
    // - expresses read-only intent
    int safeGetScore(const std::vector<int>& scores, size_t idx)
    {
        if (idx < scores.size()) return scores[idx];
        return 0;
    }
}

namespace Vue
{
    // Draw the chapter selection screen (vertical list of big blocks).
    void LevelPage::drawChapterSelection(sf::RenderWindow& window,
                                         const std::vector<Modele::ChapterInfo>& chapters,
                                         int chapterIdx,
                                         const sf::Sprite& backgroundSprite, bool bgLoaded)
    {
        // WHY RenderWindow by non-const reference:
        // - drawing requires mutating window state: clear(), draw(), display()
        window.clear(sf::Color(18, 18, 28));
        sf::Vector2u win = window.getSize();

        // Local font (loaded each call). Note: Windows-specific path.
        // (Could be cached similarly to textures if needed.)
        sf::Font font;
        font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");

        // Draw background if loaded (scaled to cover and tinted with alpha for readability).
        if (bgLoaded)
        {
            // WHY pointer here:
            // - SFML returns a pointer that can be null (sprite may not have a texture)
            const sf::Texture* bgTex = backgroundSprite.getTexture();
            if (bgTex)
            {
                float texW = static_cast<float>(bgTex->getSize().x);
                float texH = static_cast<float>(bgTex->getSize().y);
                float scaleX = static_cast<float>(win.x) / texW;
                float scaleY = static_cast<float>(win.y) / texH;
                float scale = std::max(scaleX, scaleY);

                // Copy the sprite to adjust transform/color without mutating the original reference.
                // WHY copy sprite:
                // - backgroundSprite is a const reference input (caller-owned)
                // - we need per-frame scaling and tinting
                sf::Sprite bgSprite = backgroundSprite;
                bgSprite.setScale(scale, scale);
                bgSprite.setPosition(0.f, 0.f);

                // Make background semi-transparent.
                sf::Color c = bgSprite.getColor();
                c.a = 180;
                bgSprite.setColor(c);

                window.draw(bgSprite);
            }
        }

        // Title
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

        // Draw chapters as big selectable blocks.
        float centerX = static_cast<float>(win.x) * 0.5f;
        float startY  = static_cast<float>(win.y) * 0.35f;
        float chapW   = std::min(500.f, static_cast<float>(win.x) * 0.6f);
        float chapH   = 100.f;
        float gap     = 40.f;

        for (size_t i = 0; i < chapters.size(); ++i)
        {
            float chapX = centerX - chapW / 2.f;
            float chapY = startY + i * (chapH + gap);

            sf::RectangleShape chapBlock({ chapW, chapH });
            chapBlock.setPosition(chapX, chapY);

            // Style depends on selection state (selected = brighter red).
            if (static_cast<int>(i) == chapterIdx)
            {
                chapBlock.setFillColor(sf::Color(230, 60, 60));
                chapBlock.setOutlineColor(sf::Color(255, 120, 80));
                chapBlock.setOutlineThickness(4.f);

                // Shine effect
                sf::RectangleShape shine({ chapW * 0.9f, chapH * 0.28f });
                shine.setPosition(chapX + chapW * 0.05f, chapY + 8.f);
                shine.setFillColor(sf::Color(255, 180, 140, 110));
                window.draw(shine);
            }
            else
            {
                chapBlock.setFillColor(sf::Color(170, 30, 30));
                chapBlock.setOutlineColor(sf::Color(130, 50, 50));
                chapBlock.setOutlineThickness(2.f);

                // Subtle shine
                sf::RectangleShape shine({ chapW * 0.85f, chapH * 0.22f });
                shine.setPosition(chapX + chapW * 0.075f, chapY + 10.f);
                shine.setFillColor(sf::Color(255, 150, 120, 60));
                window.draw(shine);
            }

            window.draw(chapBlock);

            // Chapter name comes from JSON (ChapterInfo.name).
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

    // Draw the level selection screen with a right preview panel (title, needed score, description, image).
    void LevelPage::drawLevelSelection(sf::RenderWindow& window,
                                       const std::vector<Modele::ChapterInfo>& chapters,
                                       int chapterIdx, int levelIdx,
                                       const sf::Sprite& backgroundSprite, bool bgLoaded)
    {
        window.clear(sf::Color(18, 18, 28));
        sf::Vector2u win = window.getSize();

        // Local font (loaded each call). Windows-specific.
        sf::Font font;
        font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");

        // DEBUG: show chapter and levels info when drawing level selection.
        if (chapterIdx >= 0 && chapterIdx < static_cast<int>(chapters.size()))
        {
            // WHY const reference:
            // - avoids copying ChapterInfo (may contain vectors/strings)
            const auto& ch = chapters[chapterIdx];

            std::cerr << "[DEBUG] drawLevelSelection chapterIdx=" << chapterIdx
                      << " name=\"" << ch.name << "\" levels=" << ch.levels.size()
                      << " levelsFile=\"" << ch.levelsFile << "\"" << std::endl;

            for (size_t i = 0; i < ch.levels.size(); ++i)
            {
                const auto& L = ch.levels[i];
                std::cerr << "[DEBUG] level " << i << " id=\"" << L.id << "\" name=\"" << L.name
                          << "\" neededScore=" << L.neededScore << " picture=\"" << L.picture << "\""
                          << std::endl;
            }
        }
        else
        {
            std::cerr << "[DEBUG] drawLevelSelection invalid chapterIdx=" << chapterIdx
                      << " chapters.size=" << chapters.size() << std::endl;
        }

        // Draw background if loaded (scaled to cover and tinted).
        if (bgLoaded)
        {
            const sf::Texture* bgTex = backgroundSprite.getTexture(); // pointer may be null
            if (bgTex)
            {
                float texW  = static_cast<float>(bgTex->getSize().x);
                float texH  = static_cast<float>(bgTex->getSize().y);
                float scaleX = static_cast<float>(win.x) / texW;
                float scaleY = static_cast<float>(win.y) / texH;
                float scale  = std::max(scaleX, scaleY);

                // Copy to adjust transform/color without mutating caller-owned sprite.
                sf::Sprite bgSprite = backgroundSprite;
                bgSprite.setScale(scale, scale);
                bgSprite.setPosition(0.f, 0.f);

                sf::Color c = bgSprite.getColor();
                c.a = 180;
                bgSprite.setColor(c);

                window.draw(bgSprite);
            }
        }

        // Layout parameters.
        const float centerX    = static_cast<float>(win.x) * 0.35f; // carousel center (left side)
        const float levelsTopY = static_cast<float>(win.y) * 0.12f;
        const float chapterBottomMargin = 60.f;

        // Levels list loaded from JSON (may be empty if not loaded).
        // WHY const reference:
        // - levels is a vector stored inside ChapterInfo; avoid copying it
        const auto& levels = chapters[chapterIdx].levels;
        const int n = static_cast<int>(levels.size());

        // Helper to word-wrap a string using SFML text bounds (simple layout).
        // WHY capture by reference [&]:
        // - reuses 'font' local variable without copying it into the closure
        auto wrapToLines = [&](const std::string& text, float maxWidth, unsigned int charSize)
        {
            // WHY input text by const reference:
            // - avoids copying potentially large strings
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
            return lines; // returned by value; small strings; NRVO/move
        };

        // --- Draw the horizontal carousel of levels (if any) ---
        if (n > 0)
        {
            // Base sizes.
            float baseW = std::min(300.f, static_cast<float>(win.x) * 0.18f);
            float baseH = std::min(140.f, static_cast<float>(win.y) * 0.12f);
            float selectedScale = 1.35f;
            float normalScale = 1.0f;
            float gap = std::max(15.f, baseW * 0.12f);

            // Precompute scaled widths/heights based on selection.
            std::vector<float> widths(n), heights(n);
            for (int i = 0; i < n; ++i)
            {
                float scale = (i == levelIdx) ? selectedScale : normalScale;
                widths[i]  = baseW * scale;
                heights[i] = baseH * scale;
            }

            // Compute centers so that selected level is centered at centerX.
            std::vector<float> centers(n, 0.f);
            centers[levelIdx] = centerX;

            // Place items to the left.
            for (int i = levelIdx - 1; i >= 0; --i)
            {
                float rightCenter = centers[i + 1];
                float wLeft  = widths[i];
                float wRight = widths[i + 1];
                centers[i] = rightCenter - (wLeft + wRight) / 2.f - gap;
            }

            // Place items to the right.
            for (int i = levelIdx + 1; i < n; ++i)
            {
                float leftCenter = centers[i - 1];
                float wLeft  = widths[i - 1];
                float wRight = widths[i];
                centers[i] = leftCenter + (wLeft + wRight) / 2.f + gap;
            }

            // Draw each level card.
            for (int i = 0; i < n; ++i)
            {
                float w = widths[i];
                float h = heights[i];

                sf::RectangleShape rect({ w, h });
                rect.setOrigin(w / 2.f, h / 2.f);
                rect.setPosition(centers[i], levelsTopY + h * 0.5f);

                if (i == levelIdx)
                {
                    rect.setFillColor(sf::Color(230, 60, 60));
                    rect.setOutlineThickness(4.f);
                    rect.setOutlineColor(sf::Color(255, 120, 80));

                    sf::RectangleShape shine({ w * 0.9f, h * 0.28f });
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

                    sf::RectangleShape shine({ w * 0.85f, h * 0.22f });
                    shine.setOrigin(shine.getSize().x / 2.f, 0.f);
                    shine.setPosition(rect.getPosition().x, rect.getPosition().y - h * 0.3f);
                    shine.setFillColor(sf::Color(255, 150, 120, 60));
                    window.draw(shine);
                }

                window.draw(rect);

                // Level label (from JSON LevelInfo.name).
                sf::Text txt;
                txt.setFont(font);
                txt.setString(levels[i].name);
                txt.setCharacterSize(static_cast<unsigned int>(
                    (i == levelIdx)
                        ? std::max(18.f, h * 0.16f)
                        : std::max(14.f, h * 0.13f)
                ));
                txt.setFillColor(i == levelIdx ? sf::Color(100, 50, 20) : sf::Color(80, 40, 20));

                sf::FloatRect lb = txt.getLocalBounds();
                txt.setPosition(rect.getPosition().x - lb.width / 2.f - lb.left,
                                rect.getPosition().y - lb.height / 2.f - lb.top);
                window.draw(txt);

                // Draw a small "lock dot" if the level requires a score (locked indicator).
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
            sf::Text noLevels;
            noLevels.setFont(font);
            noLevels.setString("No levels available");
            noLevels.setCharacterSize(20);
            noLevels.setFillColor(sf::Color(220, 220, 220));
            noLevels.setPosition(centerX - 80.f, levelsTopY + 30.f);
            window.draw(noLevels);
        }

        // --- Draw chapter block bottom-left ---
        float chapW = std::min(static_cast<float>(win.x) * 0.55f, static_cast<float>(win.x) - 160.f);
        float chapH = std::min(static_cast<float>(win.y) * 0.18f, 260.f);

        sf::RectangleShape chapRect({ chapW, chapH });
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

        // --- Right preview panel (title, neededScore, description, image) ---
        float rightW = std::min(static_cast<float>(win.x) * 0.28f, 420.f);
        float rightX = static_cast<float>(win.x) - rightW;

        sf::RectangleShape rightRect({ rightW, static_cast<float>(win.y) });
        rightRect.setPosition(rightX, 0.f);
        rightRect.setFillColor(sf::Color(18, 18, 18));
        window.draw(rightRect);

        // Title + needed score + description.
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

        // Image preview (if level exists).
        // Textures are cached in a static map to avoid reloading every frame.
        //
        // WHY static cache (composition local to this function):
        // - texture loading from disk is expensive
        // - the selection screen redraws at 60 FPS, so caching is crucial
        static std::unordered_map<std::string, sf::Texture> texCache;

        bool haveTexture = false;
        std::string texKey;

        // WHY pointer showLvl:
        // - optional selection: nullptr when there is no level (n == 0)
        // - avoids copying LevelInfo; points into the existing vector storage
        const Modele::LevelInfo* showLvl = (n > 0) ? &levels[levelIdx] : nullptr;

        if (showLvl)
        {
            texKey = !showLvl->picture.empty()
                        ? showLvl->picture
                        : (!showLvl->id.empty() ? showLvl->id : showLvl->name);

            auto it = texCache.find(texKey);

            // Load once if missing in cache.
            if (it == texCache.end())
            {
                std::vector<std::string> tryPaths;
                if (!showLvl->picture.empty()) tryPaths.push_back(showLvl->picture);
                if (!showLvl->id.empty())      tryPaths.push_back(std::string("Asset/levels/") + showLvl->id + ".png");
                tryPaths.push_back(std::string("Asset/Menu/") + showLvl->name + ".png");
                tryPaths.push_back(std::string("cmake-build-debug/Asset/Menu/") + showLvl->name + ".png");

                for (const auto& p : tryPaths)
                {
                    sf::Texture tex;
                    if (tex.loadFromFile(p))
                    {
                        // WHY move into cache:
                        // - sf::Texture may own GPU resources; moving avoids extra copies
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

                // Decorative frame around the image.
                sf::RectangleShape frame({ imgW, imgH });
                frame.setPosition(imgX, imgY);
                frame.setFillColor(sf::Color::Transparent);
                frame.setOutlineColor(sf::Color(200, 30, 30));
                frame.setOutlineThickness(4.f);

                window.draw(frame);
                window.draw(spr);
            }
            else
            {
                // Fallback placeholder if no image is found.
                sf::RectangleShape imgBox({ imgW, imgH });
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

        // --- START button in the right panel ---
        float buttonW = rightW - 40.f;
        float buttonH = 54.f;
        float buttonX = rightX + 20.f;
        float buttonY = static_cast<float>(win.y) - buttonH - 40.f;

        sf::RectangleShape startButton({ buttonW, buttonH });
        startButton.setPosition(buttonX, buttonY);

        // Determine if level is unlocked:
        // - neededScore <= 0 OR player has required score (via getScoresCb)
        bool startEnabled = false;
        if (n > 0)
        {
            if (levels[levelIdx].neededScore <= 0) startEnabled = true;
            else
            {
                try
                {
                    if (this->getScoresCb)
                    {
                        // WHY vector<int> by value from callback:
                        // - caller decides how to compute/provide scores
                        // - we take a snapshot, independent from caller's internal storage
                        std::vector<int> sc = this->getScoresCb();
                        int tutorialScore = safeGetScore(sc, 0);

                        if (tutorialScore > levels[levelIdx].neededScore) startEnabled = true;
                    }
                }
                catch (...)
                {
                    // ignore errors and keep locked
                }
            }
        }

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
        startLabel.setFillColor(startEnabled ? sf::Color(255, 240, 220) : sf::Color(180, 180, 180));
        startLabel.setString(startEnabled ? "START" : "LOCKED");

        sf::FloatRect slb = startLabel.getLocalBounds();
        startLabel.setPosition(startButton.getPosition().x + (buttonW - slb.width) / 2.f - slb.left,
                               startButton.getPosition().y + (buttonH - slb.height) / 2.f - slb.top);
        window.draw(startLabel);

        window.display();
    }

    // Utility overlay: temporary "unavailable" message shown for a short duration.
    static void showUnavailableOverlay(sf::RenderWindow& window, const std::string& msg)
    {
        // WHY RenderWindow&:
        // - draw/pollEvent modifies window state
        // WHY msg by const reference:
        // - avoids copying message string
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
                if (evt.type == sf::Event::KeyPressed ||
                    evt.type == sf::Event::MouseButtonPressed)
                {
                    // Allow user to dismiss the overlay early.
                    return;
                }
            }

            sf::Vector2u winSz = window.getSize();

            sf::RectangleShape overlayBg({ winSz.x * 0.5f, winSz.y * 0.12f });
            overlayBg.setFillColor(sf::Color(0, 0, 0, 200));
            overlayBg.setOutlineColor(sf::Color(180, 50, 50));
            overlayBg.setOutlineThickness(3.f);
            overlayBg.setPosition((winSz.x - overlayBg.getSize().x) / 2.f,
                                  (winSz.y - overlayBg.getSize().y) * 0.45f);

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

    // Main entry point: opens a fullscreen selection window and returns a Selection struct.
    LevelPage::Selection LevelPage::run()
    {
        // Load chapters from JSON (try multiple paths to handle different working directories).
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
                // WHY move:
                // - chapters takes ownership of the loaded vector content without copying
                chapters = std::move(loadedChapters);
                loaded = true;
                break;
            }
        }

        // If not loaded, create a fallback "Tutorial" chapter and try to load tutorial.json levels.
        if (!loaded)
        {
            Modele::ChapterInfo ch;
            ch.name = "Tutorial";
            ch.description = "Fallback tutorial chapter.";
            ch.levelsFile = "Asset/chapters/tutorial.json";

            std::cerr << "[DEBUG] fallback: attempt to load levels from " << ch.levelsFile << std::endl;

            ch.levels = Modele::ChapterLoader::loadLevels(ch.levelsFile);

            std::cerr << "[DEBUG] fallback loaded levels = " << ch.levels.size() << std::endl;

            chapters.push_back(std::move(ch));
        }

        // DEBUG: print loaded chapters.
        std::cerr << "[DEBUG] chapters loaded: " << chapters.size() << std::endl;
        for (size_t i = 0; i < chapters.size(); ++i)
        {
            std::cerr << "[DEBUG] chapter " << i
                      << " id=" << chapters[i].id
                      << " name=\"" << chapters[i].name << "\""
                      << " desc=\"" << chapters[i].description << "\""
                      << " levelsFile=\"" << chapters[i].levelsFile << "\""
                      << std::endl;
        }

        int chapterIdx = 0;
        int levelIdx = 0;
        bool selectingChapter = true; // which screen is currently active

        // Create the fullscreen selection window.
        sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Select Level", sf::Style::Fullscreen);
        window.setFramerateLimit(60);

        // Load background image with several fallback paths.
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

        // Main event/render loop.
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
                    // Chapter selection input handling.
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
                        else if (event.key.code == sf::Keyboard::Enter ||
                                 event.key.code == sf::Keyboard::Return)
                        {
                            // Enter level selection screen.
                            selectingChapter = false;
                            levelIdx = 0;

                            // Lazy-load levels for the selected chapter if needed.
                            if (chapters[chapterIdx].levels.empty() && !chapters[chapterIdx].levelsFile.empty())
                            {
                                std::vector<std::string> tryLvPaths;
                                tryLvPaths.push_back(chapters[chapterIdx].levelsFile);
                                tryLvPaths.push_back(std::string("cmake-build-debug/") + chapters[chapterIdx].levelsFile);

                                // If levelsFile is just a filename, try Asset/chapters/.
                                size_t lastSlash = chapters[chapterIdx].levelsFile.find_last_of("/\\");
                                std::string base = (lastSlash == std::string::npos)
                                                    ? chapters[chapterIdx].levelsFile
                                                    : chapters[chapterIdx].levelsFile.substr(lastSlash + 1);

                                tryLvPaths.push_back(std::string("Asset/chapters/") + base);
                                tryLvPaths.push_back(std::string("cmake-build-debug/Asset/chapters/") + base);

                                for (const auto& p2 : tryLvPaths)
                                {
                                    std::cerr << "[DEBUG] try load levels with: " << p2 << std::endl;

                                    auto loadedLevels = Modele::ChapterLoader::loadLevels(p2);

                                    if (!loadedLevels.empty())
                                    {
                                        chapters[chapterIdx].levels = std::move(loadedLevels);

                                        std::cerr << "[DEBUG] loaded " << chapters[chapterIdx].levels.size()
                                                  << " levels from: " << p2 << std::endl;
                                        break;
                                    }
                                }

                                // Fallback heuristic: known chapter-specific filenames.
                                if (chapters[chapterIdx].levels.empty())
                                {
                                    std::string lname = chapters[chapterIdx].name;
                                    std::transform(lname.begin(), lname.end(), lname.begin(),
                                                   [](unsigned char c) { return std::tolower(c); });

                                    if (lname.find("tutorial") != std::string::npos)
                                    {
                                        std::string p3 = std::string("Asset/chapters/tutorial.json");
                                        std::cerr << "[DEBUG] fallback try: " << p3 << std::endl;

                                        auto loadedLevels = Modele::ChapterLoader::loadLevels(p3);
                                        if (!loadedLevels.empty())
                                        {
                                            chapters[chapterIdx].levels = std::move(loadedLevels);
                                            std::cerr << "[DEBUG] loaded " << chapters[chapterIdx].levels.size()
                                                      << " levels from fallback: " << p3 << std::endl;
                                        }
                                    }
                                    else if (lname.find("hades") != std::string::npos ||
                                             lname.find("operation hades") != std::string::npos ||
                                             lname.find("oh") != std::string::npos)
                                    {
                                        std::string p3 = std::string("Asset/chapters/OH.json");
                                        std::cerr << "[DEBUG] fallback try: " << p3 << std::endl;

                                        auto loadedLevels = Modele::ChapterLoader::loadLevels(p3);
                                        if (!loadedLevels.empty())
                                        {
                                            chapters[chapterIdx].levels = std::move(loadedLevels);
                                            std::cerr << "[DEBUG] loaded " << chapters[chapterIdx].levels.size()
                                                      << " levels from fallback: " << p3 << std::endl;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                    {
                        // Mouse click chapter selection: compute chapter block rectangles and hit-test.
                        sf::Vector2i mp = sf::Mouse::getPosition(window);
                        sf::Vector2f mpos = window.mapPixelToCoords(mp);

                        float centerX = static_cast<float>(window.getSize().x) * 0.5f;
                        float startY  = static_cast<float>(window.getSize().y) * 0.35f;
                        float chapW   = std::min(500.f, static_cast<float>(window.getSize().x) * 0.6f);
                        float chapH   = 100.f;
                        float gap     = 40.f;

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

                                // Lazy-load levels similarly on mouse selection.
                                if (chapters[chapterIdx].levels.empty() && !chapters[chapterIdx].levelsFile.empty())
                                {
                                    std::vector<std::string> tryLvPaths;
                                    tryLvPaths.push_back(chapters[chapterIdx].levelsFile);
                                    tryLvPaths.push_back(std::string("cmake-build-debug/") + chapters[chapterIdx].levelsFile);

                                    size_t lastSlash = chapters[chapterIdx].levelsFile.find_last_of("/\\");
                                    std::string base = (lastSlash == std::string::npos)
                                                        ? chapters[chapterIdx].levelsFile
                                                        : chapters[chapterIdx].levelsFile.substr(lastSlash + 1);

                                    tryLvPaths.push_back(std::string("Asset/chapters/") + base);
                                    tryLvPaths.push_back(std::string("cmake-build-debug/Asset/chapters/") + base);

                                    for (const auto& p2 : tryLvPaths)
                                    {
                                        std::cerr << "[DEBUG] try load levels with: " << p2 << std::endl;

                                        auto loadedLevels = Modele::ChapterLoader::loadLevels(p2);
                                        if (!loadedLevels.empty())
                                        {
                                            chapters[chapterIdx].levels = std::move(loadedLevels);
                                            std::cerr << "[DEBUG] loaded " << chapters[chapterIdx].levels.size()
                                                      << " levels from: " << p2 << std::endl;
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
                    // Level selection input handling.
                    const auto& levelsVec = chapters[chapterIdx].levels; // const ref avoids copy
                    int nlevels = static_cast<int>(levelsVec.size());

                    if (event.type == sf::Event::KeyPressed)
                    {
                        if (event.key.code == sf::Keyboard::Escape)
                        {
                            // Back to chapter selection.
                            selectingChapter = true;
                        }
                        else if (event.key.code == sf::Keyboard::Right)
                        {
                            if (nlevels > 0) levelIdx = std::min(nlevels - 1, levelIdx + 1);
                        }
                        else if (event.key.code == sf::Keyboard::Left)
                        {
                            if (nlevels > 0) levelIdx = std::max(0, levelIdx - 1);
                        }
                        else if (event.key.code == sf::Keyboard::Enter ||
                                 event.key.code == sf::Keyboard::Return)
                        {
                            // Check unlocking rules (neededScore <= 0 OR tutorialScore > neededScore).
                            bool unlocked = false;
                            if (nlevels > 0)
                            {
                                int needed = levelsVec[levelIdx].neededScore;
                                if (needed <= 0) unlocked = true;
                                else if (this->getScoresCb)
                                {
                                    try
                                    {
                                        std::vector<int> sc = this->getScoresCb();
                                        if (safeGetScore(sc, 0) > needed) unlocked = true;
                                    }
                                    catch (...)
                                    {
                                    }
                                }
                            }

                            if (unlocked)
                            {
                                Selection sel;
                                sel.chapterIndex = chapterIdx;
                                sel.levelIndex = levelIdx;
                                sel.valid = true;

                                // Return the levelData path if present.
                                if (chapterIdx >= 0 && chapterIdx < static_cast<int>(chapters.size()))
                                {
                                    const auto& lv = chapters[chapterIdx].levels;
                                    if (levelIdx >= 0 && levelIdx < static_cast<int>(lv.size()))
                                    {
                                        sel.levelData = lv[levelIdx].levelData;
                                    }
                                }

                                window.close();
                                return sel; // return by value (NRVO/move)
                            }
                            else
                            {
                                showUnavailableOverlay(window, nlevels == 0 ? "No levels available" : "Niveau indisponible");
                            }
                        }
                    }
                    else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                    {
                        // Mouse click handling in level selection screen:
                        // - start button click
                        // - level card click (select or select+start)
                        sf::Vector2i mp = sf::Mouse::getPosition(window);
                        sf::Vector2f mpos = window.mapPixelToCoords(mp);
                        sf::Vector2u winSz = window.getSize();

                        const float centerX = static_cast<float>(winSz.x) * 0.35f;
                        const float levelsTopY = static_cast<float>(winSz.y) * 0.12f;

                        // Right panel START button rect.
                        float rightW = std::min(static_cast<float>(winSz.x) * 0.28f, 420.f);
                        float rightX = static_cast<float>(winSz.x) - rightW;

                        float buttonW = rightW - 40.f;
                        float buttonH = 54.f;
                        float buttonX = rightX + 20.f;
                        float buttonY = static_cast<float>(winSz.y) - buttonH - 40.f;
                        sf::FloatRect startRect(buttonX, buttonY, buttonW, buttonH);

                        // Compute startEnabled according to unlocking rules.
                        bool startEnabled = false;
                        if (nlevels > 0)
                        {
                            int needed = levelsVec[levelIdx].neededScore;
                            if (needed <= 0) startEnabled = true;
                            else if (this->getScoresCb)
                            {
                                try
                                {
                                    std::vector<int> sc = this->getScoresCb();
                                    if (safeGetScore(sc, 0) > needed) startEnabled = true;
                                }
                                catch (...)
                                {
                                }
                            }
                        }

                        // Click START button -> validate selection if unlocked.
                        if (startEnabled && startRect.contains(mpos))
                        {
                            Selection sel;
                            sel.chapterIndex = chapterIdx;
                            sel.levelIndex = levelIdx;
                            sel.valid = true;

                            if (chapterIdx >= 0 && chapterIdx < static_cast<int>(chapters.size()))
                            {
                                const auto& lv = chapters[chapterIdx].levels;
                                if (levelIdx >= 0 && levelIdx < static_cast<int>(lv.size()))
                                {
                                    sel.levelData = lv[levelIdx].levelData;
                                }
                            }

                            window.close();
                            return sel;
                        }

                        // Click on carousel cards.
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

                            for (int i = 0; i < nlevels; ++i)
                            {
                                float w = widths[i];
                                float h = heights[i];

                                sf::FloatRect r(centers[i] - w / 2.f, levelsTopY, w, h);
                                if (r.contains(mpos))
                                {
                                    if (i != levelIdx)
                                    {
                                        // First click selects a different level.
                                        levelIdx = i;
                                    }
                                    else
                                    {
                                        // Clicking again on selected level -> attempt to start if unlocked.
                                        bool unlockedClick = false;
                                        int needed = levelsVec[i].neededScore;

                                        if (needed <= 0) unlockedClick = true;
                                        else if (this->getScoresCb)
                                        {
                                            try
                                            {
                                                std::vector<int> sc = this->getScoresCb();
                                                if (safeGetScore(sc, 0) > needed) unlockedClick = true;
                                            }
                                            catch (...)
                                            {
                                            }
                                        }

                                        if (unlockedClick)
                                        {
                                            Selection sel;
                                            sel.chapterIndex = chapterIdx;
                                            sel.levelIndex = levelIdx;
                                            sel.valid = true;

                                            if (chapterIdx >= 0 && chapterIdx < static_cast<int>(chapters.size()))
                                            {
                                                const auto& lv = chapters[chapterIdx].levels;
                                                if (levelIdx >= 0 && levelIdx < static_cast<int>(lv.size()))
                                                {
                                                    sel.levelData = lv[levelIdx].levelData;
                                                }
                                            }

                                            window.close();
                                            return sel;
                                        }
                                        else
                                        {
                                            showUnavailableOverlay(window, "Niveau indisponible");
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            // Render appropriate screen depending on current state.
            if (selectingChapter)
                drawChapterSelection(window, chapters, chapterIdx, bgSprite, bgLoaded);
            else
                drawLevelSelection(window, chapters, chapterIdx, levelIdx, bgSprite, bgLoaded);
        }

        return LevelPage::Selection();
    }
}

// Constructor + setter
namespace Vue
{
    LevelPage::LevelPage(std::function<std::vector<int>()> cb)
    {
        // Store callback (move to avoid extra copies).
        // WHY move:
        // - std::function can hold heap allocations (type-erased callable)
        // - moving minimizes allocations/copies when caller passes a temporary
        getScoresCb = std::move(cb);
    }

    void LevelPage::setGetScoresCb(std::function<std::vector<int>()> cb)
    {
        // Same rationale as constructor: accept any callable and move it in.
        getScoresCb = std::move(cb);
    }
}
