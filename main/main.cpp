#include "ui/mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QTranslator>

#include "3rdparty/RunGuard.hpp"
#include "main/NekoRay.hpp"

#ifdef Q_OS_WIN
#include "sys/windows/MiniDump.h"
#endif

int main(int argc, char *argv[]) {
    // Core dump
#ifdef Q_OS_WIN
    Windows_SetCrashHandler();
#endif

    QApplication a(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);

    // Clean
    QDir::setCurrent(QApplication::applicationDirPath());
    QFile::remove("updater.old");

    // dirs & clean
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

    // Dir
    QDir dir;
    bool dir_success = true;
    if (!dir.exists("profiles")) {
        dir_success = dir_success && dir.mkdir("profiles");
    }
    if (!dir.exists("groups")) {
        dir_success = dir_success && dir.mkdir("groups");
    }
    if (!dir_success) {
        QMessageBox::warning(nullptr, "Error", "No permission to write " + dir.absolutePath());
        return 1;
    }

    // Load dataStore
    auto isLoaded = NekoRay::dataStore->Load();
    if (!isLoaded) {
        NekoRay::dataStore->Save();
    }

    // Translate
    QTranslator trans;
    QString locale;
    switch (NekoRay::dataStore->language) {
        case 1: // English
            break;
        case 2:
            locale = "zh_CN";
            break;
        default:
            locale = QLocale().name();
    }
    if (trans.load(":/translations/" + locale + ".qm")) {
        QCoreApplication::installTranslator(&trans);
    }

    RunGuard guard("nekoray" + wd.absolutePath());
    if (!guard.tryToRun())
        return 0;

    MainWindow w;
    return QApplication::exec();
}
