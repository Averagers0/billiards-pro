#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H
#include <QObject>

enum PlayerType { NoneType, Solid, Striped };
enum PlayerTurn { Player1, Player2 };

class GameManager : public QObject {
    Q_OBJECT
public:
    explicit GameManager(QObject *parent = nullptr);

    void nextTurn(bool foul);
    void resetGame();

    void assignBallType(int ballNumber);
    void ballPotted(int ballNumber, bool isCueBall);
    bool isGameOver() const;

    PlayerTurn currentTurn() const;
    PlayerType currentPlayerType() const;
    PlayerType playerType(PlayerTurn player) const;

    int getSolidLeft(){return this->solidLeft;}
    int getStripedLeft(){return this->stripedLeft;}

    bool getLink(){return this->link;}
    void setLink(bool flag){this->link = flag;}

    QString PlayerTypeToString(PlayerType type) {
        static const QString strings[] = {
            "NoneType", "Solid", "Striped"
        };
        return strings[type];
    }

    bool firstHitRecorded = false;


signals:
    void turnChanged(PlayerTurn turn);
    void gameOver(PlayerTurn winner, QString reason);

private:
    PlayerTurn turn;
    PlayerType player1Type;
    PlayerType player2Type;

    bool isAssigned;
    int solidLeft;
    int stripedLeft;
    bool link = false;

};
#endif // GAMEMANAGER_H
