//
// Created by bertr on 13-12-25.
//

#ifndef TESTCOLLISION_LEVEL_H
#define TESTCOLLISION_LEVEL_H
#include <string>
#include <algorithm>


class Level {
    private:
        std::string name;
        std::string description;

        int countdown;
        int lives = 3;  // Nombre de vies (3 au départ)


    public:
        Level(std::string n, std::string d);
        ~Level();

        // Getter et setter pour les vies
        int getLives() const { return lives; }
        void setLives(int l) { lives = std::max(0, l); }
        void loseLives(int amount) { lives = std::max(0, lives - amount); }
        bool isGameOver() const { return lives <= 0; }
};


#endif //TESTCOLLISION_LEVEL_H