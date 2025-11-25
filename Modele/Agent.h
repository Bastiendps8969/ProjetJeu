//
// Created by bertr on 25-11-25.
//

#ifndef TESTCOLLISION_AGENT_H
#define TESTCOLLISION_AGENT_H
#include <string>


class Agent {
    private:
        std::string agentName;
        std::string spriteFile;

        int posX;
        int posY;

    public:
        Agent();
        Agent(const std::string& name);
        ~Agent();

        void setAgentName(const std::string& name);
        void setSpriteFile(const std::string& sprite);
        void setPosX(int x);
        void setPosY(int y);

        std::string getAgentName() const;
        std::string getSpriteFile() const;
        int getPosX() const;
        int getPosY() const;


};


#endif //TESTCOLLISION_AGENT_H