#include "GameOverPage.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace Vue {

std::vector<std::string> GameOverPage::candidatePaths()
{
    return {
        "Asset/GameOver/Game_OverFinal.png",
        "cmake-build-debug/Asset/GameOver/Game_OverFinal.png",
        "Game_OverFinal.png"
    };
}



GameOverPage::GameOverPage()
{
    // Load fallback font (try system then local)
    if (!font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
        font.loadFromFile("arial.ttf"); // ignore failure, text will be blank
    }

    // Initialize textual elements
    titleText.setFont(font);
    titleText.setString("GAME OVER");
    titleText.setCharacterSize(128);
    titleText.setFillColor(sf::Color::White);
    titleText.setStyle(sf::Text::Bold);

    promptText.setFont(font);
    promptText.setString("PRESS ENTER");
    promptText.setCharacterSize(36);
    promptText.setFillColor(sf::Color::White);
    promptText.setStyle(sf::Text::Bold);

    // Attempt to load the primary final Game Over image.
    bgLoaded = false;
    for (const auto &p : candidatePaths()) {
        if (bgTexture.loadFromFile(p)) {
            bgLoaded = true;
            selectedPath = p;
            bgTexture.setSmooth(true); // enable smoothing when scaling
            bgSprite.setTexture(bgTexture);
            break;
        }
    }
}

void GameOverPage::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
{
    if (!active) return;

    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space)
        {
            active = false;
        }
    }
    else if (event.type == sf::Event::MouseButtonPressed)
    {
        active = false; // allow mouse click as an alternative
    }
    else if (event.type == sf::Event::Closed)
    {
        fenetre.close();
        active = false;
    }
}

void GameOverPage::draw(sf::RenderWindow& fenetre)
{
    // Get the current window size in floating point to compute scales/positions
    sf::Vector2u win = fenetre.getSize();
    float winW = static_cast<float>(win.x);
    float winH = static_cast<float>(win.y);


    if (bgLoaded && bgTexture.getSize().x > 0 && bgTexture.getSize().y > 0)
    {
        // Compute texture and window sizes and derive a scale that COVERS the
        // full window area. This may crop parts of the image but guarantees no
        // letterboxing and a consistent full-screen look.
        float texW = static_cast<float>(bgTexture.getSize().x);
        float texH = static_cast<float>(bgTexture.getSize().y);

        // scale factors on both axes
        float sx = winW / texW;
        float sy = winH / texH;
        // pick the larger scale so the texture fully covers the window
        float s = std::max(sx, sy);

        // Optionally prevent upscaling to avoid visible quality loss on small assets
        if (!allowUpscale) {
            s = std::min(s, 1.0f);
        }

        // Tiny overscale avoids 1-pixel borders due to rounding at some resolutions
        s *= 1.01f;

        // Apply the scale and center the sprite inside the window
        bgSprite.setScale(s, s);

        sf::FloatRect b = bgSprite.getGlobalBounds();
        bgSprite.setPosition((winW - b.width) * 0.5f, (winH - b.height) * 0.5f);

        // Draw a solid black background first to hide any edges during scaling
        sf::RectangleShape pad(sf::Vector2f(winW, winH));
        pad.setFillColor(sf::Color::Black);
        fenetre.draw(pad);
        fenetre.draw(bgSprite);

        // The prompt blinks to attract attention. We reuse the same rhythm as
        // the title splash: visible for 0.7s, faint for 0.3s (1s cycle total).
        float t = blinkClock.getElapsedTime().asSeconds();
        if (std::fmod(t, 1.0f) < 0.7f)
            promptText.setFillColor(sf::Color(255,255,255,255));
        else
            promptText.setFillColor(sf::Color(255,255,255,60));

        sf::FloatRect pb = promptText.getLocalBounds();
        promptText.setOrigin(pb.left + pb.width / 2.f, pb.top + pb.height / 2.f);
        // Place the prompt near the bottom of the window so it does not overlap
        // important parts of the full-screen image.
        promptText.setPosition(winW * 0.5f, winH * 0.94f);
        fenetre.draw(promptText);
    }
    else
    {
        // Fallback path: if the image is missing, draw a clear black background
        // and a centered title, then the same blinking prompt below it.
        sf::RectangleShape bg(sf::Vector2f(winW, winH));
        bg.setFillColor(sf::Color::Black);
        fenetre.draw(bg);

        // Center and draw the large "GAME OVER" title
        sf::FloatRect tb = titleText.getLocalBounds();
        titleText.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
        titleText.setPosition(winW * 0.5f, winH * 0.4f);
        fenetre.draw(titleText);

        // Same blinking rhythm for the prompt in the fallback mode as well
        float t = blinkClock.getElapsedTime().asSeconds();
        if (std::fmod(t, 1.0f) < 0.7f)
            promptText.setFillColor(sf::Color(255,255,255,255));
        else
            promptText.setFillColor(sf::Color(255,255,255,60));

        // Draw the prompt under the title
        sf::FloatRect pb = promptText.getLocalBounds();
        promptText.setOrigin(pb.left + pb.width / 2.f, pb.top + pb.height / 2.f);
        promptText.setPosition(winW * 0.5f, winH * 0.6f);
        fenetre.draw(promptText);
    }

}

} // namespace Vue