
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
    // Title of the objective
    std::string title;

    // Description of the objective
    std::string description;

    // Primary or secondary objective
    bool primary = true;

    // Is the objective accomplished
    bool accomplished = false;

    // Objective's box
    sf::RectangleShape hitbox;

    // WHY these SFML members are stored by value:
    // - Objective "owns" its visual/collision state directly (composition).
    // - Storing by value keeps lifetime tied to Objective and avoids manual memory management.
    sf::Sprite sprite;
    sf::Texture texture;

    // Dialogue information
    std::string dialogueFile;
    std::string dialogueRef;

    // Cesar code
    bool cesar = false;
    std::string code;

    // Could be defined randomly
    int changeValue;

public:
    // Constructor
    Objective();

    Objective(std::string t, std::string d, sf::Texture texture,
              float x, float y, float w, float h);

    // WHY parameters are passed by value in this constructor:
    // - Strings and texture are "owned" data for the Objective instance.
    // - Passing by value allows the implementation to move them into members (when possible),
    //   which can be efficient for temporaries while keeping a simple call-site API.

    // Move operations: ensure sprite's texture pointer is reset after move
    // Move operations: defined to ensure that when an Objective
    // is moved (e.g. vector reallocation), we re-bind the
    // internal `sf::Sprite` to the moved `sf::Texture` so
    // the sprite's internal texture pointer remains valid.
    Objective(Objective&& other) noexcept;
    Objective& operator=(Objective&& other) noexcept;

    // WHY move ctor/assign exist:
    // - Objective is stored in vectors (Room stores vector<Objective>), so reallocation can move elements.
    // - sf::Sprite keeps an internal pointer/reference to texture data; after moving the texture,
    //   we must call sprite.setTexture(texture) to ensure the sprite points to the right texture.

    // Copy operations: explicitly declared because declaring
    // move ctor/assign prevents implicit copy operations.
    // These copy the texture and then re-bind the sprite to
    // ensure the sprite points to the copied texture data.
    Objective(const Objective& other);
    Objective& operator=(const Objective& other);

    // WHY copy ctor/assign exist:
    // - Copying Objective must duplicate the texture and then re-bind the sprite to the *copied* texture,
    //   otherwise the sprite may keep referencing the old texture data.

    // Destructor
    ~Objective();

    // Setters
    // WHY const std::string& for setters:
    // - avoids copying potentially large strings (title/description/paths)
    // - makes it explicit that the parameter is read-only
    void setTitle(const std::string& t);
    void setDescription(const std::string& d);

    // WHY bool/int passed by value:
    // - primitive types are cheap to copy and passing by value avoids aliasing issues
    void setAccomplished(bool b);
    void setPrimary(bool b);

    // WHY floats passed by value:
    // - small primitive types, cheap to copy and clearer at call-site
    void setHitboxPosition(float x, float y);
    void setHitboxSize(float w, float h);

    // WHY const std::string&:
    // - file path/name may be long; avoid copy
    // - read-only input
    void setTexture(const std::string& t);

    void setDialogueFile(const std::string& df);
    void setDialogueRef(const std::string& dr);

    void setCesar(bool b);
    void setCode(const std::string& c);
    void setchangeValue(int v);

    // Getters
    // WHY many getters return by value (as currently implemented):
    // - returns a copy to preserve encapsulation (caller cannot modify internal members directly)
    // - for SFML objects, returning by value provides an independent snapshot
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

    bool isCesar() const;
    std::string getCode() const;
    int getChangeValue() const;

    // Returns a computed string -> returning by value is natural for a newly created result.
    std::string calculateAlteredCode() const;
};

#endif //TESTCOLLISION_OBJECTIVE_H
