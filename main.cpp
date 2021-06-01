#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    MainWindow mWin;
    mWin.show();
    return app.exec();
}
