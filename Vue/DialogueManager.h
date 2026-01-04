
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
        //
        // WHY composition (member by value) instead of pointer:
        // - DialogueManager always needs a DialogueBox to function
        // - no dynamic allocation needed, simpler ownership
        // - avoids null checks and reduces lifetime/ownership bugs
        DialogueBox dialogueBox;

        // Dialogue sequences indexed by an identifier.
        // std::map provides deterministic ordering (useful for debug).
        //
        // WHY map<string, vector<DialogueData>>:
        // - fast lookup by sequence id
        // - each sequence is an ordered list (vector) of DialogueData entries
        // - manager owns all dialogue content loaded from JSON (composition)
        std::map<std::string, std::vector<DialogueData>> dialogueSequences;

        // Current position within the active sequence.
        size_t currentSequenceIndex = 0;

        // Id of the currently active sequence (empty if none).
        std::string currentSequenceId;

        // Load and parse dialogue sequences from a JSON file.
        //
        // WHY filePath by const reference:
        // - avoids copying string data
        // - read-only intent
        void loadDialoguesFromJSON(const std::string& filePath);

    public:
        // Constructor loads dialogues from JSON file.
        // By default it uses "Asset/dialogues/dialogues.json".
        //
        // WHY parameter by const reference with default:
        // - avoids copying
        // - default argument supports typical use without specifying a path
        DialogueManager(const std::string& dialogueFilePath = "Asset/dialogues/dialogues.json");

        // Start playing a dialogue sequence by id (if it exists and is not empty).
        // WHY sequenceId by const reference:
        // - avoids copying the string
        void startDialogueSequence(const std::string& sequenceId);

        // Check if a dialogue sequence exists and contains at least one entry.
        bool hasDialogueSequence(const std::string& sequenceId) const;

        // Advance to the next dialogue within the current sequence,
        // or end the dialogue if the sequence is finished.
        void nextDialogue();

        // True while the dialogue box is visible/active.
        bool isDialogueActive() const { return dialogueBox.isActive(); }

        // Handle input events (click / key) and forward to DialogueBox.
        // WHY event by const reference:
        // - event is read-only and can be non-trivial to copy
        void handleEvent(const sf::Event& event);

        // Draw the dialogue box overlay.
        // WHY RenderWindow by non-const reference:
        // - drawing mutates the window (draw calls)
        void draw(sf::RenderWindow& fenetre);

        // Update (e.g., auto-advance when the dialogue should close).
        // Parameter windowSize exists for potential layout adaptations.
        // WHY windowSize by const reference:
        // - avoids copying small struct; expresses read-only intent
        void update(const sf::Vector2u& windowSize);

        // Public helper to add sequences programmatically (useful for tests or scripted content).
        // WHY id and sequence by const reference:
        // - avoids copying caller data on entry (but note: current impl copies into the map)
        void addDialogueSequence(const std::string& id, const std::vector<DialogueData>& sequence);
    };
}
