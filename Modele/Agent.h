//
// Created by bertr on 25-11-25.
//

#ifndef TESTCOLLISION_AGENT_H
#define TESTCOLLISION_AGENT_H
#include <string>


class Agent {
    private:
        std::string agentName;
        std::string textureFile;
        int nbLife;
        int posX;
        int posY;

    public:
        Agent();
        Agent(const std::string& name);
        ~Agent();

        void setAgentName(const std::string& name);
        void setTextureFile(const std::string& texture);
        void setNbLife(int nb);
        void setPosX(int x);
        void setPosY(int y);

        std::string getAgentName() const;
        std::string getTextureFile() const;
        int getNbLife() const;
        int getPosX() const;
        int getPosY() const;


};


#endif //TESTCOLLISION_AGENT_H