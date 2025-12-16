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
    yesButton.setFillColor(sf::Color(100,100,200));
    yesLabel.setFont(font);
    yesLabel.setString("Yes");
    yesLabel.setCharacterSize(20);
    yesLabel.setFillColor(sf::Color::White);

    noButton.setSize(sf::Vector2f(140.f, 50.f));
    noButton.setFillColor(sf::Color(150,150,150));
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
    messageText.setPosition(cx - mb.width/2.f - mb.left, cy - 60.f);

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
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mousePos = fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre));
        if (yesButton.getGlobalBounds().contains(mousePos)) { confirmed = true; active = false; }
        else if (noButton.getGlobalBounds().contains(mousePos)) { confirmed = false; active = false; }
    }
}

void ConfirmationDialog::draw(sf::RenderWindow& fenetre)
{
    if (!fontLoaded) return;

    // overlay
    sf::RectangleShape overlay(sf::Vector2f((float)fenetre.getSize().x, (float)fenetre.getSize().y));
    overlay.setFillColor(sf::Color(0,0,0,180));
    fenetre.draw(overlay);

    initUI(fenetre);

    // dialog background box
    sf::RectangleShape box(sf::Vector2f(600.f, 180.f));
    box.setFillColor(sf::Color(30,30,30));
    box.setOutlineColor(sf::Color::White);
    box.setOutlineThickness(2.f);
    box.setPosition((fenetre.getSize().x - box.getSize().x)/2.f, (fenetre.getSize().y - box.getSize().y)/2.f - 20.f);
    fenetre.draw(box);

    fenetre.draw(messageText);

    fenetre.draw(yesButton);
    fenetre.draw(yesLabel);

    fenetre.draw(noButton);
    fenetre.draw(noLabel);
}
