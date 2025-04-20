#include "ColoredBall.h"

ColoredBall::ColoredBall(int number, const QString &imgPath, qreal x, qreal y, qreal size)
    : Ball(imgPath, x, y, size), number(number) {}
