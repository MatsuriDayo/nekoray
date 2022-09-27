#include "ui/mainwindow.h"

#include <csignal>

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

void signal_handler(int signum) {
    if (qApp) {
        GetMainWindow()->on_commitDataRequest();
        qApp->exit();
    }
}

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
    if (QFile::exists("updater.old")) {
        QFile::remove("updater.old");
        QFile::remove("sing-box.exe"); // v1.11
    }
#ifndef Q_OS_WIN
    if (!QFile::exists("updater")) {
        QFile::link("launcher", "updater");
    }
#endif

    // Flags
    auto args = QApplication::arguments();
    if (args.contains("-many")) NekoRay::dataStore->flag_many = true;
    if (args.contains("-appdata")) NekoRay::dataStore->flag_use_appdata = true;

    // dirs & clean
    auto wd = QDir(QApplication::applicationDirPath());
    if (NekoRay::dataStore->flag_use_appdata) {
        QApplication::setApplicationName("nekoray");
        wd.setPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }
    if (!wd.exists()) wd.mkdir(wd.absolutePath());
    if (!wd.exists("config")) wd.mkdir("config");
    QDir::setCurrent(wd.absoluteFilePath("config"));
    QDir("temp").removeRecursively();

    // RunGuard
    RunGuard guard("nekoray" + wd.absolutePath());
    if (!NekoRay::dataStore->flag_many) {
        if (!guard.tryToRun()) {
            QMessageBox::warning(nullptr, "NekoRay", QObject::tr("Another program is running."));
            return 0;
        }
    }
    release_runguard = [&] { guard.release(); };

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

    //Signals
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    MainWindow w;
    return QApplication::exec();
}
