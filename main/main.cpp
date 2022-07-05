#include "ui/mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QTranslator>
#include <QMessageBox>
#include <QStandardPaths>

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
#ifdef NKR_USE_APPDATA
    QApplication::setApplicationName("nekoray");
    wd.setPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
#endif
    if (!wd.exists()) wd.mkdir(wd.absolutePath());
    if (!wd.exists("config")) wd.mkdir("config");
    QDir::setCurrent(wd.absoluteFilePath("config"));
    QDir("temp").removeRecursively();

    // RunGuard
    RunGuard guard("nekoray" + wd.absolutePath());
    if (!QApplication::arguments().contains("-many")) {
        if (!guard.tryToRun()) {
            QMessageBox::warning(nullptr, "NekoRay", QObject::tr("Another program is running."));
            return 0;
        }
    }

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
    if (!dir.exists("routes")) {
        dir_success = dir_success && dir.mkdir("routes");
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

    // load routing
    NekoRay::dataStore->routing->fn = "routes/" + NekoRay::dataStore->active_routing;
    isLoaded = NekoRay::dataStore->routing->Load();
    if (!isLoaded) {
        NekoRay::dataStore->routing->Save();
    }

    // Translate
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
    QTranslator trans;
    if (trans.load(":/translations/" + locale + ".qm")) {
        QCoreApplication::installTranslator(&trans);
    }
    QTranslator trans_qt;
    if (trans_qt.load(QApplication::applicationDirPath() + "/qtbase_" + locale + ".qm")) {
        QCoreApplication::installTranslator(&trans_qt);
    }

    MainWindow w;
    return QApplication::exec();
}
