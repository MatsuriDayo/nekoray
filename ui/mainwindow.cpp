#include "./ui_mainwindow.h"

#include "fmt/ConfigBuilder.hpp"
#include "sub/RawUpdater.hpp"
#include "db/traffic/TrafficLooper.hpp"
#include "db/filter/ProfileFilter.hpp"

#include "ui/mainwindow.h"
#include "ui/dialog_basic_settings.h"
#include "ui/edit/dialog_edit_profile.h"
#include "ui/dialog_manage_groups.h"
#include "ui/dialog_manage_routes.h"

#include "3rdparty/qrcodegen.hpp"

#ifndef __MINGW32__

#include "qv2ray/components/proxy/QvProxyConfigurator.hpp"

#endif

#include <QClipboard>
#include <QLabel>
#include <QTextBlock>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    mainwindow = this;

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
        MessageBoxWarning(tr("Error"), tr("No permission to write %1").arg(dir.absolutePath()));
        return;
    }

    // Clean
    QFile::remove("updater.old");

    // Load dataStore
    auto isLoaded = NekoRay::dataStore->Load();
    if (!isLoaded) {
        NekoRay::dataStore->Save();
    }

    // Load Manager
    isLoaded = NekoRay::profileManager->Load();
    if (!isLoaded) {
        auto defaultGroup = NekoRay::ProfileManager::NewGroup();
        defaultGroup->name = tr("Default");
        NekoRay::profileManager->AddGroup(defaultGroup);
    }

    // Setup misc UI
    ui->setupUi(this);
    title_base = windowTitle();
    connect(ui->menu_start, &QAction::triggered, this, [=]() { neko_start(); });
    connect(ui->menu_stop, &QAction::triggered, this, [=]() { neko_stop(); });

    // top bar
    ui->toolButton_program->setMenu(ui->menu_program);
    ui->toolButton_preferences->setMenu(ui->menu_preferences);
    ui->toolButton_server->setMenu(ui->menu_server);
    ui->menubar->setVisible(false);
    connect(ui->toolButton_ads, &QToolButton::clicked, this,
            [=] { QDesktopServices::openUrl(QUrl("https://matsuridayo.github.io/")); });

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
            writeLog_ui(log);
        });
    };

    // table UI
    ui->proxyListTable->callback_save_order = [=] {
        auto group = NekoRay::ProfileManager::CurrentGroup();
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
                refresh_proxy_list(-1, action);
            });
    // TODO header: disable auto size when size changed by user

    // refresh
    this->refresh_groups();
    this->refresh_proxy_list();
    auto last_selected = NekoRay::dataStore->current_group;
    NekoRay::dataStore->current_group = 0;
    ui->tabWidget->setCurrentIndex(last_selected);

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
    connect(ui->menu_add_from_clipboard2, &QAction::triggered, ui->menu_add_from_clipboard, &QAction::trigger);
    connect(ui->menu_manage_group, &QAction::triggered, ui->menu_manage_groups, &QAction::trigger);
    ui->menu_program_preference->addActions(ui->menu_preferences->actions());
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

#ifndef NO_GRPC
    // Start Core
    NekoRay::dataStore->core_token = GetRandomString(32);
    NekoRay::dataStore->core_port = MkPort();
    if (NekoRay::dataStore->core_port <= 0) NekoRay::dataStore->core_port = 19810;

    runOnNewThread([=]() {
        core_process = new QProcess;

        connect(core_process, &QProcess::readyReadStandardOutput, this,
                [=]() {
                    // TODO read output sometimes crash
                    showLog(core_process->readAllStandardOutput().trimmed());
                });

        QStringList args;
        args.push_back("nekoray");
        args.push_back("-port");
        args.push_back(Int2String(NekoRay::dataStore->core_port));

        while (true) {
//            core_process.setProcessChannelMode(QProcess::ForwardedChannels);
            showLog("Starting nekoray core\n");
            core_process->start(NekoRay::dataStore->core_path, args);
            core_process->write((NekoRay::dataStore->core_token + "\n").toUtf8());
            core_process->waitForFinished(-1);
            if (core_process_killed) return;
            QThread::sleep(2);
        }
    });

    // Setup Connection
    NekoRay::rpc::defaultClient = defaultClient = new NekoRay::rpc::Client([=](const QString &errStr) {
        showLog("gRPC Error: " + errStr);
    }, "127.0.0.1:" + Int2String(NekoRay::dataStore->core_port), NekoRay::dataStore->core_token);
    auto t = new QTimer();
    connect(t, &QTimer::timeout, this, [=]() {
        bool ok = defaultClient->keepAlive();
        runOnUiThread([=]() {
            if (!ok) {
                title_status = tr("Error");
            } else {
                title_status = "";
            }
            refresh_status();
        });
    });
    t->start(2000);

    // Looper
    runOnNewThread([=] { NekoRay::traffic::trafficLooper->loop(); });
#endif
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (tray->isVisible()) {
        hide(); //隐藏窗口
        event->ignore(); //忽略事件
    }
}

void MainWindow::hideEvent(QHideEvent *event) {
    if (tray->isVisible()) {
        hide(); //隐藏窗口
        event->ignore(); //忽略事件
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

// Group tab manage

inline QMap<int, int> tab_index_GroupId;

inline int tabIndex2GroupId(int index) {
    return tab_index_GroupId[index];
}

inline int groupId2TabIndex(int gid) {
    for (auto key: tab_index_GroupId) {
        if (tab_index_GroupId[key] == gid) return tab_index_GroupId[key];
    }
    return 0;
}

void MainWindow::on_tabWidget_currentChanged(int index) {
    if (index == NekoRay::dataStore->current_group) return;
    NekoRay::dataStore->current_group = tabIndex2GroupId(index);
    NekoRay::dataStore->Save();
    ui->tabWidget->widget(index)->layout()->addWidget(ui->proxyListTable);
    refresh_proxy_list();
}

// callback

void MainWindow::dialog_message(const QString &dialog, const QString &info) {
    runOnUiThread([=] {
        if (info.contains("SaveDataStore")) {
            auto changed = NekoRay::dataStore->Save();
            this->refresh_proxy_list();
            if (changed && NekoRay::dataStore->started_id >= 0 &&
                QMessageBox::question(this,
                                      tr("Confirmation"), QString(tr("Settings changed, restart proxy?"))
                ) == QMessageBox::StandardButton::Yes) {
                neko_start(NekoRay::dataStore->started_id);
            }
            refresh_status();
        }
        if (dialog == Dialog_DialogEditProfile) {
            if (info == "accept") {
                this->refresh_proxy_list();
            }
        } else if (dialog == Dialog_DialogManageGroups) {
            if (info == "refresh") {
                this->refresh_groups();
                on_tabWidget_currentChanged(groupId2TabIndex(0));
            }
        } else if (dialog == "SubUpdater") {
            // 订阅完毕
            refresh_proxy_list();
            QMessageBox::information(this, tr("Info"),
                                     QString("Imported %1 profile(s).").arg(NekoRay::dataStore->updated_count));
        }
    });
}

// menu

void MainWindow::on_menu_basic_settings_triggered() {
    auto dialog = new DialogBasicSettings(this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::on_menu_routing_settings_triggered() {
    auto dialog = new DialogManageRoutes(this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::on_menu_exit_triggered() {
    neko_set_system_proxy(false);
    neko_stop();
    core_process_killed = true;
#ifndef NO_GRPC
    defaultClient->Exit();
#endif
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
    ui->label_running->setText(tr("Running: %1").arg(running.isNull() ? tr("None") : running->bean->DisplayName()));
    auto inbound_txt = tr("Socks: %1").arg(
            DisplayAddress(NekoRay::dataStore->inbound_address, NekoRay::dataStore->inbound_socks_port));
    if (InRange(NekoRay::dataStore->inbound_http_port, 0, 65535)) {
        inbound_txt += "\n" + tr("HTTP: %1").arg(
                DisplayAddress(NekoRay::dataStore->inbound_address, NekoRay::dataStore->inbound_http_port));
    }
    ui->label_inbound->setText(inbound_txt);

    QString tt_status;
    if (!title_status.isEmpty()) {
        tt_status = "[" + title_status + "] ";
    }

    QString tt_running = "";
    if (!running.isNull()) {
        tt_running = running->bean->DisplayTypeAndName();
    }

    QString windowText = QString("%1%2 (%3)").arg(tt_status, title_base, NKR_RELEASE_DATE);
    QString trayText = QString("%1%2").arg(tt_status, title_base);

    if (!tt_running.isEmpty()) {
        windowText += " " + tt_running;
        trayText += "\n" + tt_running;
    }

    setWindowTitle(windowText);
    if (tray != nullptr) tray->setToolTip(trayText);
}

// table显示

void MainWindow::refresh_groups() {
    auto_resize_table_header = true;
    tab_index_GroupId.clear();

    // refresh group?
    for (int i = ui->tabWidget->count() - 1; i > 0; i--) {
        ui->tabWidget->removeTab(i);
    }

    int index = 0;
    for (const auto &group: NekoRay::profileManager->groups) {
        if (group->id == 0) {
            ui->tabWidget->setTabText(0, group->name);
        } else {
            auto widget2 = new QWidget();
            widget2->setLayout(new QVBoxLayout());
            ui->tabWidget->addTab(widget2, group->name);
        }
        tab_index_GroupId[index] = group->id;
        index++;
    }

//    ui->tabWidget->widget(0)->layout()->addWidget(ui->proxyListTable);
    ui->tabWidget->setCurrentIndex(0);
}


void MainWindow::refresh_proxy_list(const int &id, NekoRay::GroupSortAction groupSortAction) {
    if (id < 0) {
        //这样才能清空数据
        ui->proxyListTable->setRowCount(0);
    }

    // 绘制或更新item(s)
    int row = -1;
    for (const auto &profile: NekoRay::profileManager->profiles) {
        if (NekoRay::dataStore->current_group != profile->group_id) continue;

        row++;
        if (id >= 0 && profile->id != id) continue; // update only one item
        if (id < 0) {
            ui->proxyListTable->insertRow(row);
        } else {
            // 排序过的，要找
            row = ui->proxyListTable->id2Row[id];
        }

        // C0: is Running
        auto *f = new QTableWidgetItem("");
        f->setData(114514, profile->id);
        if (profile->id == NekoRay::dataStore->started_id) f->setText("✓");
        ui->proxyListTable->setItem(row, 0, f);

        // C1: Type
        f = f->clone();
        f->setText(profile->bean->DisplayType());
        ui->proxyListTable->setItem(row, 1, f);

        // C2: Address+Port
        f = f->clone();
        f->setText(profile->bean->DisplayAddress());
        ui->proxyListTable->setItem(row, 2, f);

        // C3: Name
        f = f->clone();
        f->setText(profile->bean->name);
        ui->proxyListTable->setItem(row, 3, f);

        // C4: Latency
        f = f->clone();
        if (profile->latency < 0) {
            // TODO reason
            f->setText(tr("Unavailable"));
        } else if (profile->latency > 0) {
            f->setText(QString("%1 ms").arg(profile->latency));
        } else {
            f->setText("");
        }
        ui->proxyListTable->setItem(row, 4, f);

        // C5: Traffic
        f = f->clone();
        f->setText(profile->traffic_data->DisplayTraffic());
        ui->proxyListTable->setItem(row, 5, f);
    }

    // 显示排序
    if (id < 0) {
        switch (groupSortAction.method) {
            case NekoRay::GroupSortMethod::Raw: {
                auto group = NekoRay::ProfileManager::CurrentGroup();
                ui->proxyListTable->order = group->order;
                break;
            }
            case NekoRay::GroupSortMethod::ById: {
                std::sort(ui->proxyListTable->order.begin(), ui->proxyListTable->order.end(),
                          [=](int a, int b) {
                              int int_a = NekoRay::profileManager->GetProfile(a)->id;
                              int int_b = NekoRay::profileManager->GetProfile(b)->id;
                              if (groupSortAction.descending) {
                                  return int_a > int_b;
                              } else {
                                  return int_a < int_b;
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
    if (auto_resize_table_header)
        ui->proxyListTable->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

// table菜单相关

void MainWindow::on_proxyListTable_itemDoubleClicked(QTableWidgetItem *item) {
    auto dialog = new DialogEditProfile("", item->data(114514).toInt(), this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::on_menu_add_from_input_triggered() {
    if (NekoRay::ProfileManager::CurrentGroup()->IsSubscription()) return;
    auto dialog = new DialogEditProfile("socks", NekoRay::dataStore->current_group, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::on_menu_add_from_clipboard_triggered() {
    if (NekoRay::ProfileManager::CurrentGroup()->IsSubscription()) return;
    auto clipboard = QApplication::clipboard()->text();
    NekoRay::sub::rawUpdater->AsyncUpdate(clipboard);
}

void MainWindow::on_menu_delete_triggered() {
    auto ents = GetNowSelected();
    if (ents.count() == 0) return;
    if (QMessageBox::question(this, tr("Confirmation"), QString(tr("Remove %1 item(s) ?")).arg(ents.count())) ==
        QMessageBox::StandardButton::Yes) {
        for (const auto &ent: ents) {
            NekoRay::profileManager->DeleteProfile(ent->id);
            refresh_proxy_list();
        }
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
    MessageBoxWarning("info", ents.first()->ToJsonBytes());
}

void MainWindow::on_menu_export_config_triggered() {
    auto ents = GetNowSelected();
    if (ents.count() != 1) return;
    auto ent = ents.first();
    auto result = NekoRay::fmt::BuildConfig(ent, false);
    auto config_core = QJsonObject2QString(result->coreConfig, true);
    MessageBoxWarning("info", config_core);
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

    // TODO 可放大
    auto w = new QDialog();
    auto l = new QLabel(w);
    w->setMinimumSize(256, 256);
    l->setMinimumSize(256, 256);
    l->setScaledContents(true);
    l->setPixmap(QPixmap::fromImage(im.scaled(256, 256, Qt::KeepAspectRatio, Qt::FastTransformation), Qt::MonoOnly));
    w->setWindowTitle(ents.first()->bean->DisplayName());
    w->exec();
    w->deleteLater();
}

void MainWindow::on_menu_scan_qr_triggered() {

}

void MainWindow::on_menu_tcp_ping_triggered() {
#ifndef NO_GRPC
    speedtest_current_group(libcore::TestMode::TcpPing);
#endif
}

void MainWindow::on_menu_url_test_triggered() {
#ifndef NO_GRPC
    speedtest_current_group(libcore::TestMode::UrlTest);
#endif
}

void MainWindow::on_menu_clear_test_result_triggered() {
    for (const auto &profile: NekoRay::profileManager->profiles) {
        if (NekoRay::dataStore->current_group != profile->group_id) continue;
        profile->latency = 0;
    }
    refresh_proxy_list();
}

void MainWindow::on_menu_delete_repeat_triggered() {
    QList<QSharedPointer<NekoRay::ProxyEntity>> out;
    QList<QSharedPointer<NekoRay::ProxyEntity>> out_del;

    NekoRay::ProfileFilter::Uniq(NekoRay::ProfileManager::CurrentGroup()->Profiles(), out, true, false);
    NekoRay::ProfileFilter::OnlyInSrc_ByPointer(NekoRay::ProfileManager::CurrentGroup()->Profiles(), out, out_del);
    if (out_del.length() > 0 &&
        QMessageBox::question(this,
                              tr("Confirmation"),
                              tr("Remove %1 item(s) ?").arg(out_del.length())
        ) == QMessageBox::StandardButton::Yes) {

        QString deleted;
        for (const auto &ent: out_del) {
            deleted += ent->bean->DisplayTypeAndName() + "\n";
            NekoRay::profileManager->DeleteProfile(ent->id);
        }

        refresh_proxy_list();
        MessageBoxWarning(tr("Deleted"), tr("Deleted %1 items:").arg(out_del.length()) + "\n" + deleted);
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
        if (ent != nullptr) {
            map[id] = ent;
        }
    }
    return map;
}

// 按键

void MainWindow::neko_start(int id) {
    auto ents = GetNowSelected();
    if (ents.isEmpty() && id < 0) return;
    auto ent = id < 0 ? ents.first() : NekoRay::profileManager->GetProfile(id);

    if (NekoRay::dataStore->started_id >= 0) neko_stop();

    auto result = NekoRay::fmt::BuildConfig(ent, false);
    if (!result->error.isEmpty()) {
        MessageBoxWarning("BuildConfig return error", result->error);
        return;
    }

#ifndef NO_GRPC
    bool rpcOK;
    QString error = defaultClient->Start(&rpcOK, QJsonObject2QString(result->coreConfig, true));
    if (rpcOK && !error.isEmpty()) {
        MessageBoxWarning("LoadConfig return error", error);
        return;
    }

    NekoRay::traffic::trafficLooper->proxy = result->outboundStat.get();
    NekoRay::traffic::trafficLooper->items = result->outboundStats;
    NekoRay::traffic::trafficLooper->loop_enabled = true;
#endif

    NekoRay::dataStore->started_id = ent->id;
    running = ent;
    refresh_status();
    refresh_proxy_list();
}

void MainWindow::neko_stop() {
    if (NekoRay::dataStore->started_id < 0) return;

#ifndef NO_GRPC
    // TODO save traffic & conflict?
    NekoRay::traffic::trafficLooper->loop_enabled = false;
    NekoRay::traffic::trafficLooper->loop_mutex.lock();
    for (const auto &item: NekoRay::traffic::trafficLooper->items) {
        NekoRay::traffic::trafficLooper->update(item.get());
        NekoRay::profileManager->GetProfile(item->id)->Save();
        refresh_proxy_list(item->id);
    }
    NekoRay::traffic::trafficLooper->loop_mutex.unlock();

    // TODO instance not restated?
    bool rpcOK;
    QString error = defaultClient->Stop(&rpcOK);
    if (rpcOK && !error.isEmpty()) {
        MessageBoxWarning("Stop return error", error);
        return;
    }
#endif

    NekoRay::dataStore->started_id = -1919;
    running = nullptr;
    refresh_status();
    refresh_proxy_list();
}

void MainWindow::neko_set_system_proxy(bool enable) {
    NekoRay::dataStore->system_proxy = enable;
#ifndef __MINGW32__
    if (enable) {
        SetSystemProxy("127.0.0.1",
                       NekoRay::dataStore->inbound_http_port,
                       NekoRay::dataStore->inbound_socks_port);
    } else {
        ClearSystemProxy();
    }
#endif
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Escape:
            break; //屏蔽esc
        case Qt::Key_Return: {//回车
            neko_start();
            break;
        }
        default:
            QMainWindow::keyPressEvent(event);
    }
}

// 测速

#ifndef NO_GRPC

void MainWindow::speedtest_current_group(libcore::TestMode mode) {
    runOnNewThread([=]() {
        auto group = NekoRay::ProfileManager::CurrentGroup();
        auto order = ui->proxyListTable->order;//copy
        auto count = order.length();
        int tested = 0;

        // 这个是按照显示的顺序
        for (auto id: order) {
            auto profile = NekoRay::profileManager->GetProfile(id);
            // TODO multi thread

            libcore::TestReq req;
            req.set_mode(mode);
            req.set_timeout(3000);
            req.set_inbound("socks-in"); // no needed?
            req.set_url(NekoRay::dataStore->test_url.toStdString());

            if (mode == libcore::TestMode::UrlTest) {
                auto c = NekoRay::fmt::BuildConfig(profile, true);
                auto config = new libcore::LoadConfigReq;
                config->set_coreconfig(QJsonObject2QString(c->coreConfig, true).toStdString());
                req.set_allocated_config(config);
            } else {
                req.set_address(profile->bean->DisplayAddress().toStdString());
            }

            bool rpcOK;
            auto result = defaultClient->Test(&rpcOK, req);
            tested++;
            if (!rpcOK) return;

            profile->latency = result.ms();
            if (profile->latency == 0) profile->latency = -1; // sn

            runOnUiThread([=] {
                if (!result.error().empty()) {
                    writeLog_ui(tr("[%1] test error: %2").arg(profile->bean->DisplayName(), result.error().c_str()));
                }
                refresh_proxy_list(profile->id);
            });
        }
    });
}

#endif

// Log

inline void FastAppendTextDocument(const QString &message, QTextDocument *doc) {
    // TODO 有个第一行空着
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End);
    cursor.beginEditBlock();
    cursor.insertBlock();
    cursor.insertText(message);
    cursor.endEditBlock();
}

void MainWindow::writeLog_ui(const QString &log) {
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

    auto action_hide = new QAction;
    action_hide->setText(tr("Hide"));
    action_hide->setData(-1);
    connect(action_hide, &QAction::triggered, this, [=] {
        ui->masterLogBrowser->setVisible(false);
    });
    menu->addAction(action_hide);

    menu->exec(ui->masterLogBrowser->viewport()->mapToGlobal(pos)); //弹出菜单
}
