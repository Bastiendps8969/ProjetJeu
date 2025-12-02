//
// Created by bertr on 25-11-25.
//
#ifndef TESTCOLLISION_AGENT_H
#include "Agent.h"

#include <utility>

Agent::Agent() {
    setAgentName("James Adams");
    setTextureFile(std::string("Resources/Textures/Agent/Agent_Sprite.png"));
    setNbLife(3);
    posX = 0;
    posY = 0;
}

Agent::Agent(const std::string& name) {
    setAgentName(name);
    setTextureFile(std::string("Resources/Textures/Agent/Agent_Sprite.png"));
    setNbLife(3);
    posX = 0;
    posY = 0;
}

Agent::~Agent() {
    //  Todo
}

//  Setters
void Agent::setAgentName(const std::string& name) {
    if (!name.empty()) {
        agentName = name;
    }
}
void Agent::setTextureFile(const std::string& path) {
    if (!path.empty()) {
        textureFile = path;
    }
}
void Agent::setNbLife(int nb) {
    nbLife = nb;
}
void Agent::setPosX(int x) {
    posX = x;
}
void Agent::setPosY(int y) {
    posY = y;
}



//  Getters
std::string Agent::getAgentName() const {
    return agentName;
}
std::string Agent::getTextureFile() const {
    return textureFile;
}
int Agent::getNbLife() const {
    return nbLife;
}
int Agent::getPosX() const {
    return posX;
}
int Agent::getPosY() const {
    return posY;
}

#endif