#ifndef STRIPEDBALL_H
#define STRIPEDBALL_H
#include "ColoredBall.h"
class StripedBall : public ColoredBall {
public:
    StripedBall(int number, qreal x, qreal y, qreal size)
        : ColoredBall(number, QString(":/assets/assets/ball%1.png").arg(number), x, y, size) {}
    QString getType() const override { return "Striped"; }
};
#endif // STRIPEDBALL_H
