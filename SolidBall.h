#ifndef SOLIDBALL_H
#define SOLIDBALL_H
#include "ColoredBall.h"
class SolidBall : public ColoredBall {
public:
    SolidBall(int number, qreal x, qreal y, qreal size)
        : ColoredBall(number, QString(":/assets/assets/ball%1.png").arg(number), x, y, size) {}
    QString getType() const override { return "Solid"; }
};

#endif // SOLIDBALL_H
