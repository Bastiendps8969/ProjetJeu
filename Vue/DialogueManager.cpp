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
        // Par défaut, on force vers Asset/dialogues/dialogues.json
        std::string path = dialogueFilePath.empty() ? "Asset/dialogues/dialogues.json" : dialogueFilePath;
        loadDialoguesFromJSON(path);
    }

    void DialogueManager::loadDialoguesFromJSON(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "DialogueManager: unable to open JSON file at " << filePath << std::endl;
            return;
        }

        json jsonData;
        file >> jsonData;
        file.close();

        // Lecture des dialogues
        for (auto& [sequenceId, dialogueArray] : jsonData["dialogues"].items())
        {
            std::vector<DialogueData> sequence;
            for (const auto& dialogueObj : dialogueArray)
            {
                DialogueData dialogue;
                dialogue.characterName = dialogueObj.value("characterName", std::string());

                // Correction du chemin du portrait
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

                dialogue.text = dialogueObj.value("text", std::string());
                dialogue.displayDuration = dialogueObj.value("displayDuration", 0.0f);

                sequence.push_back(dialogue);
            }
            addDialogueSequence(sequenceId, sequence);
        }

        std::cout << "DialogueManager: loaded dialogues from " << filePath << std::endl;
    }

    void DialogueManager::addDialogueSequence(const std::string& id, const std::vector<DialogueData>& sequence)
    {
        dialogueSequences[id] = sequence;
    }

    void DialogueManager::startDialogueSequence(const std::string& sequenceId)
    {
        auto it = dialogueSequences.find(sequenceId);
        if (it != dialogueSequences.end() && !it->second.empty())
        {
            currentSequenceId = sequenceId;
            currentSequenceIndex = 0;
            sf::VideoMode dm = sf::VideoMode::getDesktopMode();
            sf::Vector2u windowSize(static_cast<unsigned int>(dm.width), static_cast<unsigned int>(dm.height));
            dialogueBox.startDialogue(it->second[0], windowSize);
        }
    }

    void DialogueManager::nextDialogue()
    {
        auto it = dialogueSequences.find(currentSequenceId);
        if (it == dialogueSequences.end()) return;

        currentSequenceIndex++;
        if (currentSequenceIndex < it->second.size())
        {
            sf::VideoMode dm = sf::VideoMode::getDesktopMode();
            sf::Vector2u windowSize(static_cast<unsigned int>(dm.width), static_cast<unsigned int>(dm.height));
            dialogueBox.startDialogue(it->second[currentSequenceIndex], windowSize);
        }
        else
        {
            dialogueBox.endDialogue();
            currentSequenceIndex = 0;
            currentSequenceId.clear();
        }
    }

    void DialogueManager::handleEvent(const sf::Event& event)
    {
        if (dialogueBox.isActive())
        {
            if ((event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::KeyPressed) &&
                dialogueBox.shouldClose())
            {
                nextDialogue();
                return;
            }
        }
        dialogueBox.handleEvent(event);
    }

    void DialogueManager::draw(sf::RenderWindow& fenetre)
    {
        dialogueBox.draw(fenetre);
    }

    void DialogueManager::update(const sf::Vector2u& windowSize)
    {
        if (dialogueBox.isActive() && dialogueBox.shouldClose())
        {
            nextDialogue();
        }
    }
}
