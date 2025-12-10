#include "Objective.h"
#include <iostream>

Objective::Objective() {
    // valeurs neutres
    title = "Objective";
    description = "";
    accomplished = false;

    // Hitbox par défaut
    hitbox = sf::RectangleShape();
    hitbox.setSize(sf::Vector2f(32.f, 32.f));
    hitbox.setPosition(sf::Vector2f(0.f, 0.f));
    hitbox.setFillColor(sf::Color::White);
    hitbox.setOutlineColor(sf::Color::White);
    hitbox.setOutlineThickness(1.f);

    // Charger une texture par défaut si disponible
    if (texture.loadFromFile("Asset/objectives/computer_on_table.png"))
    {
        sprite.setTexture(texture);
        // positionner le sprite centré sur la hitbox
        sprite.setPosition(hitbox.getPosition());
    }
}

Objective::Objective(std::string t, std::string d, sf::Texture textureParam,
                     float x, float y, float w, float h) {
    // titre / description
    title = t.empty() ? "Objective" : t;
    description = d.empty() ? "" : d;
    accomplished = false;

    // stocker la texture fournie (copie/move)
    texture = std::move(textureParam);
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

Objective::~Objective() {
    std::cout << "Objective " << this->getTitle() << " has been destroyed" << std::endl;
}



void Objective::setTitle(std::string t) {
    if (!t.empty()) {
        title = t;
    }
}
void Objective::setDescription(std::string d) {
    if (!d.empty()) {
        description = d;
    }
}
void Objective::setAccomplished(bool b) {
    accomplished = b;
}
void Objective::setPrimary(bool b) {
    primary = b;
}

void Objective::setTexture(std::string t) {
    if (!t.empty()) {
        texture.loadFromFile(t);
    }
}


void Objective::setHitboxPosition(float x, float y) {
    hitbox.setPosition(sf::Vector2f(x, y));
    sprite.setPosition(sf::Vector2f(x, y));
}
void Objective::setHitboxSize(float w, float h) {
    hitbox.setSize(sf::Vector2f(w, h));
}

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
