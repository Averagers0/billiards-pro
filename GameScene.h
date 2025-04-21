#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include <QElapsedTimer>  // 引入 QElapsedTimer
#include "Ball.h"
#include "CueBall.h"
#include "EightBall.h"
#include "SolidBall.h"
#include "StripedBall.h"
#include "GameManager.h"

class GameScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit GameScene(QObject *parent = nullptr);
    void initBalls();
    void updatePhysics();
    void checkWallCollision(Ball *ball);
    void handleBallCollisions();
    void handleTurnChange(bool shouldChangeTurn);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    QTimer *timer;
    QGraphicsPixmapItem *table;
    QVector<Ball*> balls;
    CueBall* cueBall = nullptr;

    bool cueBallInPocket = false;
    bool isPlacingCueBall = false; //目前没有用到

    QPointF aimPoint;  // 准星位置
    bool isCharging = false;  // 是否正在蓄力
    qreal chargeStrength = 0;  // 蓄力程度
    QElapsedTimer chargeTimer;  // 使用 QElapsedTimer
    QVector<QPointF> pockets; // 袋口位置
    void checkPockets();      // 检查球是否进袋

    // 添加成员变量
    GameManager *gameManager;

    bool foulOccurred = false;
    bool wasMoving = false;

};

#endif // GAMESCENE_H
