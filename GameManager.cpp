// GameManager.cpp
#include "GameManager.h"

GameManager::GameManager(QObject *parent) : QObject(parent),
    turn(Player1),
    player1Type(NoneType),
    player2Type(NoneType),
    isAssigned(false),
    solidLeft(7),
    stripedLeft(7)
{}

void GameManager::resetGame() {
    turn = Player1;
    player1Type = NoneType;
    player2Type = NoneType;
    isAssigned = false;
    solidLeft = 7;
    stripedLeft = 7;
}

void GameManager::nextTurn(bool foul) {
    Q_UNUSED(foul);  // 如果不使用 foul 参数，可以忽略
    turn = (turn == Player1) ? Player2 : Player1;
    emit turnChanged(turn);
}

PlayerTurn GameManager::currentTurn() const {
    return turn;
}

PlayerType GameManager::currentPlayerType() const {
    return (turn == Player1) ? player1Type : player2Type;
}

PlayerType GameManager::playerType(PlayerTurn player) const {
    return (player == Player1) ? player1Type : player2Type;
}

void GameManager::assignBallType(int ballNumber) {
    if (isAssigned || ballNumber == 8) return;

    if (ballNumber < 8) {
        player1Type = (turn == Player1) ? Solid : Striped;
        player2Type = (turn == Player1) ? Striped : Solid;
    } else {
        player1Type = (turn == Player1) ? Striped : Solid;
        player2Type = (turn == Player1) ? Solid : Striped;
    }

    isAssigned = true;
}

void GameManager::ballPotted(int ballNumber, bool isCueBall) {
    if (isCueBall) return;

    if (ballNumber == 8) {
        if ((currentPlayerType() == Solid && solidLeft > 0) ||
            (currentPlayerType() == Striped && stripedLeft > 0)) {
            emit gameOver((turn == Player1) ? Player2 : Player1, "提前击打8号球，犯规！");
        } else {
            emit gameOver(turn, "击球成功，赢得比赛！");
        }
    } else if (ballNumber < 8) {
        --solidLeft;
    } else {
        --stripedLeft;
    }
}

bool GameManager::isGameOver() const {
    return solidLeft == 0 || stripedLeft == 0;
}
