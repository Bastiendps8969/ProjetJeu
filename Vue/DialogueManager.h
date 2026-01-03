
#pragma once

#include "DialogueBox.h"
#include <map>
#include <vector>

namespace Vue
{
    // DialogueManager:
    // - Loads dialogue sequences from a JSON file.
    // - Stores sequences in a map: sequenceId -> vector<DialogueData>.
    // - Controls progression through a sequence (current id + index).
    // - Delegates rendering and UI behavior to DialogueBox.
    class DialogueManager
    {
    private:
        // UI component that actually displays one dialogue at a time.
        // Owned by the manager (composition) for simpler lifetime management.
        DialogueBox dialogueBox;

        // Dialogue sequences indexed by an identifier.
        // std::map provides deterministic ordering (useful for debug).
        std::map<std::string, std::vector<DialogueData>> dialogueSequences;

        // Current position within the active sequence.
        size_t currentSequenceIndex = 0;

        // Id of the currently active sequence (empty if none).
        std::string currentSequenceId;

        // Load and parse dialogue sequences from a JSON file.
        void loadDialoguesFromJSON(const std::string& filePath);

    public:
        // Constructor loads dialogues from JSON file.
        // By default it uses "Asset/dialogues/dialogues.json".
        DialogueManager(const std::string& dialogueFilePath = "Asset/dialogues/dialogues.json");

        // Start playing a dialogue sequence by id (if it exists and is not empty).
        void startDialogueSequence(const std::string& sequenceId);

        // Check if a dialogue sequence exists and contains at least one entry.
        bool hasDialogueSequence(const std::string& sequenceId) const;

        // Advance to the next dialogue within the current sequence,
        // or end the dialogue if the sequence is finished.
        void nextDialogue();

        // True while the dialogue box is visible/active.
        bool isDialogueActive() const { return dialogueBox.isActive(); }

        // Handle input events (click / key) and forward to DialogueBox.
        void handleEvent(const sf::Event& event);

        // Draw the dialogue box overlay.
        void draw(sf::RenderWindow& fenetre);

        // Update (e.g., auto-advance when the dialogue should close).
        // Parameter windowSize exists for potential layout adaptations.
        void update(const sf::Vector2u& windowSize);

        // Public helper to add sequences programmatically (useful for tests or scripted content).
        void addDialogueSequence(const std::string& id, const std::vector<DialogueData>& sequence);
    };
}
