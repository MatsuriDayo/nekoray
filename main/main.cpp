#include <csignal>

#include <QApplication>
#include <QDir>
#include <QTranslator>
#include <QMessageBox>
#include <QStandardPaths>
#include <QLocalSocket>
#include <QLocalServer>
#include <QThread>

#include "3rdparty/RunGuard.hpp"
#include "main/NekoGui.hpp"

#include "ui/mainwindow_interface.h"
#include "ui/dialog_first_setup.h"

#ifdef Q_OS_WIN
#include "sys/windows/MiniDump.h"
#endif

void signal_handler(int signum) {
    if (qApp) {
        GetMainWindow()->on_commitDataRequest();
        qApp->exit();
    }
}

QTranslator* trans = nullptr;
QTranslator* trans_qt = nullptr;

void loadTranslate(const QString& locale) {
    if (trans != nullptr) {
        trans->deleteLater();
    }
    if (trans_qt != nullptr) {
        trans_qt->deleteLater();
    }
    //
    trans = new QTranslator;
    trans_qt = new QTranslator;
    QLocale::setDefault(QLocale(locale));
    //
    if (trans->load(":/translations/" + locale + ".qm")) {
        QCoreApplication::installTranslator(trans);
    }
    if (trans_qt->load(QApplication::applicationDirPath() + "/qtbase_" + locale + ".qm")) {
        QCoreApplication::installTranslator(trans_qt);
    }
}

#define LOCAL_SERVER_PREFIX "nekoraylocalserver-"

int main(int argc, char* argv[]) {
    // Core dump
#ifdef Q_OS_WIN
    Windows_SetCrashHandler();
#endif

    // pre-init QApplication
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
#endif
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
    NekoGui::dataStore->argv = QApplication::arguments();
    if (NekoGui::dataStore->argv.contains("-many")) NekoGui::dataStore->flag_many = true;
    if (NekoGui::dataStore->argv.contains("-appdata")) {
        NekoGui::dataStore->flag_use_appdata = true;
        int appdataIndex = NekoGui::dataStore->argv.indexOf("-appdata");
        if (NekoGui::dataStore->argv.size() > appdataIndex + 1 && !NekoGui::dataStore->argv.at(appdataIndex + 1).startsWith("-")) {
            NekoGui::dataStore->appdataDir = NekoGui::dataStore->argv.at(appdataIndex + 1);
        }
    }
    if (NekoGui::dataStore->argv.contains("-tray")) NekoGui::dataStore->flag_tray = true;
    if (NekoGui::dataStore->argv.contains("-debug")) NekoGui::dataStore->flag_debug = true;
    if (NekoGui::dataStore->argv.contains("-flag_linux_run_core_as_admin")) NekoGui::dataStore->flag_linux_run_core_as_admin = true;
    if (NekoGui::dataStore->argv.contains("-flag_restart_tun_on")) NekoGui::dataStore->flag_restart_tun_on = true;
    if (NekoGui::dataStore->argv.contains("-flag_reorder")) NekoGui::dataStore->flag_reorder = true;
#ifdef NKR_CPP_USE_APPDATA
    NekoGui::dataStore->flag_use_appdata = true; // Example: Package & MacOS
#endif
#ifdef NKR_CPP_DEBUG
    NekoGui::dataStore->flag_debug = true;
#endif

    // dirs & clean
    auto wd = QDir(QApplication::applicationDirPath());
    if (NekoGui::dataStore->flag_use_appdata) {
        QApplication::setApplicationName("nekoray");
        if (!NekoGui::dataStore->appdataDir.isEmpty()) {
            wd.setPath(NekoGui::dataStore->appdataDir);
        } else {
            wd.setPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
        }
    }
    if (!wd.exists()) wd.mkpath(wd.absolutePath());
    if (!wd.exists("config")) wd.mkdir("config");
    QDir::setCurrent(wd.absoluteFilePath("config"));
    QDir("temp").removeRecursively();

    // init QApplication
    delete preQApp;
    QApplication a(argc, argv);

    // dispatchers
    DS_cores = new QThread;
    DS_cores->start();

    // RunGuard
    RunGuard guard("nekoray" + wd.absolutePath());
    quint64 guard_data_in = GetRandomUint64();
    quint64 guard_data_out = 0;
    if (!NekoGui::dataStore->flag_many && !guard.tryToRun(&guard_data_in)) {
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    QIcon::setFallbackSearchPaths(QStringList{
        ":/nekoray",
        ":/icon",
    });
#endif

    // icon for no theme
    if (QIcon::themeName().isEmpty()) {
        QIcon::setThemeName("breeze");
    }

    // Load coreType
    auto coreLoaded = ReadFileText("groups/coreType");
    if (coreLoaded.isEmpty()) {
        NekoGui::coreType = -1;
        loadTranslate(QLocale().name());
        auto dialogFirstSetup = new DialogFirstSetup;
        dialogFirstSetup->exec();
        dialogFirstSetup->deleteLater();
        if (NekoGui::coreType < 0) {
            return 0;
        } else {
            QDir().mkdir("groups");
            QFile file;
            file.setFileName("groups/coreType");
            file.open(QIODevice::ReadWrite | QIODevice::Truncate);
            file.write(Int2String(NekoGui::coreType).toUtf8());
            file.close();
        }
    } else {
        NekoGui::coreType = coreLoaded.toInt();
    }

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
    switch (NekoGui::coreType) {
        case NekoGui::CoreType::V2RAY:
            NekoGui::dataStore->fn = "groups/nekoray.json";
            break;
        case NekoGui::CoreType::SING_BOX:
            NekoGui::dataStore->fn = "groups/nekobox.json";
            break;
        default:
            MessageBoxWarning("Error", "Unknown coreType.");
            return 0;
    }
    auto isLoaded = NekoGui::dataStore->Load();
    if (!isLoaded) {
        NekoGui::dataStore->Save();
    }

    // Datastore & Flags
    if (NekoGui::dataStore->start_minimal) NekoGui::dataStore->flag_tray = true;

    // load routing
    NekoGui::dataStore->routing = std::make_unique<NekoGui::Routing>();
    NekoGui::dataStore->routing->fn = ROUTES_PREFIX + NekoGui::dataStore->active_routing;
    isLoaded = NekoGui::dataStore->routing->Load();
    if (!isLoaded) {
        NekoGui::dataStore->routing->Save();
    }

    // Translate
    QString locale;
    switch (NekoGui::dataStore->language) {
        case 1: // English
            break;
        case 2:
            locale = "zh_CN";
            break;
        case 3:
            locale = "fa_IR"; // farsi(iran)
            break;
        case 4:
            locale = "ru_RU"; // Russian
            break;
        default:
            locale = QLocale().name();
    }
    QGuiApplication::tr("QT_LAYOUT_DIRECTION");
    loadTranslate(locale);

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

    // Do preset update
    if (NekoGui::dataStore->user_agent.isEmpty() || NekoGui::dataStore->user_agent.startsWith("Nekoray/1.0") || NekoGui::dataStore->user_agent.startsWith("ClashForAndroid")) {
        if (IS_NEKO_BOX) {
            NekoGui::dataStore->user_agent = "NekoBox/PC/2.0 (Prefer ClashMeta Format)";
        } else {
            NekoGui::dataStore->user_agent = "NekoRay/PC/2.0 (Prefer ClashMeta Format)";
        }
        NekoGui::dataStore->Save();
    }

    UI_InitMainWindow();
    return QApplication::exec();
}
