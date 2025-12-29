#include "Objective.h"
#include <iostream>
#include <utility>
#include <vector>

//  Constructors
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

    // Ne pas charger de texture par défaut ici. Si le JSON ne fournit pas de
    // texture pour cet objectif, on affichera simplement la hitbox de debug
    // (contour magenta) au lieu d'un sprite par défaut non désiré.
    // L'affectation d'une texture doit être faite explicitement via
    // `setTexture()` lorsque le JSON en spécifie une.
}

Objective::Objective(std::string t, std::string d, sf::Texture tex,
                     float x, float y, float w, float h)
{
    // titre / description
    title = t.empty() ? "Objective" : t;
    description = d.empty() ? "Objective's description" : d;
    accomplished = false;
    primary = true;

    // Copier la texture dans l'attribut interne
    this->texture = std::move(tex);

    if (this->texture.getSize().x > 0 && this->texture.getSize().y > 0)
        sprite.setTexture(this->texture);

    // hitbox selon arguments
    hitbox = sf::RectangleShape();
    hitbox.setSize(sf::Vector2f(w, h));
    hitbox.setPosition(sf::Vector2f(x, y));
    hitbox.setFillColor(sf::Color::Transparent);
    hitbox.setOutlineColor(sf::Color::White);
    hitbox.setOutlineThickness(1.f);

    // Ajuster la taille du sprite à la hitbox
    if (this->texture.getSize().x > 0)
    {
        sprite.setScale(
            hitbox.getSize().x / this->texture.getSize().x,
            hitbox.getSize().y / this->texture.getSize().y
        );
    }

    // positionner le sprite sur la hitbox
    sprite.setPosition(x, y);
}

//  Destructor
Objective::~Objective() {
    std::cout << "Objective " << this->getTitle() << " has been destroyed" << std::endl;
}

// Move constructor
// Lors d'un `move`, le contenu interne (dont la `sf::Texture`) peut être
// déplacé. `sf::Sprite` contient seulement un pointeur vers la texture et
// ne mettra pas automatiquement à jour ce pointeur si la texture est
// déplacée/copied ailleurs. Pour garantir la validité du sprite après
// déplacement, on réaffecte la texture au sprite via `sprite.setTexture(texture)`.
Objective::Objective(Objective&& other) noexcept
        : title(std::move(other.title)), description(std::move(other.description)),
            primary(other.primary), accomplished(other.accomplished),
            hitbox(std::move(other.hitbox)), sprite(std::move(other.sprite)),
            texture(std::move(other.texture)), dialogueFile(std::move(other.dialogueFile)),
            dialogueRef(std::move(other.dialogueRef)),
            cesar(other.cesar), code(std::move(other.code)), changeValue(other.changeValue)
{
    if (texture.getSize().x > 0 || texture.getSize().y > 0) {
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

        if (texture.getSize().x > 0 || texture.getSize().y > 0) {
            sprite.setTexture(texture);
        }
    }
    return *this;
}

// Copy constructor
// Lors d'une copie, nous dupliquons la `sf::Texture` interne et devons
// aussi réaffecter le sprite sur cette nouvelle texture afin d'éviter
// que le sprite référence l'ancienne texture (ou un pointeur invalide).
Objective::Objective(const Objective& other)
        : title(other.title), description(other.description),
            primary(other.primary), accomplished(other.accomplished),
            hitbox(other.hitbox), sprite(other.sprite), texture(other.texture),
            dialogueFile(other.dialogueFile), dialogueRef(other.dialogueRef),
            cesar(other.cesar), code(other.code), changeValue(other.changeValue)
{
    if (texture.getSize().x > 0 || texture.getSize().y > 0) {
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

        if (texture.getSize().x > 0 || texture.getSize().y > 0) {
            sprite.setTexture(texture);
        }
    }
    return *this;
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
    hitbox.setPosition(sf::Vector2f(x, y));
    sprite.setPosition(sf::Vector2f(x, y));
}

void Objective::setHitboxSize(float w, float h) {
    hitbox.setSize(sf::Vector2f(w, h));

    // Recalibrer le sprite si la taille change
    if (texture.getSize().x > 0)
    {
        sprite.setScale(
            w / texture.getSize().x,
            h / texture.getSize().y
        );
    }
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

