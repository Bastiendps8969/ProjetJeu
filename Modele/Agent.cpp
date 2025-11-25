//
// Created by bertr on 25-11-25.
//
#ifndef TESTCOLLISION_AGENT_H
#include "Agent.h"

#include <utility>

Agent::Agent() {
    setAgentName("James Adams");
    setSpriteFile(std::string("Resources/Sprites/Agent/Agent_Sprite.png"));

    posX = 0;
    posY = 0;
}

Agent::Agent(const std::string& name) {
    setAgentName(name);
    setSpriteFile(std::string("Resources/Sprites/Agent/Agent_Sprite.png"));

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
void Agent::setSpriteFile(const std::string& path) {
    if (!path.empty()) {
        spriteFile = path;
    }
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
std::string Agent::getSpriteFile() const {
    return spriteFile;
}
int Agent::getPosX() const {
    return posX;
}
int Agent::getPosY() const {
    return posY;
}

#endif