
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace Vue
{
    // DialogueData:
    // Lightweight data container for a single dialogue line.
    // - characterName: name displayed in the dialogue box
    // - characterPortraitPath: path to portrait image (optional)
    // - text: dialogue content
    // - displayDuration: duration (seconds) intended for auto-advance
    struct DialogueData
    {
        std::string characterName;
        std::string characterPortraitPath;
        std::string text;
        float displayDuration;
    };

    // DialogueBox:
    // UI widget that renders a dialogue overlay at the bottom of the screen.
    // It shows:
    // - a semi-transparent background panel
    // - character portrait (or placeholder)
    // - character name
    // - dialogue text
    // - a "[Click to continue]" hint
    //
    // The box is "modal-like": it remains open until the user clicks / presses Space.
    class DialogueBox
    {
    private:
        // Whether the dialogue box is currently displayed.
        bool active = false;

        // While true, we are waiting for user input to continue.
        // Once it becomes false, shouldClose() returns true.
        bool waitingForClick = false;

        // Timer restarted on each dialogue (could be used with displayDuration for auto-close).
        sf::Clock displayTimer;

        // Font for all texts.
        sf::Font font;
        bool fontLoaded = false;

        // Visual elements (background panel, portrait, texts).
        sf::RectangleShape backgroundBox;

        // Portrait resources owned by the DialogueBox (composition).
        // WHY store sf::Texture as a member:
        // - sf::Sprite references a texture; the texture must outlive the sprite
        // - keeping texture as a member guarantees correct lifetime
        sf::Texture portraitTexture;
        sf::Sprite portraitSprite;
        bool portraitLoaded = false;

        // Debug-friendly placeholder when portrait is missing (magenta).
        sf::RectangleShape portraitPlaceholder;

        // Fixed portrait drawing area size.
        sf::Vector2f portraitSize = sf::Vector2f(160.f, 160.f);

        // Text elements.
        sf::Text nameText;
        sf::Text dialogueText;
        sf::Text continueText;

        // Current dialogue data being displayed.
        // WHY store a copy:
        // - keeps DialogueBox independent from the caller's data lifetime
        // - allows rebuilding layout at any time without external references
        DialogueData currentDialogue;

        // Initialize UI layout based on current window size.
        // WHY windowSize by const reference:
        // - read-only and avoids copying
        void initializeDialogueBox(const sf::Vector2u& windowSize);

        // Declared but not implemented in the provided .cpp:
        // likely intended for dynamic updates (wrapping, repositioning).
        void updateDialogueDisplay(const sf::Vector2u& windowSize);

    public:
        DialogueBox();

        // Active state getter.
        bool isActive() const { return active; }

        // Start displaying a new dialogue entry (copies data into internal state).
        // WHY dialogue by const reference:
        // - avoids copying on function entry
        // - internal copy still happens to own the data safely
        void startDialogue(const DialogueData& dialogue, const sf::Vector2u& windowSize);

        // End the dialogue immediately.
        void endDialogue() { active = false; }

        // Indicates whether the dialogue box should close / advance to next entry.
        // Current logic: closes only after user interaction (click/space).
        bool shouldClose() const;

        // Expose whether we are waiting for the next click.
        bool isWaitingForClick() const { return waitingForClick; }

        // Handle a click (or equivalent input) and mark as ready to close.
        void handleMouseClick() { waitingForClick = false; }

        // Handle SFML events (mouse click / key press).
        // WHY event by const reference:
        // - read-only and avoids copying
        void handleEvent(const sf::Event& event);

        // Draw the dialogue box overlay.
        // WHY RenderWindow by non-const reference:
        // - drawing modifies the render target state
        void draw(sf::RenderWindow& fenetre);
    };
}
