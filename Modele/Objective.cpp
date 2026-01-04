
#include "Objective.h"
#include <iostream>
#include <utility>
#include <vector>

// Constructors
Objective::Objective() {
    // valeurs neutres
    title = "Objective";
    description = "";
    accomplished = false;
    primary = false;

    // Hitbox par défaut
    hitbox = sf::RectangleShape();
    hitbox.setSize(sf::Vector2f(32.f, 32.f));
    hitbox.setPosition(sf::Vector2f(0.f, 0.f));
    hitbox.setFillColor(sf::Color::Transparent);
    hitbox.setOutlineColor(sf::Color::Magenta);
    hitbox.setOutlineThickness(1.f);

    // Do not load texture by default here. If the JSON does not provide any
    // texture for this objective, we will simply display the debug hitbox
    // (magenta outline) instead of an unwanted default sprite.
    // The assignment of a texture must be done explicitly via
    // `setTexture()` when the JSON specifies one.

    // WHY no default texture here (design intent):
    // - avoids unexpected visuals when configuration provides no texture
    // - keeps a clear "debug hitbox only" fallback path
}

Objective::Objective(std::string t, std::string d, sf::Texture tex,
                     float x, float y, float w, float h)
{
    // title / description
    title = t.empty() ? "Objective" : t;
    description = d.empty() ? "Objective's description" : d;
    accomplished = false;
    primary = true;

    // Copy the texture in the internal attribute
    this->texture = std::move(tex);

    // WHY std::move(tex):
    // - transfers the texture resource into Objective without an extra deep copy (when possible)
    // - Objective becomes the owner of its internal sf::Texture instance

    if (this->texture.getSize().x > 0 && this->texture.getSize().y > 0)
        sprite.setTexture(this->texture);

    // hitbox according to arguments
    hitbox = sf::RectangleShape();
    hitbox.setSize(sf::Vector2f(w, h));
    hitbox.setPosition(sf::Vector2f(x, y));
    hitbox.setFillColor(sf::Color::Transparent);
    hitbox.setOutlineColor(sf::Color::White);
    hitbox.setOutlineThickness(1.f);

    // Adjust the sprite size to the hitbox
    if (this->texture.getSize().x > 0)
    {
        sprite.setScale(
            hitbox.getSize().x / this->texture.getSize().x,
            hitbox.getSize().y / this->texture.getSize().y
        );
    }

    // position the sprite on the hitbox
    sprite.setPosition(x, y);
}

// Destructor
Objective::~Objective() {
    std::cout << "Objective " << this->getTitle() << " has been destroyed" << std::endl;

    // WHY destructor prints:
    // - debugging / lifetime tracing (helps confirm moves/copies/destructions)
}

// Move constructor
// During a `move`, the internal content (of which the `sf::Texture>) can be
// moved. `sf::Sprite` contains only a pointer to the texture and
// will not automatically update this pointer if the texture is
// moved/copied elsewhere. To ensure the validity of the sprite after
// displacement, we reassign the texture to the sprite via `sprite.setTexture(texture)`.
Objective::Objective(Objective&& other) noexcept
    : title(std::move(other.title)), description(std::move(other.description)),
      primary(other.primary), accomplished(other.accomplished),
      hitbox(std::move(other.hitbox)), sprite(std::move(other.sprite)),
      texture(std::move(other.texture)), dialogueFile(std::move(other.dialogueFile)),
      dialogueRef(std::move(other.dialogueRef)),
      cesar(other.cesar), code(std::move(other.code)), changeValue(other.changeValue)
{
    // WHY re-bind sprite texture after move:
    // - sf::Sprite internally references texture data
    // - after moving sf::Texture, the sprite must be pointed to this->texture again
    if (texture.getSize().x > 0 && texture.getSize().y > 0) {
        sprite.setTexture(texture);
    }
}

// Move assignment
Objective& Objective::operator=(Objective&& other) noexcept
{
    if (this != &other) {
        title = std::move(other.title);
        description = std::move(other.description);
        primary = other.primary;
        accomplished = other.accomplished;
        hitbox = std::move(other.hitbox);
        sprite = std::move(other.sprite);
        texture = std::move(other.texture);
        dialogueFile = std::move(other.dialogueFile);
        dialogueRef = std::move(other.dialogueRef);
        cesar = other.cesar;
        code = std::move(other.code);
        changeValue = other.changeValue;

        // WHY re-bind sprite texture after move assignment:
        // - ensures sprite references the texture stored in *this* after the move
        if (texture.getSize().x > 0 && texture.getSize().y > 0) {
            sprite.setTexture(texture);
        }
    }
    return *this;
}

// Copy constructor
// During a copy, we duplicate the internal `sf::Texture` and must
// also reassign the sprite to this new texture in order to avoid
// that the sprite references the old texture (or an invalid pointer).
Objective::Objective(const Objective& other)
    : title(other.title), description(other.description),
      primary(other.primary), accomplished(other.accomplished),
      hitbox(other.hitbox), sprite(other.sprite), texture(other.texture),
      dialogueFile(other.dialogueFile), dialogueRef(other.dialogueRef),
      cesar(other.cesar), code(other.code), changeValue(other.changeValue)
{
    // WHY re-bind sprite texture after copy:
    // - sprite must reference the copied texture owned by *this*
    if (texture.getSize().x > 0 && texture.getSize().y > 0) {
        sprite.setTexture(texture);
    }
}

// Copy assignment
Objective& Objective::operator=(const Objective& other)
{
    if (this != &other) {
        title = other.title;
        description = other.description;
        primary = other.primary;
        accomplished = other.accomplished;
        hitbox = other.hitbox;
        sprite = other.sprite;
        texture = other.texture;
        dialogueFile = other.dialogueFile;
        dialogueRef = other.dialogueRef;
        cesar = other.cesar;
        code = other.code;
        changeValue = other.changeValue;

        // WHY re-bind sprite texture after copy assignment:
        // - ensures sprite points to this->texture (the copied texture)
        if (texture.getSize().x > 0 && texture.getSize().y > 0) {
            sprite.setTexture(texture);
        }
    }
    return *this;
}

// Setters
void Objective::setTitle(const std::string& t) {
    // WHY const reference:
    // - avoids copying the string parameter
    title = t;
}

void Objective::setDescription(const std::string& d) {
    // WHY const reference:
    // - avoids copying the string parameter
    description = d;
}

void Objective::setAccomplished(const bool b) {
    // WHY pass bool by value:
    // - trivial type; simplest and most efficient
    accomplished = b;
}

void Objective::setPrimary(const bool b) {
    // WHY pass bool by value:
    // - trivial type; simplest and most efficient
    primary = b;
}

void Objective::setTexture(const std::string& t) {
    // WHY const std::string&:
    // - file path can be long; avoid copy
    // - read-only input

    if (texture.loadFromFile(t)) {
        sprite.setTexture(texture);

        // Ajuster la taille du sprite à la hitbox
        sprite.setScale(
            hitbox.getSize().x / texture.getSize().x,
            hitbox.getSize().y / texture.getSize().y
        );
        sprite.setPosition(hitbox.getPosition());
    }
    else {
        std::cerr << "Erreur: impossible de charger la texture d'objectif : " << t << std::endl;
    }
}

void Objective::setHitboxPosition(float x, float y) {
    // WHY pass floats by value:
    // - primitive types; cheap and clear
    hitbox.setPosition(sf::Vector2f(x, y));
    sprite.setPosition(sf::Vector2f(x, y));
}

void Objective::setHitboxSize(float w, float h) {
    // WHY pass floats by value:
    // - primitive types; cheap and clear
    hitbox.setSize(sf::Vector2f(w, h));

    // Recalibrate the sprite if the size changes
    if (texture.getSize().x > 0)
    {
        sprite.setScale(
            w / texture.getSize().x,
            h / texture.getSize().y
        );
    }
}

void Objective::setDialogueFile(const std::string& df) {
    // WHY const reference:
    // - avoid copying a string parameter
    dialogueFile = df;
}

void Objective::setDialogueRef(const std::string& dr) {
    // WHY const reference:
    // - avoid copying a string parameter
    dialogueRef = dr;
}

void Objective::setCesar(bool b) {
    // WHY pass bool by value:
    // - trivial type
    cesar = b;
}

void Objective::setCode(const std::string& c) {
    // WHY const reference:
    // - avoid copying code string
    code = c;
}

void Objective::setchangeValue(int v) {
    // WHY pass int by value:
    // - trivial type
    changeValue = v;
}

// Getters
std::string Objective::getTitle() const {
    // Returned by value: provides a safe copy of internal state.
    return title;
}

std::string Objective::getDescription() const {
    // Returned by value: provides a safe copy of internal state.
    return description;
}

bool Objective::isAccomplished() const {
    return accomplished;
}

bool Objective::isPrimary() const {
    return primary;
}

sf::Texture Objective::getTexture() const {
    // Returned by value: caller receives a copy of the texture.
    return texture;
}

sf::Sprite Objective::getSprite() const {
    // Returned by value: caller receives a copy of the sprite.
    return sprite;
}

sf::Vector2f Objective::getHitboxPosition() const {
    return hitbox.getPosition();
}

sf::Vector2f Objective::getHitboxSize() const {
    return hitbox.getSize();
}

sf::RectangleShape Objective::getHitbox() const {
    // Returned by value: caller receives a copy of the hitbox shape.
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

// Calculate the value of the altered code
// Only use letters, no numbers or symbols
std::string Objective::calculateAlteredCode() const {
    // WHY return by value:
    // - produces a new computed string result (does not modify original code)
    std::string altered = code;
    int base = 0;
    int newLetter;

    for (int i = 0; i < code.size(); i++) {
        // Between A and Z
        if (static_cast<int>(code[i]) >= 65 && static_cast<int>(code[i]) <= 90) {
            base = 65;
        } else {
            base = 97;
        }
        // Value between 0 and 25
        newLetter = int(code[i]) - base;
        // Value between 0 and 25
        newLetter = (newLetter + changeValue) % 26;
        // Get the correct ASCII value
        newLetter = newLetter + base;
        // Change the letter
        altered[i] = char(newLetter);
    }
    return altered;
}
