#pragma once
#include "DialogueBox.h"
#include <map>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Vue
{
    class DialogueManager
    {
    private:
        DialogueBox dialogueBox;
        std::map<std::string, std::vector<DialogueData>> dialogueSequences;
        size_t currentSequenceIndex = 0;
        std::string currentSequenceId;

        void loadDialoguesFromJSON(const std::string& filePath);

    public:
        DialogueManager(const std::string& dialogueFilePath = "assets/dialogues/dialogues.json");

        void startDialogueSequence(const std::string& sequenceId);
        void nextDialogue();
        bool isDialogueActive() const { return dialogueBox.isActive(); }

        void handleEvent(const sf::Event& event);
        void draw(sf::RenderWindow& fenetre);
        void update(const sf::Vector2u& windowSize);

        // Ajouter des dialogues facilement
        void addDialogueSequence(const std::string& id, const std::vector<DialogueData>& sequence);
    };
}