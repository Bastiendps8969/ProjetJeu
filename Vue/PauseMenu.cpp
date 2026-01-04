
//
// Pause menu implementation
//

#include "PauseMenu.h"
#include <iostream>

using namespace Vue;

PauseMenu::PauseMenu(Modele::Modele& modele)
    : modelePtr(&modele)
{
    // Load font (Windows-specific path).
    fontLoaded = font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");

    // Title styling (match other menus).
    titleText.setFont(font);
    titleText.setString("PAUSE");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color(255,80,80));
    titleText.setStyle(sf::Text::Bold);

    // Shadow for consistency with other menu titles (simple offset + darker color).
    titleShadow = titleText;
    titleShadow.setFillColor(sf::Color(0,0,0,160));

    // Buttons (sizes and labels).
    resumeButton.setSize(sf::Vector2f(360.f, 72.f));
    resumeButton.setFillColor(buttonColor);
    resumeButton.setOutlineColor(sf::Color(255,100,100));
    resumeButton.setOutlineThickness(0.f);

    resumeLabel.setFont(font);
    resumeLabel.setString("Resume");
    resumeLabel.setCharacterSize(28);
    resumeLabel.setFillColor(textColor);
    resumeLabel.setStyle(sf::Text::Bold);

    exitLevelButton.setSize(sf::Vector2f(360.f, 72.f));
    exitLevelButton.setFillColor(buttonColor);
    exitLevelButton.setOutlineColor(sf::Color(255,100,100));
    exitLevelButton.setOutlineThickness(0.f);

    exitLevelLabel.setFont(font);
    exitLevelLabel.setString("Exit level");
    exitLevelLabel.setCharacterSize(28);
    exitLevelLabel.setFillColor(textColor);
    exitLevelLabel.setStyle(sf::Text::Bold);

    exitGameButton.setSize(sf::Vector2f(360.f, 72.f));
    exitGameButton.setFillColor(buttonColor);
    exitGameButton.setOutlineColor(sf::Color(255,100,100));
    exitGameButton.setOutlineThickness(0.f);

    exitGameLabel.setFont(font);
    exitGameLabel.setString("Exit game");
    exitGameLabel.setCharacterSize(28);
    exitGameLabel.setFillColor(textColor);
    exitGameLabel.setStyle(sf::Text::Bold);

    // WHY store modelePtr as pointer:
    // - PauseMenu does not own Modele; it just queries it (objectives list)
    // - pointer keeps dependency non-owning and allows null checks
}

void PauseMenu::initUI(sf::RenderWindow& fenetre)
{
    // Layout computed from window size.
    // Buttons are centered horizontally with a fixed baseline Y.
    sf::Vector2u sz = fenetre.getSize();
    float x = sz.x / 2.f;
    float y = 400.f;

    // Center title horizontally; fixed top margin.
    titleText.setPosition(x - titleText.getLocalBounds().width/2.f, 50.f);

    // Stack buttons vertically with spacing.
    resumeButton.setPosition(x - resumeButton.getSize().x/2.f, y);
    centerLabel(resumeLabel, resumeButton);

    exitLevelButton.setPosition(x - exitLevelButton.getSize().x/2.f, y + spaceBetweenButton);
    centerLabel(exitLevelLabel, exitLevelButton);

    exitGameButton.setPosition(x - exitGameButton.getSize().x/2.f, y + 2 * spaceBetweenButton);
    centerLabel(exitGameLabel, exitGameButton);
}

void PauseMenu::centerLabel(sf::Text& label, const sf::RectangleShape& button)
{
    // Center a text label inside a rectangle by using bounds.
    sf::FloatRect tb = label.getLocalBounds();
    sf::FloatRect bb = button.getGlobalBounds();

    label.setPosition(
        bb.left + (bb.width - tb.width)/2.f - tb.left,
        bb.top + (bb.height - tb.height)/2.f - tb.top
    );
}

// Reuse the same stylized button rendering used on HomePage to keep visual consistency.
static void drawStyledButton(sf::RenderWindow& window, sf::RectangleShape button, sf::Text label, bool hovered)
{
    // WHY button/label are passed by value:
    // - the function applies style modifications (colors/outlines/label position)
    // - passing by value avoids mutating the original objects owned by PauseMenu
    // - keeps the caller's "base" button geometry/labels unchanged across frames

    // Button base (color/outline depend on hover state).
    sf::RectangleShape base = button;
    sf::Color baseColor = hovered ? sf::Color(230, 60, 60) : sf::Color(170, 30, 30);
    base.setFillColor(baseColor);
    base.setOutlineColor(hovered ? sf::Color(255, 120, 80) : sf::Color(200, 80, 60));
    base.setOutlineThickness(hovered ? 4.f : 2.f);
    window.draw(base);

    // Hover glow effect.
    if (hovered)
    {
        sf::RectangleShape glow = button;
        glow.setFillColor(sf::Color::Transparent);
        glow.setOutlineColor(sf::Color(255, 160, 110, 200));
        glow.setOutlineThickness(6.f);
        window.draw(glow);
    }

    // Draw centered label.
    sf::FloatRect lb = label.getLocalBounds();
    label.setPosition(
        button.getPosition().x + (button.getSize().x - lb.width) / 2.f - lb.left,
        button.getPosition().y + (button.getSize().y - lb.height) / 2.f - lb.top
    );
    label.setFillColor(hovered ? sf::Color(255, 250, 240) : sf::Color(255, 220, 200));
    window.draw(label);
}

void PauseMenu::updateButtonColors()
{
    // Base fill color changes according to current selection.
    // Note: draw() also uses the stylized helper for hover/selection visuals.
    resumeButton.setFillColor(selectedOption == Option::Resume ? selectedColor : buttonColor);
    exitLevelButton.setFillColor(selectedOption == Option::ExitLevel ? selectedColor : buttonColor);
    exitGameButton.setFillColor(selectedOption == Option::ExitGame ? selectedColor : buttonColor);
}

void PauseMenu::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
{
    // If no font, UI cannot be rendered properly.
    if (!fontLoaded) return;

    // Keyboard navigation:
    // - Escape closes the pause menu (resume).
    // - Up/Down changes selection.
    // - Enter confirms selection and closes the menu.
    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Escape)
        {
            // Resume: close menu without changing selection.
            active = false;
            return;
        }
        else if (event.key.code == sf::Keyboard::Up)
        {
            if (selectedOption == Option::ExitGame) selectedOption = Option::ExitLevel;
            else if (selectedOption == Option::ExitLevel) selectedOption = Option::Resume;
        }
        else if (event.key.code == sf::Keyboard::Down)
        {
            if (selectedOption == Option::Resume) selectedOption = Option::ExitLevel;
            else if (selectedOption == Option::ExitLevel) selectedOption = Option::ExitGame;
        }
        else if (event.key.code == sf::Keyboard::Enter)
        {
            // Close menu and let the caller read selectedOption.
            active = false;
        }
    }

    // Mouse click: clicking a button selects it and closes the menu.
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        // WHY mapPixelToCoords:
        // - converts mouse pixel position to world coordinates (supports views/transforms)
        sf::Vector2f mousePos = fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre));

        if (resumeButton.getGlobalBounds().contains(mousePos)) {
            selectedOption = Option::Resume;
            active = false;
        }
        else if (exitLevelButton.getGlobalBounds().contains(mousePos)) {
            selectedOption = Option::ExitLevel;
            active = false;
        }
        else if (exitGameButton.getGlobalBounds().contains(mousePos)) {
            selectedOption = Option::ExitGame;
            active = false;
        }
    }
    else if (event.type == sf::Event::MouseMoved)
    {
        // Mouse hover: update selection to match hovered button.
        sf::Vector2f mousePos = fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre));

        if (resumeButton.getGlobalBounds().contains(mousePos)) selectedOption = Option::Resume;
        else if (exitLevelButton.getGlobalBounds().contains(mousePos)) selectedOption = Option::ExitLevel;
        else if (exitGameButton.getGlobalBounds().contains(mousePos)) selectedOption = Option::ExitGame;
    }

    // Refresh button colors after input.
    updateButtonColors();
}

void PauseMenu::draw(sf::RenderWindow& fenetre)
{
    if (!fontLoaded) return;

    // Dark translucent overlay to dim the gameplay behind the pause menu.
    sf::RectangleShape overlay(sf::Vector2f((float)fenetre.getSize().x, (float)fenetre.getSize().y));
    overlay.setFillColor(sf::Color(0,0,0,160));
    fenetre.draw(overlay);

    // Update layout every frame (handles resolution changes).
    initUI(fenetre);

    // Objectives list on the left (read-only from the model).
    if (modelePtr)
    {
        // NOTE: getAllLevelObjectives() returns a vector by value (copy/snapshot).
        // WHY this is acceptable here:
        // - pause menu is not a per-frame hot path during gameplay
        // - it simplifies rendering (no lifetime concerns if room/objectives change)
        auto objs = modelePtr->getAllLevelObjectives();

        float x = 40.f;
        float y = 120.f;

        sf::Text primHeader("Primary objectives", font, 24);
        primHeader.setFillColor(sf::Color::White);
        primHeader.setPosition(x, y);
        fenetre.draw(primHeader);
        y += 36.f;

        for (auto& o : objs) {
            if (!o.isPrimary()) continue;

            sf::Text t(o.getTitle(), font, 20);
            t.setPosition(x, y);

            // Green if accomplished, red otherwise (clear status feedback).
            t.setFillColor(o.isAccomplished() ? sf::Color(0,200,0) : sf::Color(200,0,0));
            fenetre.draw(t);
            y += 28.f;
        }

        y += 12.f;

        sf::Text secHeader("Secondary objectives", font, 24);
        secHeader.setFillColor(sf::Color::White);
        secHeader.setPosition(x, y);
        fenetre.draw(secHeader);
        y += 36.f;

        for (auto& o : objs) {
            if (o.isPrimary()) continue;

            sf::Text t(o.getTitle(), font, 20);
            t.setPosition(x, y);
            t.setFillColor(o.isAccomplished() ? sf::Color(0,200,0) : sf::Color(200,0,0));
            fenetre.draw(t);
            y += 28.f;
        }
    }

    // Title and buttons (using shared stylized rendering).
    // Draw shadow + title.
    titleShadow.setPosition(titleText.getPosition().x + 3.f, titleText.getPosition().y + 3.f);
    fenetre.draw(titleShadow);
    fenetre.draw(titleText);

    // Hover detection (mouse hover OR keyboard selection).
    sf::Vector2i mousePixel = sf::Mouse::getPosition(fenetre);
    sf::Vector2f mousePos = fenetre.mapPixelToCoords(mousePixel);

    bool hoveredResume =
        resumeButton.getGlobalBounds().contains(mousePos)
        || selectedOption == Option::Resume;

    bool hoveredExitLevel =
        exitLevelButton.getGlobalBounds().contains(mousePos)
        || selectedOption == Option::ExitLevel;

    bool hoveredExitGame =
        exitGameButton.getGlobalBounds().contains(mousePos)
        || selectedOption == Option::ExitGame;

    // Draw buttons using the shared styled function for consistent look.
    drawStyledButton(fenetre, resumeButton, resumeLabel, hoveredResume);
    drawStyledButton(fenetre, exitLevelButton, exitLevelLabel, hoveredExitLevel);
    drawStyledButton(fenetre, exitGameButton, exitGameLabel, hoveredExitGame);
}
