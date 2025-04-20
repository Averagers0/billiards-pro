#ifndef COLOREDBALL_H
#define COLOREDBALL_H
#include "Ball.h"
class ColoredBall : public Ball {
public:
    ColoredBall(int number, const QString &imgPath, qreal x, qreal y, qreal size);

    int getNumber() const override { return number; }

protected:
    int number;
};

#endif // COLOREDBALL_H
