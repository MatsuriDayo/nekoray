#include "./ui_mainwindow.h"
#include "mainwindow.h"

#include "db/Database.hpp"
#include "db/ConfigBuilder.hpp"
#include "db/TrafficLooper.hpp"
#include "rpc/gRPC.h"

#include <QTimer>
#include <QThread>
#include <QInputDialog>
#include <QPushButton>
#include <QDesktopServices>
#include <QMessageBox>

#ifndef NKR_NO_GRPC
using namespace NekoRay::rpc;
#endif

void MainWindow::setup_grpc() {
#ifndef NKR_NO_GRPC
    // Setup Connection
    defaultClient = new Client(
        [=](const QString &errStr) {
            MW_show_log("[Error] gRPC: " + errStr);
        },
        "127.0.0.1:" + Int2String(NekoRay::dataStore->core_port), NekoRay::dataStore->core_token);
    auto t = new QTimer();
    connect(t, &QTimer::timeout, this, [=]() {
        refresh_status();
    });
    t->start(2000);

    // Looper
    runOnNewThread([=] { NekoRay::traffic::trafficLooper->Loop(); });
#endif
}

// 测速

inline bool speedtesting = false;

void MainWindow::speedtest_current_group(int mode) {
    auto profiles = get_selected_or_group();
    if (profiles.isEmpty()) return;
    auto group = NekoRay::profileManager->CurrentGroup();
    if (group->archive) return;

#ifndef NKR_NO_GRPC
    if (speedtesting) return;
    QStringList full_test_flags;
    if (mode == libcore::FullTest) {
        bool ok;
        auto s = QInputDialog::getText(nullptr, tr("Input"),
                                       tr("Please enter the items to be tested, separated by commas\n"
                                          "1. Latency\n"
                                          "2. Download speed\n"
                                          "3. In and Out IP\n"
                                          "4. UDP Latency"),
                                       QLineEdit::Normal, "1,4", &ok);
        full_test_flags = s.trimmed().split(",");
        if (!ok) return;
    }
    speedtesting = true;

    runOnNewThread([this, profiles, mode, full_test_flags]() {
        QMutex lock_write;
        QMutex lock_return;
        int threadN = NekoRay::dataStore->test_concurrent;
        int threadN_finished = 0;
        auto profiles_test = profiles; // copy

        // Threads
        lock_return.lock();
        for (int i = 0; i < threadN; i++) {
            runOnNewThread([&] {
                forever {
                    //
                    lock_write.lock();
                    if (profiles_test.isEmpty()) {
                        threadN_finished++;
                        if (threadN == threadN_finished) lock_return.unlock();
                        lock_write.unlock();
                        return;
                    }
                    auto profile = profiles_test.takeFirst();
                    lock_write.unlock();

                    //
                    libcore::TestReq req;
                    req.set_mode((libcore::TestMode) mode);
                    req.set_timeout(3000);
                    req.set_url(NekoRay::dataStore->test_url.toStdString());

                    //
                    std::list<std::pair<NekoRay::fmt::ExternalBuildResult, QSharedPointer<NekoRay::sys::ExternalProcess>>> exts;

                    if (mode == libcore::TestMode::UrlTest || mode == libcore::FullTest) {
                        auto c = NekoRay::BuildConfig(profile, true, false);
                        // TODO refactor external test
                        if (!c->exts.empty()) {
                            exts = c->exts;
                            for (const auto &ext: exts) {
                                ext.second->Start();
                            }
                            QThread::msleep(500);
                        }
                        //
                        auto config = new libcore::LoadConfigReq;
                        config->set_core_config(QJsonObject2QString(c->coreConfig, true).toStdString());
                        req.set_allocated_config(config);
                        req.set_in_address(profile->bean->serverAddress.toStdString());

                        req.set_full_latency(full_test_flags.contains("1"));
                        req.set_full_speed(full_test_flags.contains("2"));
                        req.set_full_in_out(full_test_flags.contains("3"));
                        req.set_full_udp_latency(full_test_flags.contains("4"));
                    } else if (mode == libcore::TcpPing) {
                        req.set_address(profile->bean->DisplayAddress().toStdString());
                    }

                    bool rpcOK;
                    auto result = defaultClient->Test(&rpcOK, req);
                    for (const auto &ext: exts) {
                        ext.second->Kill();
                    }
                    if (!rpcOK) return;

                    if (result.error().empty()) {
                        profile->latency = result.ms();
                        if (profile->latency == 0) profile->latency = 1; // nekoray use 0 to represents not tested
                    } else {
                        profile->latency = -1;
                    }
                    profile->full_test_report = result.full_report().c_str(); // higher priority
                    profile->Save();

                    if (!result.error().empty()) {
                        MW_show_log(tr("[%1] test error: %2").arg(profile->bean->DisplayTypeAndName(), result.error().c_str()));
                    }

                    auto profileId = profile->id;
                    runOnUiThread([this, profileId] {
                        refresh_proxy_list(profileId);
                    });
                }
            });
        }

        // Control
        lock_return.lock();
        lock_return.unlock();
        speedtesting = false;
    });
#endif
}

void MainWindow::speedtest_current() {
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
#endif
}

void MainWindow::stop_core_daemon() {
#ifndef NKR_NO_GRPC
    NekoRay::rpc::defaultClient->Exit();
#endif
}

void MainWindow::neko_start(int _id) {
    auto ents = get_now_selected();
    auto ent = (_id < 0 && !ents.isEmpty()) ? ents.first() : NekoRay::profileManager->GetProfile(_id);
    if (ent == nullptr) return;

    if (select_mode) {
        emit profile_selected(ent->id);
        select_mode = false;
        refresh_status();
        return;
    }

    auto group = NekoRay::profileManager->GetGroup(ent->gid);
    if (group == nullptr || group->archive) return;

    auto result = NekoRay::BuildConfig(ent, false, false);
    if (!result->error.isEmpty()) {
        MessageBoxWarning("BuildConfig return error", result->error);
        return;
    }

    if (NekoRay::dataStore->started_id >= 0) neko_stop();
    show_log_impl(">>>>>>>> " + tr("Starting profile %1").arg(ent->bean->DisplayTypeAndName()));
    auto insecure_hint = ent->bean->DisplayInsecureHint();
    if (!insecure_hint.isEmpty()) show_log_impl(">>>>>>>> " + tr("Profile is insecure: %1").arg(insecure_hint));

#ifndef NKR_NO_GRPC
    libcore::LoadConfigReq req;
    req.set_core_config(QJsonObject2QString(result->coreConfig, true).toStdString());
    req.set_enable_nekoray_connections(NekoRay::dataStore->connection_statistics);
    //
    bool rpcOK;
    QString error = defaultClient->Start(&rpcOK, req);
    if (rpcOK && !error.isEmpty()) {
        MessageBoxWarning("LoadConfig return error", error);
        return;
    }
    //
    NekoRay::traffic::trafficLooper->proxy = result->outboundStat.get();
    NekoRay::traffic::trafficLooper->items = result->outboundStats;
    NekoRay::dataStore->ignoreConnTag = result->ignoreConnTag;
    NekoRay::traffic::trafficLooper->loop_enabled = true;
#endif

    for (const auto &ext: result->exts) {
        NekoRay::sys::running_ext.push_back(ext.second);
        ext.second->Start();
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
    }

#ifndef NKR_NO_GRPC
    NekoRay::traffic::trafficLooper->loop_enabled = false;
    NekoRay::traffic::trafficLooper->loop_mutex.lock();
    if (NekoRay::dataStore->traffic_loop_interval != 0) {
        NekoRay::traffic::trafficLooper->UpdateAll();
        for (const auto &item: NekoRay::traffic::trafficLooper->items) {
            NekoRay::profileManager->GetProfile(item->id)->Save();
            refresh_proxy_list(item->id);
        }
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
    NekoRay::dataStore->need_keep_vpn_off = false;
    running = nullptr;
    refresh_status();
    refresh_proxy_list(id);
}

void MainWindow::CheckUpdate() {
    // on new thread...
#ifndef NKR_NO_GRPC
    bool ok;
    libcore::UpdateReq request;
    request.set_action(libcore::UpdateAction::Check);
    request.set_check_pre_release(NekoRay::dataStore->check_include_pre);
    auto response = NekoRay::rpc::defaultClient->Update(&ok, request);
    if (!ok) return;

    auto err = response.error();
    if (!err.empty()) {
        runOnUiThread([=] {
            MessageBoxWarning(QObject::tr("Update"), err.c_str());
        });
        return;
    }

    if (response.release_download_url() == nullptr) {
        runOnUiThread([=] {
            MessageBoxInfo(QObject::tr("Update"), QObject::tr("No update"));
        });
        return;
    }

    runOnUiThread([=] {
        auto allow_updater = !NekoRay::dataStore->flag_use_appdata;
        auto note_pre_release = response.is_pre_release() ? " (Pre-release)" : "";
        QMessageBox box(QMessageBox::Question, QObject::tr("Update") + note_pre_release,
                        QObject::tr("Update found: %1\nRelease note:\n%2").arg(response.assets_name().c_str(), response.release_note().c_str()));
        //
        QAbstractButton *btn1 = nullptr;
        if (allow_updater) {
            btn1 = box.addButton(QObject::tr("Update"), QMessageBox::AcceptRole);
        }
        QAbstractButton *btn2 = box.addButton(QObject::tr("Open in browser"), QMessageBox::AcceptRole);
        box.addButton(QObject::tr("Close"), QMessageBox::RejectRole);
        box.exec();
        //
        if (btn1 == box.clickedButton() && allow_updater) {
            // Download Update
            runOnNewThread([=] {
                bool ok2;
                libcore::UpdateReq request2;
                request2.set_action(libcore::UpdateAction::Download);
                auto response2 = NekoRay::rpc::defaultClient->Update(&ok2, request2);
                runOnUiThread([=] {
                    if (response2.error().empty()) {
                        auto q = QMessageBox::question(nullptr, QObject::tr("Update"),
                                                       QObject::tr("Update is ready, restart to install?"));
                        if (q == QMessageBox::StandardButton::Yes) {
                            this->exit_reason = 1;
                            on_menu_exit_triggered();
                        }
                    } else {
                        MessageBoxWarning(QObject::tr("Update"), response2.error().c_str());
                    }
                });
            });
        } else if (btn2 == box.clickedButton()) {
            QDesktopServices::openUrl(QUrl(response.release_url().c_str()));
        }
    });
#endif
}
