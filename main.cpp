#include "ui/mainwindow.h"

#include <QApplication>
#include <QDir>

#include "3rdparty/RunGuard.hpp"

int main(int argc, char *argv[]) {
    RunGuard guard("nekoray");
    if (!guard.tryToRun())
        return 0;

    QApplication a(argc, argv);
    QDir::setCurrent(QApplication::applicationDirPath());
    MainWindow w;
    w.show();
    return a.exec();
}
