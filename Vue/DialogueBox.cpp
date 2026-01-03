
#include "DialogueBox.h"
#include <sstream>
#include <iostream>
#include <algorithm>

namespace Vue
{
    DialogueBox::DialogueBox()
    {
        // Load font (Windows-specific path).
        if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            fontLoaded = true;
        }
    }

    void DialogueBox::initializeDialogueBox(const sf::Vector2u& windowSize)
    {
        float windowWidth  = static_cast<float>(windowSize.x);
        float windowHeight = static_cast<float>(windowSize.y);

        // Background panel (full width, fixed height, anchored to bottom).
        backgroundBox.setSize(sf::Vector2f(windowWidth, 250.f));
        backgroundBox.setPosition(0.f, windowHeight - 250.f);
        backgroundBox.setFillColor(sf::Color(0, 0, 0, 200));
        backgroundBox.setOutlineThickness(2.f);
        backgroundBox.setOutlineColor(sf::Color::White);

        // Character name text.
        nameText.setFont(font);
        nameText.setString(currentDialogue.characterName);
        nameText.setCharacterSize(28);
        nameText.setFillColor(sf::Color::Yellow);
        nameText.setStyle(sf::Text::Bold);
        nameText.setPosition(220.f, windowHeight - 230.f);

        // Dialogue body text.
        dialogueText.setFont(font);
        dialogueText.setString(currentDialogue.text);
        dialogueText.setCharacterSize(20);
        dialogueText.setFillColor(sf::Color::White);
        dialogueText.setPosition(220.f, windowHeight - 180.f);

        // "Click to continue" hint.
        continueText.setFont(font);
        continueText.setString("[Cliquez pour continuer]");
        continueText.setCharacterSize(16);
        continueText.setFillColor(sf::Color::Cyan);
        continueText.setPosition(windowWidth - 350.f, windowHeight - 50.f);

        // Prepare magenta placeholder for missing portrait (very visible debug color).
        portraitLoaded = false;
        portraitPlaceholder.setSize(portraitSize);
        portraitPlaceholder.setFillColor(sf::Color(255, 0, 255)); // magenta
        portraitPlaceholder.setOutlineThickness(2.f);
        portraitPlaceholder.setOutlineColor(sf::Color::White);
        portraitPlaceholder.setPosition(20.f, windowHeight - 240.f);

        // Try to load portrait texture if a path is provided.
        if (!currentDialogue.characterPortraitPath.empty())
        {
            if (portraitTexture.loadFromFile(currentDialogue.characterPortraitPath))
            {
                portraitLoaded = true;
                std::cout << "[DEBUG] Portrait chargé : " << currentDialogue.characterPortraitPath << std::endl;

                portraitSprite.setTexture(portraitTexture);

                // Scale the portrait to fit within portraitSize while preserving aspect ratio.
                sf::Vector2u texSize = portraitTexture.getSize();
                if (texSize.x > 0 && texSize.y > 0)
                {
                    float sx = portraitSize.x / static_cast<float>(texSize.x);
                    float sy = portraitSize.y / static_cast<float>(texSize.y);
                    float s = std::min(sx, sy);

                    portraitSprite.setScale(s, s);

                    // Center the sprite inside the placeholder rectangle area.
                    sf::FloatRect bounds = portraitSprite.getLocalBounds();
                    portraitSprite.setPosition(
                        portraitPlaceholder.getPosition().x + (portraitSize.x - bounds.width * s) / 2.f,
                        portraitPlaceholder.getPosition().y + (portraitSize.y - bounds.height * s) / 2.f
                    );
                }
                else
                {
                    // Invalid texture size -> fallback to placeholder.
                    portraitLoaded = false;
                }
            }
            else
            {
                std::cerr << "[DEBUG] Warning: unable to load portrait: " << currentDialogue.characterPortraitPath << std::endl;
                portraitLoaded = false;
            }
        }

        // Start in "waiting for click" mode (manual advance).
        waitingForClick = true;
    }

    void DialogueBox::startDialogue(const DialogueData& dialogue, const sf::Vector2u& windowSize)
    {
        // Copy the new dialogue data into internal state.
        currentDialogue = dialogue;

        // Activate dialogue box and restart timer.
        active = true;
        displayTimer.restart();

        // Build UI layout for the current window size.
        initializeDialogueBox(windowSize);
    }

    bool DialogueBox::shouldClose() const
    {
        // Close (advance) only when the user has clicked / pressed Space.
        return !waitingForClick;
    }

    void DialogueBox::handleEvent(const sf::Event& event)
    {
        // NOTE: The "merge" comment suggests input logic was adjusted during integration.
        // Current behavior: mouse click OR Space key press advances the dialogue.

        if ((event.type == sf::Event::MouseButtonPressed ||
             (event.type == sf::Event::KeyPressed && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))) &&
            waitingForClick)
        {
            handleMouseClick(); // mark as clicked / ready to close
        }
    }

    void DialogueBox::draw(sf::RenderWindow& fenetre)
    {
        // Do not draw if inactive or if font is missing.
        if (!active || !fontLoaded)
            return;

        // Draw overlay components.
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

        // Draw "Click to continue" hint.
        fenetre.draw(continueText);
    }
}
