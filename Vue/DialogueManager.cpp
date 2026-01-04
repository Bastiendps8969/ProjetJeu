
#include "DialogueManager.h"

#include <fstream>
#include <iostream>
#include <filesystem>

// NOTE: this include path is project-specific in your codebase.
#include "../cmake-build-debug/json.hpp"

using json = nlohmann::json;

namespace Vue
{
    DialogueManager::DialogueManager(const std::string& dialogueFilePath)
    {
        // If caller provides an empty string, fallback to default JSON path.
        //
        // WHY make a local std::string by value:
        // - we need a normalized "effective path" to pass to the loader
        // - local value is convenient to modify and keeps a stable lifetime
        std::string path = dialogueFilePath.empty()
            ? "Asset/dialogues/dialogues.json"
            : dialogueFilePath;

        loadDialoguesFromJSON(path);
    }

    void DialogueManager::loadDialoguesFromJSON(const std::string& filePath)
    {
        // Open JSON file.
        //
        // WHY filePath by const reference:
        // - avoid copying path string
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "DialogueManager: unable to open JSON file at " << filePath << std::endl;
            return;
        }

        // Parse JSON using nlohmann::json.
        json jsonData;
        file >> jsonData;
        file.close();

        // Read all dialogue sequences under jsonData["dialogues"].
        // WHY structured binding by reference (auto& [id, array]):
        // - avoids copying JSON keys/values
        // - iterates efficiently through the JSON object items
        for (auto& [sequenceId, dialogueArray] : jsonData["dialogues"].items())
        {
            std::vector<DialogueData> sequence;

            // Build each DialogueData entry from JSON fields.
            // WHY const auto& dialogueObj:
            // - avoid copying JSON nodes
            for (const auto& dialogueObj : dialogueArray)
            {
                DialogueData dialogue;

                // Character name (optional).
                dialogue.characterName = dialogueObj.value("characterName", std::string());

                // Portrait path normalization:
                // If the JSON provides only a filename, prefix it with "Asset/portraits/".
                std::string portrait = dialogueObj.value("characterPortraitPath", std::string());
                if (!portrait.empty() &&
                    portrait.find("Asset/portraits/") == std::string::npos &&
                    portrait.find("assets/portraits/") == std::string::npos)
                {
                    dialogue.characterPortraitPath = "Asset/portraits/" + portrait;
                }
                else
                {
                    dialogue.characterPortraitPath = portrait;
                }

                // Dialogue text and display duration (optional).
                dialogue.text = dialogueObj.value("text", std::string());
                dialogue.displayDuration = dialogueObj.value("displayDuration", 0.0f);

                // WHY push_back by value:
                // - DialogueData is a small aggregate of strings/numbers
                // - stored in the sequence vector owned by the manager
                sequence.push_back(dialogue);
            }

            // Store the full sequence.
            addDialogueSequence(sequenceId, sequence);
        }

        std::cout << "DialogueManager: loaded dialogues from " << filePath << std::endl;
    }

    void DialogueManager::addDialogueSequence(const std::string& id, const std::vector<DialogueData>& sequence)
    {
        // Copy/assign the sequence into the map under the given id.
        //
        // WHY sequence is copied here (current implementation):
        // - simple and safe (dialogueSequences owns its content)
        // - caller can pass a temporary/local vector without lifetime issues
        // NOTE: if sequences become large, consider taking sequence by value
        //       and moving into the map to avoid a deep copy.
        dialogueSequences[id] = sequence;
    }

    bool DialogueManager::hasDialogueSequence(const std::string& sequenceId) const
    {
        // A valid sequence exists if found and non-empty.
        auto it = dialogueSequences.find(sequenceId);

        // WHY iterator access:
        // - avoids copying the stored vector
        // - checks existence + emptiness efficiently
        return it != dialogueSequences.end() && !it->second.empty();
    }

    void DialogueManager::startDialogueSequence(const std::string& sequenceId)
    {
        // Only start if sequence exists and has at least one dialogue.
        auto it = dialogueSequences.find(sequenceId);
        if (it != dialogueSequences.end() && !it->second.empty())
        {
            currentSequenceId = sequenceId;
            currentSequenceIndex = 0;

            // Use desktop resolution as window size (assumes fullscreen usage).
            sf::VideoMode dm = sf::VideoMode::getDesktopMode();
            sf::Vector2u windowSize(
                static_cast<unsigned int>(dm.width),
                static_cast<unsigned int>(dm.height)
            );

            // Display the first dialogue entry.
            // WHY pass DialogueData as value/reference (depends on DialogueBox API):
            // - DialogueBox needs the data to build UI (text, portrait, duration)
            // - using the current sequence entry avoids duplicating storage here
            dialogueBox.startDialogue(it->second[0], windowSize);
        }
    }

    void DialogueManager::nextDialogue()
    {
        // Find current sequence.
        auto it = dialogueSequences.find(currentSequenceId);
        if (it == dialogueSequences.end())
            return;

        // Move to next entry.
        ++currentSequenceIndex;

        if (currentSequenceIndex < it->second.size())
        {
            // Use desktop resolution (assumes fullscreen).
            sf::VideoMode dm = sf::VideoMode::getDesktopMode();
            sf::Vector2u windowSize(
                static_cast<unsigned int>(dm.width),
                static_cast<unsigned int>(dm.height)
            );

            // Display the next dialogue entry.
            dialogueBox.startDialogue(it->second[currentSequenceIndex], windowSize);
        }
        else
        {
            // End sequence: close dialogue box and reset state.
            dialogueBox.endDialogue();
            currentSequenceIndex = 0;
            currentSequenceId.clear();
        }
    }

    void DialogueManager::handleEvent(const sf::Event& event)
    {
        // If the dialogue box is active and ready to close,
        // any mouse click or key press advances to the next dialogue.
        if (dialogueBox.isActive())
        {
            // FIXED: restored "||" which was broken in extraction.
            if ((event.type == sf::Event::MouseButtonPressed ||
                 event.type == sf::Event::KeyPressed) &&
                dialogueBox.shouldClose())
            {
                nextDialogue();
                return;
            }
        }

        // Forward event handling to the dialogue box (for internal UI logic).
        dialogueBox.handleEvent(event);
    }

    void DialogueManager::draw(sf::RenderWindow& fenetre)
    {
        // Delegate drawing to DialogueBox.
        dialogueBox.draw(fenetre);
    }

    void DialogueManager::update(const sf::Vector2u& windowSize)
    {
        // Auto-advance when dialogue is active and should close (e.g., timer finished).
        // NOTE: windowSize is currently unused but kept for possible layout updates.
        (void)windowSize;

        if (dialogueBox.isActive() && dialogueBox.shouldClose())
        {
            nextDialogue();
        }
    }
}
