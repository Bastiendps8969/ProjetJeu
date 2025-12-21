//
// Created by bertr on 09-12-25.
//

#ifndef TESTCOLLISION_OBJECTIVE_H
#define TESTCOLLISION_OBJECTIVE_H
#include <string>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>


class Objective {
private:
    //  Title of the objective
    std::string title;
    //  Description of the objective
    std::string description;

    //  Primary or secondary objective
    bool primary = true;

    //  Is the objective accomplished
    bool accomplished = false;

    //  Objective's box
    sf::RectangleShape hitbox;
    sf::Sprite sprite;
    sf::Texture texture;

    //  Dialogue information
    std::string dialogueFile;
    std::string dialogueRef;


public:
    //  Constructor
    Objective();
    Objective(std::string t, std::string d, sf::Texture texture,
        float x, float y, float w, float h);

    // Move operations: ensure sprite's texture pointer is reset after move
    // Move operations: defined to ensure that when an Objective
    // is moved (e.g. vector reallocation), we re-bind the
    // internal `sf::Sprite` to the moved `sf::Texture` so
    // the sprite's internal texture pointer remains valid.
    Objective(Objective&& other) noexcept;
    Objective& operator=(Objective&& other) noexcept;

    // Copy operations: explicitly declared because declaring
    // move ctor/assign prevents implicit copy operations.
    // These copy the texture and then re-bind the sprite to
    // ensure the sprite points to the copied texture data.
    Objective(const Objective& other);
    Objective& operator=(const Objective& other);

    //  Destructor
    ~Objective();

    //  Setters
    void setTitle(const std::string& t);
    void setDescription(const std::string& d);
    void setAccomplished(bool b);
    void setPrimary(bool b);
    void setHitboxPosition(float x, float y);
    void setHitboxSize(float w, float h);
    void setTexture(const std::string& t);
    void setDialogueFile(const std::string& df);
    void setDialogueRef(const std::string& dr);

    //  Getters
    std::string getTitle() const;
    std::string getDescription() const;
    bool isAccomplished() const;
    bool isPrimary() const;
    sf::Texture getTexture() const;
    sf::Sprite getSprite() const;
    sf::Vector2f getHitboxPosition() const;
    sf::Vector2f getHitboxSize() const;
    sf::RectangleShape getHitbox() const;
    std::string getDialogueFile() const;
    std::string getDialogueRef() const;
};


#endif //TESTCOLLISION_OBJECTIVE_H