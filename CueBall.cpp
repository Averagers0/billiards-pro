// CueBall.cpp
#include "CueBall.h"

CueBall::CueBall(qreal x, qreal y, qreal size)
    : Ball(":/assets/assets/ball16.png", x, y, size) {}

QString CueBall::getType() const { return "cue"; }
int CueBall::getNumber() const { return 0; }

