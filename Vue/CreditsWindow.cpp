#include "CreditsWindow.h"

namespace Vue
{
    CreditsWindow::CreditsWindow()
        : window(sf::VideoMode::getDesktopMode(), "Crédits", sf::Style::Fullscreen)
    {
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontLoaded = true;
            initializeTexts();
        }
    }

    void CreditsWindow::initializeTexts()
    {
        // Titre
        titleText.setFont(font);
        titleText.setString("CREDITS");
        titleText.setCharacterSize(48);
        titleText.setFillColor(sf::Color::White);
        titleText.setStyle(sf::Text::Bold);

        sf::FloatRect titleBounds = titleText.getLocalBounds();
        sf::Vector2u windowSize = window.getSize();
        titleText.setPosition(
            (windowSize.x - titleBounds.width) / 2.f,
            50.f
        );

        // Contenu des crédits
        std::vector<std::string> creditLines = {
            "Développé par: Equipe de Développement",
            "",
            "Programmation: Bert R.",
            "Design: Design Team",
            "Art: Art Team",
            "",
            "Moteur Graphique: SFML",
            "",
            "Appuyez sur ECHAP ou ENTREE pour fermer"
        };

        float yPos = 150.f;
        for (const auto& line : creditLines)
        {
            sf::Text creditText;
            creditText.setFont(font);
            creditText.setString(line);
            creditText.setCharacterSize(24);
            creditText.setFillColor(sf::Color::White);

            sf::FloatRect textBounds = creditText.getLocalBounds();
            creditText.setPosition(
                (windowSize.x - textBounds.width) / 2.f,
                yPos
            );
            creditTexts.push_back(creditText);
            yPos += 50.f;
        }
    }

    void CreditsWindow::handleEvent(const sf::Event& event)
    {
        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Escape ||
                event.key.code == sf::Keyboard::Return)
            {
                window.close();
                active = false;
            }
        }

        if (event.type == sf::Event::Closed)
        {
            window.close();
            active = false;
        }
    }

    void CreditsWindow::update()
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            handleEvent(event);
        }
    }

    void CreditsWindow::draw()
    {
        window.clear(sf::Color::Black);

        if (fontLoaded)
        {
            window.draw(titleText);
            for (const auto& creditText : creditTexts)
            {
                window.draw(creditText);
            }
        }

        window.display();
    }
}