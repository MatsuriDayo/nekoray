#include "ui/mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QTranslator>

#include "3rdparty/RunGuard.hpp"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // dirs
    auto wd = QDir(QApplication::applicationDirPath());
    if (!wd.exists("config")) wd.mkdir("config");
    QDir::setCurrent(wd.absoluteFilePath("config"));

    // icons
    QIcon::setFallbackSearchPaths(QStringList{
            ":neko-breeze",
            ":nekoray",
    });

    // TODO theme
    if (QIcon::themeName().isEmpty()) {
        QIcon::setThemeName("breeze");
    }

    // Locale
    QTranslator trans;
    if (trans.load(":/translations/" + QLocale().name() + ".qm")) {
        QCoreApplication::installTranslator(&trans);
    }

    RunGuard guard("nekoray" + wd.absolutePath());
    if (!guard.tryToRun())
        return 0;

    MainWindow w;
    w.show();
    return a.exec();
}
