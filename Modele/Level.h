//
// Created by bertr on 13-12-25.
//

#ifndef TESTCOLLISION_LEVEL_H
#define TESTCOLLISION_LEVEL_H
#include <string>


class Level {
    private:
        std::string name;
        std::string description;

    public:
        Level(std::string n, std::string d);
        ~Level();
};


#endif //TESTCOLLISION_LEVEL_H