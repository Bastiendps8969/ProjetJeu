#include "DialogueBox.h"
#include <sstream>
#include <iostream>
#include <algorithm>

namespace Vue
{
    DialogueBox::DialogueBox()
    {
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontLoaded = true;
        }
    }

    void DialogueBox::initializeDialogueBox(const sf::Vector2u& windowSize)
    {
        float windowWidth = static_cast<float>(windowSize.x);
        float windowHeight = static_cast<float>(windowSize.y);

        // Fond du dialogue (rectangle en bas)
        backgroundBox.setSize(sf::Vector2f(windowWidth, 250.f));
        backgroundBox.setPosition(0.f, windowHeight - 250.f);
        backgroundBox.setFillColor(sf::Color(0, 0, 0, 200));
        backgroundBox.setOutlineThickness(2.f);
        backgroundBox.setOutlineColor(sf::Color::White);

        // Nom du personnage
        nameText.setFont(font);
        nameText.setString(currentDialogue.characterName);
        nameText.setCharacterSize(28);
        nameText.setFillColor(sf::Color::Yellow);
        nameText.setStyle(sf::Text::Bold);
        nameText.setPosition(220.f, windowHeight - 230.f);

        // Texte du dialogue
        dialogueText.setFont(font);
        dialogueText.setString(currentDialogue.text);
        dialogueText.setCharacterSize(20);
        dialogueText.setFillColor(sf::Color::White);
        dialogueText.setPosition(220.f, windowHeight - 180.f);

        // Texte "Cliquez pour continuer"
        continueText.setFont(font);
        continueText.setString("[Cliquez pour continuer]");
        continueText.setCharacterSize(16);
        continueText.setFillColor(sf::Color::Cyan);
        continueText.setPosition(windowWidth - 350.f, windowHeight - 50.f);

        // Préparer placeholder rose
        portraitLoaded = false;
        portraitPlaceholder.setSize(portraitSize);
        portraitPlaceholder.setFillColor(sf::Color(255, 0, 255)); // magenta
        portraitPlaceholder.setOutlineThickness(2.f);
        portraitPlaceholder.setOutlineColor(sf::Color::White);
        portraitPlaceholder.setPosition(20.f, windowHeight - 240.f);

        // Charger le portrait si possible
        if (!currentDialogue.characterPortraitPath.empty())
        {
            if (portraitTexture.loadFromFile(currentDialogue.characterPortraitPath))
            {
                portraitLoaded = true;
                std::cout << "[DEBUG] Portrait chargé : " << currentDialogue.characterPortraitPath << std::endl;
                portraitSprite.setTexture(portraitTexture);

                sf::Vector2u texSize = portraitTexture.getSize();
                if (texSize.x > 0 && texSize.y > 0)
                {
                    float sx = portraitSize.x / static_cast<float>(texSize.x);
                    float sy = portraitSize.y / static_cast<float>(texSize.y);
                    float s = std::min(sx, sy);
                    portraitSprite.setScale(s, s);

                    // positionner centré dans l'espace du placeholder
                    sf::FloatRect bounds = portraitSprite.getLocalBounds();
                    portraitSprite.setPosition(
                        portraitPlaceholder.getPosition().x + (portraitSize.x - bounds.width * s) / 2.f,
                        portraitPlaceholder.getPosition().y + (portraitSize.y - bounds.height * s) / 2.f
                    );
                }
                else
                {
                    portraitLoaded = false;
                }
            }
            else
            {
                std::cerr << "[DEBUG] Warning: unable to load portrait: " << currentDialogue.characterPortraitPath << std::endl;
                portraitLoaded = false;
            }
        }

        // Attendre un clic pour continuer
        waitingForClick = true;
    }

    void DialogueBox::startDialogue(const DialogueData& dialogue, const sf::Vector2u& windowSize)
    {
        currentDialogue = dialogue;
        active = true;
        displayTimer.restart();
        initializeDialogueBox(windowSize);
    }

    bool DialogueBox::shouldClose() const
    {
        // Ne fermer que si le clic a été effectué
        return !waitingForClick;
    }

    void DialogueBox::handleEvent(const sf::Event& event)
    {
        //  Pour le merge
        //  Modification des touches pour passer les dialogues
        if ((event.type == sf::Event::MouseButtonPressed ||
             (event.type == sf::Event::KeyPressed && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))) &&
            waitingForClick)
        {
            handleMouseClick(); // Marquer comme cliqué
        }
    }

    void DialogueBox::draw(sf::RenderWindow& fenetre)
    {
        if (!active || !fontLoaded)
            return;

        // Dessin overlay
        fenetre.draw(backgroundBox);

        if (portraitLoaded)
        {
            fenetre.draw(portraitSprite);
        }
        else
        {
            fenetre.draw(portraitPlaceholder);
        }

        fenetre.draw(nameText);
        fenetre.draw(dialogueText);
        // Afficher le texte "Cliquez pour continuer"
        fenetre.draw(continueText);
    }
}