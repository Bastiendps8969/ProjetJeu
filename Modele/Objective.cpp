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
    {
        // Try a list of candidate paths so executable working dir doesn't block loading
        std::vector<std::string> tryPaths = {
            std::string("Asset/objectives/computer_on_table.png"),
            std::string("cmake-build-debug/Asset/objectives/computer_on_table.png"),
            std::string("objectives/computer_on_table.png")
        };
        for (const auto &p : tryPaths) {
            if (texture.loadFromFile(p)) {
                sprite.setTexture(texture);
                sprite.setPosition(hitbox.getPosition());
                break;
            }
        }
    }
}

Objective::Objective(std::string t, std::string d, sf::Texture texture,
                     float x, float y, float w, float h) {
    // titre / description
    title = t.empty() ? "Objective" : t;
    description = d.empty() ? "Objective's description" : d;
    accomplished = false;
    primary = true;

    // stocker la texture fournie (copie/move)
    texture = std::move(texture);
    if (texture.getSize().x > 0 && texture.getSize().y > 0)
        sprite.setTexture(texture);

    // hitbox selon arguments
    hitbox = sf::RectangleShape();
    hitbox.setSize(sf::Vector2f(w, h));
    hitbox.setPosition(sf::Vector2f(x, y));
    hitbox.setFillColor(sf::Color::Transparent);
    hitbox.setOutlineColor(sf::Color::White);
    hitbox.setOutlineThickness(1.f);

    // positionner le sprite sur la hitbox
    sprite.setPosition(x, y);
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
    if (t.empty()) return;
    if (texture.loadFromFile(t)) return;
    std::string alt = std::string("cmake-build-debug/") + t;
    if (texture.loadFromFile(alt)) return;
    // last try: strip leading Asset/ if present
    if (t.rfind("Asset/", 0) == 0) {
        std::string stripped = t.substr(6);
        texture.loadFromFile(stripped);
    }
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

