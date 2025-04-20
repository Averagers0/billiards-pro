#include <QApplication>
#include <QGraphicsView>
#include "GameScene.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QGraphicsView view;
    GameScene *scene = new GameScene();
    view.setScene(scene);
    view.setRenderHint(QPainter::Antialiasing);
    view.setWindowTitle("Billiards Game - Step 1");
    view.resize(1300, 800); // 可根据需要缩放显示
    view.show();

    return a.exec();
}
