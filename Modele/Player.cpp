//
// Created by bertr on 20-11-25.
//

#include "Player.h"

Player::Player() {
    setPlayerName("James Adams");
    playerScore[12] = {0};
}

Player::Player(std::string name) {
    setPlayerName(name);
    playerScore[12] = {0};
}

Player::~Player() {
    // Todo
}

void Player::setPlayerName(std::string name) {
    if (name != "") {
        playerName = name;
    }
}

void Player::setPlayerScore(int missionIndex, int newScore) {
    if (newScore > playerScore[missionIndex]) {
        playerScore[missionIndex] = newScore;
    }
}

std::string Player::getPlayerName() {
    return playerName;
}
int getPlayerScore(int missionIndex) {
    return playerScore[missionIndex];
}
int getPlayerTotalScore() {
    int totalScore = 0;
    for (int i = 0; i < 12; i++) {
        totalScore += playerScore[i];
    }
}


