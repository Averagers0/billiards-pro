#include "GameScene.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <cmath>
#include <QDateTime>

GameScene::GameScene(QObject *parent) : QGraphicsScene(parent) {
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GameScene::updatePhysics);
    timer->start(16); // 60帧左右

    int windowWidth = 1200;
    int windowHeight = 700;

    // 桌球桌大概边距与坐标参考设定
    pockets = {
        QPointF(60, 60),                       // 左上
        QPointF(windowWidth / 2 + 10, 55),         // 中上
        QPointF(windowWidth - 30, 60),        // 右上
        QPointF(60, windowHeight - 60),       // 左下
        QPointF(windowWidth / 2 + 10, windowHeight - 55), // 中下
        QPointF(windowWidth - 30, windowHeight - 60) // 右下
    };

    setSceneRect(0, 0, windowWidth, windowHeight);

    QGraphicsTextItem *playerInfoText;
    playerInfoText = new QGraphicsTextItem();
    playerInfoText->setDefaultTextColor(Qt::black);
    playerInfoText->setFont(QFont("Arial", 16, QFont::Bold));
    playerInfoText->setZValue(1); // 确保在前面
    playerInfoText->setPos(-50, -50); // 放在左上角
    addItem(playerInfoText);

    // 初始化内容
    playerInfoText->setPlainText("当前玩家: 玩家1");

    // 缩放背景图以适配窗口
    QPixmap bg(":/assets/assets/table.png");
    QPixmap scaledBg = bg.scaled(windowWidth, windowHeight, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    table = addPixmap(scaledBg);
    table->setZValue(-1); // 最底层
    table->setPos(0, 0);  // 左上角对齐
    initBalls(); // 初始化球

    gameManager = new GameManager(this);

    connect(gameManager, &GameManager::turnChanged, this, [=](PlayerTurn turn){
        qDebug() << "现在轮到玩家：" << (turn == Player1 ? "玩家1" : "玩家2");
        playerInfoText->setPlainText(QString("当前玩家: %1").arg(turn == Player1 ? "玩家1" : "玩家2"));
    });
    connect(gameManager, &GameManager::gameOver, this, [=](PlayerTurn winner, QString reason){
        qDebug() << "🎉 游戏结束，" << (winner == Player1 ? "玩家1" : "玩家2") << " 获胜，原因：" << reason;
    });
}

void GameScene::initBalls() {
    const int originalWidth = 4551;
    const int originalHeight = 2570;
    const int windowWidth = 1200;
    const int windowHeight = 700;

    qreal scaleX = static_cast<qreal>(windowWidth) / originalWidth;
    qreal scaleY = static_cast<qreal>(windowHeight) / originalHeight;

    const int scaledSize = 30;
    const int spacing = scaledSize + 3;

    // ---- 添加白球 ----
    qreal cueX = 1000 * scaleX;
    qreal cueY = (1285 - scaledSize / 2) * scaleY;

    cueBall = new CueBall(cueX, cueY, scaledSize);
    addItem(cueBall);
    balls.append(cueBall);

    // ---- 彩球起始位置 ----
    QPointF startPos(3000 * scaleX, 1285 * scaleY);
    int index = 1;

    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col <= row; ++col) {
            if (index > 15) return;

            qreal x = startPos.x() + row * spacing;
            qreal y = startPos.y() - (row * spacing / 2) + col * spacing;

            Ball *ball = nullptr;

            if (index == 8) {
                ball = new EightBall(x, y, scaledSize);
            } else if (index <= 7) {
                ball = new SolidBall(index, x, y, scaledSize);
            } else {
                ball = new StripedBall(index, x, y, scaledSize);
            }

            addItem(ball);
            balls.append(ball);
            ++index;
        }
    }
}

void GameScene::updatePhysics() {
    const qreal friction = 0.99;
    const qreal minSpeed = 0.1;
    const qreal maxCharge = 30.0;

    bool allStopped = true; // 新增：标记所有球是否静止

    // 处理蓄力
    if (isCharging) {
        qreal t = chargeTimer.elapsed() / 1000.0;
        chargeStrength = std::min(t * 10.0, maxCharge);
    }

    for (Ball *ball : balls) {
        // 更新位置
        QPointF v = ball->velocity;
        QPointF newPos = ball->pos() + v;
        ball->setPos(newPos);

        // 摩擦力减速
        ball->velocity *= friction;

        // 停止判断
        if (std::hypot(ball->velocity.x(), ball->velocity.y()) < minSpeed) {
            ball->velocity = QPointF(0, 0);
        } else {
            allStopped = false; // 只要有球在动，就不算静止
        }

        // 边界碰撞
        checkWallCollision(ball);
    }
    checkPockets();
    handleBallCollisions();

    handleTurnChange(allStopped);
    update();
}

void GameScene::checkWallCollision(Ball *ball) {
    QRectF bounds = sceneRect();
    QPointF pos = ball->pos();
    QPointF v = ball->velocity;
    int r = ball->radius();

    // 碰左
    if (pos.x() - r <= bounds.left() + 50) {
        pos.setX(bounds.left() + 50 + r);
        v.setX(-v.x());
    }
    // 碰右
    else if (pos.x() + r >= bounds.right() - 35) {
        pos.setX(bounds.right() - 35 - r);
        v.setX(-v.x());
    }

    // 碰上
    if (pos.y() - r <= bounds.top() + 50) {
        pos.setY(bounds.top() + 50 + r);
        v.setY(-v.y());
    }
    // 碰下
    else if (pos.y() + r >= bounds.bottom() - 80) {
        pos.setY(bounds.bottom() - 80 - r);
        v.setY(-v.y());
    }

    ball->setPos(pos);
    ball->velocity = v;
}

void GameScene::handleBallCollisions() {
    for (int i = 0; i < balls.size(); ++i) {
        for (int j = i + 1; j < balls.size(); ++j) {
            Ball *a = balls[i];
            Ball *b = balls[j];

            QPointF delta = b->pos() - a->pos();
            qreal dist = std::hypot(delta.x(), delta.y());
            int rSum = a->radius() + b->radius();

            if (dist < rSum && dist > 0) {
                // 简单弹性碰撞
                QPointF normal = delta / dist;
                QPointF relativeVelocity = b->velocity - a->velocity;
                qreal velAlongNormal = QPointF::dotProduct(relativeVelocity, normal);

                if (velAlongNormal < 0) { // 正在靠近
                    // 碰撞响应：简单的速度交换（同质量）
                    QPointF impulse = normal * velAlongNormal;
                    a->velocity += impulse;
                    b->velocity -= impulse;

                    // 修正重叠
                    qreal overlap = rSum - dist;
                    QPointF correction = normal * (overlap / 2.0);
                    a->setPos(a->pos() - correction);
                    b->setPos(b->pos() + correction);
                }

                Ball *c;
                Ball *d;
                if(a->getType() == "cue" || b->getType() == "cue"){
                    if(a->getType() == "cue"){
                        c = a;
                        d = b;
                    }
                    else{
                        c = b;
                        d = a;
                    }

                    if(!gameManager->firstHitRecorded && gameManager->currentPlayerType() != 0){
                        if(gameManager->PlayerTypeToString(gameManager->currentPlayerType()) != d->getType()){
                            qDebug()<< "test" << c->getNumber();
                            qDebug() << "test" << d->getNumber();
                            gameManager->firstHitRecorded = true;
                            foulOccurred = true;
                        }
                    }
                }
            }
        }
    }
}

void GameScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        // 右键取消蓄力
        isCharging = false;
        chargeStrength = 0;
        update(); // 触发界面刷新（蓄力条会消失）
        return;
    }

    // 👉 白球没停下就不能蓄力
    Ball *cue = cueBall;
    if (cue->velocity != QPointF(0, 0)) return;

    isCharging = true;
    chargeStrength = 0;
    aimPoint = event->scenePos();  // 记录准星位置
    chargeTimer.start();           // 启动计时器
    update();
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    // 这里不再更新 aimPoint，除非你想让准星跟鼠标走
    update();
}

void GameScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (!isCharging) return;

    // 计算击球方向
    Ball *cue = cueBall;  // 白球
    QPointF cueCenter = cue->pos() + QPointF(cue->pixmap().width()/2, cue->pixmap().height()/2);
    QPointF dir = aimPoint - cueCenter;
    qreal dist = std::hypot(dir.x(), dir.y());
    if (dist > 0) {
        QPointF unitDir = dir / dist;
        cue->velocity = unitDir * chargeStrength;
    }

    isCharging = false;  // 释放蓄力
    update();
}

void GameScene::drawForeground(QPainter *painter, const QRectF &rect) {
    Q_UNUSED(rect);

    // ✅ 临时绘制洞口位置（用于测试）
    painter->setPen(QPen(Qt::green, 2));  // 绿色空心圆
    for (const QPointF &pocket : pockets) {
        painter->drawEllipse(pocket, 35, 35);  // 和 checkPockets 一致的口径
    }

    if (!isCharging) return;  // 🧠 没在蓄力就别画任何辅助图层

    Ball *cue = cueBall;
    QPointF cueCenter = cue->pos() + QPointF(cue->pixmap().width() / 2, cue->pixmap().height() / 2);

    // 画准星
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::red);
    painter->drawEllipse(aimPoint, 5, 5);

    // 画辅助线
    QPen pen(Qt::gray, 2, Qt::DashLine);
    painter->setPen(pen);
    painter->drawLine(cueCenter, aimPoint);

    // 画蓄力条
    int barW = 100, barH = 8;
    QPointF barPos = cueCenter + QPointF(-barW / 2, -50);

    painter->setBrush(Qt::gray);
    painter->drawRect(QRectF(barPos, QSizeF(barW, barH)));
    painter->setBrush(Qt::blue);
    painter->drawRect(QRectF(barPos, QSizeF(chargeStrength / 20.0 * barW, barH)));
}

void GameScene::checkPockets() {
    const int pocketRadius = 35;

    for (int i = balls.size() - 1; i >= 0; --i) {
        Ball *ball = balls[i];
        QPointF center = ball->pos() + QPointF(ball->radius(), ball->radius());

        for (const QPointF &pocket : pockets) {
            if (QLineF(center, pocket).length() < pocketRadius) {
                // 如果是白球，特殊处理（犯规）
                if (dynamic_cast<CueBall*>(ball)) {
                    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "白球进袋";
                    foulOccurred = true;

                    // 隐藏白球（移出画面）
                    ball->setPos(-100, -100);
                    ball->velocity = QPointF(0, 0);
                    cueBallInPocket = true; // 添加这个变量用于重置时判断

                } else {
                    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "球进袋，编号:" << ball->getNumber();
                    gameManager->assignBallType(ball->getNumber());

                    if(gameManager->PlayerTypeToString(gameManager->currentPlayerType()) == ball->getType() && !foulOccurred){
                        qDebug()<< gameManager->PlayerTypeToString(gameManager->currentPlayerType()) ;
                        gameManager->setLink(true);
                    }

                    removeItem(ball);
                    balls.removeAt(i);
                    gameManager->ballPotted(ball->getNumber(),false);
                    qDebug() << gameManager->getSolidLeft() << " " << gameManager->getStripedLeft();
                    delete ball;
                }

                break; // 进袋后跳出检查
            }
        }
    }
}

void GameScene::handleTurnChange(bool allStopped) {
    // 处理球移动状态
    if (!allStopped) {
        wasMoving = true;
        return;
    }

    // 所有球都静止了，但之前有移动
    if (wasMoving) {
        // 情况2：发生犯规（未记录第一击球或其它犯规）
        if (foulOccurred) {
            if(gameManager->firstHitRecorded){
                qDebug() << "犯规，打错球了";
                gameManager->nextTurn(true); // 犯规换人

                foulOccurred = false;
                gameManager->firstHitRecorded = false;
                wasMoving = false;
                gameManager->setLink(true);

                return;
            }
            else if(cueBallInPocket){
                cueBall->setPos(1000 * sceneRect().width() / 4551.0,
                                (1285 - cueBall->radius()) * sceneRect().height() / 2570.0);
                cueBall->velocity = QPointF(0, 0);

                cueBallInPocket = false;
            }
            else{
                qDebug() << "犯规";
                gameManager->nextTurn(true); // 犯规换人

                foulOccurred = false;
                wasMoving = false;
                gameManager->setLink(true);
            }
            return;
        }

        // 情况3：连续击球状态
        if (gameManager->getLink()) {
            gameManager->setLink(false);
            wasMoving = false;
            return;
        }

        // 情况4：正常换人
        gameManager->nextTurn(false);
        wasMoving = false;
    }
}

