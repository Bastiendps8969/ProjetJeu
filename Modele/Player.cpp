//
// Created by bertr on 20-11-25.
//

#include "Player.h"

Player::Player() {
    setPlayerName("James Adams");
    for (int i = 0; i < playerName.length(); i++) {
        playerScore[i] = 0;
    }
}

Player::Player(const std::string& name) {
    setPlayerName(name);
    for (int i = 0; i < playerName.length(); i++) {
        playerScore[i] = 0;
    }
}

Player::~Player() {
    // Todo
}

void Player::setPlayerName(const std::string& name) {
    if (!name.empty()) {
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
int Player::getPlayerScore(int missionIndex) const {
    return playerScore[missionIndex];
}
int Player::getPlayerTotalScore() const {
    int totalScore = 0;
    for (int i : playerScore) {
        totalScore += i;
    }

    return totalScore;
}


