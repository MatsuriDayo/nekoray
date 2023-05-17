#include "./ui_mainwindow.h"
#include "mainwindow.h"

#include "fmt/Preset.hpp"
#include "db/ProfileFilter.hpp"
#include "db/ConfigBuilder.hpp"
#include "sub/GroupUpdater.hpp"
#include "sys/ExternalProcess.hpp"
#include "sys/AutoRun.hpp"

#include "ui/ThemeManager.hpp"
#include "ui/Icon.hpp"
#include "ui/edit/dialog_edit_profile.h"
#include "ui/dialog_basic_settings.h"
#include "ui/dialog_manage_groups.h"
#include "ui/dialog_manage_routes.h"
#include "ui/dialog_vpn_settings.h"
#include "ui/dialog_hotkey.h"

#include "3rdparty/qrcodegen.hpp"
#include "3rdparty/VT100Parser.hpp"
#include "3rdparty/qv2ray/v2/components/proxy/QvProxyConfigurator.hpp"
#include "3rdparty/qv2ray/v2/ui/LogHighlighter.hpp"

#ifndef NKR_NO_ZXING
#include "3rdparty/ZxingQtReader.hpp"
#endif

#ifdef Q_OS_WIN
#include "3rdparty/WinCommander.hpp"
#else
#include <unistd.h>
#endif

#include <QClipboard>
#include <QLabel>
#include <QTextBlock>
#include <QScrollBar>
#include <QMutex>
#include <QScreen>
#include <QDesktopServices>
#include <QInputDialog>
#include <QThread>
#include <QTimer>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QElapsedTimer>

QElapsedTimer coreRestartTimer;

void UI_InitMainWindow() {
    mainwindow = new MainWindow;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    mainwindow = this;
    MW_dialog_message = [=](const QString &a, const QString &b) {
        runOnUiThread([=] { dialog_message_impl(a, b); });
    };

    // Load Manager
    auto isLoaded = NekoRay::profileManager->Load();
    if (!isLoaded) {
        auto defaultGroup = NekoRay::ProfileManager::NewGroup();
        defaultGroup->name = tr("Default");
        NekoRay::profileManager->AddGroup(defaultGroup);
    }

    // Setup misc UI
    themeManager->ApplyTheme(NekoRay::dataStore->theme);
    ui->setupUi(this);
    connect(ui->menu_start, &QAction::triggered, this, [=]() { neko_start(); });
    connect(ui->menu_stop, &QAction::triggered, this, [=]() { neko_stop(); });
    connect(ui->tabWidget->tabBar(), &QTabBar::tabMoved, this, [=](int from, int to) {
        // use tabData to track tab & gid
        NekoRay::profileManager->_groups.clear();
        for (int i = 0; i < ui->tabWidget->tabBar()->count(); i++) {
            NekoRay::profileManager->_groups += ui->tabWidget->tabBar()->tabData(i).toInt();
        }
        NekoRay::profileManager->Save();
    });
    ui->label_running->installEventFilter(this);
    ui->label_inbound->installEventFilter(this);
    RegisterHotkey(false);
    auto last_size = NekoRay::dataStore->mw_size.split("x");
    if (last_size.length() == 2) {
        auto w = last_size[0].toInt();
        auto h = last_size[1].toInt();
        if (w > 0 && h > 0) {
            resize(w, h);
        }
    }

    // software_name
    if (IS_NEKO_BOX) {
        software_name = "NekoBox";
        software_core_name = "sing-box";
        // replace default values
        if (NekoRay::dataStore->log_level == "warning") NekoRay::dataStore->log_level = "info";
        if (!Preset::SingBox::DomainStrategy.contains(NekoRay::dataStore->routing->domain_strategy)) NekoRay::dataStore->routing->domain_strategy = "";
        if (!Preset::SingBox::DomainStrategy.contains(NekoRay::dataStore->routing->outbound_domain_strategy)) NekoRay::dataStore->routing->outbound_domain_strategy = "";
        //
        if (QDir("dashboard").isEmpty()) {
            QDir().mkdir("dashboard");
            QFile::copy(":/neko/dashboard-notice.html", "dashboard/index.html");
        }
    }

    // top bar
    ui->toolButton_program->setMenu(ui->menu_program);
    ui->toolButton_preferences->setMenu(ui->menu_preferences);
    ui->toolButton_server->setMenu(ui->menu_server);
    ui->menubar->setVisible(false);
    connect(ui->toolButton_document, &QToolButton::clicked, this, [=] { QDesktopServices::openUrl(QUrl("https://matsuridayo.github.io/")); });
    connect(ui->toolButton_ads, &QToolButton::clicked, this, [=] { QDesktopServices::openUrl(QUrl("https://matsuricom.github.io/")); });
    connect(ui->toolButton_update, &QToolButton::clicked, this, [=] { runOnNewThread([=] { CheckUpdate(); }); });

    // Setup log UI
    ui->splitter->restoreState(DecodeB64IfValid(NekoRay::dataStore->splitter_state));
    new SyntaxHighlighter(false, qvLogDocument);
    qvLogDocument->setUndoRedoEnabled(false);
    ui->masterLogBrowser->setUndoRedoEnabled(false);
    ui->masterLogBrowser->setDocument(qvLogDocument);
    ui->masterLogBrowser->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    {
        auto font = ui->masterLogBrowser->font();
        font.setPointSize(9);
        ui->masterLogBrowser->setFont(font);
        qvLogDocument->setDefaultFont(font);
    }
    connect(ui->masterLogBrowser->verticalScrollBar(), &QSlider::valueChanged, this, [=](int value) {
        if (ui->masterLogBrowser->verticalScrollBar()->maximum() == value)
            qvLogAutoScoll = true;
        else
            qvLogAutoScoll = false;
    });
    connect(ui->masterLogBrowser, &QTextBrowser::textChanged, this, [=]() {
        if (!qvLogAutoScoll)
            return;
        auto bar = ui->masterLogBrowser->verticalScrollBar();
        bar->setValue(bar->maximum());
    });
    MW_show_log = [=](const QString &log) {
        runOnUiThread([=] { show_log_impl(log); });
    };
    MW_show_log_ext = [=](const QString &tag, const QString &log) {
        runOnUiThread([=] { show_log_impl("[" + tag + "] " + log); });
    };
    MW_show_log_ext_vt100 = [=](const QString &log) {
        runOnUiThread([=] { show_log_impl(cleanVT100String(log)); });
    };

    // table UI
    ui->proxyListTable->callback_save_order = [=] {
        auto group = NekoRay::profileManager->CurrentGroup();
        group->order = ui->proxyListTable->order;
        group->Save();
    };
    ui->proxyListTable->refresh_data = [=](int id) { refresh_proxy_list_impl_refresh_data(id); };
    if (auto button = ui->proxyListTable->findChild<QAbstractButton *>(QString(), Qt::FindDirectChildrenOnly)) {
        // Corner Button
        connect(button, &QAbstractButton::clicked, this, [=] { refresh_proxy_list_impl(-1, {NekoRay::GroupSortMethod::ById}); });
    }
    connect(ui->proxyListTable->horizontalHeader(), &QHeaderView::sectionClicked, this, [=](int logicalIndex) {
        NekoRay::GroupSortAction action;
        // 不正确的descending实现
        if (proxy_last_order == logicalIndex) {
            action.descending = true;
            proxy_last_order = -1;
        } else {
            proxy_last_order = logicalIndex;
        }
        action.save_sort = true;
        // 表头
        if (logicalIndex == 0) {
            action.method = NekoRay::GroupSortMethod::ByType;
        } else if (logicalIndex == 1) {
            action.method = NekoRay::GroupSortMethod::ByAddress;
        } else if (logicalIndex == 2) {
            action.method = NekoRay::GroupSortMethod::ByName;
        } else if (logicalIndex == 3) {
            action.method = NekoRay::GroupSortMethod::ByLatency;
        } else {
            return;
        }
        refresh_proxy_list_impl(-1, action);
    });
    connect(ui->proxyListTable->horizontalHeader(), &QHeaderView::sectionResized, this, [=](int logicalIndex, int oldSize, int newSize) {
        auto group = NekoRay::profileManager->CurrentGroup();
        if (NekoRay::dataStore->refreshing_group || group == nullptr || !group->manually_column_width) return;
        // save manually column width
        group->column_width.clear();
        for (int i = 0; i < ui->proxyListTable->horizontalHeader()->count(); i++) {
            group->column_width.push_back(ui->proxyListTable->horizontalHeader()->sectionSize(i));
        }
        group->column_width[logicalIndex] = newSize;
        group->Save();
    });
    ui->tableWidget_conn->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget_conn->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableWidget_conn->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->proxyListTable->verticalHeader()->setDefaultSectionSize(24);

    // search box
    ui->search->setVisible(false);
    connect(shortcut_ctrl_f, &QShortcut::activated, this, [=] {
        ui->search->setVisible(true);
        ui->search->setFocus();
    });
    connect(shortcut_esc, &QShortcut::activated, this, [=] {
        if (ui->search->isVisible()) {
            ui->search->setText("");
            ui->search->textChanged("");
            ui->search->setVisible(false);
        }
        if (select_mode) {
            emit profile_selected(-1);
            select_mode = false;
            refresh_status();
        }
    });
    connect(ui->search, &QLineEdit::textChanged, this, [=](const QString &text) {
        if (text.isEmpty()) {
            for (int i = 0; i < ui->proxyListTable->rowCount(); i++) {
                ui->proxyListTable->setRowHidden(i, false);
            }
        } else {
            QList<QTableWidgetItem *> findItem = ui->proxyListTable->findItems(text, Qt::MatchContains);
            for (int i = 0; i < ui->proxyListTable->rowCount(); i++) {
                ui->proxyListTable->setRowHidden(i, true);
            }
            for (auto item: findItem) {
                if (item != nullptr) ui->proxyListTable->setRowHidden(item->row(), false);
            }
        }
    });

    // refresh
    this->refresh_groups();

    // Setup Tray
    tray = new QSystemTrayIcon(this); // 初始化托盘对象tray
    tray->setIcon(Icon::GetTrayIcon(Icon::NONE));
    tray->setContextMenu(ui->menu_program); // 创建托盘菜单
    tray->show();                           // 让托盘图标显示在系统托盘上
    connect(tray, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason) {
        switch (reason) {
            case QSystemTrayIcon::Trigger:
                if (this->isVisible()) {
                    hide();
                } else {
                    ACTIVE_THIS_WINDOW
                }
                break;
            default:
                break;
        }
    });

    // Misc menu
    connect(ui->menu_open_config_folder, &QAction::triggered, this, [=] { QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::currentPath())); });
    ui->menu_program_preference->addActions(ui->menu_preferences->actions());
    connect(ui->menu_add_from_clipboard2, &QAction::triggered, ui->menu_add_from_clipboard, &QAction::trigger);
    connect(ui->actionRestart_Proxy, &QAction::triggered, this, [=] { if (NekoRay::dataStore->started_id>=0) neko_start(NekoRay::dataStore->started_id); });
    connect(ui->actionRestart_Program, &QAction::triggered, this, [=] { MW_dialog_message("", "RestartProgram"); });
    connect(ui->actionShow_window, &QAction::triggered, this, [=] { tray->activated(QSystemTrayIcon::ActivationReason::Trigger); });
    //
    connect(ui->menu_program, &QMenu::aboutToShow, this, [=]() {
        ui->actionRemember_last_proxy->setChecked(NekoRay::dataStore->remember_enable);
        ui->actionStart_with_system->setChecked(AutoRun_IsEnabled());
        ui->actionAllow_LAN->setChecked(QStringList{"::", "0.0.0.0"}.contains(NekoRay::dataStore->inbound_address));
        // active server
        for (const auto &old: ui->menuActive_Server->actions()) {
            ui->menuActive_Server->removeAction(old);
            old->deleteLater();
        }
        int active_server_item_count = 0;
        for (const auto &pf: NekoRay::profileManager->CurrentGroup()->ProfilesWithOrder()) {
            auto a = new QAction(pf->bean->DisplayTypeAndName());
            a->setProperty("id", pf->id);
            a->setCheckable(true);
            if (NekoRay::dataStore->started_id == pf->id) a->setChecked(true);
            ui->menuActive_Server->addAction(a);
            if (++active_server_item_count == 50) break;
        }
        // active routing
        for (const auto &old: ui->menuActive_Routing->actions()) {
            ui->menuActive_Routing->removeAction(old);
            old->deleteLater();
        }
        for (const auto &name: NekoRay::Routing::List()) {
            auto a = new QAction(name);
            a->setCheckable(true);
            a->setChecked(name == NekoRay::dataStore->active_routing);
            ui->menuActive_Routing->addAction(a);
        }
    });
    connect(ui->menuActive_Server, &QMenu::triggered, this, [=](QAction *a) {
        bool ok;
        auto id = a->property("id").toInt(&ok);
        if (!ok) return;
        if (NekoRay::dataStore->started_id == id) {
            neko_stop();
        } else {
            neko_start(id);
        }
    });
    connect(ui->menuActive_Routing, &QMenu::triggered, this, [=](QAction *a) {
        auto fn = a->text();
        if (!fn.isEmpty()) {
            NekoRay::Routing r;
            r.load_control_must = true;
            r.fn = ROUTES_PREFIX + fn;
            if (r.Load()) {
                if (QMessageBox::question(GetMessageBoxParent(), software_name, tr("Load routing and apply: %1").arg(fn) + "\n" + r.DisplayRouting()) == QMessageBox::Yes) {
                    NekoRay::Routing::SetToActive(fn);
                    if (NekoRay::dataStore->started_id >= 0) {
                        neko_start(NekoRay::dataStore->started_id);
                    } else {
                        refresh_status();
                    }
                }
            }
        }
    });
    connect(ui->actionRemember_last_proxy, &QAction::triggered, this, [=](bool checked) {
        NekoRay::dataStore->remember_enable = checked;
        NekoRay::dataStore->Save();
    });
    connect(ui->actionStart_with_system, &QAction::triggered, this, [=](bool checked) {
        AutoRun_SetEnabled(checked);
    });
    connect(ui->actionAllow_LAN, &QAction::triggered, this, [=](bool checked) {
        NekoRay::dataStore->inbound_address = checked ? "::" : "127.0.0.1";
        MW_dialog_message("", "UpdateDataStore");
    });
    //
    connect(ui->checkBox_VPN, &QCheckBox::clicked, this, [=](bool checked) { neko_set_spmode_vpn(checked); });
    connect(ui->checkBox_SystemProxy, &QCheckBox::clicked, this, [=](bool checked) { neko_set_spmode_system_proxy(checked); });
    connect(ui->menu_spmode, &QMenu::aboutToShow, this, [=]() {
        ui->menu_spmode_disabled->setChecked(!(NekoRay::dataStore->spmode_system_proxy || NekoRay::dataStore->spmode_vpn));
        ui->menu_spmode_system_proxy->setChecked(NekoRay::dataStore->spmode_system_proxy);
        ui->menu_spmode_vpn->setChecked(NekoRay::dataStore->spmode_vpn);
    });
    connect(ui->menu_spmode_system_proxy, &QAction::triggered, this, [=](bool checked) { neko_set_spmode_system_proxy(checked); });
    connect(ui->menu_spmode_vpn, &QAction::triggered, this, [=](bool checked) { neko_set_spmode_vpn(checked); });
    connect(ui->menu_spmode_disabled, &QAction::triggered, this, [=]() {
        neko_set_spmode_system_proxy(false);
        neko_set_spmode_vpn(false);
    });
    connect(ui->menu_qr, &QAction::triggered, this, [=]() { display_qr_link(false); });
    connect(ui->menu_tcp_ping, &QAction::triggered, this, [=]() { speedtest_current_group(0); });
    connect(ui->menu_url_test, &QAction::triggered, this, [=]() { speedtest_current_group(1); });
    connect(ui->menu_full_test, &QAction::triggered, this, [=]() { speedtest_current_group(2); });
    //
    auto set_selected_or_group = [=](int mode) {
        // 0=group 1=select 2=unknown(menu is hide)
        ui->menu_server->setProperty("selected_or_group", mode);
    };
    auto move_tests_to_menu = [=](bool menuCurrent_Select) {
        return [=] {
            if (menuCurrent_Select) {
                ui->menuCurrent_Select->insertAction(ui->actionfake_4, ui->menu_tcp_ping);
                ui->menuCurrent_Select->insertAction(ui->actionfake_4, ui->menu_url_test);
                ui->menuCurrent_Select->insertAction(ui->actionfake_4, ui->menu_full_test);
                ui->menuCurrent_Select->insertAction(ui->actionfake_4, ui->menu_clear_test_result);
                ui->menuCurrent_Select->insertAction(ui->actionfake_4, ui->menu_resolve_domain);
            } else {
                ui->menuCurrent_Group->insertAction(ui->actionfake_5, ui->menu_tcp_ping);
                ui->menuCurrent_Group->insertAction(ui->actionfake_5, ui->menu_url_test);
                ui->menuCurrent_Group->insertAction(ui->actionfake_5, ui->menu_full_test);
                ui->menuCurrent_Group->insertAction(ui->actionfake_5, ui->menu_clear_test_result);
                ui->menuCurrent_Group->insertAction(ui->actionfake_5, ui->menu_resolve_domain);
            }
            ui->menu_full_test->setVisible(!IS_NEKO_BOX);
            set_selected_or_group(menuCurrent_Select ? 1 : 0);
        };
    };
    connect(ui->menuCurrent_Select, &QMenu::aboutToShow, this, move_tests_to_menu(true));
    connect(ui->menuCurrent_Group, &QMenu::aboutToShow, this, move_tests_to_menu(false));
    connect(ui->menu_server, &QMenu::aboutToHide, this, [=] {
        setTimeout([=] { set_selected_or_group(2); }, this, 200);
    });
    set_selected_or_group(2);
    //
    connect(ui->menu_share_item, &QMenu::aboutToShow, this, [=] {
        QString name;
        auto selected = get_now_selected();
        if (!selected.isEmpty()) {
            auto ent = selected.first();
            name = ent->bean->DisplayCoreType();
        }
        ui->menu_export_config->setVisible(name == software_core_name);
        ui->menu_export_config->setText(tr("Export %1 config").arg(name));
    });
    refresh_status();

    // Prepare core
    NekoRay::dataStore->core_token = GetRandomString(32);
    NekoRay::dataStore->core_port = MkPort();
    if (NekoRay::dataStore->core_port <= 0) NekoRay::dataStore->core_port = 19810;

    auto core_path = QApplication::applicationDirPath() + "/";
    core_path += IS_NEKO_BOX ? "nekobox_core" : "nekoray_core";

    QStringList args;
    args.push_back(IS_NEKO_BOX ? "nekobox" : "nekoray");
    args.push_back("-port");
    args.push_back(Int2String(NekoRay::dataStore->core_port));
    if (NekoRay::dataStore->flag_debug) args.push_back("-debug");

    // Start core
    core_process = new NekoRay::sys::CoreProcess(core_path, args);
    core_process->Start();

    setup_grpc();

    // Start last
    if (NekoRay::dataStore->remember_enable) {
        if (NekoRay::dataStore->remember_spmode.contains("system_proxy")) {
            neko_set_spmode_system_proxy(true, false);
        }
        if (NekoRay::dataStore->remember_spmode.contains("vpn")) {
            neko_set_spmode_vpn(true, false);
        }
        if (NekoRay::dataStore->remember_id >= 0) {
            runOnUiThread([=] { neko_start(NekoRay::dataStore->remember_id); });
        }
    }

    connect(qApp, &QGuiApplication::commitDataRequest, this, &MainWindow::on_commitDataRequest);

    if (!NekoRay::dataStore->flag_tray) show();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (tray->isVisible()) {
        hide();          // 隐藏窗口
        event->ignore(); // 忽略事件
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

// Group tab manage

inline int tabIndex2GroupId(int index) {
    if (NekoRay::profileManager->_groups.length() <= index) return -1;
    return NekoRay::profileManager->_groups[index];
}

inline int groupId2TabIndex(int gid) {
    for (int key = 0; key < NekoRay::profileManager->_groups.count(); key++) {
        if (NekoRay::profileManager->_groups[key] == gid) return key;
    }
    return 0;
}

void MainWindow::on_tabWidget_currentChanged(int index) {
    if (NekoRay::dataStore->refreshing_group_list) return;
    if (tabIndex2GroupId(index) == NekoRay::dataStore->current_group) return;
    show_group(tabIndex2GroupId(index));
}

void MainWindow::show_group(int gid) {
    if (NekoRay::dataStore->refreshing_group) return;
    NekoRay::dataStore->refreshing_group = true;

    auto group = NekoRay::profileManager->GetGroup(gid);
    if (group == nullptr) {
        MessageBoxWarning(tr("Error"), QString("No such group: %1").arg(gid));
        NekoRay::dataStore->refreshing_group = false;
        return;
    }

    if (NekoRay::dataStore->current_group != gid) {
        NekoRay::dataStore->current_group = gid;
        NekoRay::dataStore->Save();
    }
    ui->tabWidget->widget(groupId2TabIndex(gid))->layout()->addWidget(ui->proxyListTable);

    // 列宽是否可调
    if (group->manually_column_width) {
        for (int i = 0; i <= 4; i++) {
            ui->proxyListTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Interactive);
            auto size = group->column_width.value(i);
            if (size <= 0) size = ui->proxyListTable->horizontalHeader()->defaultSectionSize();
            ui->proxyListTable->horizontalHeader()->resizeSection(i, size);
        }
    } else {
        ui->proxyListTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->proxyListTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        ui->proxyListTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        ui->proxyListTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        ui->proxyListTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    }

    // show proxies
    NekoRay::GroupSortAction gsa;
    gsa.scroll_to_started = true;
    refresh_proxy_list_impl(-1, gsa);

    NekoRay::dataStore->refreshing_group = false;
}

// callback

void MainWindow::dialog_message_impl(const QString &sender, const QString &info) {
    // info
    if (info.contains("UpdateIcon")) {
        icon_status = -1;
        refresh_status();
    }
    if (info.contains("UpdateDataStore")) {
        auto changed = NekoRay::dataStore->Save();
        if (info.contains("RouteChanged")) changed = true;
        refresh_proxy_list();
        if (info.contains("VPNChanged") && NekoRay::dataStore->spmode_vpn) {
            MessageBoxWarning(tr("VPN settings changed"), tr("Restart VPN to take effect."));
        } else if (changed && NekoRay::dataStore->started_id >= 0 &&
                   QMessageBox::question(GetMessageBoxParent(), tr("Confirmation"), tr("Settings changed, restart proxy?")) == QMessageBox::StandardButton::Yes) {
            neko_start(NekoRay::dataStore->started_id);
        }
        refresh_status();
    }
    if (info.contains("NeedRestart")) {
        auto n = QMessageBox::warning(GetMessageBoxParent(), tr("Settings changed"), tr("Restart nekoray to take effect."), QMessageBox::Yes | QMessageBox::No);
        if (n == QMessageBox::Yes) {
            this->exit_reason = 2;
            on_menu_exit_triggered();
        }
    }
    //
    if (info == "RestartProgram") {
        this->exit_reason = 2;
        on_menu_exit_triggered();
    } else if (info == "Raise") {
        ACTIVE_THIS_WINDOW
    } else if (info == "ClearConnectionList") {
        refresh_connection_list({});
    }
    // sender
    if (sender == Dialog_DialogEditProfile) {
        auto msg = info.split(",");
        if (msg.contains("accept")) {
            refresh_proxy_list();
            if (msg.contains("restart")) {
                if (QMessageBox::question(GetMessageBoxParent(), tr("Confirmation"), tr("Settings changed, restart proxy?")) == QMessageBox::StandardButton::Yes) {
                    neko_start(NekoRay::dataStore->started_id);
                }
            }
        }
    } else if (sender == Dialog_DialogManageGroups) {
        if (info.startsWith("refresh")) {
            this->refresh_groups();
        }
    } else if (sender == "SubUpdater") {
        // 订阅完毕
        refresh_proxy_list();
        if (!info.contains("dingyue")) {
            show_log_impl(tr("Imported %1 profile(s)").arg(NekoRay::dataStore->imported_count));
        }
    } else if (sender == "ExternalProcess") {
        if (info == "Crashed") {
            neko_stop();
        } else if (info.startsWith("CoreRestarted")) {
            if (coreRestartTimer.isValid()) {
                auto elasped = coreRestartTimer.restart();
                if (elasped < 10 * 1000) {
                    coreRestartTimer = QElapsedTimer();
                    show_log_impl("[Error] " + tr("Core exits too frequently, stop automatic restart this profile."));
                    return;
                }
            } else {
                coreRestartTimer.start();
            }
            neko_start(info.split(",")[1].toInt());
        }
    }
}

// top bar & tray menu

inline bool dialog_is_using = false;

#define USE_DIALOG(a)            \
    if (dialog_is_using) return; \
    dialog_is_using = true;      \
    auto dialog = new a(this);   \
    dialog->exec();              \
    dialog->deleteLater();       \
    dialog_is_using = false;

void MainWindow::on_menu_basic_settings_triggered() {
    USE_DIALOG(DialogBasicSettings)
}

void MainWindow::on_menu_manage_groups_triggered() {
    USE_DIALOG(DialogManageGroups)
}

void MainWindow::on_menu_routing_settings_triggered() {
    USE_DIALOG(DialogManageRoutes)
}

void MainWindow::on_menu_vpn_settings_triggered() {
    USE_DIALOG(DialogVPNSettings)
}

void MainWindow::on_menu_hotkey_settings_triggered() {
    USE_DIALOG(DialogHotkey)
}

void MainWindow::on_commitDataRequest() {
    qDebug() << "Start of data save";
    //
    if (!isMaximized()) {
        auto olds = NekoRay::dataStore->mw_size;
        auto news = QString("%1x%2").arg(size().width()).arg(size().height());
        if (olds != news) {
            NekoRay::dataStore->mw_size = news;
        }
    }
    //
    NekoRay::dataStore->splitter_state = ui->splitter->saveState().toBase64();
    //
    auto last_id = NekoRay::dataStore->started_id;
    if (NekoRay::dataStore->remember_enable && last_id >= 0) {
        NekoRay::dataStore->remember_id = last_id;
    }
    //
    NekoRay::dataStore->Save();
    qDebug() << "End of data save";
}

void MainWindow::on_menu_exit_triggered() {
    if (mu_exit.tryLock()) {
        NekoRay::dataStore->prepare_exit = true;
        //
        neko_set_spmode_system_proxy(false, false);
        neko_set_spmode_vpn(false, false);
        if (NekoRay::dataStore->spmode_vpn) {
            mu_exit.unlock(); // retry
            return;
        }
        RegisterHotkey(true);
        //
        on_commitDataRequest();
        //
        NekoRay::dataStore->save_control_no_save = true; // don't change datastore after this line
        neko_stop(false, true);
        //
        hide();
        runOnNewThread([=] {
            sem_stopped.acquire();
            stop_core_daemon();
            runOnUiThread([=] {
                on_menu_exit_triggered(); // continue exit progress
            });
        });
        return;
    }
    //
    MF_release_runguard();
    if (exit_reason == 1) {
        QDir::setCurrent(QApplication::applicationDirPath());
        QProcess::startDetached("./updater", QStringList{});
    } else if (exit_reason == 2 || exit_reason == 3) {
        QDir::setCurrent(QApplication::applicationDirPath());

        auto arguments = NekoRay::dataStore->argv;
        if (arguments.length() > 0) {
            arguments.removeFirst();
            arguments.removeAll("-tray");
        }
        auto isLauncher = qEnvironmentVariable("NKR_FROM_LAUNCHER") == "1";
        if (isLauncher) arguments.prepend("--");
        auto program = isLauncher ? "./launcher" : QApplication::applicationFilePath();

        if (exit_reason == 3) { // restart as admin
#ifdef Q_OS_WIN
            WinCommander::runProcessElevated(program, arguments, "", WinCommander::SW_NORMAL, false);
#else
            arguments << "-flag_linux_run_core_as_admin";
            QProcess::startDetached(program, arguments);
#endif
        } else {
            QProcess::startDetached(program, arguments);
        }
    }
    tray->hide();
    QCoreApplication::quit();
}

#define neko_set_spmode_FAILED \
    refresh_status();          \
    return;

void MainWindow::neko_set_spmode_system_proxy(bool enable, bool save) {
    if (enable != NekoRay::dataStore->spmode_system_proxy) {
        if (enable) {
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
            if (!IS_NEKO_BOX && !IsValidPort(NekoRay::dataStore->inbound_http_port)) {
                auto btn = QMessageBox::warning(this, software_name,
                                                tr("Http inbound is not enabled, can't set system proxy."),
                                                "OK", tr("Settings"), "", 0, 0);
                if (btn == 1) {
                    on_menu_basic_settings_triggered();
                }
                return;
            }
#endif
            auto socks_port = NekoRay::dataStore->inbound_socks_port;
            auto http_port = NekoRay::dataStore->inbound_http_port;
            if (IS_NEKO_BOX) http_port = socks_port;
            SetSystemProxy(http_port, socks_port);
        } else {
            ClearSystemProxy();
        }
    }

    if (save) {
        NekoRay::dataStore->remember_spmode.removeAll("system_proxy");
        if (enable && NekoRay::dataStore->remember_enable) {
            NekoRay::dataStore->remember_spmode.append("system_proxy");
        }
        NekoRay::dataStore->Save();
    }

    NekoRay::dataStore->spmode_system_proxy = enable;
    refresh_status();
}

void MainWindow::neko_set_spmode_vpn(bool enable, bool save) {
    if (enable != NekoRay::dataStore->spmode_vpn) {
        if (enable) {
            if (IS_NEKO_BOX_INTERNAL_TUN) {
                bool requestPermission = false;
#ifdef Q_OS_WIN
                if (!Windows_IsInAdmin()) {
                    requestPermission = true;
                }
#else
                requestPermission = !NekoRay::isAdmin();
#endif
                if (requestPermission) {
                    auto n = QMessageBox::warning(GetMessageBoxParent(), software_name, tr("Please run NekoBox as admin"), QMessageBox::Yes | QMessageBox::No);
                    if (n == QMessageBox::Yes) {
                        this->exit_reason = 3;
                        on_menu_exit_triggered();
                    }
                    neko_set_spmode_FAILED
                }
            } else {
                if (NekoRay::dataStore->need_keep_vpn_off) {
                    MessageBoxWarning(software_name, tr("Current server is incompatible with VPN. Please stop the server first, enable VPN mode, and then restart."));
                    neko_set_spmode_FAILED
                }
                if (!StartVPNProcess()) {
                    neko_set_spmode_FAILED
                }
            }
        } else {
            if (IS_NEKO_BOX_INTERNAL_TUN) {
                // current core is sing-box
            } else {
                if (!StopVPNProcess()) {
                    neko_set_spmode_FAILED
                }
            }
        }
    }

    if (save) {
        NekoRay::dataStore->remember_spmode.removeAll("vpn");
        if (enable && NekoRay::dataStore->remember_enable) {
            NekoRay::dataStore->remember_spmode.append("vpn");
        }
        NekoRay::dataStore->Save();
    }

    NekoRay::dataStore->spmode_vpn = enable;
    refresh_status();

    if (NekoRay::dataStore->started_id >= 0) neko_start(NekoRay::dataStore->started_id);
}

void MainWindow::refresh_status(const QString &traffic_update) {
    // From TrafficLooper
    if (!traffic_update.isEmpty()) {
        traffic_update_cache = traffic_update;
        if (traffic_update == "STOP") {
            traffic_update_cache = "";
        } else {
            ui->label_speed->setText(traffic_update);
            return;
        }
    }
    ui->label_speed->setText(traffic_update_cache);

    // From UI
    if (last_test_time.addSecs(2) < QTime::currentTime()) {
        auto txt = running == nullptr ? tr("Not Running")
                                      : tr("Running: %1").arg(running->bean->DisplayName().left(50));
        ui->label_running->setText(txt);
    }
    //
    auto display_http = tr("None");
    if (IsValidPort(NekoRay::dataStore->inbound_http_port)) {
        display_http = DisplayAddress(NekoRay::dataStore->inbound_address, NekoRay::dataStore->inbound_http_port);
    }
    auto display_socks = DisplayAddress(NekoRay::dataStore->inbound_address, NekoRay::dataStore->inbound_socks_port);
    auto inbound_txt = QString("Socks: %1\nHTTP: %2").arg(display_socks, display_http);
    if (IS_NEKO_BOX) inbound_txt = QString("Mixed: %1").arg(display_socks);
    ui->label_inbound->setText(inbound_txt);
    //
    ui->checkBox_VPN->setChecked(NekoRay::dataStore->spmode_vpn);
    ui->checkBox_SystemProxy->setChecked(NekoRay::dataStore->spmode_system_proxy);
    if (select_mode) ui->label_running->setText("[" + tr("Select") + "]");

    auto make_title = [=](bool isTray) {
        QStringList tt;
        if (!isTray && NekoRay::isAdmin()) tt << "[Admin]";
        if (select_mode) tt << "[" + tr("Select") + "]";
        if (!title_error.isEmpty()) tt << "[" + title_error + "]";
        if (NekoRay::dataStore->spmode_vpn) tt << "[VPN]";
        if (NekoRay::dataStore->spmode_system_proxy) tt << "[" + tr("System Proxy") + "]";
        tt << software_name;
        if (!isTray) tt << "(" + QString(NKR_VERSION) + ")";
        if (!NekoRay::dataStore->active_routing.isEmpty() && NekoRay::dataStore->active_routing != "Default") {
            tt << "[" + NekoRay::dataStore->active_routing + "]";
        }
        if (!running.isNull()) tt << running->bean->DisplayTypeAndName();
        return tt.join(isTray ? "\n" : " ");
    };

    auto icon_status_new = Icon::NONE;

    if (!running.isNull()) {
        if (NekoRay::dataStore->spmode_vpn) {
            icon_status_new = Icon::VPN;
        } else if (NekoRay::dataStore->spmode_system_proxy) {
            icon_status_new = Icon::SYSTEM_PROXY;
        } else {
            icon_status_new = Icon::RUNNING;
        }
    }

    // refresh title & window icon
    setWindowTitle(make_title(false));
    if (icon_status_new != icon_status) QApplication::setWindowIcon(Icon::GetTrayIcon(Icon::NONE));

    // refresh tray
    if (tray != nullptr) {
        tray->setToolTip(make_title(true));
        if (icon_status_new != icon_status) tray->setIcon(Icon::GetTrayIcon(icon_status_new));
    }

    icon_status = icon_status_new;
}

// table显示

// refresh_groups -> show_group -> refresh_proxy_list
void MainWindow::refresh_groups() {
    NekoRay::dataStore->refreshing_group_list = true;

    // refresh group?
    for (int i = ui->tabWidget->count() - 1; i > 0; i--) {
        ui->tabWidget->removeTab(i);
    }

    int index = 0;
    for (const auto &gid: NekoRay::profileManager->_groups) {
        auto group = NekoRay::profileManager->GetGroup(gid);
        if (index == 0) {
            ui->tabWidget->setTabText(0, group->name);
        } else {
            auto widget2 = new QWidget();
            auto layout2 = new QVBoxLayout();
            layout2->setContentsMargins(QMargins());
            layout2->setSpacing(0);
            widget2->setLayout(layout2);
            ui->tabWidget->addTab(widget2, group->name);
        }
        ui->tabWidget->tabBar()->setTabData(index, gid);
        index++;
    }

    // show after group changed
    if (NekoRay::profileManager->CurrentGroup() == nullptr) {
        NekoRay::dataStore->current_group = -1;
        ui->tabWidget->setCurrentIndex(groupId2TabIndex(0));
        show_group(NekoRay::profileManager->_groups.count() > 0 ? NekoRay::profileManager->_groups.first() : 0);
    } else {
        ui->tabWidget->setCurrentIndex(groupId2TabIndex(NekoRay::dataStore->current_group));
        show_group(NekoRay::dataStore->current_group);
    }

    NekoRay::dataStore->refreshing_group_list = false;
}

void MainWindow::refresh_proxy_list(const int &id) {
    refresh_proxy_list_impl(id, {});
}

void MainWindow::refresh_proxy_list_impl(const int &id, NekoRay::GroupSortAction groupSortAction) {
    // id < 0 重绘
    if (id < 0) {
        // 清空数据
        ui->proxyListTable->row2Id.clear();
        ui->proxyListTable->setRowCount(0);
        // 添加行
        int row = -1;
        for (const auto &profile: NekoRay::profileManager->profiles) {
            if (NekoRay::dataStore->current_group != profile->gid) continue;
            row++;
            ui->proxyListTable->insertRow(row);
            ui->proxyListTable->row2Id += profile->id;
        }
    }

    // 显示排序
    if (id < 0) {
        switch (groupSortAction.method) {
            case NekoRay::GroupSortMethod::Raw: {
                auto group = NekoRay::profileManager->CurrentGroup();
                if (group == nullptr) return;
                ui->proxyListTable->order = group->order;
                break;
            }
            case NekoRay::GroupSortMethod::ById: {
                // Clear Order
                ui->proxyListTable->order.clear();
                ui->proxyListTable->callback_save_order();
                break;
            }
            case NekoRay::GroupSortMethod::ByAddress:
            case NekoRay::GroupSortMethod::ByName:
            case NekoRay::GroupSortMethod::ByLatency:
            case NekoRay::GroupSortMethod::ByType: {
                std::sort(ui->proxyListTable->order.begin(), ui->proxyListTable->order.end(),
                          [=](int a, int b) {
                              QString ms_a;
                              QString ms_b;
                              if (groupSortAction.method == NekoRay::GroupSortMethod::ByType) {
                                  ms_a = NekoRay::profileManager->GetProfile(a)->bean->DisplayType();
                                  ms_b = NekoRay::profileManager->GetProfile(b)->bean->DisplayType();
                              } else if (groupSortAction.method == NekoRay::GroupSortMethod::ByName) {
                                  ms_a = NekoRay::profileManager->GetProfile(a)->bean->name;
                                  ms_b = NekoRay::profileManager->GetProfile(b)->bean->name;
                              } else if (groupSortAction.method == NekoRay::GroupSortMethod::ByAddress) {
                                  ms_a = NekoRay::profileManager->GetProfile(a)->bean->DisplayAddress();
                                  ms_b = NekoRay::profileManager->GetProfile(b)->bean->DisplayAddress();
                              } else if (groupSortAction.method == NekoRay::GroupSortMethod::ByLatency) {
                                  ms_a = NekoRay::profileManager->GetProfile(a)->full_test_report;
                                  ms_b = NekoRay::profileManager->GetProfile(b)->full_test_report;
                              }
                              auto get_latency_for_sort = [](int id) {
                                  auto i = NekoRay::profileManager->GetProfile(id)->latency;
                                  if (i == 0) i = 100000;
                                  if (i < 0) i = 99999;
                                  return i;
                              };
                              if (groupSortAction.descending) {
                                  if (groupSortAction.method == NekoRay::GroupSortMethod::ByLatency) {
                                      if (ms_a.isEmpty() && ms_b.isEmpty()) {
                                          // compare latency if full_test_report is empty
                                          return get_latency_for_sort(a) > get_latency_for_sort(b);
                                      }
                                  }
                                  return ms_a > ms_b;
                              } else {
                                  if (groupSortAction.method == NekoRay::GroupSortMethod::ByLatency) {
                                      auto int_a = NekoRay::profileManager->GetProfile(a)->latency;
                                      auto int_b = NekoRay::profileManager->GetProfile(b)->latency;
                                      if (ms_a.isEmpty() && ms_b.isEmpty()) {
                                          // compare latency if full_test_report is empty
                                          return get_latency_for_sort(a) < get_latency_for_sort(b);
                                      }
                                  }
                                  return ms_a < ms_b;
                              }
                          });
                break;
            }
        }
        ui->proxyListTable->update_order(groupSortAction.save_sort);
    }

    // refresh data
    refresh_proxy_list_impl_refresh_data(id);
}

void MainWindow::refresh_proxy_list_impl_refresh_data(const int &id) {
    // 绘制或更新item(s)
    for (int row = 0; row < ui->proxyListTable->rowCount(); row++) {
        auto profileId = ui->proxyListTable->row2Id[row];
        if (id >= 0 && profileId != id) continue; // refresh ONE item
        auto profile = NekoRay::profileManager->GetProfile(profileId);
        if (profile == nullptr) continue;

        auto f0 = std::make_unique<QTableWidgetItem>();
        f0->setData(114514, profileId);

        // Check state
        auto check = f0->clone();
        check->setText(profileId == NekoRay::dataStore->started_id ? "✓" : Int2String(row + 1));
        ui->proxyListTable->setVerticalHeaderItem(row, check);

        // C0: Type
        auto f = f0->clone();
        f->setText(profile->bean->DisplayType());
        ui->proxyListTable->setItem(row, 0, f);

        // C1: Address+Port
        f = f0->clone();
        f->setText(profile->bean->DisplayAddress());
        ui->proxyListTable->setItem(row, 1, f);

        // C2: Name
        f = f0->clone();
        f->setText(profile->bean->name);
        ui->proxyListTable->setItem(row, 2, f);

        // C3: Test Result
        f = f0->clone();
        if (profile->full_test_report.isEmpty()) {
            auto color = profile->DisplayLatencyColor();
            if (color.isValid()) f->setForeground(color);
            f->setText(profile->DisplayLatency());
        } else {
            f->setText(profile->full_test_report);
        }
        ui->proxyListTable->setItem(row, 3, f);

        // C4: Traffic
        f = f0->clone();
        f->setText(profile->traffic_data->DisplayTraffic());
        ui->proxyListTable->setItem(row, 4, f);
    }
}

// table菜单相关

void MainWindow::on_proxyListTable_itemDoubleClicked(QTableWidgetItem *item) {
    auto id = item->data(114514).toInt();
    if (select_mode) {
        emit profile_selected(id);
        select_mode = false;
        refresh_status();
        return;
    }
    auto dialog = new DialogEditProfile("", id, this);
    connect(dialog, &QDialog::finished, dialog, &QDialog::deleteLater);
}

void MainWindow::on_menu_add_from_input_triggered() {
    auto dialog = new DialogEditProfile("socks", NekoRay::dataStore->current_group, this);
    connect(dialog, &QDialog::finished, dialog, &QDialog::deleteLater);
}

void MainWindow::on_menu_add_from_clipboard_triggered() {
    auto clipboard = QApplication::clipboard()->text();
    NekoRay::sub::groupUpdater->AsyncUpdate(clipboard);
}

void MainWindow::on_menu_clone_triggered() {
    auto ents = get_now_selected();
    if (ents.isEmpty()) return;

    auto btn = QMessageBox::question(this, tr("Clone"), tr("Clone %1 item(s)").arg(ents.count()));
    if (btn != QMessageBox::Yes) return;

    QStringList sls;
    for (const auto &ent: ents) {
        sls << ent->bean->ToNekorayShareLink(ent->type);
    }

    NekoRay::sub::groupUpdater->AsyncUpdate(sls.join("\n"));
}

void MainWindow::on_menu_move_triggered() {
    auto ents = get_now_selected();
    if (ents.isEmpty()) return;

    auto items = QStringList{};
    for (auto &&group: NekoRay::profileManager->groups) {
        items += Int2String(group->id) + " " + group->name;
    }

    bool ok;
    auto a = QInputDialog::getItem(nullptr,
                                   tr("Move"),
                                   tr("Move %1 item(s)").arg(ents.count()),
                                   items, 0, false, &ok);
    if (!ok) return;
    auto gid = SubStrBefore(a, " ").toInt();
    for (const auto &ent: ents) {
        NekoRay::profileManager->MoveProfile(ent, gid);
    }
    refresh_proxy_list();
}

void MainWindow::on_menu_delete_triggered() {
    auto ents = get_now_selected();
    if (ents.count() == 0) return;
    if (QMessageBox::question(this, tr("Confirmation"), QString(tr("Remove %1 item(s) ?")).arg(ents.count())) ==
        QMessageBox::StandardButton::Yes) {
        for (const auto &ent: ents) {
            NekoRay::profileManager->DeleteProfile(ent->id);
        }
        refresh_proxy_list();
    }
}

void MainWindow::on_menu_reset_traffic_triggered() {
    auto ents = get_now_selected();
    if (ents.count() == 0) return;
    if (QMessageBox::question(this, tr("Confirmation"), QString(tr("Reset traffic of %1 item(s) ?")).arg(ents.count())) == QMessageBox::StandardButton::Yes) {
        for (const auto &ent: ents) {
            ent->traffic_data->Reset();
            ent->Save();
            refresh_proxy_list(ent->id);
        }
    }
}

void MainWindow::on_menu_profile_debug_info_triggered() {
    auto ents = get_now_selected();
    if (ents.count() != 1) return;
    auto btn = QMessageBox::information(this, software_name, ents.first()->ToJsonBytes(), "OK", "Edit", "Reload", 0, 0);
    if (btn == 1) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(QString("profiles/%1.json").arg(ents.first()->id)).absoluteFilePath()));
    } else if (btn == 2) {
        NekoRay::dataStore->Load();
        NekoRay::profileManager->Load();
        refresh_proxy_list();
    }
}

void MainWindow::on_menu_copy_links_triggered() {
    if (ui->masterLogBrowser->hasFocus()) {
        ui->masterLogBrowser->copy();
        return;
    }
    auto ents = get_now_selected();
    QStringList links;
    for (const auto &ent: ents) {
        links += ent->bean->ToShareLink();
    }
    if (links.length() == 0) return;
    QApplication::clipboard()->setText(links.join("\n"));
    show_log_impl(tr("Copied %1 item(s)").arg(links.length()));
}

void MainWindow::on_menu_copy_links_nkr_triggered() {
    auto ents = get_now_selected();
    QStringList links;
    for (const auto &ent: ents) {
        links += ent->bean->ToNekorayShareLink(ent->type);
    }
    if (links.length() == 0) return;
    QApplication::clipboard()->setText(links.join("\n"));
    show_log_impl(tr("Copied %1 item(s)").arg(links.length()));
}

void MainWindow::on_menu_export_config_triggered() {
    auto ents = get_now_selected();
    if (ents.count() != 1) return;
    auto ent = ents.first();
    QString config_core;

    auto result = NekoRay::BuildConfig(ent, false, true);
    config_core = QJsonObject2QString(result->coreConfig, true);
    QApplication::clipboard()->setText(config_core);

    QMessageBox msg(QMessageBox::Information, tr("Config copied"), config_core);
    msg.addButton("Copy core config", QMessageBox::YesRole);
    msg.addButton("Copy test config", QMessageBox::YesRole);
    msg.addButton(QMessageBox::Ok);
    msg.setEscapeButton(QMessageBox::Ok);
    msg.setDefaultButton(QMessageBox::Ok);
    auto ret = msg.exec();
    if (ret == 0) {
        result = NekoRay::BuildConfig(ent, false, false);
        config_core = QJsonObject2QString(result->coreConfig, true);
        QApplication::clipboard()->setText(config_core);
    } else if (ret == 1) {
        result = NekoRay::BuildConfig(ent, true, false);
        config_core = QJsonObject2QString(result->coreConfig, true);
        QApplication::clipboard()->setText(config_core);
    }
}

void MainWindow::display_qr_link(bool nkrFormat) {
    auto ents = get_now_selected();
    if (ents.count() != 1) return;

    class W : public QDialog {
    public:
        QLabel *l = nullptr;
        QCheckBox *cb = nullptr;
        //
        QPlainTextEdit *l2 = nullptr;
        QImage im;
        //
        QString link;
        QString link_nk;

        void show_qr(const QSize &size) const {
            auto side = size.height() - 20 - l2->size().height() - cb->size().height();
            l->setPixmap(QPixmap::fromImage(im.scaled(side, side, Qt::KeepAspectRatio, Qt::FastTransformation),
                                            Qt::MonoOnly));
            l->resize(side, side);
        }

        void refresh(bool is_nk) {
            auto link_display = is_nk ? link_nk : link;
            l2->setPlainText(link_display);
            constexpr qint32 qr_padding = 2;
            //
            try {
                qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(link_display.toUtf8().data(), qrcodegen::QrCode::Ecc::MEDIUM);
                qint32 sz = qr.getSize();
                im = QImage(sz + qr_padding * 2, sz + qr_padding * 2, QImage::Format_RGB32);
                QRgb black = qRgb(0, 0, 0);
                QRgb white = qRgb(255, 255, 255);
                im.fill(white);
                for (int y = 0; y < sz; y++)
                    for (int x = 0; x < sz; x++)
                        if (qr.getModule(x, y))
                            im.setPixel(x + qr_padding, y + qr_padding, black);
                show_qr(size());
            } catch (const std::exception &ex) {
                QMessageBox::warning(nullptr, "error", ex.what());
            }
        }

        W(const QString &link_, const QString &link_nk_) {
            link = link_;
            link_nk = link_nk_;
            //
            setLayout(new QVBoxLayout);
            setMinimumSize(256, 256);
            QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            sizePolicy.setHeightForWidth(true);
            setSizePolicy(sizePolicy);
            //
            l = new QLabel();
            l->setMinimumSize(256, 256);
            l->setMargin(6);
            l->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            l->setScaledContents(true);
            layout()->addWidget(l);
            cb = new QCheckBox;
            cb->setText("Neko Links");
            layout()->addWidget(cb);
            l2 = new QPlainTextEdit();
            l2->setReadOnly(true);
            layout()->addWidget(l2);
            //
            connect(cb, &QCheckBox::toggled, this, &W::refresh);
            refresh(false);
        }

        void resizeEvent(QResizeEvent *resizeEvent) override {
            show_qr(resizeEvent->size());
        }
    };

    auto link = ents.first()->bean->ToShareLink();
    auto link_nk = ents.first()->bean->ToNekorayShareLink(ents.first()->type);
    auto w = new W(link, link_nk);
    w->setWindowTitle(ents.first()->bean->DisplayTypeAndName());
    w->exec();
    w->deleteLater();
}

void MainWindow::on_menu_scan_qr_triggered() {
#ifndef NKR_NO_ZXING
    using namespace ZXingQt;

    hide();
    QThread::sleep(1);

    auto screen = QGuiApplication::primaryScreen();
    auto geom = screen->geometry();
    auto qpx = screen->grabWindow(0, geom.x(), geom.y(), geom.width(), geom.height());

    show();

    auto hints = DecodeHints()
                     .setFormats(BarcodeFormat::QRCode)
                     .setTryRotate(false)
                     .setBinarizer(Binarizer::FixedThreshold);

    auto result = ReadBarcode(qpx.toImage(), hints);
    const auto &text = result.text();
    if (text.isEmpty()) {
        MessageBoxInfo(software_name, tr("QR Code not found"));
    } else {
        show_log_impl("QR Code Result:\n" + text);
        NekoRay::sub::groupUpdater->AsyncUpdate(text);
    }
#endif
}

void MainWindow::on_menu_clear_test_result_triggered() {
    for (const auto &profile: get_selected_or_group()) {
        profile->latency = 0;
        profile->full_test_report = "";
        profile->Save();
    }
    refresh_proxy_list();
}

void MainWindow::on_menu_select_all_triggered() {
    if (ui->masterLogBrowser->hasFocus()) {
        ui->masterLogBrowser->selectAll();
        return;
    }
    ui->proxyListTable->selectAll();
}

void MainWindow::on_menu_delete_repeat_triggered() {
    QList<QSharedPointer<NekoRay::ProxyEntity>> out;
    QList<QSharedPointer<NekoRay::ProxyEntity>> out_del;

    NekoRay::ProfileFilter::Uniq(NekoRay::profileManager->CurrentGroup()->Profiles(), out, true, false);
    NekoRay::ProfileFilter::OnlyInSrc_ByPointer(NekoRay::profileManager->CurrentGroup()->Profiles(), out, out_del);

    int remove_display_count = 0;
    QString remove_display;
    for (const auto &ent: out_del) {
        remove_display += ent->bean->DisplayTypeAndName() + "\n";
        if (++remove_display_count == 20) {
            remove_display += "...";
            break;
        }
    }

    if (out_del.length() > 0 &&
        QMessageBox::question(this, tr("Confirmation"), tr("Remove %1 item(s) ?").arg(out_del.length()) + "\n" + remove_display) == QMessageBox::StandardButton::Yes) {
        for (const auto &ent: out_del) {
            NekoRay::profileManager->DeleteProfile(ent->id);
        }
        refresh_proxy_list();
    }
}

bool mw_sub_updating = false;

void MainWindow::on_menu_update_subscription_triggered() {
    auto group = NekoRay::profileManager->CurrentGroup();
    if (group->url.isEmpty()) return;
    if (mw_sub_updating) return;
    mw_sub_updating = true;
    NekoRay::sub::groupUpdater->AsyncUpdate(group->url, group->id, [&] { mw_sub_updating = false; });
}

void MainWindow::on_menu_remove_unavailable_triggered() {
    QList<QSharedPointer<NekoRay::ProxyEntity>> out_del;

    for (const auto &profile: NekoRay::profileManager->profiles) {
        if (NekoRay::dataStore->current_group != profile->gid) continue;
        if (profile->latency < 0) out_del += profile;
    }

    int remove_display_count = 0;
    QString remove_display;
    for (const auto &ent: out_del) {
        remove_display += ent->bean->DisplayTypeAndName() + "\n";
        if (++remove_display_count == 20) {
            remove_display += "...";
            break;
        }
    }

    if (out_del.length() > 0 &&
        QMessageBox::question(this, tr("Confirmation"), tr("Remove %1 item(s) ?").arg(out_del.length()) + "\n" + remove_display) == QMessageBox::StandardButton::Yes) {
        for (const auto &ent: out_del) {
            NekoRay::profileManager->DeleteProfile(ent->id);
        }
        refresh_proxy_list();
    }
}

void MainWindow::on_menu_resolve_domain_triggered() {
    auto profiles = get_selected_or_group();
    if (profiles.isEmpty()) return;

    if (QMessageBox::question(this,
                              tr("Confirmation"),
                              tr("Resolving domain to IP, if support.")) != QMessageBox::StandardButton::Yes) {
        return;
    }
    if (mw_sub_updating) return;
    mw_sub_updating = true;
    NekoRay::dataStore->resolve_count = profiles.count();

    for (const auto &profile: profiles) {
        profile->bean->ResolveDomainToIP([=] {
            profile->Save();
            if (--NekoRay::dataStore->resolve_count != 0) return;
            refresh_proxy_list();
            mw_sub_updating = false;
        });
    }
}

void MainWindow::on_proxyListTable_customContextMenuRequested(const QPoint &pos) {
    ui->menu_server->popup(ui->proxyListTable->viewport()->mapToGlobal(pos)); // 弹出菜单
}

QMap<int, QSharedPointer<NekoRay::ProxyEntity>> MainWindow::get_now_selected() {
    auto items = ui->proxyListTable->selectedItems();
    QMap<int, QSharedPointer<NekoRay::ProxyEntity>> map;
    for (auto item: items) {
        auto id = item->data(114514).toInt();
        auto ent = NekoRay::profileManager->GetProfile(id);
        if (ent != nullptr) map[id] = ent;
    }
    return map;
}

QList<QSharedPointer<NekoRay::ProxyEntity>> MainWindow::get_now_selected_list() {
    auto items = ui->proxyListTable->selectedItems();
    QList<QSharedPointer<NekoRay::ProxyEntity>> list;
    for (auto item: items) {
        auto id = item->data(114514).toInt();
        auto ent = NekoRay::profileManager->GetProfile(id);
        if (ent != nullptr && !list.contains(ent)) list += ent;
    }
    return list;
}

QList<QSharedPointer<NekoRay::ProxyEntity>> MainWindow::get_selected_or_group() {
    auto selected_or_group = ui->menu_server->property("selected_or_group").toInt();
    QList<QSharedPointer<NekoRay::ProxyEntity>> profiles;
    if (selected_or_group > 0) {
        profiles = get_now_selected_list();
        if (profiles.isEmpty() && selected_or_group == 2) profiles = NekoRay::profileManager->CurrentGroup()->ProfilesWithOrder();
    } else {
        profiles = NekoRay::profileManager->CurrentGroup()->ProfilesWithOrder();
    }
    return profiles;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Escape:
            // take over by shortcut_esc
            break;
        case Qt::Key_Enter:
            neko_start();
            break;
        default:
            QMainWindow::keyPressEvent(event);
    }
}

// Log

inline void FastAppendTextDocument(const QString &message, QTextDocument *doc) {
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End);
    cursor.beginEditBlock();
    cursor.insertBlock();
    cursor.insertText(message);
    cursor.endEditBlock();
}

void MainWindow::show_log_impl(const QString &log) {
    auto lines = SplitLines(log.trimmed());
    if (lines.isEmpty()) return;

    QStringList newLines;
    auto log_ignore = NekoRay::dataStore->log_ignore;
    for (const auto &line: lines) {
        bool showThisLine = true;
        for (const auto &str: log_ignore) {
            if (line.contains(str)) {
                showThisLine = false;
                break;
            }
        }
        if (showThisLine) newLines << line;
    }
    if (newLines.isEmpty()) return;

    FastAppendTextDocument(newLines.join("\n"), qvLogDocument);
    // qvLogDocument->setPlainText(qvLogDocument->toPlainText() + log);
    // From https://gist.github.com/jemyzhang/7130092
    auto block = qvLogDocument->begin();

    while (block.isValid()) {
        if (qvLogDocument->blockCount() > NekoRay::dataStore->max_log_line) {
            QTextCursor cursor(block);
            block = block.next();
            cursor.select(QTextCursor::BlockUnderCursor);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            continue;
        }
        break;
    }
}

#define ADD_TO_CURRENT_ROUTE(a, b)                                                                   \
    NekoRay::dataStore->routing->a = (SplitLines(NekoRay::dataStore->routing->a) << (b)).join("\n"); \
    NekoRay::dataStore->routing->Save();

void MainWindow::on_masterLogBrowser_customContextMenuRequested(const QPoint &pos) {
    QMenu *menu = ui->masterLogBrowser->createStandardContextMenu();

    auto sep = new QAction;
    sep->setSeparator(true);
    menu->addAction(sep);

    auto action_add_ignore = new QAction;
    action_add_ignore->setText(tr("Set ignore keyword"));
    connect(action_add_ignore, &QAction::triggered, this, [=] {
        auto list = NekoRay::dataStore->log_ignore;
        auto newStr = ui->masterLogBrowser->textCursor().selectedText().trimmed();
        if (!newStr.isEmpty()) list << newStr;
        bool ok;
        newStr = QInputDialog::getMultiLineText(GetMessageBoxParent(), tr("Set ignore keyword"), tr("Set the following keywords to ignore?\nSplit by line."), list.join("\n"), &ok);
        if (ok) {
            NekoRay::dataStore->log_ignore = SplitLines(newStr);
            NekoRay::dataStore->Save();
        }
    });
    menu->addAction(action_add_ignore);

    auto action_add_route = new QAction;
    action_add_route->setText(tr("Save as route"));
    connect(action_add_route, &QAction::triggered, this, [=] {
        auto newStr = ui->masterLogBrowser->textCursor().selectedText().trimmed();
        if (newStr.isEmpty()) return;
        //
        bool ok;
        newStr = QInputDialog::getText(GetMessageBoxParent(), tr("Save as route"), tr("Edit"), {}, newStr, &ok).trimmed();
        if (!ok) return;
        if (newStr.isEmpty()) return;
        //
        auto select = IsIpAddress(newStr) ? 0 : 3;
        QStringList items = {"proxyIP", "bypassIP", "blockIP", "proxyDomain", "bypassDomain", "blockDomain"};
        auto item = QInputDialog::getItem(GetMessageBoxParent(), tr("Save as route"),
                                          tr("Save \"%1\" as a routing rule?").arg(newStr),
                                          items, select, false, &ok);
        if (ok) {
            auto index = items.indexOf(item);
            switch (index) {
                case 0:
                    ADD_TO_CURRENT_ROUTE(proxy_ip, newStr);
                    break;
                case 1:
                    ADD_TO_CURRENT_ROUTE(direct_ip, newStr);
                    break;
                case 2:
                    ADD_TO_CURRENT_ROUTE(block_ip, newStr);
                    break;
                case 3:
                    ADD_TO_CURRENT_ROUTE(proxy_domain, newStr);
                    break;
                case 4:
                    ADD_TO_CURRENT_ROUTE(direct_domain, newStr);
                    break;
                case 5:
                    ADD_TO_CURRENT_ROUTE(block_domain, newStr);
                    break;
                default:
                    break;
            }
            MW_dialog_message("", "UpdateDataStore,RouteChanged");
        }
    });
    menu->addAction(action_add_route);

    auto action_clear = new QAction;
    action_clear->setText(tr("Clear"));
    connect(action_clear, &QAction::triggered, this, [=] {
        qvLogDocument->clear();
        ui->masterLogBrowser->clear();
    });
    menu->addAction(action_clear);

    menu->exec(ui->masterLogBrowser->viewport()->mapToGlobal(pos)); // 弹出菜单
}

// eventFilter

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if (obj == ui->label_running && mouseEvent->button() == Qt::LeftButton && running != nullptr) {
            speedtest_current();
            return true;
        } else if (obj == ui->label_inbound && mouseEvent->button() == Qt::LeftButton) {
            on_menu_basic_settings_triggered();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

// profile selector

void MainWindow::start_select_mode(QObject *context, const std::function<void(int)> &callback) {
    select_mode = true;
    connectOnce(this, &MainWindow::profile_selected, context, callback);
    refresh_status();
}

// 连接列表

inline QJsonArray last_arr; // format is nekoray_connections_json

void MainWindow::refresh_connection_list(const QJsonArray &arr) {
    if (last_arr == arr) {
        return;
    }
    last_arr = arr;

    if (NekoRay::dataStore->flag_debug) qDebug() << arr;

    ui->tableWidget_conn->setRowCount(0);

    int row = -1;
    for (const auto &_item: arr) {
        auto item = _item.toObject();
        if (NekoRay::dataStore->ignoreConnTag.contains(item["Tag"].toString())) continue;

        row++;
        ui->tableWidget_conn->insertRow(row);

        auto f0 = std::make_unique<QTableWidgetItem>();
        f0->setData(114514, item["ID"].toInt());

        // C0: Status
        auto c0 = new QLabel;
        auto start_t = item["Start"].toInt();
        auto end_t = item["End"].toInt();
        // icon
        auto outboundTag = item["Tag"].toString();
        if (outboundTag == "block") {
            c0->setPixmap(Icon::GetMaterialIcon("cancel"));
        } else {
            if (end_t > 0) {
                c0->setPixmap(Icon::GetMaterialIcon("history"));
            } else {
                c0->setPixmap(Icon::GetMaterialIcon("swap-vertical"));
            }
        }
        c0->setAlignment(Qt::AlignCenter);
        c0->setToolTip(tr("Start: %1\nEnd: %2").arg(DisplayTime(start_t), end_t > 0 ? DisplayTime(end_t) : ""));
        ui->tableWidget_conn->setCellWidget(row, 0, c0);

        // C1: Outbound
        auto f = f0->clone();
        f->setToolTip("");
        f->setText(outboundTag);
        ui->tableWidget_conn->setItem(row, 1, f);

        // C2: Destination
        f = f0->clone();
        QString target1 = item["Dest"].toString();
        QString target2 = item["RDest"].toString();
        if (target2.isEmpty() || target1 == target2) {
            target2 = "";
        }
        f->setText("[" + target1 + "] " + target2);
        ui->tableWidget_conn->setItem(row, 2, f);
    }
}

// Hotkey

#ifndef NKR_NO_QHOTKEY

#include <QHotkey>

inline QList<QSharedPointer<QHotkey>> RegisteredHotkey;

void MainWindow::RegisterHotkey(bool unregister) {
    while (!RegisteredHotkey.isEmpty()) {
        auto hk = RegisteredHotkey.takeFirst();
        hk->deleteLater();
    }
    if (unregister) return;

    QStringList regstr{
        NekoRay::dataStore->hotkey_mainwindow,
        NekoRay::dataStore->hotkey_group,
        NekoRay::dataStore->hotkey_route,
        NekoRay::dataStore->hotkey_system_proxy_menu,
    };

    for (const auto &key: regstr) {
        if (key.isEmpty()) continue;
        if (regstr.count(key) > 1) return; // Conflict hotkey
    }
    for (const auto &key: regstr) {
        QKeySequence k(key);
        if (k.isEmpty()) continue;
        auto hk = QSharedPointer<QHotkey>(new QHotkey(k, true));
        if (hk->isRegistered()) {
            RegisteredHotkey += hk;
            connect(hk.get(), &QHotkey::activated, this, [=] { HotkeyEvent(key); });
        } else {
            hk->deleteLater();
        }
    }
}

void MainWindow::HotkeyEvent(const QString &key) {
    if (key.isEmpty()) return;
    runOnUiThread([=] {
        if (key == NekoRay::dataStore->hotkey_mainwindow) {
            tray->activated(QSystemTrayIcon::ActivationReason::Trigger);
        } else if (key == NekoRay::dataStore->hotkey_group) {
            on_menu_manage_groups_triggered();
        } else if (key == NekoRay::dataStore->hotkey_route) {
            on_menu_routing_settings_triggered();
        } else if (key == NekoRay::dataStore->hotkey_system_proxy_menu) {
            ui->menu_spmode->popup(QCursor::pos());
        }
    });
}

#else

void MainWindow::RegisterHotkey(bool unregister) {}

void MainWindow::HotkeyEvent(const QString &key) {}

#endif

// VPN Launcher

bool MainWindow::StartVPNProcess() {
    //
    if (vpn_pid != 0) {
        return true;
    }
    //
    auto protectPath = QDir::currentPath() + "/protect";
    auto configPath = NekoRay::WriteVPNSingBoxConfig();
    auto scriptPath = NekoRay::WriteVPNLinuxScript(protectPath, configPath);
    //
#ifdef Q_OS_WIN
    runOnNewThread([=] {
        vpn_pid = 1; // TODO get pid?
        WinCommander::runProcessElevated(QApplication::applicationDirPath() + "/nekobox_core.exe",
                                         {"--disable-color", "run", "-c", configPath}, "",
                                         NekoRay::dataStore->vpn_hide_console ? WinCommander::SW_HIDE : WinCommander::SW_SHOWMINIMIZED); // blocking
        vpn_pid = 0;
        runOnUiThread([=] { neko_set_spmode_vpn(false); });
    });
#else
    QFile::remove(protectPath);
    if (QFile::exists(protectPath)) {
        MessageBoxWarning("Error", "protect cannot be removed");
        return false;
    }
    //
    auto vpn_process = new QProcess;
    QProcess::connect(vpn_process, &QProcess::stateChanged, this, [=](QProcess::ProcessState state) {
        if (state == QProcess::NotRunning) {
            vpn_pid = 0;
            vpn_process->deleteLater();
            GetMainWindow()->neko_set_spmode_vpn(false);
        }
    });
    //
    vpn_process->setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef Q_OS_MACOS
    vpn_process->start("osascript", {"-e", QString("do shell script \"%1\" with administrator privileges")
                                               .arg("bash " + scriptPath)});
#else
    vpn_process->start("pkexec", {"bash", scriptPath});
#endif
    vpn_process->waitForStarted();
    vpn_pid = vpn_process->processId(); // actually it's pkexec or bash PID
#endif
    return true;
}

bool MainWindow::StopVPNProcess(bool unconditional) {
    if (unconditional || vpn_pid != 0) {
        bool ok;
        core_process->processId();
#ifdef Q_OS_WIN
        auto ret = WinCommander::runProcessElevated("taskkill", {"/IM", "nekobox_core.exe",
                                                                 "/FI",
                                                                 "PID ne " + Int2String(core_process->processId())});
        ok = ret == 0;
#else
        QProcess p;
#ifdef Q_OS_MACOS
        p.start("osascript", {"-e", QString("do shell script \"%1\" with administrator privileges")
                                        .arg("pkill -2 -U 0 nekobox_core")});
#else
        if (unconditional) {
            p.start("pkexec", {"killall", "-2", "nekobox_core"});
        } else {
            p.start("pkexec", {"pkill", "-2", "-P", Int2String(vpn_pid)});
        }
#endif
        p.waitForFinished();
        ok = p.exitCode() == 0;
#endif
        if (!unconditional) {
            ok ? vpn_pid = 0 : MessageBoxWarning(tr("Error"), tr("Failed to stop VPN process"));
        }
        return ok;
    }
    return true;
}
