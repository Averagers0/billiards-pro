#ifndef EIGHTBALL_H
#define EIGHTBALL_H
#include "ColoredBall.h"
class EightBall : public ColoredBall {
public:
    EightBall(qreal x, qreal y, qreal size)
        : ColoredBall(8, QString(":/assets/assets/ball8.png"), x, y, size) {}
    QString getType() const override { return "eight"; }
};

#endif // EIGHTBALL_H
