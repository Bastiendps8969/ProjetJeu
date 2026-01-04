//
// Created by bertr on 14-12-25.
//

#include "CesarVue.h"

#include <iostream>
#include <sstream>
#include <cctype>

// Constructor accepts a pointer to the objective so the original can be modified
CesarVue::CesarVue(Objective* objective)
    : objective(objective)
{
    // Font
    font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");

    // Background image for the in-game PC (search both src and build locations)
    std::vector<std::string> bgCandidates = {
        "Asset/cesar/pc_cesar.png",
        "cmake-build-debug/Asset/cesar/pc_cesar.png",
        "Asset/pc_background.png",
        "cmake-build-debug/Asset/pc_background.png"
    };
    for (auto &p : bgCandidates) {
        if (bgTexture.loadFromFile(p)) {
            bgSprite.setTexture(bgTexture);
            bgLoaded = true;
            break;
        }
    }
    if (!bgLoaded) {
        std::cerr << "CesarVue: background 'Asset/cesar/pc_cesar.png' not found. Using plain UI." << std::endl;
    }

    // Text
    text.setFont(font);
    if (objective) {
        text.setString("The scripted code is "
            + objective->calculateAlteredCode()
            + ". The number used is "
            + std::to_string(objective->getChangeValue())
            + ". What is the original word?");
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
    exitButtonText.setString("Exit");
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
    // compute window center
    sf::Vector2u win = window.getSize();
    float cx = win.x / 2.f;
    float cy = win.y / 2.f;

    // Compute inner screen rectangle (from PC image if available)
    sf::FloatRect screenRect;
    if (bgLoaded && bgSprite.getTexture()) {
        sf::FloatRect tb = bgSprite.getLocalBounds();
        float scaleX = float(win.x) / tb.width;
        float scaleY = float(win.y) / tb.height;
        float scale = std::min(scaleX, scaleY);
        bgSprite.setScale(scale, scale);
        bgSprite.setOrigin(tb.width/2.f, tb.height/2.f);
        bgSprite.setPosition(cx, cy);
        window.draw(bgSprite);

        sf::FloatRect gb = bgSprite.getGlobalBounds();
        // use padding to find the actual screen area inside the monitor graphic
        float padX = gb.width * 0.12f;
        float padY = gb.height * 0.12f;
        screenRect = sf::FloatRect(gb.left + padX, gb.top + padY, gb.width - 2*padX, gb.height - 2*padY);
    } else {
        // fallback to centered box
        float w = std::min(900.f, float(win.x) - 200.f);
        float h = std::min(600.f, float(win.y) - 200.f);
        screenRect = sf::FloatRect(cx - w/2.f, cy - h/2.f, w, h);
    }

    // Wrap the descriptive red text so it fits inside the screenRect and center it
    {
        text.setFont(font);

        // Aggressive wrapping: try decreasing font sizes until everything fits within a narrower max width
        unsigned int startSize = 40;
        unsigned int minSize = 18;
        float maxW = screenRect.width - 80.f; // more padding to force more line breaks
        std::string src = text.getString().toAnsiString();

        std::string finalWrapped;
        unsigned int usedSize = startSize;

        for (int size = startSize; size >= int(minSize); size -= 2) {
            sf::Text tmpBase = text;
            tmpBase.setCharacterSize(size);

            std::istringstream iss(src);
            std::string word;
            std::string line;
            std::string wrapped;

            while (iss >> word) {
                std::string candidate = line.empty() ? word : (line + " " + word);
                tmpBase.setString(candidate);
                if (tmpBase.getLocalBounds().width > maxW) {
                    if (!wrapped.empty()) wrapped += '\n';
                    wrapped += line;
                    line = word;
                } else {
                    line = candidate;
                }
            }
            if (!line.empty()) {
                if (!wrapped.empty()) wrapped += '\n';
                wrapped += line;
            }

            // measure the longest line
            float longest = 0.f;
            sf::Text meas = tmpBase;
            std::istringstream lines(wrapped);
            std::string l;
            while (std::getline(lines, l)) {
                meas.setString(l);
                longest = std::max(longest, meas.getLocalBounds().width);
            }

            if (longest <= maxW) {
                finalWrapped = wrapped;
                usedSize = size;
                break;
            }
        }

        if (finalWrapped.empty()) {
            // fallback: use original text but mark it at min size
            finalWrapped = src;
            usedSize = minSize;
        }

        text.setCharacterSize(usedSize);
        text.setString(finalWrapped);

        // Center the multi-line description slightly lower to be more centered vertically
        sf::FloatRect descBounds = text.getLocalBounds();
        float descY = screenRect.top + screenRect.height * 0.20f; // move down from top
        text.setPosition(screenRect.left + (screenRect.width - descBounds.width) / 2.f, descY);
        window.draw(text);

        // compute Y position below the description for the altered code
        float belowY = descY + descBounds.height + 8.f;

        // Display the altered code larger but scale down if needed to keep it inside screenRect
        std::string altered = objective ? objective->calculateAlteredCode() : std::string("-----");
        alteredCodeText.setFont(font);

        unsigned int targetSize = 56; // larger than before
        alteredCodeText.setCharacterSize(targetSize);
        alteredCodeText.setFillColor(sf::Color::White);
        alteredCodeText.setString(altered);

        // Reduce font size until it fits the available width (with padding)
        sf::FloatRect aB = alteredCodeText.getLocalBounds();
        while ((aB.width > screenRect.width - 40.f) && targetSize > 18) {
            targetSize -= 2;
            alteredCodeText.setCharacterSize(targetSize);
            aB = alteredCodeText.getLocalBounds();
        }

        alteredCodeText.setPosition(screenRect.left + (screenRect.width - aB.width)/2.f, belowY);
        window.draw(alteredCodeText);
    }

    // Input box: centered horizontally, more centered vertically
    float inputW = std::max(300.f, screenRect.width * 0.6f);
    inputBox.setSize(sf::Vector2f(inputW, 56.f));
    float inputY = screenRect.top + (screenRect.height * 0.5f) - (56.f / 2.f);
    inputBox.setPosition(screenRect.left + (screenRect.width - inputW)/2.f, inputY);
    inputBox.setFillColor(sf::Color(16,18,20));
    inputBox.setOutlineThickness(2.f);
    inputBox.setOutlineColor(sf::Color(100,100,100));
    window.draw(inputBox);

    // Input text inside box
    inputText.setFont(font);
    inputText.setCharacterSize(28);
    inputText.setFillColor(sf::Color::White);
    inputText.setPosition(inputBox.getPosition().x + 12.f, inputBox.getPosition().y + 10.f);
    window.draw(inputText);

    // Instruction text below input (centered)
    sf::Text instructionText;
    instructionText.setFont(font);
    instructionText.setString("Press Enter to submit");
    instructionText.setCharacterSize(16);
    instructionText.setFillColor(sf::Color(150, 255, 150));
    sf::FloatRect instrBounds = instructionText.getLocalBounds();
    instructionText.setPosition(screenRect.left + (screenRect.width - instrBounds.width)/2.f, inputBox.getPosition().y + inputBox.getSize().y + 12.f);
    window.draw(instructionText);

    // Success message when validated (centered above the input box)
    if (isValidated) {
        sf::Text vm;
        vm.setFont(font);
        vm.setString(validationMessage);
        vm.setCharacterSize(28);
        vm.setFillColor(sf::Color(150, 255, 150));
        sf::FloatRect vmB = vm.getLocalBounds();
        vm.setPosition(screenRect.left + (screenRect.width - vmB.width) / 2.f, inputBox.getPosition().y - 56.f);
        window.draw(vm);
    }

    // Exit button: centered below input box
    if (exitButton.getSize().x <= 0.f) exitButton.setSize(sf::Vector2f(150.f, 48.f));
    float btnX = screenRect.left + (screenRect.width - exitButton.getSize().x) / 2.f;
    float btnY = inputBox.getPosition().y + inputBox.getSize().y + 36.f;
    exitButton.setPosition(btnX, btnY);
    window.draw(exitButton);

    // Exit text centered
    sf::FloatRect btnTextBounds = exitButtonText.getLocalBounds();
    exitButtonText.setPosition(btnX + (exitButton.getSize().x - btnTextBounds.width)/2.f, btnY + 12.f);
    window.draw(exitButtonText);

    // Brute results (if enabled) centered below the exit button
    if (showBruteResults) {
        float y = btnY + exitButton.getSize().y + 12.f;
        for (auto &t : bruteLines) {
            sf::FloatRect tb = t.getLocalBounds();
            float x = screenRect.left + (screenRect.width - tb.width) / 2.f;
            t.setPosition(x, y);
            window.draw(t);
            y += 22.f;
        }
    }
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
    
    // Small helper to validate current input
    auto validateInput = [&]() {
        std::string userInput = inputText.getString().toAnsiString();
        auto trim = [](std::string &s){
            while (!s.empty() && std::isspace((unsigned char)s.front())) s.erase(s.begin());
            while (!s.empty() && std::isspace((unsigned char)s.back())) s.pop_back();
        };
        trim(userInput);

        if (objective && userInput == objective->getCode()) {
            objective->setAccomplished(true);
            isValidated = true;
            validationMessage = "Computer unlocked!";
            std::cout << "✓ Correct code entered! Objective accomplished." << std::endl;
        } else {
            if (objective) std::cout << "✗ Wrong code. Expected: " << objective->getCode()
                      << ", got: " << userInput << std::endl;
            inputText.setString("");
        }
    };

    // Handle key press events (e.g., Escape to close the window)
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            // Close CesarVue window
            std::cout << "CesarVue closed via Escape key" << std::endl;
            shouldClose = true;
        }
        else if ((event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return) && !isValidated) {
            validateInput();
        }
    }

    // Handle text input for the code input field (only if not validated)
    if (event.type == sf::Event::TextEntered && !isValidated) {
        // If Enter/Return is sent as a text event, handle it here too
        if (event.text.unicode == '\r' || event.text.unicode == '\n') {
            validateInput();
            return;
        }

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
