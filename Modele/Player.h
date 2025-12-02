//
// Created by bertr on 20-11-25.
//

#ifndef TESTCOLLISION_PLAYER_H
#define TESTCOLLISION_PLAYER_H
#include <string>

class Player {
    private:
        std::string playerName;
        //  1 Tuto
        //  1 Mission test
        //  10 Missions
        int playerScore[12]{};

    public:
        Player();
        Player(const std::string& name);
        ~Player();

        void setPlayerName(const std::string& name);
        void setPlayerScore(int missionIndex, int newScore);

        std::string getPlayerName();
        int getPlayerScore(int missionIndex) const;
        int getPlayerTotalScore() const;

};


#endif //TESTCOLLISION_PLAYER_H