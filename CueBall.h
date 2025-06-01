#ifndef CUEBALL_H
#define CUEBALL_H
#include "Ball.h"
// CueBall.h
class CueBall : public Ball {
public:
    CueBall(qreal x, qreal y, qreal size);
    QString getType() const override;
    int getNumber() const override;
};

#endif // CUEBALL_H
