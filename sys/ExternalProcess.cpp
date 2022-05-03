#include "ExternalProcess.hpp"

namespace NekoRay::sys {
    ExternalProcess::ExternalProcess(const QString &tag,
                                     const QString &program,
                                     const QStringList &arguments,
                                     const QStringList &env)
            : QProcess() {
        this->tag = tag;
        this->program = program;
        this->arguments = arguments;
        this->env = env;
        this->running_list = &running_ext;
    }

    void ExternalProcess::Start() {
        if (started) return;
        started = true;
        *running_list += this;

        if (show_log) {
            connect(this, &QProcess::readyReadStandardOutput, this,
                    [&]() {
                        showLog_ext_vt100(readAllStandardOutput().trimmed());
                    });
            connect(this, &QProcess::readyReadStandardError, this,
                    [&]() {
                        showLog_ext_vt100(readAllStandardError().trimmed());
                    });
        }

        connect(this, &QProcess::errorOccurred, this,
                [&](QProcess::ProcessError error) {
                    if (!killed) {
                        crashed = true;
                        showLog_ext(tag, "[Error] Crashed:" + QProcess::errorString());
                        dialog_message("ExternalProcess", "Crashed");
                    }
                });
        connect(this, &QProcess::stateChanged, this,
                [&](QProcess::ProcessState state) {
                    if (state == QProcess::NotRunning) {
                        if (killed) {
                            showLog_ext(tag, "Stopped");
                        } else if (!crashed) {
                            crashed = true;
                            Kill();
                            showLog_ext(tag, "[Error] Crashed?");
                            dialog_message("ExternalProcess", "Crashed");
                        }
                    }
                });

        showLog_ext(tag, "[Starting] " + env.join(" ") + " " + program + " " + arguments.join(" "));

        QProcess::setEnvironment(env);
        QProcess::start(program, arguments);
    }

    void ExternalProcess::Kill() {
        if (killed) return;
        killed = true;
        running_list->removeAll(this);
        if (!crashed) {
            QProcess::kill();
            QProcess::waitForFinished(500);
        }
    }

}
