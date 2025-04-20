#include "GameScene.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <cmath>

GameScene::GameScene(QObject *parent) : QGraphicsScene(parent) {
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GameScene::updatePhysics);
    timer->start(16); // 60帧左右

    int windowWidth = 1200;
    int windowHeight = 700;

    setSceneRect(0, 0, windowWidth, windowHeight);

    // 缩放背景图以适配窗口
    QPixmap bg(":/assets/assets/table.png");
    QPixmap scaledBg = bg.scaled(windowWidth, windowHeight, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    table = addPixmap(scaledBg);
    table->setZValue(-1); // 最底层
    table->setPos(0, 0);  // 左上角对齐

    initBalls(); // 初始化球
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

    Ball *cueBall = new CueBall(cueX, cueY, scaledSize);
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
    const qreal maxCharge = 30.0;  // 最大蓄力强度

    // 处理蓄力：根据时间调整 chargeStrength
    if (isCharging) {
        qreal t = chargeTimer.elapsed() / 1000.0; // 秒
        chargeStrength = std::min(t * 10.0, maxCharge); // 每秒充10单位，最多 maxCharge
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
        }

        // 边界碰撞
        checkWallCollision(ball);
    }

    // 球与球的碰撞
    handleBallCollisions();
    update();  // ✅ 强制刷新画面（蓄力条就能每帧更新）
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
    Ball *cue = balls[0];
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
    Ball *cue = balls[0];  // 白球
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
    if (!isCharging) return;  // 🧠 没在蓄力就别画任何辅助图层

    Ball *cue = balls[0];
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

