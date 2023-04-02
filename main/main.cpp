#include "ui/mainwindow_interface.h"

#include <csignal>

#include <QApplication>
#include <QDir>
#include <QTranslator>
#include <QMessageBox>
#include <QStandardPaths>
#include <QLocalSocket>
#include <QLocalServer>

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

#define LOCAL_SERVER_PREFIX "nekoraylocalserver-"

int main(int argc, char *argv[]) {
    // Core dump
#ifdef Q_OS_WIN
    Windows_SetCrashHandler();
#endif

    // pre-init QApplication
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication::setQuitOnLastWindowClosed(false);
    auto preQApp = new QApplication(argc, argv);

    // Clean
    QDir::setCurrent(QApplication::applicationDirPath());
    if (QFile::exists("updater.old")) {
        QFile::remove("updater.old");
    }
#ifndef Q_OS_WIN
    if (!QFile::exists("updater")) {
        QFile::link("launcher", "updater");
    }
#endif

    // Flags
    NekoRay::dataStore->argv = QApplication::arguments();
    if (NekoRay::dataStore->argv.contains("-many")) NekoRay::dataStore->flag_many = true;
    if (NekoRay::dataStore->argv.contains("-appdata")) NekoRay::dataStore->flag_use_appdata = true;
    if (NekoRay::dataStore->argv.contains("-tray")) NekoRay::dataStore->flag_tray = true;
    if (NekoRay::dataStore->argv.contains("-debug")) NekoRay::dataStore->flag_debug = true;
#ifdef NKR_CPP_USE_APPDATA
    NekoRay::dataStore->flag_use_appdata = true; // Example: Package & MacOS
#endif
#ifdef NKR_CPP_DEBUG
    NekoRay::dataStore->flag_debug = true;
#endif

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

    // HiDPI workaround
    if (ReadFileText("./groups/HiDPI").toInt() == 1) {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    }

    // init QApplication
    delete preQApp;
    QApplication a(argc, argv);

    // RunGuard
    RunGuard guard("nekoray" + wd.absolutePath());
    quint64 guard_data_in = GetRandomUint64();
    quint64 guard_data_out = 0;
    if (!NekoRay::dataStore->flag_many && !guard.tryToRun(&guard_data_in)) {
        // Some Good System
        if (guard.isAnotherRunning(&guard_data_out)) {
            // Wake up a running instance
            QLocalSocket socket;
            socket.connectToServer(LOCAL_SERVER_PREFIX + Int2String(guard_data_out));
            qDebug() << socket.fullServerName();
            if (!socket.waitForConnected(500)) {
                qDebug() << "Failed to wake a running instance.";
                return 0;
            }
            qDebug() << "connected to local server, try to raise another program";
            return 0;
        }
        // Some Bad System
        QMessageBox::warning(nullptr, "NekoRay", "RunGuard disallow to run, use -many to force start.");
        return 0;
    }
    MF_release_runguard = [&] { guard.release(); };

    // icons
    QIcon::setFallbackSearchPaths(QStringList{
        ":/nekoray",
        ":/icon",
    });

    // icon for no theme
    if (QIcon::themeName().isEmpty()) {
        QIcon::setThemeName("breeze");
    }

    // Load coreType
    NekoRay::coreType = ReadFileText("groups/coreType").toInt(); // default to 0

    // Dir
    QDir dir;
    bool dir_success = true;
    if (!dir.exists("profiles")) {
        dir_success &= dir.mkdir("profiles");
    }
    if (!dir.exists("groups")) {
        dir_success &= dir.mkdir("groups");
    }
    if (!dir.exists(ROUTES_PREFIX_NAME)) {
        dir_success &= dir.mkdir(ROUTES_PREFIX_NAME);
    }
    if (!dir_success) {
        QMessageBox::warning(nullptr, "Error", "No permission to write " + dir.absolutePath());
        return 1;
    }

    // Load dataStore
    switch (NekoRay::coreType) {
        case NekoRay::CoreType::V2RAY:
            NekoRay::dataStore->fn = "groups/nekoray.json";
            break;
        case NekoRay::CoreType::SING_BOX:
            NekoRay::dataStore->fn = "groups/nekobox.json";
            break;
        default:
            MessageBoxWarning("Error", "Unknown coreType.");
            return 0;
    }
    auto isLoaded = NekoRay::dataStore->Load();
    if (!isLoaded) {
        NekoRay::dataStore->Save();
    }

    // Datastore & Flags
    if (NekoRay::dataStore->start_minimal) NekoRay::dataStore->flag_tray = true;

    // load routing
    NekoRay::dataStore->routing->fn = ROUTES_PREFIX + NekoRay::dataStore->active_routing;
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
        case 3:
            locale = "fa_IR"; // farsi(iran)
            break;
        default:
            locale = QLocale().name();
    }
    QGuiApplication::tr("QT_LAYOUT_DIRECTION");
    QLocale::setDefault(QLocale(locale));
    //
    QTranslator trans;
    if (trans.load(":/translations/" + locale + ".qm")) {
        QCoreApplication::installTranslator(&trans);
    }
    QTranslator trans_qt;
    if (trans_qt.load(QApplication::applicationDirPath() + "/qtbase_" + locale + ".qm")) {
        QCoreApplication::installTranslator(&trans_qt);
    }

    // Signals
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    // QLocalServer
    QLocalServer server;
    auto server_name = LOCAL_SERVER_PREFIX + Int2String(guard_data_in);
    QLocalServer::removeServer(server_name);
    server.listen(server_name);
    QObject::connect(&server, &QLocalServer::newConnection, &a, [&] {
        auto socket = server.nextPendingConnection();
        qDebug() << "nextPendingConnection:" << server_name << socket;
        socket->deleteLater();
        // raise main window
        MW_dialog_message("", "Raise");
    });

    UI_InitMainWindow();
    return QApplication::exec();
}
