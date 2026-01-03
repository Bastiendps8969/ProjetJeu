
// ConfirmationDialog implementation
#include "ConfirmationDialog.h"

using namespace Vue;

ConfirmationDialog::ConfirmationDialog(const std::string& msg)
    : message(msg)
{
    // Load font (Windows-specific path).
    fontLoaded = font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");

    // Message text setup.
    messageText.setFont(font);
    messageText.setString(message);
    messageText.setCharacterSize(22);
    messageText.setFillColor(sf::Color::White);

    // Yes button setup.
    yesButton.setSize(sf::Vector2f(140.f, 50.f));
    yesButton.setFillColor(sf::Color(100,100,200));

    yesLabel.setFont(font);
    yesLabel.setString("Yes");
    yesLabel.setCharacterSize(20);
    yesLabel.setFillColor(sf::Color::White);

    // No button setup.
    noButton.setSize(sf::Vector2f(140.f, 50.f));
    noButton.setFillColor(sf::Color(150,150,150));

    noLabel.setFont(font);
    noLabel.setString("No");
    noLabel.setCharacterSize(20);
    noLabel.setFillColor(sf::Color::White);
}

void ConfirmationDialog::initUI(sf::RenderWindow& fenetre)
{
    // Center elements based on current window size (responsive layout).
    sf::Vector2u sz = fenetre.getSize();
    float cx = sz.x / 2.f;
    float cy = sz.y / 2.f;

    // Center message above the buttons.
    sf::FloatRect mb = messageText.getLocalBounds();
    messageText.setPosition(cx - mb.width/2.f - mb.left, cy - 90.f);

    // Place buttons symmetrically.
    yesButton.setPosition(cx - yesButton.getSize().x - 10.f, cy + 10.f);
    centerLabel(yesLabel, yesButton);

    noButton.setPosition(cx + 10.f, cy + 10.f);
    centerLabel(noLabel, noButton);
}

void ConfirmationDialog::centerLabel(sf::Text& label, const sf::RectangleShape& button)
{
    // Center the label inside the button using local bounds (accounts for glyph offsets).
    sf::FloatRect tb = label.getLocalBounds();
    sf::FloatRect bb = button.getGlobalBounds();
    label.setPosition(
        bb.left + (bb.width - tb.width)/2.f - tb.left,
        bb.top  + (bb.height - tb.height)/2.f - tb.top
    );
}

void ConfirmationDialog::handleEvent(const sf::Event& event, sf::RenderWindow& fenetre)
{
    // If font isn't loaded, ignore events (UI cannot be displayed correctly).
    if (!fontLoaded) return;

    // Escape cancels and closes the dialog.
    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Escape)
        {
            active = false;
            confirmed = false;
            return;
        }
    }

    // Mouse click selects Yes/No and closes the dialog.
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mousePos = fenetre.mapPixelToCoords(sf::Mouse::getPosition(fenetre));

        if (yesButton.getGlobalBounds().contains(mousePos)) { confirmed = true;  active = false; }
        else if (noButton.getGlobalBounds().contains(mousePos)) { confirmed = false; active = false; }
    }
}

void ConfirmationDialog::draw(sf::RenderWindow& fenetre)
{
    if (!fontLoaded) return;

    // Dim the background behind the dialog (modal overlay).
    sf::RectangleShape overlay(sf::Vector2f((float)fenetre.getSize().x, (float)fenetre.getSize().y));
    overlay.setFillColor(sf::Color(0,0,0,180));
    fenetre.draw(overlay);

    // Update layout each frame (handles window size changes).
    initUI(fenetre);

    // Dialog background box (fixed size, centered).
    sf::RectangleShape box(sf::Vector2f(600.f, 180.f));
    box.setFillColor(sf::Color(30,30,30));
    box.setOutlineColor(sf::Color::White);
    box.setOutlineThickness(2.f);
    box.setPosition((fenetre.getSize().x - box.getSize().x)/2.f,
                    (fenetre.getSize().y - box.getSize().y)/2.f - 20.f);
    fenetre.draw(box);

    // Draw contents.
    fenetre.draw(messageText);
    fenetre.draw(yesButton);
    fenetre.draw(yesLabel);
    fenetre.draw(noButton);
    fenetre.draw(noLabel);
}
