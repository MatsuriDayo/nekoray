#include "./ui_mainwindow.h"

#include "db/TrafficLooper.hpp"
#include "db/ProfileFilter.hpp"
#include "fmt/ConfigBuilder.hpp"
#include "sub/RawUpdater.hpp"
#include "sys/ExternalProcess.hpp"
#include "sys/AutoRun.hpp"

#include "ui/CheckUpdate.hpp"
#include "ui/ThemeManager.hpp"
#include "ui/mainwindow.h"
#include "ui/dialog_basic_settings.h"
#include "ui/edit/dialog_edit_profile.h"
#include "ui/dialog_manage_groups.h"
#include "ui/dialog_manage_routes.h"

#include "3rdparty/qrcodegen.hpp"
#include "3rdparty/VT100Parser.hpp"

#ifndef NKR_NO_EXTERNAL

#include "3rdparty/ZxingQtReader.hpp"
#include "qv2ray/components/proxy/QvProxyConfigurator.hpp"

#endif

#include <QClipboard>
#include <QLabel>
#include <QTextBlock>
#include <QScrollBar>
#include <QMutex>
#include <QScreen>
#include <QDesktopServices>
#include <QInputDialog>

#ifndef NKR_NO_GRPC
using namespace NekoRay::rpc;
#endif

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    mainwindow = this;
    dialog_message = [=](const QString &a, const QString &b) {
        runOnUiThread([=] {
            dialog_message_impl(a, b);
        });
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
    title_base = windowTitle();
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

    // top bar
    ui->toolButton_program->setMenu(ui->menu_program);
    ui->toolButton_preferences->setMenu(ui->menu_preferences);
    ui->toolButton_server->setMenu(ui->menu_server);
    ui->menubar->setVisible(false);
    connect(ui->toolButton_document, &QToolButton::clicked, this,
            [=] { QDesktopServices::openUrl(QUrl("https://matsuridayo.github.io/")); });
    connect(ui->toolButton_ads, &QToolButton::clicked, this,
            [=] { QDesktopServices::openUrl(QUrl("https://matsuricom.github.io/")); });
    connect(ui->toolButton_update, &QToolButton::clicked, this,
            [] { runOnNewThread([] { CheckUpdate(); }); });

    // Setup log UI
    qvLogHighlighter = new SyntaxHighlighter(false, qvLogDocument);
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
    showLog = [=](const QString &log) {
        runOnUiThread([=] {
            show_log_impl(log);
        });
    };
    showLog_ext = [=](const QString &tag, const QString &log) {
        runOnUiThread([=] {
            show_log_impl("[" + tag + "] " + log);
        });
    };
    showLog_ext_vt100 = [=](const QString &log) {
        runOnUiThread([=] {
            show_log_impl(cleanVT100String(log));
        });
    };

    // table UI
    ui->proxyListTable->callback_save_order = [=] {
        auto group = NekoRay::profileManager->CurrentGroup();
        group->order = ui->proxyListTable->order;
        group->Save();
    };
    connect(ui->proxyListTable->horizontalHeader(), &QHeaderView::sectionClicked, this,
            [=](int logicalIndex) {
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
                if (logicalIndex == 1) {
                    action.method = NekoRay::GroupSortMethod::ByType;
                } else if (logicalIndex == 2) {
                    action.method = NekoRay::GroupSortMethod::ByAddress;
                } else if (logicalIndex == 3) {
                    action.method = NekoRay::GroupSortMethod::ByName;
                } else if (logicalIndex == 4) {
                    action.method = NekoRay::GroupSortMethod::ByLatency;
                } else if (logicalIndex == 0) {
                    action.method = NekoRay::GroupSortMethod::ById;
                } else {
                    return;
                }
                refresh_proxy_list_impl(-1, action);
            });
    ui->proxyListTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->proxyListTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->proxyListTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->proxyListTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->proxyListTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->proxyListTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    ui->tableWidget_conn->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget_conn->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableWidget_conn->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

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
    auto icon = QIcon::fromTheme("nekoray");
    auto pixmap = QPixmap("../nekoray.png");
    if (!pixmap.isNull()) icon = QIcon(pixmap);
    setWindowIcon(icon);

    tray = new QSystemTrayIcon(this);//初始化托盘对象tray
    tray->setIcon(icon);//设定托盘图标，引号内是自定义的png图片路径
    tray->setContextMenu(ui->menu_program);//创建托盘菜单
    tray->show();//让托盘图标显示在系统托盘上
    connect(tray, &QSystemTrayIcon::activated, this,
            [=](QSystemTrayIcon::ActivationReason reason) {
                switch (reason) {
                    case QSystemTrayIcon::Trigger:
                        if (this->isVisible()) {
                            hide();
                        } else {
                            this->showNormal();
                            this->raise();
                            this->activateWindow();
                        }
                        break;
                    default:
                        break;
                }
            });

    //
    ui->menu_program_preference->addActions(ui->menu_preferences->actions());
    connect(ui->menu_add_from_clipboard2, &QAction::triggered, ui->menu_add_from_clipboard, &QAction::trigger);
    //
    connect(ui->menu_program, &QMenu::aboutToShow, this, [=]() {
        ui->actionRemember_last_proxy->setChecked(NekoRay::dataStore->remember_last_proxy);
        ui->actionStart_with_system->setChecked(GetProcessAutoRunSelf());
        ui->actionStart_minimal->setChecked(NekoRay::dataStore->start_minimal);
    });
    connect(ui->actionRemember_last_proxy, &QAction::triggered, this, [=](bool checked) {
        NekoRay::dataStore->remember_last_proxy = checked;
        NekoRay::dataStore->Save();
    });
    connect(ui->actionStart_with_system, &QAction::triggered, this, [=](bool checked) {
        SetProcessAutoRunSelf(checked);
    });
    connect(ui->actionStart_minimal, &QAction::triggered, this, [=](bool checked) {
        NekoRay::dataStore->start_minimal = checked;
        NekoRay::dataStore->Save();
    });
    //
    connect(ui->menu_system_proxy, &QMenu::aboutToShow, this, [=]() {
        if (NekoRay::dataStore->system_proxy) {
            ui->menu_system_proxy_enabled->setChecked(true);
            ui->menu_system_proxy_disabled->setChecked(false);
        } else {
            ui->menu_system_proxy_enabled->setChecked(false);
            ui->menu_system_proxy_disabled->setChecked(true);
        }
    });
    connect(ui->menu_system_proxy_enabled, &QAction::triggered, this, [=]() { neko_set_system_proxy(true); });
    connect(ui->menu_system_proxy_disabled, &QAction::triggered, this, [=]() { neko_set_system_proxy(false); });
    refresh_status();

#ifndef NKR_NO_GRPC
    // Start Core
    NekoRay::dataStore->core_token = GetRandomString(32);
    NekoRay::dataStore->core_port = MkPort();
    if (NekoRay::dataStore->core_port <= 0) NekoRay::dataStore->core_port = 19810;

    runOnNewThread([=]() {
        core_process = new QProcess;
        QString starting_info;

#ifdef Q_OS_LINUX
        auto core_path = NekoRay::dataStore->core_cap_path;
        if (QFile(core_path).exists()) {
            starting_info = "with cap_net_admin";
        } else {
            starting_info = "as normal user";
            core_path = NekoRay::dataStore->core_path;
        }
#else
        auto core_path = NekoRay::dataStore->core_path;
#endif

        connect(core_process, &QProcess::readyReadStandardOutput, this,
                [&]() {
                    showLog(core_process->readAllStandardOutput().trimmed());
                });
        connect(core_process, &QProcess::readyReadStandardError, this,
                [&]() {
                    auto log = core_process->readAllStandardError().trimmed();
                    if (log.contains("token is set")) {
                        core_process_show_stderr = true;
                        return;
                    }
                    if (core_process_show_stderr) showLog(log);
                });

        QStringList args;
        args.push_back("nekoray");
        args.push_back("-port");
        args.push_back(Int2String(NekoRay::dataStore->core_port));
#ifdef NKR_DEBUG
        args.push_back("-debug");
#endif

        while (true) {
//            core_process.setProcessChannelMode(QProcess::ForwardedChannels);
            showLog("Starting nekoray core " + starting_info + "\n");
            if (!NekoRay::dataStore->v2ray_asset_dir.isEmpty()) {
                core_process->setEnvironment(QStringList{
                        "V2RAY_LOCATION_ASSET=" + NekoRay::dataStore->v2ray_asset_dir
                });
            }
            core_process_show_stderr = false;
            core_process->start(core_path, args);
            core_process->write((NekoRay::dataStore->core_token + "\n").toUtf8());
            core_process->waitForFinished(-1);
            if (core_process_killed) return;
            runOnUiThread([=] { neko_stop(true); });
            QThread::sleep(2);
        }
    });

    // Setup Connection
    defaultClient = new Client([=](const QString &errStr) {
        showLog("gRPC Error: " + errStr);
    }, "127.0.0.1:" + Int2String(NekoRay::dataStore->core_port), NekoRay::dataStore->core_token);
    auto t = new QTimer();
    connect(t, &QTimer::timeout, this, [=]() {
        bool ok = defaultClient->keepAlive();
        runOnUiThread([=]() {
            if (!ok) {
                title_error = tr("Error");
            } else {
                title_error = "";
            }
            refresh_status();
        });
    });
    t->start(2000);

    // Looper
    runOnNewThread([=] { NekoRay::traffic::trafficLooper->loop(); });
#endif

    // Start last
    if (NekoRay::dataStore->remember_last_proxy && NekoRay::dataStore->remember_id >= 0) {
        runOnUiThread([=] { neko_start(NekoRay::dataStore->remember_id); });
    }

    if (!NekoRay::dataStore->start_minimal) show();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (tray->isVisible()) {
        hide(); //隐藏窗口
        event->ignore(); //忽略事件
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

// changed
void MainWindow::on_tabWidget_currentChanged(int index) {
    if (NekoRay::dataStore->refreshing_group_list) return;
    if (tabIndex2GroupId(index) == NekoRay::dataStore->current_group) return;
    show_group(tabIndex2GroupId(index));
}

void MainWindow::show_group(int gid) {
    auto group = NekoRay::profileManager->GetGroup(gid);
    if (group == nullptr) {
        MessageBoxWarning("Error", QString("No such group: %1").arg(gid));
        return;
    }
    if (NekoRay::dataStore->current_group != gid) {
        NekoRay::dataStore->current_group = gid;
        NekoRay::dataStore->Save();
    }
    ui->tabWidget->widget(groupId2TabIndex(gid))->layout()->addWidget(ui->proxyListTable);
    refresh_proxy_list();
}

// callback

void MainWindow::dialog_message_impl(const QString &sender, const QString &info) {
    if (info.contains("SaveDataStore")) {
        auto changed = NekoRay::dataStore->Save();
        refresh_proxy_list();
        if (changed && NekoRay::dataStore->started_id >= 0 &&
            QMessageBox::question(this,
                                  tr("Confirmation"), QString(tr("Settings changed, restart proxy?"))
            ) == QMessageBox::StandardButton::Yes) {
            neko_start(NekoRay::dataStore->started_id);
        }
        refresh_status();
    }
    if (sender == Dialog_DialogEditProfile) {
        if (info == "accept") {
            refresh_proxy_list();
        }
    } else if (sender == Dialog_DialogManageGroups) {
        if (info.startsWith("refresh")) {
            this->refresh_groups();
        }
    } else if (sender == "SubUpdater") {
        // 订阅完毕
        refresh_proxy_list();
        if (!info.contains("dingyue")) {
            QMessageBox::information(this, tr("Info"),
                                     tr("Imported %1 profile(s)").arg(NekoRay::dataStore->updated_count));
        }
    } else if (sender == "ExternalProcess") {
        if (info == "Crashed") {
            neko_stop();
        }
    }
}

// menu

void MainWindow::on_menu_basic_settings_triggered() {
    auto dialog = new DialogBasicSettings(this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::on_menu_routing_settings_triggered() {
    auto dialog = new DialogManageRoutes(this);
    connect(dialog, &QDialog::finished, dialog, &QDialog::deleteLater);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::on_menu_exit_triggered() {
    neko_set_system_proxy(false);

    auto last_id = NekoRay::dataStore->started_id;
    neko_stop();
    if (NekoRay::dataStore->remember_last_proxy && last_id >= 0) {
        NekoRay::dataStore->UpdateStartedId(last_id);
    }

    core_process_killed = true;
    hide();
#ifndef NKR_NO_GRPC
    defaultClient->Exit();
#endif
    if (exit_update) {
        QDir::setCurrent(QApplication::applicationDirPath());
        QProcess::startDetached("./updater", QStringList{});
    }
    qApp->quit();
}


void MainWindow::on_menu_manage_groups_triggered() {
    auto dialog = new DialogManageGroups(this);
    dialog->exec();
    dialog->deleteLater();
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

    // From UI
    ui->label_speed->setText(traffic_update_cache);
    if (last_test_time.addSecs(1) < QTime::currentTime()) {
        ui->label_running->setText(tr("Running: %1").arg(running.isNull()
                                                         ? tr("None")
                                                         : running->bean->DisplayName().left(50)));
    }
    auto inbound_txt = tr("Socks: %1").arg(
            DisplayAddress(NekoRay::dataStore->inbound_address, NekoRay::dataStore->inbound_socks_port));
    if (InRange(NekoRay::dataStore->inbound_http_port, 0, 65535)) {
        inbound_txt += "\n" + tr("HTTP: %1").arg(
                DisplayAddress(NekoRay::dataStore->inbound_address, NekoRay::dataStore->inbound_http_port));
    }
    ui->label_inbound->setText(inbound_txt);
    if (select_mode) {
        ui->label_running->setText("[" + tr("Select") + "]");
    }

    auto make_title = [=](bool isTray) {
        QStringList tt;
        if (select_mode) tt << "[" + tr("Select") + "]";
        if (!title_error.isEmpty()) tt << "[" + title_error + "]";
        if (!title_system_proxy.isEmpty()) tt << "[" + title_system_proxy + "]";
        tt << title_base;
        if (!isTray) tt << "(" + QString(NKR_VERSION) + ")";
        if (!running.isNull()) tt << running->bean->DisplayTypeAndName();
        return tt.join(tray ? "\n" : " ");
    };

    setWindowTitle(make_title(false));
    if (tray != nullptr) tray->setToolTip(make_title(true));
}

// table显示

// update tab_index_GroupId
// refresh proxy list
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
            widget2->setLayout(new QVBoxLayout());
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


void MainWindow::refresh_proxy_list_impl(const int &id, NekoRay::GroupSortAction groupSortAction) {
    if (id < 0) {
        //这样才能清空数据
        ui->proxyListTable->setRowCount(0);
    }

    // 绘制或更新item(s)
    int row = -1;
    for (const auto &profile: NekoRay::profileManager->profiles) {
        if (NekoRay::dataStore->current_group != profile->gid) continue;

        row++;
        if (id >= 0 && profile->id != id) continue; // update only one item
        if (id < 0) {
            ui->proxyListTable->insertRow(row);
        } else {
            // 排序过的，要找
            row = ui->proxyListTable->id2Row[id];
        }

        auto *f0 = new QTableWidgetItem("");
        f0->setData(114514, profile->id);

        // C0: is Running
        auto f = f0->clone();
        if (profile->id == NekoRay::dataStore->started_id) {
            f->setText("✓");
        } else {
            f->setText("　");
        }
        ui->proxyListTable->setItem(row, 0, f);

        // C1: Type
        f = f0->clone();
        f->setText(profile->bean->DisplayType());
        auto insecure_hint = DisplayInsecureHint(profile->bean);
        if (!insecure_hint.isEmpty()) {
            f->setBackground(Qt::red);
            f->setToolTip(insecure_hint);
        }
        ui->proxyListTable->setItem(row, 1, f);

        // C2: Address+Port
        f = f0->clone();
        f->setText(profile->bean->DisplayAddress());
        ui->proxyListTable->setItem(row, 2, f);

        // C3: Name
        f = f0->clone();
        f->setText(profile->bean->name);
        ui->proxyListTable->setItem(row, 3, f);

        // C4: Latency
        f = f0->clone();
        f->setText(profile->DisplayLatency());
        ui->proxyListTable->setItem(row, 4, f);

        // C5: Traffic
        f = f0->clone();
        f->setText(profile->traffic_data->DisplayTraffic());
        ui->proxyListTable->setItem(row, 5, f);
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
                std::sort(ui->proxyListTable->order.begin(), ui->proxyListTable->order.end(),
                          [=](int a, int b) {
                              if (groupSortAction.descending) {
                                  return a > b;
                              } else {
                                  return a < b;
                              }
                          });
                break;
            }
            case NekoRay::GroupSortMethod::ByAddress:
            case NekoRay::GroupSortMethod::ByName:
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
                              }
                              if (groupSortAction.descending) {
                                  return ms_a > ms_b;
                              } else {
                                  return ms_a < ms_b;
                              }
                          });
                break;
            }
            case NekoRay::GroupSortMethod::ByLatency: {
                std::sort(ui->proxyListTable->order.begin(), ui->proxyListTable->order.end(),
                          [=](int a, int b) {
                              auto ms_a = NekoRay::profileManager->GetProfile(a)->latency;
                              auto ms_b = NekoRay::profileManager->GetProfile(b)->latency;
                              if (ms_a <= 0) ms_a = 114514;
                              if (ms_b <= 0) ms_b = 114514;
                              if (groupSortAction.descending) {
                                  return ms_a > ms_b;
                              } else {
                                  return ms_a < ms_b;
                              }
                          });
                break;
            }
        }
        ui->proxyListTable->update_order(groupSortAction.save_sort);
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
    if (!NekoRay::profileManager->CurrentGroup()->url.isEmpty()) return;
    auto dialog = new DialogEditProfile("socks", NekoRay::dataStore->current_group, this);
    connect(dialog, &QDialog::finished, dialog, &QDialog::deleteLater);
}

void MainWindow::on_menu_add_from_clipboard_triggered() {
    if (!NekoRay::profileManager->CurrentGroup()->url.isEmpty()) return;
    auto clipboard = QApplication::clipboard()->text();
    NekoRay::sub::rawUpdater->AsyncUpdate(clipboard);
}

void MainWindow::on_menu_move_triggered() {
    auto ents = GetNowSelected();
    if (ents.isEmpty()) return;

    auto items = QStringList{};
    for (auto &&group: NekoRay::profileManager->groups) {
        if (!group->url.isEmpty()) continue;
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
    auto ents = GetNowSelected();
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
    auto ents = GetNowSelected();
    if (ents.count() == 0) return;
    if (QMessageBox::question(this,
                              tr("Confirmation"),
                              QString(tr("Reset traffic of %1 item(s) ?")).arg(ents.count()))
        == QMessageBox::StandardButton::Yes) {
        for (const auto &ent: ents) {
            ent->traffic_data->Reset();
            ent->Save();
            refresh_proxy_list(ent->id);
        }
    }
}

void MainWindow::on_menu_profile_debug_info_triggered() {
    auto ents = GetNowSelected();
    if (ents.count() != 1) return;
    auto btn = QMessageBox::information(nullptr, "info", ents.first()->ToJsonBytes(), "OK", "Edit", "", 0, 0);
    if (btn == 1) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(
                QFileInfo(QString("profiles/%1.json").arg(ents.first()->id)).absoluteFilePath()
        ));
    }
}

void MainWindow::on_menu_export_config_triggered() {
    auto ents = GetNowSelected();
    if (ents.count() != 1) return;
    auto ent = ents.first();
    auto result = NekoRay::fmt::BuildConfig(ent, false);
    auto config_core = QJsonObject2QString(result->coreConfig, true);
    QApplication::clipboard()->setText(config_core);
    MessageBoxWarning(tr("Config copied"), config_core);
}

void MainWindow::on_menu_copy_link_triggered() {
    auto ents = GetNowSelected();
    if (ents.count() != 1) return;
    QApplication::clipboard()->setText(ents.first()->bean->ToShareLink());
}

void MainWindow::on_menu_qr_triggered() {
    auto ents = GetNowSelected();
    if (ents.count() != 1) return;

    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(ents.first()->bean->ToShareLink().toUtf8().data(),
                                                         qrcodegen::QrCode::Ecc::MEDIUM);
    qint32 sz = qr.getSize();
    QImage im(sz, sz, QImage::Format_RGB32);
    QRgb black = qRgb(0, 0, 0);
    QRgb white = qRgb(255, 255, 255);
    for (int y = 0; y < sz; y++)
        for (int x = 0; x < sz; x++)
            im.setPixel(x, y, qr.getModule(x, y) ? black : white);

    class W : public QDialog {
    public:
        QLabel *l;
        QImage im;

        void set(QLabel *qLabel, QImage qImage) {
            this->l = qLabel;
            this->im = std::move(qImage);
        }

        void resizeEvent(QResizeEvent *resizeEvent) override {
            auto size = resizeEvent->size();
            l->resize(size.width() - 20, size.width() - 20);
        }
    };

    auto w = new W();
    auto l = new QLabel(w);
    w->set(l, im);
    w->setMinimumSize(256, 256);
    l->setMinimumSize(256, 256);
    l->setMargin(6);
    l->setAlignment(Qt::AlignmentFlag::AlignCenter);
    l->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    l->setScaledContents(true);
    l->setPixmap(QPixmap::fromImage(im.scaled(512, 512, Qt::KeepAspectRatio, Qt::FastTransformation), Qt::MonoOnly));
    w->setWindowTitle(ents.first()->bean->DisplayName());
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHeightForWidth(true);
    w->setSizePolicy(sizePolicy);
    w->exec();
    w->deleteLater();
}

void MainWindow::on_menu_scan_qr_triggered() {
    if (!NekoRay::profileManager->CurrentGroup()->url.isEmpty()) return;
#ifndef NKR_NO_EXTERNAL
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
        MessageBoxInfo(tr("Info"), tr("QR Code not found"));
    } else {
        NekoRay::sub::rawUpdater->AsyncUpdate(text);
    }
#endif
}

void MainWindow::on_menu_tcp_ping_triggered() {
#ifndef NKR_NO_GRPC
    speedtest_current_group(libcore::TestMode::TcpPing);
#endif
}

void MainWindow::on_menu_url_test_triggered() {
#ifndef NKR_NO_GRPC
    speedtest_current_group(libcore::TestMode::UrlTest);
#endif
}

void MainWindow::on_menu_clear_test_result_triggered() {
    for (const auto &profile: NekoRay::profileManager->profiles) {
        if (NekoRay::dataStore->current_group != profile->gid) continue;
        profile->latency = 0;
    }
    refresh_proxy_list();
}

void MainWindow::on_menu_delete_repeat_triggered() {
    QList<QSharedPointer<NekoRay::ProxyEntity>> out;
    QList<QSharedPointer<NekoRay::ProxyEntity>> out_del;

    NekoRay::ProfileFilter::Uniq(NekoRay::profileManager->CurrentGroup()->Profiles(), out, true, false);
    NekoRay::ProfileFilter::OnlyInSrc_ByPointer(NekoRay::profileManager->CurrentGroup()->Profiles(), out, out_del);

    QString remove_display;
    for (const auto &ent: out_del) {
        remove_display += ent->bean->DisplayTypeAndName() + "\n";
    }
    if (out_del.length() > 0 &&
        QMessageBox::question(this,
                              tr("Confirmation"),
                              tr("Remove %1 item(s) ?").arg(out_del.length()) + "\n" + remove_display
        ) == QMessageBox::StandardButton::Yes) {

        for (const auto &ent: out_del) {
            NekoRay::profileManager->DeleteProfile(ent->id);
        }
        refresh_proxy_list();
    }
}

void MainWindow::on_menu_remove_unavailable_triggered() {
    QList<QSharedPointer<NekoRay::ProxyEntity>> out_del;

    for (const auto &profile: NekoRay::profileManager->profiles) {
        if (NekoRay::dataStore->current_group != profile->gid) continue;
        if (profile->latency < 0) out_del += profile;
    }

    QString remove_display;
    for (const auto &ent: out_del) {
        remove_display += ent->bean->DisplayTypeAndName() + "\n";
    }
    if (out_del.length() > 0 &&
        QMessageBox::question(this,
                              tr("Confirmation"),
                              tr("Remove %1 item(s) ?").arg(out_del.length()) + "\n" + remove_display
        ) == QMessageBox::StandardButton::Yes) {

        for (const auto &ent: out_del) {
            NekoRay::profileManager->DeleteProfile(ent->id);
        }
        refresh_proxy_list();
    }
}

// table 菜单弹出

void MainWindow::on_proxyListTable_customContextMenuRequested(const QPoint &pos) {
    ui->menu_server->popup(ui->proxyListTable->viewport()->mapToGlobal(pos)); //弹出菜单
}

QMap<int, QSharedPointer<NekoRay::ProxyEntity>> MainWindow::GetNowSelected() {
    auto items = ui->proxyListTable->selectedItems();
    QMap<int, QSharedPointer<NekoRay::ProxyEntity>> map;
    for (auto item: items) {
        auto id = item->data(114514).toInt();
        auto ent = NekoRay::profileManager->GetProfile(id);
        if (ent != nullptr) map[id] = ent;
    }
    return map;
}

// 按键

void MainWindow::neko_start(int _id) {
    auto ents = GetNowSelected();
    auto ent = (_id < 0 && !ents.isEmpty()) ? ents.first() : NekoRay::profileManager->GetProfile(_id);
    if (ent == nullptr) return;

    if (select_mode) {
        emit profile_selected(ent->id);
        select_mode = false;
        refresh_status();
        return;
    }

    if (NekoRay::profileManager->GetGroup(ent->gid)->archive) return;

    auto result = NekoRay::fmt::BuildConfig(ent, false);
    if (!result->error.isEmpty()) {
        MessageBoxWarning("BuildConfig return error", result->error);
        return;
    }

    if (NekoRay::dataStore->started_id >= 0) neko_stop();
    show_log_impl(">>>>>>>> " + tr("Starting profile %1").arg(ent->bean->DisplayTypeAndName()));
    auto insecure_hint = DisplayInsecureHint(ent->bean);
    if (!insecure_hint.isEmpty()) show_log_impl(">>>>>>>> " + tr("Profile is insecure: %1").arg(insecure_hint));

#ifndef NKR_NO_GRPC
    bool rpcOK;
    QString error = defaultClient->Start(&rpcOK,
                                         QJsonObject2QString(result->coreConfig, true),
                                         result->tryDomains
    );
    if (rpcOK && !error.isEmpty()) {
        MessageBoxWarning("LoadConfig return error", error);
        return;
    }

    NekoRay::traffic::trafficLooper->proxy = result->outboundStat.get();
    NekoRay::traffic::trafficLooper->items = result->outboundStats;
    NekoRay::traffic::trafficLooper->loop_enabled = true;
#endif

    for (auto extC: result->ext) {
        extC->Start();
    }

    NekoRay::dataStore->UpdateStartedId(ent->id);
    running = ent;
    refresh_status();
    refresh_proxy_list(ent->id);
}

void MainWindow::neko_stop(bool crash) {
    auto id = NekoRay::dataStore->started_id;
    if (id < 0) return;
    show_log_impl(">>>>>>>> " + tr("Stopping profile %1").arg(running->bean->DisplayTypeAndName()));

    while (!NekoRay::sys::running_ext.isEmpty()) {
        auto extC = NekoRay::sys::running_ext.takeFirst();
        extC->Kill();
        extC->deleteLater();
    }

#ifndef NKR_NO_GRPC
    NekoRay::traffic::trafficLooper->loop_enabled = false;
    NekoRay::traffic::trafficLooper->loop_mutex.lock();
    for (const auto &item: NekoRay::traffic::trafficLooper->items) {
        NekoRay::traffic::TrafficLooper::update(item.get());
        NekoRay::profileManager->GetProfile(item->id)->Save();
        refresh_proxy_list(item->id);
    }
    NekoRay::traffic::trafficLooper->loop_mutex.unlock();

    if (!crash) {
        bool rpcOK;
        QString error = defaultClient->Stop(&rpcOK);
        if (rpcOK && !error.isEmpty()) {
            MessageBoxWarning("Stop return error", error);
            return;
        }
    }
#endif

    NekoRay::dataStore->UpdateStartedId(-1919);
    running = nullptr;
    refresh_status();
    refresh_proxy_list(id);
}

void MainWindow::neko_set_system_proxy(bool enable) {
#ifdef Q_OS_WIN
    if (enable && !InRange(NekoRay::dataStore->inbound_http_port, 0, 65535)) {
        MessageBoxWarning(tr("Error"), tr("Http inbound is not enabled, can't set system proxy."));
        return;
    }
#endif
    NekoRay::dataStore->system_proxy = enable;
#ifndef NKR_NO_EXTERNAL
    if (enable) {
        SetSystemProxy("127.0.0.1",
                       NekoRay::dataStore->inbound_http_port,
                       NekoRay::dataStore->inbound_socks_port);
        title_system_proxy = tr("System Proxy");
    } else {
        ClearSystemProxy();
        title_system_proxy = "";
    }
#endif
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Escape:
            break;
        case Qt::Key_Return: {
            neko_start();
            break;
        }
        case Qt::Key_Delete: {
            on_menu_delete_triggered();
            break;
        }
        default:
            QMainWindow::keyPressEvent(event);
    }
}

// 测速

#ifndef NKR_NO_GRPC

void MainWindow::speedtest_current_group(libcore::TestMode mode) {
    neko_stop();

    runOnNewThread([=]() {
        auto group = NekoRay::profileManager->CurrentGroup();
        if (group->archive) return;
        auto order = ui->proxyListTable->order;//copy

        QList<QSharedPointer<NekoRay::ProxyEntity>> profiles;
        QMutex lock;
        QMutex lock2;
        int threadN = NekoRay::dataStore->test_concurrent;
        int threadN_finished = 0;

        // 这个是按照显示的顺序
        for (auto id: order) {
            auto profile = NekoRay::profileManager->GetProfile(id);
            if (profile != nullptr) profiles += profile;
        }

        // Threads
        for (int i = 0; i < threadN; i++) {
            runOnNewThread([&] {
                forever {
                    //
                    lock.lock();
                    if (profiles.isEmpty()) {
                        threadN_finished++;
                        if (threadN == threadN_finished) lock2.unlock();
                        lock.unlock();
                        return;
                    }
                    auto profile = profiles.takeFirst();
                    lock.unlock();

                    //
                    libcore::TestReq req;
                    req.set_mode(mode);
                    req.set_timeout(3000);
                    req.set_url(NekoRay::dataStore->test_url.toStdString());

                    //
                    QList<NekoRay::sys::ExternalProcess *> ext;

                    if (mode == libcore::TestMode::UrlTest) {
                        auto c = NekoRay::fmt::BuildConfig(profile, true);
                        // external test ???
                        if (!c->ext.isEmpty()) {
                            ext = c->ext;
                            for (auto extC: ext) {
                                extC->Start();
                            }
                            QThread::msleep(500);
                        }
                        //
                        auto config = new libcore::LoadConfigReq;
                        config->set_coreconfig(QJsonObject2QString(c->coreConfig, true).toStdString());
                        req.set_allocated_config(config);
                    } else {
                        req.set_address(profile->bean->DisplayAddress().toStdString());
                    }

                    bool rpcOK;
                    auto result = defaultClient->Test(&rpcOK, req);
                    for (auto extC: ext) {
                        extC->Kill();
                        extC->deleteLater();
                    }
                    if (!rpcOK) return;

                    profile->latency = result.ms();
                    if (profile->latency == 0) profile->latency = -1; // sn

                    runOnUiThread([=] {
                        if (!result.error().empty()) {
                            show_log_impl(
                                    tr("[%1] test error: %2").arg(profile->bean->DisplayName(),
                                                                  result.error().c_str()));
                        }
                        refresh_proxy_list(profile->id);
                    });
                }
            });
        }

        // Control
        lock2.lock();
        lock2.lock();
        lock2.unlock();
    });
}

#endif

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
    if (log.trimmed().isEmpty()) return;

    FastAppendTextDocument(log, qvLogDocument);
    // qvLogDocument->setPlainText(qvLogDocument->toPlainText() + log);
    // From https://gist.github.com/jemyzhang/7130092
    auto maxLines = 1000;
    auto block = qvLogDocument->begin();

    while (block.isValid()) {
        if (qvLogDocument->blockCount() > maxLines) {
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

void MainWindow::on_masterLogBrowser_customContextMenuRequested(const QPoint &pos) {
    QMenu *menu = ui->masterLogBrowser->createStandardContextMenu();

    auto sep = new QAction;
    sep->setSeparator(true);
    menu->addAction(sep);

    auto action_clear = new QAction;
    action_clear->setText(tr("Clear"));
    action_clear->setData(-1);
    connect(action_clear, &QAction::triggered, this, [=] {
        qvLogDocument->clear();
    });
    menu->addAction(action_clear);

    menu->exec(ui->masterLogBrowser->viewport()->mapToGlobal(pos)); //弹出菜单
}

// eventFilter

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        auto mouseEvent = dynamic_cast<QMouseEvent *>(event);

        if (obj == ui->label_running && mouseEvent->button() == Qt::LeftButton && running != nullptr) {
#ifndef NKR_NO_GRPC
            last_test_time = QTime::currentTime();
            ui->label_running->setText(tr("Testing"));

            runOnNewThread([=] {
                libcore::TestReq req;
                req.set_mode(libcore::UrlTest);
                req.set_timeout(3000);
                req.set_url(NekoRay::dataStore->test_url.toStdString());

                bool rpcOK;
                auto result = defaultClient->Test(&rpcOK, req);
                if (!rpcOK) return;

                auto latency = result.ms();
                last_test_time = QTime::currentTime();

                runOnUiThread([=] {
                    if (latency <= 0) {
                        ui->label_running->setText(tr("Test Result") + ": " + tr("Unavailable"));
                    } else if (latency > 0) {
                        ui->label_running->setText(tr("Test Result") + ": " + QString("%1 ms").arg(latency));
                    }
                });
            });
            return true;
#endif
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

inline QJsonArray last_arr;

void MainWindow::refresh_connection_list(const QJsonArray &arr) {
    if (last_arr == arr) {
        return;
    }
    last_arr = arr;

    ui->tableWidget_conn->setRowCount(0);

    int row = -1;
    for (const auto &_item: arr) {
        auto item = _item.toObject();

        row++;
        ui->tableWidget_conn->insertRow(row);

        // C0: Status
        auto *f = new QTableWidgetItem("");
        f->setData(114514, item["ID"].toInt());
        auto start_t = item["Start"].toInt();
        auto end_t = item["End"].toInt();
        if (end_t > 0) {
            f->setText(tr("End"));
        } else {
            f->setText(tr("Active"));
        }
        f->setToolTip(tr("Start: %1\nEnd: %2").arg(
                DisplayTime(start_t),
                end_t > 0 ? DisplayTime(end_t) : ""
        ));
        ui->tableWidget_conn->setItem(row, 0, f);

        // C1: Outbound
        f = f->clone();
        f->setToolTip("");
        f->setText(item["Tag"].toString());
        ui->tableWidget_conn->setItem(row, 1, f);

        // C2: Destination
        f = f->clone();
        QString target1 = item["Dest"].toString();
        QString target2 = item["RDest"].toString();
        if (!target2.isEmpty() && target1 != target2) {
            std::swap(target1, target2);
        } else {
            target2 = "";
        }
        f->setText("[" + target1 + "] " + target2);
        ui->tableWidget_conn->setItem(row, 2, f);
    }
}
