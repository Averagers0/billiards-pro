// Ball.cpp
#include "Ball.h"

Ball::Ball(const QString &imgPath, qreal x, qreal y, qreal size) {
    QPixmap pix(imgPath);
    setPixmap(pix.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    setPos(x, y);

}
int Ball::radius() {
    // radius 是一半的缩放后的宽度（假设球是正圆）
    return pixmap().width() / 2;
}
