#include "HomePage.h"
#include "CreditsWindow.h"
#include "ScoreWindow.h"
#include <numeric> // accumulate

namespace Vue
{
    HomePage::HomePage(std::function<std::vector<int>()> getScores)
        : getScoresCb(std::move(getScores))
    {
        // load system font if possible
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontLoaded = true;

            //  Title of the game
            titleText.setFont(font);
            titleText.setString("Cherub: Hades Operation");
            titleText.setCharacterSize(48);
            titleText.setFillColor(sf::Color::White);
            titleText.setStyle(sf::Text::Bold);

            //  Button to start the game
            //  Will move later
            playButton.setSize({200.f, 50.f});
            playButton.setFillColor(sf::Color(70,130,180)); // steelblue
            playLabel.setFont(font);
            playLabel.setString("Play");
            playLabel.setCharacterSize(24);
            playLabel.setFillColor(sf::Color::White);

            //  Button for the scores
            scoreButton.setSize({200.f, 40.f});
            scoreButton.setFillColor(sf::Color(100,100,100));
            scoreLabel.setFont(font);
            scoreLabel.setString("Score");
            scoreLabel.setCharacterSize(18);
            scoreLabel.setFillColor(sf::Color::White);

            // Credits button
            creditsButton.setSize({200.f, 40.f});
            creditsButton.setFillColor(sf::Color(100,100,100));
            creditsLabel.setFont(font);
            creditsLabel.setString("Credits");
            creditsLabel.setCharacterSize(18);
            creditsLabel.setFillColor(sf::Color::White);

            // Sounds button
            soundsButton.setSize({200.f, 36.f});
            soundsButton.setFillColor(sf::Color(100,100,100));
            soundsLabel.setFont(font);
            soundsLabel.setCharacterSize(16);
            soundsLabel.setFillColor(sf::Color::White);
            soundsLabel.setString(soundsOn ? "Sounds : On" : "Sounds : Off");
        }
    }

    void HomePage::centerLabel(sf::Text& label, const sf::RectangleShape& button)
    {
        sf::FloatRect lb = label.getLocalBounds();
        label.setPosition(
            button.getPosition().x + (button.getSize().x - lb.width) / 2.f - lb.left,
            button.getPosition().y + (button.getSize().y - lb.height) / 2.f - lb.top
        );
    }

    void HomePage::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
    {
        if (!active) return;

        if (event.type == sf::Event::TextEntered)
        {
            if (event.text.unicode >= 32 && event.text.unicode < 127 && playerName.size() < 32 && inputFocused)
            {
                playerName.push_back(static_cast<char>(event.text.unicode));
                inputText.setString(playerName);
            }
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::BackSpace && !playerName.empty() && inputFocused)
            {
                playerName.pop_back();
                inputText.setString(playerName);
            }
            else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return)
            {
                active = false; // start game
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2i mp = sf::Mouse::getPosition(fenetre);
            sf::Vector2f world = fenetre.mapPixelToCoords(mp);

            if (playButton.getGlobalBounds().contains(world))
            {
                active = false;
                return;
            }

            if (scoreButton.getGlobalBounds().contains(world))
            {
                // open separate score window
                openScoreWindow();
                return;
            }

            if (creditsButton.getGlobalBounds().contains(world))
            {
                showCredits = !showCredits;
                return;
            }

            if (soundsButton.getGlobalBounds().contains(world))
            {
                soundsOn = !soundsOn;
                soundsLabel.setString(soundsOn ? "Sons : On" : "Sons : Off");
                return;
            }

            if (inputBox.getGlobalBounds().contains(world))
            {
                inputFocused = true;
            }
            else
            {
                inputFocused = false;
            }
        }
    }

    void HomePage::draw(sf::RenderWindow& fenetre)
    {
        // compute centered column layout based on current window size
        sf::Vector2u win = fenetre.getSize();
        float centerX = static_cast<float>(win.x) * 0.5f;
        float startY = static_cast<float>(win.y) * 0.30f;

        // title
        titleText.setPosition(centerX - titleText.getLocalBounds().width / 2.f, startY - 80.f);

        // input box centered
        inputBox.setPosition(centerX - inputBox.getSize().x / 2.f, startY);
        inputText.setPosition(inputBox.getPosition() + sf::Vector2f(8.f, 6.f));

        // vertical column positions
        float gap = 14.f;
        float y = startY + inputBox.getSize().y + 30.f;

        playButton.setPosition(centerX - playButton.getSize().x / 2.f, y);
        centerLabel(playLabel, playButton);
        y += playButton.getSize().y + gap;

        scoreButton.setPosition(centerX - scoreButton.getSize().x / 2.f, y);
        centerLabel(scoreLabel, scoreButton);
        y += scoreButton.getSize().y + gap;

        creditsButton.setPosition(centerX - creditsButton.getSize().x / 2.f, y);
        centerLabel(creditsLabel, creditsButton);
        y += creditsButton.getSize().y + gap;

        soundsButton.setPosition(centerX - soundsButton.getSize().x / 2.f, y);
        centerLabel(soundsLabel, soundsButton);

        // draw UI
        fenetre.clear(sf::Color(30,30,40)); // dark background

        if (fontLoaded)
        {
            fenetre.draw(titleText);
            fenetre.draw(inputBox);
            fenetre.draw(inputText);

            fenetre.draw(playButton);
            fenetre.draw(playLabel);

            fenetre.draw(scoreButton);
            fenetre.draw(scoreLabel);

            fenetre.draw(creditsButton);
            fenetre.draw(creditsLabel);

            fenetre.draw(soundsButton);
            fenetre.draw(soundsLabel);
        }

        // credits overlay
        if (showCredits)
        {
            sf::RectangleShape overlay({win.x * 0.6f, win.y * 0.35f});
            overlay.setFillColor(sf::Color(0,0,0,200));
            overlay.setPosition(centerX - overlay.getSize().x/2.f, centerX*0.15f);
            fenetre.draw(overlay);

            sf::Text txt;
            txt.setFont(font);
            txt.setString("Crédits\nDéveloppeur: ...\nGraphismes: ...");
            txt.setCharacterSize(18);
            txt.setFillColor(sf::Color::White);
            txt.setPosition(overlay.getPosition() + sf::Vector2f(20.f,20.f));
            fenetre.draw(txt);
        }

        fenetre.display();
    }

    void HomePage::openScoreWindow()
    {
        ScoreWindow scoreWindow(getScoresCb);

        sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Scores", sf::Style::Fullscreen);

        while (window.isOpen() && scoreWindow.isActive())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                scoreWindow.handleEvent(event);
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            scoreWindow.draw(window);
        }
    }

    void HomePage::openCreditsWindow()
    {
        CreditsWindow creditsWindow;

        while (creditsWindow.isWindowOpen() && creditsWindow.isActive())
        {
            creditsWindow.update();
            creditsWindow.draw();
        }
    }
}