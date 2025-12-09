#include "DialogueManager.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace Vue
{
    DialogueManager::DialogueManager(const std::string& dialogueFilePath)
    {
        loadDialoguesFromJSON(dialogueFilePath);
    }

    void DialogueManager::loadDialoguesFromJSON(const std::string& filePath)
    {
        try
        {
            // Essayer plusieurs chemins possibles
            std::vector<std::string> possiblePaths = {
                filePath,
                "../" + filePath,
                "../../" + filePath,
                "C:/Users/bertr/CLionProjects/TestCollision/" + filePath
            };

            std::ifstream file;
            std::string foundPath;

            for (const auto& path : possiblePaths)
            {
                file.open(path);
                if (file.is_open())
                {
                    foundPath = path;
                    std::cout << "Fichier JSON trouvé: " << foundPath << std::endl;
                    break;
                }
            }

            if (!file.is_open())
            {
                std::cerr << "Erreur: impossible d'ouvrir le fichier JSON" << std::endl;
                std::cerr << "Répertoire courant: " << std::filesystem::current_path() << std::endl;
                return;
            }

            json jsonData;
            file >> jsonData;
            file.close();

            // Parcourir chaque séquence de dialogues
            for (auto& [sequenceId, dialogueArray] : jsonData["dialogues"].items())
            {
                std::vector<DialogueData> sequence;

                for (const auto& dialogueObj : dialogueArray)
                {
                    DialogueData dialogue;
                    dialogue.characterName = dialogueObj["characterName"].get<std::string>();
                    dialogue.characterPortraitPath = dialogueObj["characterPortraitPath"].get<std::string>();
                    dialogue.text = dialogueObj["text"].get<std::string>();
                    dialogue.displayDuration = dialogueObj["displayDuration"].get<float>();

                    sequence.push_back(dialogue);
                }

                addDialogueSequence(sequenceId, sequence);
            }

            std::cout << "Dialogues chargés avec succès depuis " << foundPath << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Erreur lors du chargement des dialogues: " << e.what() << std::endl;
        }
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
            sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
            sf::Vector2u windowSize(desktopMode.width, desktopMode.height);
            dialogueBox.startDialogue(it->second[0], windowSize);
        }
    }

    void DialogueManager::nextDialogue()
    {
        auto it = dialogueSequences.find(currentSequenceId);
        if (it != dialogueSequences.end())
        {
            currentSequenceIndex++;
            if (currentSequenceIndex < it->second.size())
            {
                sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
                sf::Vector2u windowSize(desktopMode.width, desktopMode.height);
                dialogueBox.startDialogue(it->second[currentSequenceIndex], windowSize);
            }
            else
            {
                dialogueBox.endDialogue();
                currentSequenceIndex = 0;
            }
        }
    }

    void DialogueManager::handleEvent(const sf::Event& event)
    {
        if (dialogueBox.isActive())
        {
            if ((event.type == sf::Event::MouseButtonPressed ||
                 event.type == sf::Event::KeyPressed) &&
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