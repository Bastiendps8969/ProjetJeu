#include "Objective.h"
#include <iostream>
#include <utility>

//  Constructors
Objective::Objective() {
    // valeurs neutres
    title = "Objective";
    description = "";
    accomplished = false;

    // Hitbox par défaut
    hitbox = sf::RectangleShape();
    hitbox.setSize(sf::Vector2f(32.f, 32.f));
    hitbox.setPosition(sf::Vector2f(0.f, 0.f));
    hitbox.setFillColor(sf::Color::Magenta);
    hitbox.setOutlineColor(sf::Color::Magenta);
    hitbox.setOutlineThickness(1.f);

    // Charger une texture par défaut si disponible
    if (texture.loadFromFile("Asset/objectives/computer_on_table.png"))
    {
        sprite.setTexture(texture);
        // positionner le sprite centré sur la hitbox
        sprite.setPosition(hitbox.getPosition());
    }
}

//  Destructor
Objective::~Objective() {
    std::cout << "Objective " << this->getTitle() << " has been destroyed" << std::endl;
}


//  Setters
void Objective::setTitle(const std::string& t) {
    title = t;
}
void Objective::setDescription(const std::string& d) {
    description = d;
}
void Objective::setAccomplished(const bool b) {
    accomplished = b;
}
void Objective::setPrimary(const bool b) {
    primary = b;
}

void Objective::setTexture(const std::string& t) {
        texture.loadFromFile(t);
}

void Objective::setHitboxPosition(float x, float y) {
    hitbox.setPosition(sf::Vector2f(x, y));
    sprite.setPosition(sf::Vector2f(x, y));
}
void Objective::setHitboxSize(float w, float h) {
    hitbox.setSize(sf::Vector2f(w, h));
}

void Objective::setDialogueFile(const std::string& df) {
    dialogueFile = df;
}
void Objective::setDialogueRef(const std::string& dr) {
    dialogueRef = dr;
}

void Objective::setCesar(bool b) {
    cesar = b;
}
void Objective::setCode(const std::string& c) {
    code = c;
}
void Objective::setchangeValue(int v) {
    changeValue = v;
}




//  Getters
std::string Objective::getTitle() const {
    return title;
}
std::string Objective::getDescription() const {
    return description;
}
bool Objective::isAccomplished() const {
    return accomplished;
}
bool Objective::isPrimary() const {
    return primary;
}
sf::Texture Objective::getTexture() const {
    return texture;
}
sf::Sprite Objective::getSprite() const {
    return sprite;
}
sf::Vector2f Objective::getHitboxPosition() const {
    return hitbox.getPosition();
}
sf::Vector2f Objective::getHitboxSize() const {
    return hitbox.getSize();
}
sf::RectangleShape Objective::getHitbox() const {
    return hitbox;
}
std::string Objective::getDialogueFile() const {
    return dialogueFile;
}
std::string Objective::getDialogueRef() const {
    return dialogueRef;
}

bool Objective::isCesar() const {
    return cesar;
}
std::string Objective::getCode() const {
    return code;
}
int Objective::getChangeValue() const {
    return changeValue;
}

//  Calculate the value of the altered code
//  Only use letters, no numbers or symbols
std::string Objective::calculateAlteredCode() const {
    std::string altered = code;
    int base = 0;
    int newLetter;
    for (int i = 0; i < code.size(); i++) {
        //  Between A and Z
        if (static_cast<int>(code[i]) >= 65 && static_cast<int>(code[i]) <= 90) {
            base = 65;
        } else {
            base = 97;
        }

        //  Value between 0 and 25
        newLetter = int(code[i]) - base;

        //  Value between 0 and 25
        newLetter = (newLetter + changeValue) % 26;

        //  Get the correct ASCII value
        newLetter = newLetter + base;

        //  Change the letter
        altered[i] = char(newLetter);
    }

    return altered;
}

