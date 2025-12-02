//
// Created by bertr on 20-11-25.
//

#include "Player.h"
//  Bastien fait des rapports moches
//  Créer un joueur avec un nom par défaut
Player::Player() {
    setPlayerName("James Adams");
    for (int & i : playerScore) {
        i = 0;
    }
    setNbLife(3);
}

//  Créer un joueur avec un nom spécifique
Player::Player(const std::string &name) {
    setPlayerName(name);
    for (int & i : playerScore) {
        i = 0;
    }
    setNbLife(3);
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
int Player::getPlayerScore(int missionIndex) const {
    return playerScore[missionIndex];
}
//  Récupérer le score total du joueur
int Player::getPlayerTotalScore() const {
    int totalScore = 0;
    for (int i = 0; i < 12; i++) {
        totalScore += playerScore[i];
    }
    return totalScore;
}


