
#include "DialogueManager.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include "../cmake-build-debug/json.hpp"

using json = nlohmann::json;

namespace Vue
{
    DialogueManager::DialogueManager(const std::string& dialogueFilePath)
    {
        // If caller provides an empty string, fallback to default JSON path.
        std::string path = dialogueFilePath.empty() ? "Asset/dialogues/dialogues.json" : dialogueFilePath;
        loadDialoguesFromJSON(path);
    }

    void DialogueManager::loadDialoguesFromJSON(const std::string& filePath)
    {
        // Open JSON file.
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
        for (auto& [sequenceId, dialogueArray] : jsonData["dialogues"].items())
        {
            std::vector<DialogueData> sequence;

            // Build each DialogueData entry from JSON fields.
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
        dialogueSequences[id] = sequence;
    }

    bool DialogueManager::hasDialogueSequence(const std::string& sequenceId) const
    {
        // A valid sequence exists if found and non-empty.
        auto it = dialogueSequences.find(sequenceId);
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
            sf::Vector2u windowSize(static_cast<unsigned int>(dm.width),
                                    static_cast<unsigned int>(dm.height));

            // Display the first dialogue entry.
            dialogueBox.startDialogue(it->second[0], windowSize);
        }
    }

    void DialogueManager::nextDialogue()
    {
        // Find current sequence.
        auto it = dialogueSequences.find(currentSequenceId);
        if (it == dialogueSequences.end()) return;

        // Move to next entry.
        currentSequenceIndex++;

        if (currentSequenceIndex < it->second.size())
        {
            // Use desktop resolution (assumes fullscreen).
            sf::VideoMode dm = sf::VideoMode::getDesktopMode();
            sf::Vector2u windowSize(static_cast<unsigned int>(dm.width),
                                    static_cast<unsigned int>(dm.height));

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
            if ((event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::KeyPressed) &&
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
        if (dialogueBox.isActive() && dialogueBox.shouldClose())
        {
            nextDialogue();
        }
    }
}
