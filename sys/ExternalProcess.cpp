#include "ExternalProcess.hpp"
#include "main/NekoGui.hpp"

#include <QTimer>
#include <QDir>
#include <QApplication>
#include <QElapsedTimer>

namespace NekoGui_sys {

    ExternalProcess::ExternalProcess() : QProcess() {
        // qDebug() << "[Debug] ExternalProcess()" << this << running_ext;
        this->env = QProcessEnvironment::systemEnvironment().toStringList();
    }

    ExternalProcess::~ExternalProcess() {
        // qDebug() << "[Debug] ~ExternalProcess()" << this << running_ext;
    }

    void ExternalProcess::Start() {
        if (started) return;
        started = true;

        if (managed) {
            connect(this, &QProcess::readyReadStandardOutput, this, [&]() {
                auto log = readAllStandardOutput();
                if (logCounter.fetchAndAddRelaxed(log.count("\n")) > NekoGui::dataStore->max_log_line) return;
                MW_show_log_ext_vt100(log);
            });
            connect(this, &QProcess::readyReadStandardError, this, [&]() {
                MW_show_log_ext_vt100(readAllStandardError().trimmed());
            });
            connect(this, &QProcess::errorOccurred, this, [&](QProcess::ProcessError error) {
                if (!killed) {
                    crashed = true;
                    MW_show_log_ext(tag, "errorOccurred:" + errorString());
                    MW_dialog_message("ExternalProcess", "Crashed");
                }
            });
            connect(this, &QProcess::stateChanged, this, [&](QProcess::ProcessState state) {
                if (state == QProcess::NotRunning) {
                    if (killed) { // 用户命令退出
                        MW_show_log_ext(tag, "External core stopped");
                    } else if (!crashed) { // 异常退出
                        crashed = true;
                        MW_show_log_ext(tag, "[Error] Program exited accidentally: " + errorString());
                        Kill();
                        MW_dialog_message("ExternalProcess", "Crashed");
                    }
                }
            });
            MW_show_log_ext(tag, "External core starting: " + env.join(" ") + " " + program + " " + arguments.join(" "));
        }

        QProcess::setEnvironment(env);
        QProcess::start(program, arguments);
    }

    void ExternalProcess::Kill() {
        if (killed) return;
        killed = true;

        if (!crashed) {
            QProcess::kill();
            QProcess::waitForFinished(500);
        }
    }

    //

    QElapsedTimer coreRestartTimer;

    CoreProcess::CoreProcess(const QString &core_path, const QStringList &args) : ExternalProcess() {
        ExternalProcess::managed = false;
        ExternalProcess::program = core_path;
        ExternalProcess::arguments = args;

        connect(this, &QProcess::readyReadStandardOutput, this, [&]() {
            auto log = readAllStandardOutput();
            if (!NekoGui::dataStore->core_running) {
                if (log.contains("grpc server listening")) {
                    // The core really started
                    NekoGui::dataStore->core_running = true;
                    if (start_profile_when_core_is_up >= 0) {
                        MW_dialog_message("ExternalProcess", "CoreStarted," + Int2String(start_profile_when_core_is_up));
                        start_profile_when_core_is_up = -1;
                    }
                } else if (log.contains("failed to serve")) {
                    // The core failed to start
                    QProcess::kill();
                }
            }
            if (logCounter.fetchAndAddRelaxed(log.count("\n")) > NekoGui::dataStore->max_log_line) return;
            MW_show_log(log);
        });
        connect(this, &QProcess::readyReadStandardError, this, [&]() {
            auto log = readAllStandardError().trimmed();
            if (show_stderr) {
                MW_show_log(log);
                return;
            }
            if (log.contains("token is set")) {
                show_stderr = true;
            }
        });
        connect(this, &QProcess::errorOccurred, this, [&](QProcess::ProcessError error) {
            if (error == QProcess::ProcessError::FailedToStart) {
                failed_to_start = true;
                MW_show_log("start core error occurred: " + errorString() + "\n");
            }
        });
        connect(this, &QProcess::stateChanged, this, [&](QProcess::ProcessState state) {
            if (state == QProcess::NotRunning) {
                NekoGui::dataStore->core_running = false;
            }

            if (!NekoGui::dataStore->prepare_exit && state == QProcess::NotRunning) {
                if (failed_to_start) return; // no retry
                if (restarting) return;

                MW_dialog_message("ExternalProcess", "CoreCrashed");

                // Retry rate limit
                if (coreRestartTimer.isValid()) {
                    if (coreRestartTimer.restart() < 10 * 1000) {
                        coreRestartTimer = QElapsedTimer();
                        MW_show_log("[Error] " + QObject::tr("Core exits too frequently, stop automatic restart this profile."));
                        return;
                    }
                } else {
                    coreRestartTimer.start();
                }

                // Restart
                start_profile_when_core_is_up = NekoGui::dataStore->started_id;
                MW_show_log("[Error] " + QObject::tr("Core exited, restarting."));
                setTimeout([=] { Restart(); }, this, 1000);
            }
        });
    }

    void CoreProcess::Start() {
        show_stderr = false;
        // set extra env
        auto v2ray_asset_dir = NekoGui::FindCoreAsset("geoip.dat");
        if (!v2ray_asset_dir.isEmpty()) {
            v2ray_asset_dir = QFileInfo(v2ray_asset_dir).absolutePath();
            env << "XRAY_LOCATION_ASSET=" + v2ray_asset_dir;
        }
        if (NekoGui::dataStore->core_ray_direct_dns) env << "NKR_CORE_RAY_DIRECT_DNS=1";
        if (NekoGui::dataStore->core_ray_windows_disable_auto_interface) env << "NKR_CORE_RAY_WINDOWS_DISABLE_AUTO_INTERFACE=1";
        //
        ExternalProcess::Start();
        write((NekoGui::dataStore->core_token + "\n").toUtf8());
    }

    void CoreProcess::Restart() {
        restarting = true;
        QProcess::kill();
        QProcess::waitForFinished(500);
        ExternalProcess::started = false;
        Start();
        restarting = false;
    }

} // namespace NekoGui_sys
