//
// Created by bertr on 20-11-25.
//

#include "Player.h"

//  Créer un joueur avec un nom par défaut
Player::Player() {
    setPlayerName("James Adams");
    playerScore[12] = {0};
}

//  Créer un joueur avec un nom spécifique
Player::Player(std::string name) {
    setPlayerName(name);
    playerScore[12] = {0};
}

//  Détruire un joueur
Player::~Player() {
    // Todo
}

//  Modifier le nom d'un joueur
void Player::setPlayerName(std::string name) {
    if (name != "") {
        playerName = name;
    }
}

// Modifier le score d'une mission du joueur
// Modification uniquement si meilleur score
void Player::setPlayerScore(int missionIndex, int newScore) {
    if (newScore > playerScore[missionIndex]) {
        playerScore[missionIndex] = newScore;
    }
}

//  Récupérer le nom du joueur
std::string Player::getPlayerName() {
    return playerName;
}
//  Récupérer le score du joueur pour une mission
int Player::getPlayerScore(int missionIndex) {
    return playerScore[missionIndex];
}
//  Récupérer le score total du joueur
int Player::getPlayerTotalScore() {
    int totalScore = 0;
    for (int i = 0; i < 12; i++) {
        totalScore += playerScore[i];
    }
}


