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


public:
    //  Constructor
    Objective();
    Objective(std::string t, std::string d, sf::Texture texture,
        float x, float y, float w, float h);

    //  Destructor
    ~Objective();

    //  Setters
    void setTitle(std::string t);
    void setDescription(std::string d);
    void setAccomplished(bool b);
    void setPrimary(bool b);
    void setHitboxPosition(float x, float y);
    void setHitboxSize(float w, float h);
    void setTexture(std::string t);

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
};


#endif //TESTCOLLISION_OBJECTIVE_H