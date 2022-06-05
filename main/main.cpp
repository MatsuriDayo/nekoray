#include "ui/mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QTranslator>

#include "3rdparty/RunGuard.hpp"

#ifdef Q_OS_WIN
#include "sys/windows/MiniDump.h"
#endif

int main(int argc, char *argv[]) {
    // Core dump
#ifdef Q_OS_WIN
    Windows_SetCrashHandler();
#endif

    QApplication a(argc, argv);

    // Clean
    QDir::setCurrent(QApplication::applicationDirPath());
    QFile::remove("updater.old");

    // dirs
    auto wd = QDir(QApplication::applicationDirPath());
    if (!wd.exists("config")) wd.mkdir("config");
    QDir::setCurrent(wd.absoluteFilePath("config"));
    QDir("temp").removeRecursively();

    // icons
    QIcon::setFallbackSearchPaths(QStringList{
            ":/nekoray",
            ":/icon",
    });

    // icon for no theme
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
    return a.exec();
}
