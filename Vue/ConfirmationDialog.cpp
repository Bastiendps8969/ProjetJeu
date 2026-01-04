// ConfirmationDialog implementation
#include "ConfirmationDialog.h"

using namespace Vue;

ConfirmationDialog::ConfirmationDialog(const std::string& msg)
    : message(msg)
{
    fontLoaded = font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");

    messageText.setFont(font);
    messageText.setString(message);
    messageText.setCharacterSize(22);
    messageText.setFillColor(sf::Color::White);

    yesButton.setSize(sf::Vector2f(140.f, 50.f));
    // Use red tones to match other menus
    yesButton.setFillColor(sf::Color(170,30,30));
    yesLabel.setFont(font);
    yesLabel.setString("Yes");
    yesLabel.setCharacterSize(20);
    yesLabel.setFillColor(sf::Color::White);

    noButton.setSize(sf::Vector2f(140.f, 50.f));
    // Slightly darker red for No
    noButton.setFillColor(sf::Color(120,20,20));
    noLabel.setFont(font);
    noLabel.setString("No");
    noLabel.setCharacterSize(20);
    noLabel.setFillColor(sf::Color::White);
}

void ConfirmationDialog::initUI(sf::RenderWindow& fenetre)
{
    sf::Vector2u sz = fenetre.getSize();
    float cx = sz.x / 2.f;
    float cy = sz.y / 2.f;

    // center message
    sf::FloatRect mb = messageText.getLocalBounds();
    messageText.setPosition(cx - mb.width/2.f - mb.left, cy - 90.f);

    yesButton.setPosition(cx - yesButton.getSize().x - 10.f, cy + 10.f);
    centerLabel(yesLabel, yesButton);

    noButton.setPosition(cx + 10.f, cy + 10.f);
    centerLabel(noLabel, noButton);
}

void ConfirmationDialog::centerLabel(sf::Text& label, const sf::RectangleShape& button)
{
    sf::FloatRect tb = label.getLocalBounds();
    sf::FloatRect bb = button.getGlobalBounds();
    label.setPosition(bb.left + (bb.width - tb.width)/2.f - tb.left, bb.top + (bb.height - tb.height)/2.f - tb.top);
}

void ConfirmationDialog::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
{
    if (!fontLoaded) return;

    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Escape)
        {
            // cancel
            active = false;
            confirmed = false;
            return;
        }
        // Arrow navigation (Left/Right/Up/Down) and Enter to confirm
        if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Up)
        {
            selectedIndex = 0;
        }
        else if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::Down)
        {
            selectedIndex = 1;
        }
        else if (event.key.code == sf::Keyboard::Enter)
        {
            if (selectedIndex == 0) { confirmed = true; active = false; }
            else { confirmed = false; active = false; }
            return;
        }
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mousePos = fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre));
        if (yesButton.getGlobalBounds().contains(mousePos)) { confirmed = true; active = false; }
        else if (noButton.getGlobalBounds().contains(mousePos)) { confirmed = false; active = false; }
    }

    // Update hover selection on mouse move
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f mousePos = fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre));
        if (yesButton.getGlobalBounds().contains(mousePos)) selectedIndex = 0;
        else if (noButton.getGlobalBounds().contains(mousePos)) selectedIndex = 1;
    }
}

void ConfirmationDialog::draw(sf::RenderWindow& fenetre)
{
    if (!fontLoaded) return;

    // overlay (dim background)
    sf::RectangleShape overlay(sf::Vector2f((float)fenetre.getSize().x, (float)fenetre.getSize().y));
    overlay.setFillColor(sf::Color(0,0,0,180));
    fenetre.draw(overlay);

    initUI(fenetre);

    // dialog background box (styled similarly to PauseMenu)
    sf::RectangleShape box(sf::Vector2f(600.f, 180.f));
    box.setFillColor(sf::Color(30,30,30));
    box.setOutlineColor(sf::Color(200,100,90));
    box.setOutlineThickness(2.f);
    box.setPosition((fenetre.getSize().x - box.getSize().x)/2.f, (fenetre.getSize().y - box.getSize().y)/2.f - 20.f);
    fenetre.draw(box);

    // draw message centered (already positioned in initUI)
    fenetre.draw(messageText);

    // determine mouse hover for nicer button feedback, include keyboard selection
    sf::Vector2i mousePixel = sf::Mouse::getPosition(fenetre);
    sf::Vector2f mousePos = fenetre.mapPixelToCoords(mousePixel);
    bool hoverYes = yesButton.getGlobalBounds().contains(mousePos) || selectedIndex == 0;
    bool hoverNo  = noButton.getGlobalBounds().contains(mousePos) || selectedIndex == 1;

    // Styled button drawing (keeps labels and sizes intact)
    auto drawStyledButton = [&](sf::RenderWindow& w, sf::RectangleShape& btn, sf::Text& label, bool hovered) {        sf::RectangleShape base = btn;
        // adapt colors from existing fill but emphasize on hover
        sf::Color baseFill = btn.getFillColor();
        if (hovered) {
            // brighten base color on hover
            sf::Color h = sf::Color(std::min(255, baseFill.r + 40), std::min(255, baseFill.g + 40), std::min(255, baseFill.b + 40));
            base.setFillColor(h);
            base.setOutlineColor(sf::Color(255,160,110));
            base.setOutlineThickness(3.f);
        } else {
            base.setFillColor(baseFill);
            base.setOutlineColor(sf::Color(200,200,200));
            base.setOutlineThickness(2.f);
        }
        w.draw(base);

        // subtle glow when hovered
        if (hovered) {
            sf::RectangleShape glow = btn;
            glow.setFillColor(sf::Color::Transparent);
            glow.setOutlineColor(sf::Color(255,160,110,160));
            glow.setOutlineThickness(6.f);
            w.draw(glow);
        }

        // center label inside the button
        sf::FloatRect lb = label.getLocalBounds();
        label.setPosition(btn.getPosition().x + (btn.getSize().x - lb.width)/2.f - lb.left,
                          btn.getPosition().y + (btn.getSize().y - lb.height)/2.f - lb.top);
        label.setFillColor(hovered ? sf::Color::White : sf::Color(230,230,230));
        w.draw(label);
    };

    // Draw Yes / No with styled appearance
    drawStyledButton(fenetre, yesButton, yesLabel, hoverYes);
    drawStyledButton(fenetre, noButton, noLabel, hoverNo);
}
