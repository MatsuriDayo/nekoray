#include "ExternalProcess.hpp"

#include <QTimer>
#include <QDir>
#include <QApplication>

namespace NekoRay::sys {
    ExternalProcess::ExternalProcess() : QProcess() {}

    void ExternalProcess::Start() {
        if (started) return;
        started = true;
        if (managed) running_ext.push_back(this);

        if (show_log) {
            connect(this, &QProcess::readyReadStandardOutput, this, [&]() {
                showLog_ext_vt100(readAllStandardOutput().trimmed());
            });
            connect(this, &QProcess::readyReadStandardError, this, [&]() {
                showLog_ext_vt100(readAllStandardError().trimmed());
            });
        }

        if (managed) {
            connect(this, &QProcess::errorOccurred, this, [&](QProcess::ProcessError error) {
                if (!killed) {
                    crashed = true;
                    showLog_ext(tag, "errorOccurred:" + errorString());
                    dialog_message("ExternalProcess", "Crashed");
                }
            });
            connect(this, &QProcess::stateChanged, this, [&](QProcess::ProcessState state) {
                if (state == QProcess::NotRunning) {
                    if (killed) { // 用户命令退出
                        showLog_ext(tag, "Stopped");
                    } else if (!crashed) { // 异常退出
                        crashed = true;
                        showLog_ext(tag, "[Error] Program exited accidentally: " + errorString());
                        Kill();
                        dialog_message("ExternalProcess", "Crashed");
                    }
                }
            });
            showLog_ext(tag, "[Starting] " + env.join(" ") + " " + program + " " + arguments.join(" "));
        }

        QProcess::setEnvironment(env);
        QProcess::start(program, arguments);
    }

    void ExternalProcess::Kill() {
        if (killed) return;
        killed = true;
        if (managed) running_ext.removeAll(this);

        if (!crashed) {
            QProcess::kill();
            QProcess::waitForFinished(500);
        }
    }

    CoreProcess::CoreProcess(const QString &core_path, const QStringList &args) {
        ExternalProcess::managed = false;
        ExternalProcess::show_log = false;
        ExternalProcess::program = core_path;
        ExternalProcess::arguments = args;

        connect(this, &QProcess::readyReadStandardOutput, this, [&]() {
            showLog(readAllStandardOutput().trimmed());
        });
        connect(this, &QProcess::readyReadStandardError, this, [&]() {
            auto log = readAllStandardError().trimmed();
            if (show_stderr) {
                showLog(log);
                return;
            }
            if (log.contains("token is set")) {
                show_stderr = true;
            }
        });
        connect(this, &QProcess::errorOccurred, this, [&](QProcess::ProcessError error) {
            if (error == QProcess::ProcessError::FailedToStart) {
                failed_to_start = true;
                showLog("start core error occurred: " + errorString() + "\n");
            }
        });
        connect(this, &QProcess::stateChanged, this, [&](QProcess::ProcessState state) {
            NekoRay::dataStore->core_running = state == QProcess::Running;

            if (!dataStore->core_prepare_exit && state == QProcess::NotRunning) {
                if (failed_to_start) return; // no retry

                showLog("[Error] core exited, restarting.\n");

                // Restart
                auto t = new QTimer;
                connect(t, &QTimer::timeout, this, [=] {
                    Kill();
                    ExternalProcess::started = false;
                    Start();
                    t->deleteLater();
                });
                t->setSingleShot(true);
                t->setInterval(1000);
                t->start();
            }
        });
    }

    void CoreProcess::Start() {
        show_stderr = false;
        auto v2ray_asset_dir = dataStore->v2ray_asset_dir;
        if (v2ray_asset_dir.isEmpty() || QDir(v2ray_asset_dir).exists()) {
            v2ray_asset_dir = QApplication::applicationDirPath();
        }
        env = QStringList{
                "V2RAY_LOCATION_ASSET=" + v2ray_asset_dir
        };
        ExternalProcess::Start();
        write((dataStore->core_token + "\n").toUtf8());
    }

}
