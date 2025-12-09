#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace Vue
{
    struct DialogueData
    {
        std::string characterName;
        std::string characterPortraitPath;
        std::string text;
        float displayDuration;
    };

    class DialogueBox
    {
    private:
        bool active = false;
        bool waitingForClick = false;
        sf::Clock displayTimer;

        sf::Font font;
        bool fontLoaded = false;

        // Éléments visuels
        sf::RectangleShape backgroundBox;
        sf::Texture portraitTexture;
        sf::Sprite portraitSprite;
        bool portraitLoaded = false;
        sf::RectangleShape portraitPlaceholder;
        sf::Vector2f portraitSize = sf::Vector2f(160.f, 160.f);
        
        sf::Text nameText;
        sf::Text dialogueText;
        sf::Text continueText;

        // Données du dialogue actuel
        DialogueData currentDialogue;

        void initializeDialogueBox(const sf::Vector2u& windowSize);
        void updateDialogueDisplay(const sf::Vector2u& windowSize);

    public:
        DialogueBox();

        bool isActive() const { return active; }

        void startDialogue(const DialogueData& dialogue, const sf::Vector2u& windowSize);
        void endDialogue() { active = false; }

        bool shouldClose() const;
        bool isWaitingForClick() const { return waitingForClick; }
        void handleMouseClick() { waitingForClick = false; }
        void handleEvent(const sf::Event& event);
        void draw(sf::RenderWindow& fenetre);
    };
}