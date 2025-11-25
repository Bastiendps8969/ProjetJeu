//
// Created by bertr on 25-11-25.
//

#include "HomePage.h"

namespace Vue
{
    HomePage::HomePage()
    {
        // charge une police système Windows si possible
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontLoaded = true;

            titleText.setFont(font);
            titleText.setString("Mon Jeu");
            titleText.setCharacterSize(48);
            titleText.setFillColor(sf::Color::White);
            titleText.setStyle(sf::Text::Bold);
            titleText.setPosition(220.f, 80.f);

            inputBox.setSize({360.f, 40.f});
            inputBox.setFillColor(sf::Color(50,50,50));
            inputBox.setOutlineColor(sf::Color::White);
            inputBox.setOutlineThickness(2.f);
            inputBox.setPosition(220.f, 200.f);

            inputText.setFont(font);
            inputText.setString("");
            inputText.setCharacterSize(20);
            inputText.setFillColor(sf::Color::White);
            inputText.setPosition(inputBox.getPosition() + sf::Vector2f(8.f, 6.f));

            playButton.setSize({160.f, 50.f});
            playButton.setFillColor(sf::Color(70,130,180)); // steelblue
            playButton.setPosition(320.f, 270.f);

            playLabel.setFont(font);
            playLabel.setString("Play");
            playLabel.setCharacterSize(24);
            playLabel.setFillColor(sf::Color::White);
            // center label in button
            sf::FloatRect lb = playLabel.getLocalBounds();
            playLabel.setPosition(
                playButton.getPosition().x + (playButton.getSize().x - lb.width) / 2.f - lb.left,
                playButton.getPosition().y + (playButton.getSize().y - lb.height) / 2.f - lb.top
            );
        }
    }

    void HomePage::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
    {
        if (!active) return;

        if (event.type == sf::Event::TextEntered)
        {
            if (event.text.unicode >= 32 && event.text.unicode < 127 && playerName.size() < 32)
            {
                playerName.push_back(static_cast<char>(event.text.unicode));
                inputText.setString(playerName);

            }
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::BackSpace && !playerName.empty())
            {
                playerName.pop_back();
                inputText.setString(playerName);
            }
            else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return)
            {
                active = false; // démarre le jeu
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2i mp = sf::Mouse::getPosition(fenetre);
            sf::Vector2f world = fenetre.mapPixelToCoords(mp);
            if (playButton.getGlobalBounds().contains(world))
            {
                active = false;
            }
            if (inputBox.getGlobalBounds().contains(world))
            {
                // focus could be tracked if needed
            }
        }
    }

    void HomePage::draw(sf::RenderWindow& fenetre)
    {
        // dessine l'écran d'accueil et gère l'affichage
        fenetre.clear(sf::Color(30,30,40)); // fond sombre

        if (fontLoaded)
        {
            fenetre.draw(titleText);
            fenetre.draw(inputBox);
            fenetre.draw(inputText);
            fenetre.draw(playButton);
            fenetre.draw(playLabel);
        }

        fenetre.display();
    }
}