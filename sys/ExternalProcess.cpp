#include "ExternalProcess.hpp"
#include "main/NekoRay.hpp"

#include <QTimer>
#include <QDir>
#include <QApplication>

namespace NekoRay::sys {
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

        if (show_log) {
            connect(this, &QProcess::readyReadStandardOutput, this, [&]() {
                MW_show_log_ext_vt100(readAllStandardOutput().trimmed());
            });
            connect(this, &QProcess::readyReadStandardError, this, [&]() {
                MW_show_log_ext_vt100(readAllStandardError().trimmed());
            });
            connect(this, &QProcess::errorOccurred, this, [&](QProcess::ProcessError error) {
                if (!killed) {
                    crashed = true;
                    MW_show_log_ext(tag, "errorOccurred:" + errorString());
                    if (managed) MW_dialog_message("ExternalProcess", "Crashed");
                }
            });
            connect(this, &QProcess::stateChanged, this, [&](QProcess::ProcessState state) {
                if (state == QProcess::NotRunning) {
                    if (killed) { // 用户命令退出
                        MW_show_log_ext(tag, "Stopped");
                    } else if (!crashed) { // 异常退出
                        crashed = true;
                        MW_show_log_ext(tag, "[Error] Program exited accidentally: " + errorString());
                        Kill();
                        if (managed) MW_dialog_message("ExternalProcess", "Crashed");
                    }
                }
            });
            MW_show_log_ext(tag, "[Starting] " + env.join(" ") + " " + program + " " + arguments.join(" "));
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

    CoreProcess::CoreProcess(const QString &core_path, const QStringList &args) : ExternalProcess() {
        ExternalProcess::managed = false;
        ExternalProcess::show_log = false;
        ExternalProcess::program = core_path;
        ExternalProcess::arguments = args;

        connect(this, &QProcess::readyReadStandardOutput, this, [&]() {
            MW_show_log(readAllStandardOutput().trimmed());
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
            NekoRay::dataStore->core_running = state == QProcess::Running;

            if (!dataStore->core_prepare_exit && state == QProcess::NotRunning) {
                if (failed_to_start) return; // no retry

                restart_id = NekoRay::dataStore->started_id;
                MW_dialog_message("ExternalProcess", "Crashed");
                MW_show_log("[Error] core exited, restarting.\n");

                // Restart
                setTimeout(
                    [=] {
                        Kill();
                        ExternalProcess::started = false;
                        Start();
                    },
                    this, 1000);
            } else if (state == QProcess::Running && restart_id >= 0) {
                // Restart profile
                setTimeout(
                    [=] {
                        MW_dialog_message("ExternalProcess", "CoreRestarted," + Int2String(restart_id));
                        restart_id = -1;
                    },
                    this, 1000);
            }
        });
    }

    void CoreProcess::Start() {
        show_stderr = false;
        // set extra env
        auto v2ray_asset_dir = FindCoreAsset("geoip.dat");
        if (!v2ray_asset_dir.isEmpty()) {
            v2ray_asset_dir = QFileInfo(v2ray_asset_dir).absolutePath();
            env << "V2RAY_LOCATION_ASSET=" + v2ray_asset_dir;
        }
        if (NekoRay::dataStore->core_ray_direct_dns) env << "NKR_CORE_RAY_DIRECT_DNS=1";
        if (NekoRay::dataStore->core_ray_windows_disable_auto_interface) env << "NKR_CORE_RAY_WINDOWS_DISABLE_AUTO_INTERFACE=1";
        //
        ExternalProcess::Start();
        write((dataStore->core_token + "\n").toUtf8());
    }

} // namespace NekoRay::sys
