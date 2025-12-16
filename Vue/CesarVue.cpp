//
// Created by bertr on 14-12-25.
//

#include "CesarVue.h"

#include <iostream>

// Constructor accepts a pointer to the objective so the original can be modified
CesarVue::CesarVue(Objective* objective)
    : objective(objective)
{
    // Font
    font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");

    // Text
    text.setFont(font);
    if (objective) {
        text.setString("The scripted code is "
            + objective->calculateAlteredCode()
            + ". The number to use is "
            + std::to_string(objective->getChangeValue())
            + ". What is the correct word?");
    } else {
        text.setString("Cesar puzzle");
    }
    text.setCharacterSize(40);
    text.setFillColor(sf::Color::Red);
    text.setStyle(sf::Text::Bold);
    text.setPosition(10.f, 10.f);

    // Initialize exit button
    exitButton.setSize(sf::Vector2f(150.f, 50.f));
    exitButton.setFillColor(sf::Color::Blue);
    exitButton.setOutlineThickness(2.f);
    exitButton.setOutlineColor(sf::Color::White);

    exitButtonText.setFont(font);
    exitButtonText.setString("Sortir");
    exitButtonText.setCharacterSize(20);
    exitButtonText.setFillColor(sf::Color::White);
}

CesarVue::~CesarVue() {
    if (objective) {
        std::cout << "The CesarVue from the objective "
        << objective->getTitle()
        << " has been destroyed."
        << std::endl;
    }
}

void CesarVue::setObjective(Objective* o) {
    objective = o;
}
Objective* CesarVue::getObjective() const {
    return objective;
}

void CesarVue::draw(sf::RenderWindow &window) {
    //  Get the center of the screen
    sf::Vector2u size = window.getSize();
    float centerX = size.x / 2.0f;
    float centerY = size.y / 2.0f;

    // Position the description text at the top center
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setPosition(centerX - textBounds.width / 2.0f, 50.f);
    window.draw(text);

    // If validation succeeded, show success message
    if (isValidated) {
        sf::Text successMessage;
        successMessage.setFont(font);
        successMessage.setString(objective ? (objective->getTitle() + " ouvert !") : "Ordinateur deverrouille !");
        successMessage.setCharacterSize(50);
        successMessage.setFillColor(sf::Color::Green);
        sf::FloatRect successBounds = successMessage.getLocalBounds();
        successMessage.setPosition(centerX - successBounds.width / 2.0f, centerY - 100.f);
        window.draw(successMessage);
    } else {
        // Draw input box (rectangle for user input)
        inputBox.setSize(sf::Vector2f(400.f, 60.f));
        inputBox.setPosition(centerX - 200.f, centerY + 100.f);
        inputBox.setFillColor(sf::Color::Black);
        inputBox.setOutlineThickness(2.f);
        inputBox.setOutlineColor(sf::Color::White);
        window.draw(inputBox);

        // Draw input text
        inputText.setFont(font);
        inputText.setCharacterSize(30);
        inputText.setFillColor(sf::Color::White);
        inputText.setPosition(centerX - 190.f, centerY + 110.f);
        window.draw(inputText);

        // Draw instruction text
        sf::Text instructionText;
        instructionText.setFont(font);
        instructionText.setString("Press Enter to submit");
        instructionText.setCharacterSize(20);
        instructionText.setFillColor(sf::Color::Green);
        instructionText.setPosition(centerX - 200.f, centerY + 200.f);
        window.draw(instructionText);
    }

    // Draw exit button (always visible)
    exitButton.setPosition(centerX - 75.f, centerY + 300.f);
    exitButton.setFillColor(sf::Color::Blue);
    window.draw(exitButton);
    
    // Position and draw exit button text
    sf::FloatRect buttonTextBounds = exitButtonText.getLocalBounds();
    exitButtonText.setPosition(
        centerX - buttonTextBounds.width / 2.0f,
        centerY + 310.f
    );
    window.draw(exitButtonText);
}

void CesarVue::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    // Handle window close events
    if (event.type == sf::Event::Closed) {
        window.close();
        shouldClose = true;
    }
    
    // Handle mouse click on exit button (always clickable)
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
            if (exitButton.getGlobalBounds().contains(mousePos)) {
                std::cout << "Exit button clicked - Closing CesarVue" << std::endl;
                shouldClose = true;
            }
        }
    }
    
    // Handle key press events (e.g., Escape to close the window)
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            // Close CesarVue window
            std::cout << "CesarVue closed via Escape key" << std::endl;
            shouldClose = true;
        } else if (event.key.code == sf::Keyboard::Enter && !isValidated) {
            // Validate the input code (only if not already validated)
            std::string userInput = inputText.getString();
            if (objective && userInput == objective->getCode()) {
                objective->setAccomplished(true);
                isValidated = true;
                validationMessage = "Ordinateur deverrouille !";
                std::cout << "✓ Correct code entered! Objective accomplished." << std::endl;
            } else {
                // Wrong code
                if (objective) std::cout << "✗ Wrong code. Expected: " << objective->getCode() 
                          << ", got: " << userInput << std::endl;
                inputText.setString("");  // Clear input
            }
        }
    }
    
    // Handle text input for the code input field (only if not validated)
    if (event.type == sf::Event::TextEntered && !isValidated) {
        if (event.text.unicode < 128) {  // ASCII characters only
            char c = static_cast<char>(event.text.unicode);

            // Handle backspace
            if (c == '\b') {
                if (!inputText.getString().isEmpty()) {
                    std::string str = inputText.getString();
                    str.pop_back();
                    inputText.setString(str);
                }
            }
            // Handle regular characters
            else if (c >= 32 && c < 127) {
                std::string str = inputText.getString();
                str += c;
                inputText.setString(str);
            }
        }
    }
}
