#ifndef BALL_H
#define BALL_H
#include <QGraphicsPixmapItem>
class Ball : public QGraphicsPixmapItem {
public:
    Ball(const QString &imgPath, qreal x, qreal y, qreal size);
    virtual QString getType() const = 0;
    virtual int getNumber() const = 0;
    QPointF velocity;
    int radius(); // 球的半径
};

#endif // BALL_H
